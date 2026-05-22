/**
 * based on ESP32 dev module
 * 
 * Copyright (c) 2026 [ayuuXploits]
 * 
 * All rights reserved.
 * 
 * This software and its associated documentation are proprietary to the author.
 * No part of this software may be reproduced, distributed, or transmitted in any form
 * or by any means, including photocopying, recording, or other electronic or mechanical
 * methods, without the prior written permission of the author, except in the case of
 * brief quotations embodied in critical reviews and certain other noncommercial uses
 * permitted by copyright law.
 * 
 * For permission requests, contact the author.
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <freertos/semphr.h>
#include <math.h>
#include <string.h>

// ============================================================
//  USER CONFIG
// ============================================================
const char*   AP_SSID       = "GhostNet_Radar";
const char*   AP_PASS       = "radar12345";
const uint8_t HOP_MIN_CH    = 1;
const uint8_t HOP_MAX_CH    = 13;
const uint32_t HOP_DWELL_MS = 150;    // ms per channel
const int8_t  DEFAULT_RSSI_FILTER = -95;  // ignore weaker than this
const float   TX_POWER_REF  = -59.0f; // RSSI at 1m (calibrate for your env)
const float   PATH_LOSS_N   = 2.7f;   // 2=free-space, 2.7=typical indoor
const float   MAX_DISTANCE  = 15.0f;  // metres, clamping limit
const uint32_t DEVICE_TIMEOUT_MS = 30000; // remove device after 30s silence

// ============================================================
//  IEEE 802.11 structs
// ============================================================
typedef struct {
  uint16_t frame_ctrl;
  uint8_t  duration_id[2];
  uint8_t  addr1[6];
  uint8_t  addr2[6];
  uint8_t  addr3[6];
  uint16_t sequence_ctrl;
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0];
} wifi_ieee80211_packet_t;

// Frame control type/subtype helpers
#define FC_TYPE(fc)    (((fc) >> 2) & 0x3)
#define FC_SUBTYPE(fc) (((fc) >> 4) & 0xF)
#define FTYPE_MGMT     0
#define FTYPE_DATA     2
#define STYPE_DEAUTH   12
#define STYPE_PROBE_REQ 4

// ============================================================
//  OUI table  (add more as needed)
// ============================================================
struct OuiEntry { uint8_t oui[3]; const char* vendor; const char* icon; };
static const OuiEntry OUI_TABLE[] = {
  {{0x00,0x17,0xF2}, "Apple",    "[Apple]"},
  {{0xAC,0xDE,0x48}, "Apple",    "[Apple]"},
  {{0xF8,0x1E,0xDF}, "Apple",    "[Apple]"},
  {{0x3C,0x22,0xFB}, "Apple",    "[Apple]"},
  {{0xB8,0x27,0xEB}, "Raspberry","[RPi]"},
  {{0xDC,0xA6,0x32}, "Raspberry","[RPi]"},
  {{0xE4,0x5F,0x01}, "Raspberry","[RPi]"},
  {{0x00,0x0C,0xE7}, "Samsung",  "[Mobile]"},
  {{0x8C,0xF5,0xA3}, "Samsung",  "[Mobile]"},
  {{0xCC,0xB2,0x55}, "Samsung",  "[Mobile]"},
  {{0x00,0x1A,0x11}, "Google",   "[Google]"},
  {{0xF4,0xF5,0xDB}, "Google",   "[Google]"},
  {{0x54,0x60,0x09}, "Xiaomi",   "[Mobile]"},
  {{0xD4,0x97,0x0B}, "Xiaomi",   "[Mobile]"},
  {{0x78,0x02,0xF8}, "Realtek",  "[PC]"},
  {{0x00,0x50,0xF2}, "Microsoft","[PC]"},
  {{0x00,0x0D,0x3A}, "Microsoft","[PC]"},
  {{0x00,0x13,0x10}, "Cisco",    "[Net]"},
  {{0x00,0x1E,0xBD}, "Cisco",    "[Net]"},
  {{0x00,0x25,0x9C}, "Cisco",    "[Net]"},
  {{0x18,0xB4,0x30}, "Nest",     "[IoT]"},
  {{0x64,0x16,0x66}, "Amazon",   "[Amazon]"},
  {{0x68,0x37,0xE9}, "Amazon",   "[Amazon]"},
  {{0xFC,0x65,0xDE}, "TP-Link",  "[Net]"},
  {{0xB0,0x4E,0x26}, "TP-Link",  "[Net]"},
};
static const int OUI_COUNT = sizeof(OUI_TABLE) / sizeof(OUI_TABLE[0]);

const char* lookupVendor(const uint8_t* mac) {
  for (int i = 0; i < OUI_COUNT; i++)
    if (memcmp(mac, OUI_TABLE[i].oui, 3) == 0) return OUI_TABLE[i].vendor;
  return "Unknown";
}
const char* lookupIcon(const uint8_t* mac) {
  for (int i = 0; i < OUI_COUNT; i++)
    if (memcmp(mac, OUI_TABLE[i].oui, 3) == 0) return OUI_TABLE[i].icon;
  return "[?]";
}

// ============================================================
//  Device table
// ============================================================
#define MAX_DEVICES  30
#define RSSI_HISTORY 30

struct WiFiDevice {
  uint8_t  mac[6];
  int8_t   rssi;
  int8_t   rssiHistory[RSSI_HISTORY];
  uint8_t  historyLen;
  uint8_t  historyIdx;
  char     ssid[33];      // from probe requests
  bool     hasSSID;
  bool     deauthSeen;
  unsigned long lastSeen;
  unsigned long firstSeen;
  uint32_t packetCount;
};

static WiFiDevice  devices[MAX_DEVICES];
static int         deviceCount = 0;
static SemaphoreHandle_t deviceMutex;
static volatile int8_t rssiFilter = DEFAULT_RSSI_FILTER;

// ============================================================
//  Channel hopping state
// ============================================================
static volatile uint8_t currentChannel = HOP_MIN_CH;
static unsigned long lastHop = 0;

// ============================================================
//  Alert ring buffer (deauth / new device events)
// ============================================================
#define MAX_ALERTS 10
struct Alert { char msg[80]; unsigned long ts; };
static Alert  alerts[MAX_ALERTS];
static uint8_t alertHead = 0, alertCount = 0;
static SemaphoreHandle_t alertMutex;

void pushAlert(const char* msg) {
  xSemaphoreTake(alertMutex, portMAX_DELAY);
  snprintf(alerts[alertHead].msg, 80, "%s", msg);
  alerts[alertHead].ts = millis();
  alertHead = (alertHead + 1) % MAX_ALERTS;
  if (alertCount < MAX_ALERTS) alertCount++;
  xSemaphoreGive(alertMutex);
}

// ============================================================
//  Internal helpers (called only from sniffer callback)
// ============================================================
static void addOrUpdateDevice_ISR(const uint8_t* mac, int8_t rssi,
                                   const char* ssid, bool isDeauth) {
  unsigned long now = millis();
  // search existing
  for (int i = 0; i < deviceCount; i++) {
    if (memcmp(devices[i].mac, mac, 6) == 0) {
      devices[i].rssi = rssi;
      devices[i].rssiHistory[devices[i].historyIdx] = rssi;
      devices[i].historyIdx = (devices[i].historyIdx + 1) % RSSI_HISTORY;
      if (devices[i].historyLen < RSSI_HISTORY) devices[i].historyLen++;
      devices[i].lastSeen = now;
      devices[i].packetCount++;
      if (isDeauth && !devices[i].deauthSeen) {
        devices[i].deauthSeen = true;
        char buf[80];
        snprintf(buf, 80, "[!] Deauth from %02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        pushAlert(buf);
      }
      if (ssid && ssid[0] && !devices[i].hasSSID) {
        strncpy(devices[i].ssid, ssid, 32);
        devices[i].hasSSID = true;
      }
      return;
    }
  }
  // new device - find slot (LRU eviction if full)
  int slot = deviceCount < MAX_DEVICES ? deviceCount : -1;
  if (slot == -1) {
    unsigned long oldest = ULONG_MAX;
    for (int i = 0; i < MAX_DEVICES; i++) {
      if (devices[i].lastSeen < oldest) { oldest = devices[i].lastSeen; slot = i; }
    }
  } else {
    deviceCount++;
  }
  memcpy(devices[slot].mac, mac, 6);
  devices[slot].rssi = rssi;
  memset(devices[slot].rssiHistory, rssi, RSSI_HISTORY);
  devices[slot].historyLen = 1;
  devices[slot].historyIdx = 1;
  devices[slot].rssiHistory[0] = rssi;
  devices[slot].hasSSID   = false;
  devices[slot].ssid[0]   = '\0';
  devices[slot].deauthSeen = isDeauth;
  devices[slot].lastSeen  = now;
  devices[slot].firstSeen = now;
  devices[slot].packetCount = 1;
  if (ssid && ssid[0]) { strncpy(devices[slot].ssid, ssid, 32); devices[slot].hasSSID = true; }

  char buf[80];
  snprintf(buf, 80, "[+] New device: %02X:%02X:%02X:%02X:%02X:%02X (%s)",
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], lookupVendor(mac));
  pushAlert(buf);
}

// ============================================================
//  Promiscuous callback - plain C function, IRAM_ATTR
// ============================================================
static void IRAM_ATTR snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA && type != WIFI_PKT_MISC) return;

  const wifi_promiscuous_pkt_t* ppkt = (const wifi_promiscuous_pkt_t*)buf;
  if (ppkt->rx_ctrl.sig_len < (int)sizeof(wifi_ieee80211_mac_hdr_t)) return;

  const wifi_ieee80211_packet_t* pkt = (const wifi_ieee80211_packet_t*)ppkt->payload;
  const uint8_t* mac = pkt->hdr.addr2;
  int8_t rssi = ppkt->rx_ctrl.rssi;

  // MAC sanity check (ignore multicast/broadcast)
  if (mac[0] & 0x01) return;
  // Ignore our own AP MAC
  uint8_t apMac[6]; esp_wifi_get_mac(WIFI_IF_AP, apMac);
  if (memcmp(mac, apMac, 6) == 0) return;

  if (rssi < rssiFilter) return;

  uint16_t fc   = pkt->hdr.frame_ctrl;
  uint8_t ftype = FC_TYPE(fc);
  uint8_t fsub  = FC_SUBTYPE(fc);
  bool isDeauth = (ftype == FTYPE_MGMT && fsub == STYPE_DEAUTH);

  // Parse probe request SSID
  char probeSSID[33] = {0};
  if (ftype == FTYPE_MGMT && fsub == STYPE_PROBE_REQ) {
    const uint8_t* ie = ppkt->payload + sizeof(wifi_ieee80211_mac_hdr_t);
    int remaining = ppkt->rx_ctrl.sig_len - sizeof(wifi_ieee80211_mac_hdr_t);
    if (remaining > 2 && ie[0] == 0x00) {
      uint8_t len = ie[1];
      if (len > 0 && len <= 32 && remaining >= (int)(2 + len)) {
        memcpy(probeSSID, ie + 2, len);
        probeSSID[len] = '\0';
      }
    }
  }

  if (xSemaphoreTakeFromISR(deviceMutex, NULL) == pdTRUE) {
    addOrUpdateDevice_ISR(mac, rssi, probeSSID[0] ? probeSSID : nullptr, isDeauth);
    xSemaphoreGiveFromISR(deviceMutex, NULL);
  }
}

// ============================================================
//  Utility: FNV-1a MAC -> angle
// ============================================================
static uint16_t hashMacToAngle(const uint8_t* mac) {
  uint32_t h = 2166136261u;
  for (int i = 0; i < 6; i++) { h ^= mac[i]; h *= 16777619u; }
  return (uint16_t)(h % 360);
}

// ============================================================
//  JSON builder
// ============================================================
static String generateJSON() {
  JsonDocument doc;
  JsonArray devArr = doc["devices"].to<JsonArray>();

  bool usedAngles[360] = {};
  unsigned long now = millis();

  xSemaphoreTake(deviceMutex, portMAX_DELAY);
  for (int i = 0; i < deviceCount; i++) {
    WiFiDevice& d = devices[i];
    if (now - d.lastSeen > DEVICE_TIMEOUT_MS) continue;

    JsonObject obj = devArr.add<JsonObject>();

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             d.mac[0],d.mac[1],d.mac[2],d.mac[3],d.mac[4],d.mac[5]);
    obj["mac"]     = macStr;
    obj["vendor"]  = lookupVendor(d.mac);
    obj["icon"]    = lookupIcon(d.mac);
    obj["rssi"]    = d.rssi;
    obj["packets"] = d.packetCount;
    obj["deauth"]  = d.deauthSeen;
    obj["channel"] = (int)currentChannel;
    if (d.hasSSID) obj["ssid"] = d.ssid;

    // Distance via log-distance path-loss model
    float dist = powf(10.0f, (TX_POWER_REF - (float)d.rssi) / (10.0f * PATH_LOSS_N));
    if (dist > MAX_DISTANCE) dist = MAX_DISTANCE;
    obj["distance"] = dist;

    // Angle with collision avoidance
    uint16_t angle = hashMacToAngle(d.mac);
    for (int s = 0; s < 24; s++) {
      uint16_t c = (angle + s * 15) % 360;
      if (!usedAngles[c]) { angle = c; break; }
    }
    usedAngles[angle] = true;
    obj["angle"]    = angle;
    obj["strength"] = (int)constrain(map(d.rssi, -95, -35, 0, 100), 0, 100);

    // RSSI history
    JsonArray hist = obj["history"].to<JsonArray>();
    for (int j = 0; j < d.historyLen; j++) {
      uint8_t idx = (d.historyIdx - d.historyLen + j + RSSI_HISTORY) % RSSI_HISTORY;
      hist.add((int)d.rssiHistory[idx]);
    }
  }
  xSemaphoreGive(deviceMutex);

  // Alerts
  xSemaphoreTake(alertMutex, portMAX_DELAY);
  JsonArray alArr = doc["alerts"].to<JsonArray>();
  for (int i = 0; i < alertCount; i++) {
    uint8_t idx = (alertHead - alertCount + i + MAX_ALERTS) % MAX_ALERTS;
    alArr.add(alerts[idx].msg);
  }
  alertCount = 0;  // clear after send
  xSemaphoreGive(alertMutex);

  doc["channel"] = (int)currentChannel;

  String out;
  serializeJson(doc, out);
  return out;
}

// ============================================================
//  HTML Dashboard (full rewrite - dark tactical UI)
// ============================================================
#include "html_page.h"

// ============================================================
//  Server / WebSocket
// ============================================================
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("[WS] Client #%u connected\n", client->id());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("[WS] Client #%u disconnected\n", client->id());
  } else if (type == WS_EVT_DATA) {
    // Handle filter command from UI
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->opcode == WS_TEXT && len > 0) {
      data[len] = 0;
      JsonDocument cmd;
      if (!deserializeJson(cmd, data)) {
        if (cmd.containsKey("rssiFilter")) {
          rssiFilter = (int8_t)(int)cmd["rssiFilter"];
          Serial.printf("[WS] RSSI filter set to %d dBm\n", (int)rssiFilter);
        }
      }
    }
  }
}

// ============================================================
//  Setup
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== ESP32 Wi-Fi Radar BOOT ===");

  deviceMutex = xSemaphoreCreateMutex();
  alertMutex  = xSemaphoreCreateMutex();

  // Start AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS, HOP_MIN_CH);
  Serial.printf("[WiFi] AP started  SSID=%s  IP=%s\n",
                AP_SSID, WiFi.softAPIP().toString().c_str());

  // Promiscuous mode
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(snifferCallback);
  esp_wifi_set_channel(HOP_MIN_CH, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_max_tx_power(84);
  Serial.println("[WiFi] Promiscuous mode ON");

  // WebSocket + HTTP
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", HTML_PAGE);
  });
  server.begin();
  Serial.println("[HTTP] Server started on port 80");
}

// ============================================================
//  Loop - non-blocking timers
// ============================================================
void loop() {
  unsigned long now = millis();

  // Channel hopping
  if (now - lastHop >= HOP_DWELL_MS) {
    currentChannel = (currentChannel % HOP_MAX_CH) + 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    lastHop = now;
  }

  // Broadcast radar data every 800ms
  static unsigned long lastSend = 0;
  if (now - lastSend >= 800) {
    ws.cleanupClients();
    if (ws.count() > 0) {
      String json = generateJSON();
      ws.textAll(json);
    }
    lastSend = now;
  }

  // Prune timed-out devices from table every 5s
  static unsigned long lastPrune = 0;
  if (now - lastPrune >= 5000) {
    xSemaphoreTake(deviceMutex, portMAX_DELAY);
    for (int i = 0; i < deviceCount; ) {
      if (now - devices[i].lastSeen > DEVICE_TIMEOUT_MS) {
        Serial.printf("[Radar] Pruned device %02X:%02X:%02X:%02X:%02X:%02X\n",
          devices[i].mac[0],devices[i].mac[1],devices[i].mac[2],
          devices[i].mac[3],devices[i].mac[4],devices[i].mac[5]);
        devices[i] = devices[--deviceCount];
      } else { i++; }
    }
    xSemaphoreGive(deviceMutex);
    lastPrune = now;
  }
}
