#pragma once
// Auto-generated HTML dashboard page
// Included by esp32_wifi_radar.ino

const char HTML_PAGE[] PROGMEM = R"ESP32RADAR(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Radar</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Exo+2:wght@300;600;800&display=swap" rel="stylesheet">
<style>
*{box-sizing:border-box;margin:0;padding:0}
:root{
  --bg:#080c10;--panel:#0d1520;--border:#1a2d45;
  --green:#00ff88;--cyan:#00d4ff;--red:#ff3b5c;
  --yellow:#ffd166;--dim:#3a5270;--text:#c8dff0;
  --font-mono:'Share Tech Mono',monospace;
  --font-ui:'Exo 2',sans-serif;
}
body{background:var(--bg);color:var(--text);font-family:var(--font-ui);height:100vh;overflow:hidden;display:flex;flex-direction:column}
/* scanline overlay */
body::after{content:'';position:fixed;inset:0;background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,0.07) 2px,rgba(0,0,0,0.07) 4px);pointer-events:none;z-index:999}

header{
  display:flex;align-items:center;justify-content:space-between;
  padding:10px 20px;border-bottom:1px solid var(--border);
  background:rgba(0,212,255,0.04);flex-shrink:0;
}
.logo{font-family:var(--font-mono);font-size:1.1rem;color:var(--cyan);letter-spacing:3px;text-transform:uppercase}
.logo span{color:var(--green)}
.status-row{display:flex;gap:16px;align-items:center;font-family:var(--font-mono);font-size:0.75rem}
.pill{padding:3px 10px;border-radius:20px;border:1px solid;font-size:0.7rem}
.pill-green{color:var(--green);border-color:var(--green);background:rgba(0,255,136,0.06)}
.pill-cyan{color:var(--cyan);border-color:var(--cyan);background:rgba(0,212,255,0.06)}
.pill-red{color:var(--red);border-color:var(--red);background:rgba(255,59,92,0.1);animation:blink 1s infinite}
@keyframes blink{0%,100%{opacity:1}50%{opacity:0.4}}

main{display:flex;flex:1;overflow:hidden;gap:0}

/* LEFT: Radar */
.radar-wrap{flex:0 0 55%;position:relative;border-right:1px solid var(--border)}
#radarCanvas{width:100%;height:100%;display:block}

/* RIGHT: Panels */
.side{flex:1;display:flex;flex-direction:column;overflow:hidden}
.tab-bar{display:flex;border-bottom:1px solid var(--border);flex-shrink:0}
.tab{flex:1;padding:10px;text-align:center;font-family:var(--font-mono);font-size:0.75rem;letter-spacing:1px;cursor:pointer;border-bottom:2px solid transparent;transition:.2s;color:var(--dim)}
.tab.active{color:var(--cyan);border-color:var(--cyan)}
.tab-panel{display:none;flex:1;overflow-y:auto;padding:12px}
.tab-panel.active{display:flex;flex-direction:column;gap:8px}

/* Device card */
.dcard{background:var(--panel);border:1px solid var(--border);border-radius:6px;padding:12px;transition:.2s;position:relative;overflow:hidden}
.dcard:hover{border-color:var(--dim)}
.dcard.deauth{border-color:rgba(255,59,92,0.5)}
.dcard::before{content:'';position:absolute;left:0;top:0;bottom:0;width:3px}
.dcard.deauth::before{background:var(--red)}
.dcard-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:6px}
.dcard-mac{font-family:var(--font-mono);font-size:0.78rem;color:var(--cyan)}
.dcard-icon{font-size:1.1rem}
.dcard-vendor{font-size:0.7rem;color:var(--dim);margin-bottom:4px}
.dcard-ssid{font-size:0.7rem;color:var(--yellow);margin-bottom:6px;font-family:var(--font-mono)}
.dcard-meta{display:flex;gap:12px;font-size:0.7rem;color:var(--dim);margin-bottom:6px}
.meta-val{color:var(--text)}
.bar{height:4px;background:rgba(255,255,255,0.06);border-radius:2px;overflow:hidden;margin-top:6px}
.bar-fill{height:100%;border-radius:2px;background:linear-gradient(90deg,var(--cyan),var(--green));transition:width .5s}
/* mini spark */
.spark{height:30px;width:100%;margin-top:6px}

/* Alerts */
.alert-item{background:rgba(255,59,92,0.08);border:1px solid rgba(255,59,92,0.3);border-radius:4px;padding:8px 12px;font-family:var(--font-mono);font-size:0.72rem;color:var(--red);animation:fadeIn .3s}
@keyframes fadeIn{from{opacity:0;transform:translateY(-4px)}to{opacity:1;transform:none}}

/* Filter bar */
.filter-row{display:flex;align-items:center;gap:10px;padding:10px 12px;border-bottom:1px solid var(--border);flex-shrink:0;font-family:var(--font-mono);font-size:0.75rem}
.filter-row label{color:var(--dim)}
input[type=range]{-webkit-appearance:none;background:var(--border);height:4px;border-radius:2px;outline:none;flex:1}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:12px;height:12px;background:var(--cyan);border-radius:50%}
#rssiVal{color:var(--cyan);min-width:50px}

footer{padding:6px 20px;border-top:1px solid var(--border);display:flex;justify-content:space-between;font-family:var(--font-mono);font-size:0.65rem;color:var(--dim);flex-shrink:0}
</style>
</head>
<body>
<header>
  <div class="logo">ESP<span>32</span> // RADAR</div>
  <div class="status-row">
    <span class="pill pill-green" id="statusPill">● ONLINE</span>
    <span class="pill pill-cyan" id="chPill">CH --</span>
    <span id="deauthPill" class="pill pill-red" style="display:none">⚠ DEAUTH</span>
    <span style="color:var(--dim)" id="devCount">0 devices</span>
  </div>
</header>

<main>
  <div class="radar-wrap">
    <canvas id="radarCanvas"></canvas>
  </div>

  <div class="side">
    <div class="filter-row">
      <label>RSSI FILTER</label>
      <input type="range" id="rssiSlider" min="-95" max="-30" value="-95" step="1">
      <span id="rssiVal">≥ -95 dBm</span>
    </div>
    <div class="tab-bar">
      <div class="tab active" onclick="switchTab('devices',this)">DEVICES</div>
      <div class="tab" onclick="switchTab('alerts',this)">ALERTS</div>
    </div>
    <div class="tab-panel active" id="tab-devices"></div>
    <div class="tab-panel" id="tab-alerts"></div>
  </div>
</main>

<footer>
  <span>⚠ ESTIMATED ANGLE - RSSI-ONLY HARDWARE</span>
  <span id="lastUpdate">--</span>
</footer>

<script>
// ---------- Tab switching ----------
function switchTab(id, el) {
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.tab-panel').forEach(p=>p.classList.remove('active'));
  el.classList.add('active');
  document.getElementById('tab-'+id).classList.add('active');
}

// ---------- RSSI filter ----------
const rssiSlider = document.getElementById('rssiSlider');
const rssiValEl  = document.getElementById('rssiVal');
rssiSlider.addEventListener('input',()=>{
  rssiValEl.textContent = '≥ '+rssiSlider.value+' dBm';
  if(ws && ws.readyState===1) ws.send(JSON.stringify({rssiFilter:parseInt(rssiSlider.value)}));
});

// ---------- Radar canvas ----------
const canvas = document.getElementById('radarCanvas');
const ctx    = canvas.getContext('2d');
let   W, H, CX, CY, R;
let   dotData = [];
let   sweepAngle = 0;
let   radarTrails = [];

function resize() {
  W = canvas.width  = canvas.offsetWidth;
  H = canvas.height = canvas.offsetHeight;
  CX = W/2; CY = H/2;
  R = Math.min(CX,CY)*0.88;
}
window.addEventListener('resize', resize);
resize();

function drawRadar() {
  ctx.clearRect(0,0,W,H);

  // grid rings
  for(let i=1;i<=5;i++){
    ctx.beginPath();
    ctx.arc(CX,CY,R*i/5,0,Math.PI*2);
    ctx.strokeStyle = i===5?'rgba(0,212,255,0.2)':'rgba(0,212,255,0.08)';
    ctx.lineWidth=1;
    ctx.stroke();
    // range labels
    if(i<5){
      ctx.fillStyle='rgba(0,212,255,0.35)';
      ctx.font='10px Share Tech Mono';
      ctx.fillText((15*i/5).toFixed(1)+'m', CX+4, CY-R*i/5-3);
    }
  }
  // crosshairs
  ctx.strokeStyle='rgba(0,212,255,0.1)';
  ctx.lineWidth=1;
  [[CX,0,CX,H],[0,CY,W,CY]].forEach(([x1,y1,x2,y2])=>{
    ctx.beginPath();ctx.moveTo(x1,y1);ctx.lineTo(x2,y2);ctx.stroke();
  });
  // diagonal grids
  [45,135].forEach(a=>{
    const rad=a*Math.PI/180;
    ctx.beginPath();
    ctx.moveTo(CX-Math.cos(rad)*R,CY-Math.sin(rad)*R);
    ctx.lineTo(CX+Math.cos(rad)*R,CY+Math.sin(rad)*R);
    ctx.stroke();
  });

  // sweep trail (arc fade)
  const TRAIL=120;
  for(let i=0;i<TRAIL;i++){
    const a=(sweepAngle-i)*Math.PI/180;
    const alpha=((TRAIL-i)/TRAIL)*0.18;
    ctx.beginPath();
    ctx.moveTo(CX,CY);
    ctx.arc(CX,CY,R,a-Math.PI/2-0.02,a-Math.PI/2,false);
    ctx.closePath();
    ctx.fillStyle=`rgba(0,255,136,${alpha})`;
    ctx.fill();
  }
  // sweep arm
  {
    const a=sweepAngle*Math.PI/180-Math.PI/2;
    const grad=ctx.createLinearGradient(CX,CY,CX+R*Math.cos(a),CY+R*Math.sin(a));
    grad.addColorStop(0,'rgba(0,255,136,0)');
    grad.addColorStop(1,'rgba(0,255,136,0.9)');
    ctx.beginPath();ctx.moveTo(CX,CY);
    ctx.lineTo(CX+R*Math.cos(a),CY+R*Math.sin(a));
    ctx.strokeStyle=grad;ctx.lineWidth=2;ctx.stroke();
  }

  // device dots
  dotData.forEach(d=>{
    const ang = d.angle*Math.PI/180 - Math.PI/2;
    const dist= R*(d.distance/15);
    const x   = CX+dist*Math.cos(ang);
    const y   = CY+dist*Math.sin(ang);
    const alpha= 0.4+0.6*(d.strength/100);

    // ping rings
    const t=(Date.now()/1000)%2;
    ctx.beginPath();
    ctx.arc(x,y,8+t*12,0,Math.PI*2);
    ctx.strokeStyle=`rgba(0,212,255,${0.4*(1-t/2)})`;
    ctx.lineWidth=1;ctx.stroke();

    // dot
    ctx.beginPath();ctx.arc(x,y,5,0,Math.PI*2);
    if(d.deauth) ctx.fillStyle=`rgba(255,59,92,${alpha})`;
    else         ctx.fillStyle=`rgba(0,255,136,${alpha})`;
    ctx.fill();

    // label
    ctx.fillStyle='rgba(200,223,240,0.8)';
    ctx.font='9px Share Tech Mono';
    ctx.fillText(d.mac.slice(-5), x+8, y-6);
  });
}

function animate() {
  sweepAngle=(sweepAngle+1.2)%360;
  drawRadar();
  requestAnimationFrame(animate);
}
animate();

// ---------- Spark chart ----------
function drawSpark(canvas, history) {
  const c=canvas.getContext('2d');
  const w=canvas.width=canvas.offsetWidth||120;
  const h=canvas.height=30;
  c.clearRect(0,0,w,h);
  if(!history||history.length<2)return;
  const mn=-95,mx=-30,range=mx-mn;
  c.beginPath();
  history.forEach((v,i)=>{
    const x=i/(history.length-1)*w;
    const y=h-(v-mn)/range*h;
    i===0?c.moveTo(x,y):c.lineTo(x,y);
  });
  c.strokeStyle='rgba(0,212,255,0.7)';c.lineWidth=1.5;c.stroke();
  // fill
  c.lineTo(w,h);c.lineTo(0,h);c.closePath();
  c.fillStyle='rgba(0,212,255,0.08)';c.fill();
}

// ---------- Device list ----------
function updateDevices(devices){
  const el=document.getElementById('tab-devices');
  const filter=parseInt(rssiSlider.value);
  const filtered=devices.filter(d=>d.rssi>=filter);
  document.getElementById('devCount').textContent=filtered.length+' devices';

  // preserve scroll
  const scroll=el.scrollTop;
  el.innerHTML='';
  if(!filtered.length){
    el.innerHTML='<div style="color:var(--dim);font-family:var(--font-mono);font-size:.8rem;padding:20px;text-align:center">No devices detected</div>';
    return;
  }
  filtered.sort((a,b)=>b.rssi-a.rssi);
  filtered.forEach(d=>{
    const div=document.createElement('div');
    div.className='dcard'+(d.deauth?' deauth':'');
    const ssidLine=d.ssid?`<div class="dcard-ssid">📶 Probing: "${d.ssid}"</div>`:'';
    div.innerHTML=`
      <div class="dcard-header">
        <span class="dcard-mac">${d.mac}</span>
        <span class="dcard-icon">${d.icon||'?'}</span>
      </div>
      <div class="dcard-vendor">${d.vendor||'Unknown'}${d.deauth?' <span style="color:var(--red)">⚠ DEAUTH</span>':''}</div>
      ${ssidLine}
      <div class="dcard-meta">
        <span>RSSI <span class="meta-val">${d.rssi} dBm</span></span>
        <span>Dist <span class="meta-val">${d.distance.toFixed(1)}m</span></span>
        <span>Pkts <span class="meta-val">${d.packets}</span></span>
        <span>CH <span class="meta-val">${d.channel}</span></span>
      </div>
      <div class="bar"><div class="bar-fill" style="width:${d.strength}%"></div></div>
      <canvas class="spark" id="spark-${d.mac.replace(/:/g,'')}"></canvas>
    `;
    el.appendChild(div);
    if(d.history&&d.history.length>1){
      const sc=document.getElementById('spark-'+d.mac.replace(/:/g,''));
      if(sc) setTimeout(()=>drawSpark(sc,d.history),0);
    }
  });
  el.scrollTop=scroll;
}

// ---------- Alerts ----------
function updateAlerts(alerts){
  if(!alerts||!alerts.length)return;
  const el=document.getElementById('tab-alerts');
  alerts.forEach(msg=>{
    const d=document.createElement('div');
    d.className='alert-item';
    d.textContent=msg;
    el.prepend(d);
  });
  // keep only 50
  while(el.children.length>50) el.removeChild(el.lastChild);
}

// ---------- WebSocket ----------
let ws;
let deauthActive=false;

function connect(){
  ws=new WebSocket('ws://'+window.location.hostname+'/ws');
  ws.onopen=()=>{
    document.getElementById('statusPill').textContent='● ONLINE';
    document.getElementById('statusPill').style.cssText='';
  };
  ws.onclose=()=>{
    document.getElementById('statusPill').textContent='● OFFLINE';
    document.getElementById('statusPill').style.color='var(--red)';
    setTimeout(connect,2000);
  };
  ws.onmessage=e=>{
    const data=JSON.parse(e.data);
    dotData=data.devices||[];
    updateDevices(dotData);
    updateAlerts(data.alerts);
    if(data.channel) document.getElementById('chPill').textContent='CH '+data.channel;
    document.getElementById('lastUpdate').textContent='Last update: '+new Date().toLocaleTimeString();
    // deauth pill
    const hasDeauth=dotData.some(d=>d.deauth);
    document.getElementById('deauthPill').style.display=hasDeauth?'':'none';
  };
}
connect();
</script>
</body>
</html>
)ESP32RADAR";
