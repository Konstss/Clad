#include <WiFi.h>
#include <WebServer.h>

const char* SSID     = "SmartHaus";
const char* PASSWORD = "12345678";

#define NUM_LIGHTS 20
#define NUM_SCENES 5

struct Light { const char* name; const char* icon; const char* color; int pin; };

const Light LIGHTS[NUM_LIGHTS] = {
  // EG (Index 0–6)
  {"Wohnzimmer",     "\xF0\x9F\x9B\x8B\xEF\xB8\x8F", "#f59e0b", 2 },
  {"K\xC3\xBCche",   "\xF0\x9F\x8D\xB3",             "#fb923c", 4 },
  {"Esszimmer",      "\xF0\x9F\x8D\xBD\xEF\xB8\x8F", "#fbbf24", 5 },
  {"Flur EG",        "\xF0\x9F\x9A\xAA",             "#94a3b8", 12},
  {"Bad EG",         "\xF0\x9F\x9A\xBF",             "#06b6d4", 13},
  {"Garage",         "\xF0\x9F\x9A\x97",             "#22c55e", 14},
  {"Garten",         "\xF0\x9F\x8C\xBF",             "#10b981", 15},
  // 1.OG (Index 7–13)
  {"Schlafzimmer",   "\xF0\x9F\x9B\x8F\xEF\xB8\x8F", "#6366f1", 16},
  {"Kinderzimmer 1", "\xF0\x9F\xA7\xB8",             "#ec4899", 17},
  {"Kinderzimmer 2", "\xF0\x9F\xA7\xB8",             "#a855f7", 18},
  {"B\xC3\xBCro",    "\xF0\x9F\x92\xBB",             "#8b5cf6", 19},
  {"Bad 1.OG",       "\xF0\x9F\x9A\xBF",             "#06b6d4", 21},
  {"Ankleide",       "\xF0\x9F\x91\x97",             "#f43f5e", 22},
  {"Flur 1.OG",      "\xF0\x9F\x9A\xAA",             "#94a3b8", 23},
  // 2.OG (Index 14–19)
  {"G\xC3\xA4stezimmer", "\xF0\x9F\x9B\x8F\xEF\xB8\x8F", "#3b82f6", 25},
  {"Fitness",            "\xF0\x9F\x8F\x8B\xEF\xB8\x8F", "#ef4444", 26},
  {"Bad 2.OG",           "\xF0\x9F\x9A\xBF",             "#06b6d4", 27},
  {"Dachboden",          "\xF0\x9F\x93\xA6",             "#eab308", 32},
  {"Terrasse",           "\xE2\x98\x80\xEF\xB8\x8F",     "#facc15", 33},
  {"Flur 2.OG",          "\xF0\x9F\x9A\xAA",             "#94a3b8", 0 },
};

const bool SCENES[NUM_SCENES][NUM_LIGHTS] = {
  {1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1},  // Alle AN
  {1,1,1,1,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0},  // Abend
  {0,0,0,1,0,0,0, 0,0,0,0,0,0,1, 0,0,0,0,0,1},  // Nacht (Flure)
  {1,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0},  // Film (nur Wohnzimmer)
  {0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0},  // Alle AUS
};

bool lightState[NUM_LIGHTS] = {};
WebServer server(80);

// ── Statisches HTML/CSS/JS-Template ──────────────────────────
const char PAGE[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="de"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Haus</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,'SF Pro Display','Segoe UI',Arial,sans-serif;background:#08080f;color:#e5e7eb;min-height:100vh;padding-bottom:50px;-webkit-font-smoothing:antialiased}
body::before{content:'';position:fixed;inset:0;background:radial-gradient(ellipse 600px 400px at 20% 10%,rgba(245,158,11,.08),transparent),radial-gradient(ellipse 500px 400px at 80% 30%,rgba(99,102,241,.08),transparent),radial-gradient(ellipse 700px 500px at 50% 90%,rgba(168,85,247,.06),transparent);z-index:0;pointer-events:none}
header{position:sticky;top:0;z-index:100;backdrop-filter:blur(24px);-webkit-backdrop-filter:blur(24px);background:rgba(8,8,15,.7);border-bottom:1px solid rgba(255,255,255,.05);padding:14px 18px;display:flex;align-items:center;gap:10px}
.logo{font-size:1.2rem;font-weight:700;color:#fff;flex:1;letter-spacing:-.3px}
.logo .icon{font-size:1.3rem;margin-right:4px}
.logo .accent{background:linear-gradient(135deg,#f59e0b,#ec4899);-webkit-background-clip:text;background-clip:text;color:transparent}
.hstat{display:flex;align-items:center;gap:7px;background:rgba(255,255,255,.06);border:1px solid rgba(255,255,255,.08);border-radius:14px;padding:5px 12px;font-size:.78rem;color:#cbd5e1;white-space:nowrap}
.hstat b{color:#fff;font-weight:600}
.dot{width:7px;height:7px;border-radius:50%;background:#22c55e;box-shadow:0 0 8px #22c55e;flex-shrink:0;animation:pulse 2s ease-in-out infinite}
.dot.off{background:#ef4444;box-shadow:0 0 8px #ef4444;animation:none}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
main{position:relative;z-index:1;max-width:760px;margin:0 auto;padding:18px 14px;display:flex;flex-direction:column;gap:18px}
.hero{position:relative;border-radius:24px;padding:24px 22px;background:linear-gradient(135deg,rgba(245,158,11,.12),rgba(236,72,153,.08));border:1px solid rgba(255,255,255,.08);overflow:hidden}
.hero::before{content:'';position:absolute;top:-50px;right:-50px;width:200px;height:200px;border-radius:50%;background:radial-gradient(circle,rgba(245,158,11,.15),transparent 70%);pointer-events:none}
.hero-greet{font-size:.88rem;color:#94a3b8;margin-bottom:6px;font-weight:500;display:flex;align-items:center;gap:6px}
.hero-num{font-size:3rem;font-weight:800;line-height:1;letter-spacing:-2px;background:linear-gradient(135deg,#fff,#cbd5e1);-webkit-background-clip:text;background-clip:text;color:transparent}
.hero-num .of{font-size:1.3rem;color:#64748b;font-weight:500;margin-left:4px;-webkit-text-fill-color:#64748b}
.hero-lbl{font-size:.85rem;color:#94a3b8;margin-top:4px;margin-bottom:18px}
.hero-btns{display:flex;gap:8px}
.hero-btn{flex:1;padding:11px 16px;border-radius:14px;border:1px solid rgba(255,255,255,.12);background:rgba(255,255,255,.06);color:#fff;font-size:.85rem;font-weight:600;cursor:pointer;transition:all .2s;font-family:inherit}
.hero-btn:hover{background:rgba(255,255,255,.12);transform:translateY(-1px)}
.hero-btn.primary{background:linear-gradient(135deg,#f59e0b,#ec4899);border-color:transparent;box-shadow:0 4px 16px rgba(245,158,11,.3)}
.section-title{font-size:.78rem;text-transform:uppercase;letter-spacing:1px;color:#64748b;font-weight:700;padding:0 4px}
.scenes{display:flex;gap:8px;overflow-x:auto;padding:4px 4px 8px;margin:0 -4px;scrollbar-width:none}
.scenes::-webkit-scrollbar{display:none}
.chip{display:flex;align-items:center;gap:6px;padding:9px 16px;border-radius:14px;border:1px solid rgba(255,255,255,.08);background:rgba(255,255,255,.04);color:#cbd5e1;cursor:pointer;font-size:.83rem;white-space:nowrap;flex-shrink:0;font-family:inherit;font-weight:500;transition:all .2s}
.chip:hover{background:rgba(255,255,255,.08);transform:translateY(-1px)}
.chip.active{background:#fff;color:#0c0c1d;border-color:#fff;font-weight:600}
.tabs{display:flex;gap:4px;background:rgba(255,255,255,.04);padding:5px;border-radius:14px;border:1px solid rgba(255,255,255,.06)}
.tab{flex:1;padding:9px 8px;border-radius:10px;border:none;background:transparent;color:#94a3b8;font-size:.82rem;font-weight:600;cursor:pointer;transition:all .25s;font-family:inherit;display:flex;flex-direction:column;align-items:center;gap:1px}
.tab .tn{font-size:.65rem;color:#64748b;font-weight:500}
.tab.active{background:#fff;color:#0c0c1d;box-shadow:0 2px 8px rgba(0,0,0,.3)}
.tab.active .tn{color:#64748b}
.tiles{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:11px}
.tile{position:relative;aspect-ratio:1;border-radius:22px;padding:16px;background:rgba(255,255,255,.04);border:1px solid rgba(255,255,255,.07);display:flex;flex-direction:column;justify-content:space-between;cursor:pointer;transition:all .35s cubic-bezier(.34,1.56,.64,1);overflow:hidden;-webkit-tap-highlight-color:transparent}
.tile::before{content:'';position:absolute;inset:0;background:linear-gradient(135deg,var(--tc),transparent 70%);opacity:0;transition:opacity .4s}
.tile.on::before{opacity:.4}
.tile.on{border-color:var(--tc);background:rgba(255,255,255,.06);box-shadow:0 0 30px color-mix(in srgb,var(--tc) 25%,transparent),inset 0 1px 0 rgba(255,255,255,.1)}
.tile:active{transform:scale(.95)}
.tile-top{display:flex;justify-content:space-between;align-items:flex-start;position:relative;z-index:1}
.tile-icon{font-size:1.7rem;line-height:1;transition:filter .35s,transform .35s}
.tile.on .tile-icon{filter:drop-shadow(0 0 12px var(--tc));transform:scale(1.1)}
.tile-toggle{width:36px;height:22px;border-radius:22px;background:rgba(255,255,255,.08);border:1px solid rgba(255,255,255,.1);position:relative;flex-shrink:0;transition:all .3s}
.tile-toggle::after{content:'';position:absolute;width:16px;height:16px;left:2px;top:2px;border-radius:50%;background:#475569;transition:all .3s}
.tile.on .tile-toggle{background:var(--tc);border-color:var(--tc);box-shadow:0 0 12px var(--tc)}
.tile.on .tile-toggle::after{transform:translateX(14px);background:#fff}
.tile-bottom{position:relative;z-index:1}
.tile-name{font-size:.92rem;font-weight:600;color:#fff;line-height:1.2;margin-bottom:2px}
.tile-status{font-size:.7rem;color:#64748b;font-weight:500}
.tile.on .tile-status{color:var(--tc);font-weight:600}
.fade-in{animation:fadeIn .35s ease}
@keyframes fadeIn{from{opacity:0;transform:translateY(8px)}to{opacity:1;transform:none}}
footer{text-align:center;color:#1e293b;font-size:.7rem;margin-top:14px;position:relative;z-index:1}
@media(max-width:430px){.tiles{grid-template-columns:repeat(2,1fr);gap:10px}.hero-num{font-size:2.6rem}.hero{padding:20px 18px}}
</style>
</head><body>
<header>
  <div class="logo"><span class="icon">&#127968;</span> Smart <span class="accent">Haus</span></div>
  <div class="hstat"><span class="dot" id="dot"></span><b id="hon">0</b>/20</div>
</header>
<main>
  <section class="hero">
    <div class="hero-greet" id="greet">&#128075; Hallo</div>
    <div class="hero-num"><span id="heroNum">0</span><span class="of">/20</span></div>
    <div class="hero-lbl" id="heroLbl">Lichter eingeschaltet</div>
    <div class="hero-btns">
      <button class="hero-btn primary" onclick="scene(0)">Alle einschalten</button>
      <button class="hero-btn" onclick="scene(4)">Alle aus</button>
    </div>
  </section>
  <div class="section-title">Szenen</div>
  <div class="scenes">
    <button class="chip" onclick="scene(0)">&#9728; Alle AN</button>
    <button class="chip" onclick="scene(1)">&#127750; Abend</button>
    <button class="chip" onclick="scene(2)">&#127769; Nacht</button>
    <button class="chip" onclick="scene(3)">&#127916; Film</button>
    <button class="chip" onclick="scene(4)">&#9899; Alle AUS</button>
  </div>
  <div class="tabs">
    <button class="tab active" onclick="setFloor(0)">Erdgeschoss<span class="tn" id="tn0">0/7 an</span></button>
    <button class="tab" onclick="setFloor(1)">1. OG<span class="tn" id="tn1">0/7 an</span></button>
    <button class="tab" onclick="setFloor(2)">2. OG<span class="tn" id="tn2">0/6 an</span></button>
  </div>
  <div class="tiles" id="tiles"></div>
</main>
<footer>ESP32 &bull; %IP%</footer>
<script>
var LIGHTS = %LIGHTS%;
var S = %STATES%;
var FS = [0,7,14], FC = [7,7,6];
var currentFloor = 0;

function setGreet(){
  var h = new Date().getHours(), g = 'Hallo', e = '\u{1F44B}';
  if(h<6){g='Gute Nacht';e='\u{1F319}'}
  else if(h<11){g='Guten Morgen';e='☀️'}
  else if(h<18){g='Guten Tag';e='\u{1F324}️'}
  else if(h<22){g='Guten Abend';e='\u{1F306}'}
  else{g='Gute Nacht';e='\u{1F319}'}
  document.getElementById('greet').innerHTML = e+' '+g;
}
function renderTiles(){
  var tiles = document.getElementById('tiles');
  tiles.innerHTML = '';
  var s = FS[currentFloor], c = FC[currentFloor];
  for(var i=s; i<s+c; i++){
    var L = LIGHTS[i], on = S[i];
    var el = document.createElement('div');
    el.className = 'tile fade-in'+(on?' on':'');
    el.id = 't'+i;
    el.style = '--tc:'+L.c;
    el.onclick = (function(id){return function(){toggle(id)}})(i);
    el.innerHTML =
      '<div class="tile-top"><span class="tile-icon">'+L.i+'</span><span class="tile-toggle"></span></div>'+
      '<div class="tile-bottom"><div class="tile-name">'+L.n+'</div><div class="tile-status">'+(on?'An':'Aus')+'</div></div>';
    tiles.appendChild(el);
  }
}
function setFloor(f){
  currentFloor = f;
  document.querySelectorAll('.tab').forEach(function(t,i){t.classList.toggle('active', i===f)});
  renderTiles();
}
function uc(id, on){
  S[id] = on;
  var t = document.getElementById('t'+id);
  if(t){
    t.classList.toggle('on', on);
    t.querySelector('.tile-status').textContent = on?'An':'Aus';
  }
  upd();
}
function upd(){
  var total = S.filter(Boolean).length;
  document.getElementById('hon').textContent = total;
  document.getElementById('heroNum').textContent = total;
  document.getElementById('heroLbl').textContent = total===0?'Alle Lichter sind aus':total===20?'Alle Lichter sind an':'Lichter eingeschaltet';
  for(var f=0; f<3; f++){
    var on = 0;
    for(var i=FS[f]; i<FS[f]+FC[f]; i++) if(S[i]) on++;
    document.getElementById('tn'+f).textContent = on+'/'+FC[f]+' an';
  }
}
function toggle(id){
  var newState = !S[id];
  uc(id, newState);
  document.querySelectorAll('.chip').forEach(function(b){b.classList.remove('active')});
  fetch('/set?relay='+id+'&state='+(newState?1:0)).catch(function(){uc(id, !newState)});
}
function scene(id){
  fetch('/scene?id='+id).then(function(r){return r.json()}).then(function(d){
    if(d.states) for(var i=0;i<20;i++) uc(i, !!d.states[i]);
    document.querySelectorAll('.chip').forEach(function(b,i){b.classList.toggle('active', i===id)});
  });
}
function ping(){
  fetch('/status').then(function(){document.getElementById('dot').className='dot'}).catch(function(){document.getElementById('dot').className='dot off'});
}
setGreet(); setFloor(0); upd();
setInterval(setGreet, 60000);
setInterval(ping, 6000);
</script>
</body></html>)HTML";

// ── Relay-Steuerung ───────────────────────────────────────────

void setRelay(int idx, bool on) {
  lightState[idx] = on;
  digitalWrite(LIGHTS[idx].pin, on ? LOW : HIGH);
}

// ── Seitenaufbau ──────────────────────────────────────────────

String buildLightsJson() {
  String j = "[";
  for (int i = 0; i < NUM_LIGHTS; i++) {
    j += "{\"n\":\"" + String(LIGHTS[i].name) +
         "\",\"i\":\"" + String(LIGHTS[i].icon) +
         "\",\"c\":\"" + String(LIGHTS[i].color) + "\"}";
    if (i < NUM_LIGHTS - 1) j += ",";
  }
  j += "]";
  return j;
}

String buildStatesJson() {
  String j = "[";
  for (int i = 0; i < NUM_LIGHTS; i++) {
    j += lightState[i] ? "true" : "false";
    if (i < NUM_LIGHTS - 1) j += ",";
  }
  j += "]";
  return j;
}

void handleRoot() {
  String page = String(FPSTR(PAGE));
  page.replace("%LIGHTS%", buildLightsJson());
  page.replace("%STATES%", buildStatesJson());
  page.replace("%IP%", WiFi.softAPIP().toString());
  server.send(200, "text/html; charset=utf-8", page);
}

void handleSet() {
  if (!server.hasArg("relay") || !server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"args\"}"); return;
  }
  int idx = server.arg("relay").toInt();
  if (idx < 0 || idx >= NUM_LIGHTS) {
    server.send(400, "application/json", "{\"error\":\"range\"}"); return;
  }
  setRelay(idx, server.arg("state").toInt() != 0);
  server.send(200, "application/json",
    "{\"relay\":" + String(idx) + ",\"state\":" + (lightState[idx] ? "true" : "false") + "}");
}

void handleScene() {
  if (!server.hasArg("id")) { server.send(400, "application/json", "{\"error\":\"id\"}"); return; }
  int id = server.arg("id").toInt();
  if (id < 0 || id >= NUM_SCENES) { server.send(400, "application/json", "{\"error\":\"range\"}"); return; }
  for (int i = 0; i < NUM_LIGHTS; i++) setRelay(i, SCENES[id][i]);
  server.send(200, "application/json",
    "{\"scene\":" + String(id) + ",\"states\":" + buildStatesJson() + "}");
}

void handleStatus() {
  server.send(200, "application/json", "{\"states\":" + buildStatesJson() + "}");
}

// ── Setup & Loop ──────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(300);

  for (int i = 0; i < NUM_LIGHTS; i++) {
    pinMode(LIGHTS[i].pin, OUTPUT);
    setRelay(i, false);
  }

  WiFi.softAP(SSID, PASSWORD);
  IPAddress ip = WiFi.softAPIP();

  Serial.println("\n===== Smart Haus =====");
  Serial.print("WLAN:   "); Serial.println(SSID);
  Serial.print("PW:     "); Serial.println(PASSWORD);
  Serial.print("URL:    http://"); Serial.println(ip);

  server.on("/",       handleRoot);
  server.on("/set",    handleSet);
  server.on("/scene",  handleScene);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
}
