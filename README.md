# GhostNet_Sniffer

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue)](https://www.espressif.com/en/products/socs/esp32)
[![Arduino Compatible](https://img.shields.io/badge/Arduino-IDE-orange)](https://www.arduino.cc/)

**Passive Wi‑Fi sensing radar using an ESP32 in promiscuous mode.**  
Detects nearby 2.4 GHz Wi‑Fi devices, estimates their distance, and visualizes them in real time on a web‑based radar dashboard.

**Passive Wi‑Fi radar that haunts the airwaves – detect hidden devices and motion using just an ESP32.**


---

## 📡 Overview

Traditional radar emits signals and listens for echoes. This project flips the concept: it passively listens to existing Wi‑Fi traffic from any router or device. By analyzing the **Received Signal Strength Indicator (RSSI)** of captured packets, it can:

- Detect the presence of Wi‑Fi devices (phones, laptops, IoT gadgets)
- Estimate their distance (crude, but useful for motion detection)
- Display them on a live radar‑style interface

The ESP32 acts as a **standalone access point** hosting the radar dashboard, so no external network or internet connection is required.

---

## ✨ Features

- ✅ **Passive sniffing** – works with any 2.4 GHz router (no need to connect to it)
- ✅ **Real‑time web interface** – live radar sweep + device list, updated via WebSocket
- ✅ **Device fingerprinting** – MAC address, RSSI, estimated distance
- ✅ **Configurable channel** – set to match your router or scan manually
- ✅ **Zero external dependencies** – all processing on ESP32, HTML/JS embedded
- ✅ **Lightweight** – uses ~77% of flash, runs on any ESP32 dev board

---

## 🧰 Hardware Requirements

| Component | Notes |
|-----------|-------|
| **ESP32 Development Board** | Any variant with 2.4 GHz Wi‑Fi (ESP32‑D0WD, ESP32‑S3, etc.) |
| **USB Cable** | Data‑sync capable (not charge‑only) |
| **Power supply** | USB port (computer or 5V adapter) |

No additional sensors, antennas, or transceivers required – the built‑in Wi‑Fi radio does all the work.

---

## 📦 Software & Libraries

Install the following libraries via the **Arduino Library Manager** (Sketch → Include Library → Manage Libraries…):

| Library | Author | Minimum Version |
|---------|--------|----------------|
| `AsyncTCP` | me‑no‑dev | 1.1.1 |
| `ESPAsyncWebServer` | me‑no‑dev | 1.2.3 |
| `ArduinoJson` | Benoit Blanchon | 6.19.4 |

**Platform**: Arduino IDE 2.x or 1.8.x with ESP32 board support (Espressif core 2.0.14+).

---

## 🚀 Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/yourusername/esp32-wifi-radar.git
cd esp32-wifi-radar
