#include <WiFi.h>
#include <WebServer.h>

const char* SSID     = "SmartHaus";
const char* PASSWORD = "12345678";

// ── Licht-Definitionen ────────────────────────────────────────
#define NUM_LIGHTS 20
#define NUM_SCENES 5

struct Light { const char* name; const char* icon; int pin; };

const Light LIGHTS[NUM_LIGHTS] = {
  {"Wohnzimmer",   "&#128715;", 2 }, {"K&uuml;che",   "&#127859;", 4 },
  {"Esszimmer",    "&#127869;", 5 }, {"Flur EG",      "&#128682;", 12},
  {"Bad EG",       "&#128703;", 13}, {"Garage",       "&#128663;", 14},
  {"Garten",       "&#127807;", 15},
  {"Schlafzimmer", "&#128717;", 16}, {"Kinderzimmer 1","&#129528;", 17},
  {"Kinderzimmer 2","&#129528;",18}, {"B&uuml;ro",    "&#128187;", 19},
  {"Bad 1.OG",     "&#128703;", 21}, {"Ankleide",     "&#128131;", 22},
  {"Flur 1.OG",    "&#128682;", 23},
  {"G&auml;stezimmer","&#128717;",25},{"Fitness",      "&#127947;", 26},
  {"Bad 2.OG",     "&#128703;", 27}, {"Dachboden",    "&#128230;", 32},
  {"Terrasse",     "&#9728;",   33}, {"Flur 2.OG",    "&#128682;", 0 },
};

struct Floor  { int start; int count; const char* name; const char* badge; const char* color; };
const Floor FLOORS[3] = {
  {0,  7, "Erdgeschoss",     "EG",   "#f59e0b"},
  {7,  7, "1. Obergeschoss", "1.OG", "#3b82f6"},
  {14, 6, "2. Obergeschoss", "2.OG", "#8b5cf6"},
};

struct Scene  { const char* name; const char* icon; bool states[NUM_LIGHTS]; };
const Scene SCENES[NUM_SCENES] = {
  {"Alle AN",  "&#9728;",   {1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1}},
  {"Abend",    "&#127748;", {1,1,1,1,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0}},
  {"Nacht",    "&#127769;", {0,0,0,1,0,0,0, 0,0,0,0,0,0,1, 0,0,0,0,0,1}},
  {"Film",     "&#127916;", {1,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0}},
  {"Alle AUS", "&#9899;",   {0,0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0,0,0}},
};

bool lightState[NUM_LIGHTS] = {};
WebServer server(80);

// ── CSS + JS ──────────────────────────────────────────────────
const char PAGE_CSS[] PROGMEM = R"rawliteral(<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',Arial,sans-serif;background:#060614;color:#e2e8f0;min-height:100vh;padding-bottom:60px}
body::before,body::after{content:'';position:fixed;border-radius:50%;filter:blur(130px);z-index:0;pointer-events:none}
body::before{width:600px;height:600px;background:rgba(59,130,246,.06);top:-200px;left:-200px}
body::after{width:500px;height:500px;background:rgba(139,92,246,.06);bottom:-100px;right:-100px}
header{position:sticky;top:0;z-index:100;backdrop-filter:blur(20px);background:rgba(6,6,20,.92);border-bottom:1px solid rgba(255,255,255,.07);padding:13px 18px;display:flex;align-items:center;gap:10px}
.logo{font-size:1.25rem;font-weight:700;color:#fff;flex:1}.logo span{color:#f59e0b}
.hbadge{background:rgba(255,255,255,.07);border:1px solid rgba(255,255,255,.1);border-radius:20px;padding:4px 11px;font-size:.76rem;color:#94a3b8;white-space:nowrap}
.hbadge b{color:#fff}
.sdot{width:8px;height:8px;border-radius:50%;background:#22c55e;box-shadow:0 0 8px #22c55e;flex-shrink:0}
.sdot.off{background:#ef4444;box-shadow:0 0 8px #ef4444}
.sbar{display:flex;gap:7px;padding:12px 14px;overflow-x:auto;position:relative;z-index:1;border-bottom:1px solid rgba(255,255,255,.05)}
.sbar::-webkit-scrollbar{display:none}
.sbtn{display:flex;align-items:center;gap:5px;padding:6px 14px;border-radius:20px;border:1px solid rgba(255,255,255,.1);background:rgba(255,255,255,.04);color:#64748b;cursor:pointer;font-size:.78rem;white-space:nowrap;transition:all .2s;flex-shrink:0;font-family:inherit}
.sbtn:hover{background:rgba(255,255,255,.09);color:#cbd5e1;border-color:rgba(255,255,255,.2)}
.sbtn.active{background:rgba(245,158,11,.12);border-color:rgba(245,158,11,.4);color:#fbbf24}
main{position:relative;z-index:1;max-width:920px;margin:0 auto;padding:18px 12px;display:flex;flex-direction:column;gap:16px}
.floor{border-radius:18px;overflow:hidden;border:1px solid rgba(255,255,255,.07);background:rgba(255,255,255,.02)}
.fhdr{display:flex;align-items:center;gap:9px;padding:13px 16px;cursor:pointer;user-select:none;transition:background .2s}
.fhdr:hover{background:rgba(255,255,255,.03)}
.fpill{font-size:.67rem;font-weight:700;padding:2px 8px;border-radius:10px;border:1px solid currentColor;flex-shrink:0;letter-spacing:.5px}
.ftitle{font-size:.92rem;font-weight:600;color:#cbd5e1;flex:1}
.fcnt{font-size:.73rem;color:#475569;flex-shrink:0}
.fbtns{display:flex;gap:5px;flex-shrink:0}
.fbtns button{font-size:.68rem;padding:4px 9px;border-radius:7px;border:1px solid rgba(255,255,255,.1);background:rgba(255,255,255,.04);color:#64748b;cursor:pointer;transition:.2s;font-family:inherit}
.fbtns button:hover{background:rgba(255,255,255,.09);color:#e2e8f0}
.chev{color:#334155;font-size:.6rem;transition:transform .35s;flex-shrink:0}
.floor.closed .chev{transform:rotate(-90deg)}
.fprog{height:2px;background:rgba(255,255,255,.04)}
.fprogb{height:100%;border-radius:2px;transition:width .5s ease}
.fbody{overflow:hidden;transition:max-height .4s cubic-bezier(.4,0,.2,1)}
.floor.closed .fbody{max-height:0!important}
.cards{display:grid;grid-template-columns:repeat(auto-fill,minmax(148px,1fr));gap:9px;padding:12px}
.card{border-radius:13px;padding:14px 10px;background:rgba(255,255,255,.03);border:1px solid rgba(255,255,255,.07);display:flex;flex-direction:column;align-items:center;gap:7px;transition:border-color .3s,background .3s,box-shadow .3s;cursor:pointer;-webkit-tap-highlight-color:transparent}
.card:active{transform:scale(.95)}
.card:hover{background:rgba(255,255,255,.05)}
@keyframes glow{0%,100%{box-shadow:0 0 14px color-mix(in srgb,var(--fc) 18%,transparent)}50%{box-shadow:0 0 28px color-mix(in srgb,var(--fc) 32%,transparent)}}
.card.on{border-color:var(--fc);background:color-mix(in srgb,var(--fc) 6%,transparent);animation:glow 3s ease-in-out infinite}
.cicon{font-size:1.9rem;transition:filter .3s;line-height:1}
.card.on .cicon{filter:drop-shadow(0 0 8px var(--fc))}
.cname{font-size:.73rem;color:#94a3b8;text-align:center;font-weight:500;line-height:1.3}
.cstat{font-size:.67rem;padding:2px 9px;border-radius:9px;background:rgba(255,255,255,.04);color:#334155;transition:all .3s}
.card.on .cstat{background:color-mix(in srgb,var(--fc) 12%,transparent);color:var(--fc);font-weight:600}
.tgl{position:relative;width:42px;height:23px;flex-shrink:0;margin-top:1px}
.tgl input{opacity:0;width:0;height:0}
.tslider{position:absolute;inset:0;background:#1a2332;border-radius:23px;cursor:pointer;transition:.3s;border:1px solid rgba(255,255,255,.08)}
.tslider:before{content:'';position:absolute;width:15px;height:15px;left:3px;top:3px;background:#374151;border-radius:50%;transition:.3s}
input:checked+.tslider{background:var(--fc);border-color:var(--fc)}
input:checked+.tslider:before{transform:translateX(19px);background:#fff}
footer{text-align:center;color:#1e293b;font-size:.68rem;margin-top:16px;position:relative;z-index:1}
@media(max-width:500px){.fhdr{flex-wrap:wrap}.fbtns{order:5;width:100%}.fbtns button{flex:1}.cards{grid-template-columns:repeat(2,1fr)}.chev{display:none}}
</style>)rawliteral";

const char PAGE_JS[] PROGMEM = R"rawliteral(<script>
var S=%STATES%;
var FS=[0,7,14],FC=[7,7,6];
var activeScene=-1;
function uc(id,on){
  var c=document.getElementById('c'+id),st=document.getElementById('s'+id),cb=document.getElementById('b'+id);
  S[id]=on;cb.checked=on;
  c.classList.toggle('on',on);
  st.textContent=on?'An':'Aus';
  upd();
}
function upd(){
  var t=S.filter(Boolean).length;
  document.getElementById('hon').textContent=t;
  for(var f=0;f<3;f++){
    var on=0;
    for(var i=FS[f];i<FS[f]+FC[f];i++) if(S[i])on++;
    document.getElementById('fc'+f).textContent=on+'/'+FC[f]+' an';
    var pct=Math.round(on/FC[f]*100);
    document.getElementById('fp'+f).style.width=pct+'%';
  }
}
function toggle(id){
  var on=document.getElementById('b'+id).checked;
  uc(id,on);
  activeScene=-1;updScene(-1);
  fetch('/set?relay='+id+'&state='+(on?1:0)).catch(function(){uc(id,!on)});
}
function floorAll(f,state){
  for(var i=FS[f];i<FS[f]+FC[f];i++){uc(i,state);fetch('/set?relay='+i+'&state='+(state?1:0));}
  activeScene=-1;updScene(-1);
}
function scene(id){
  fetch('/scene?id='+id).then(function(r){return r.json();}).then(function(d){
    if(d.states){d.states.forEach(function(on,i){uc(i,on);});}
    activeScene=id;updScene(id);
  }).catch(function(){});
}
function updScene(id){
  document.querySelectorAll('.sbtn').forEach(function(b,i){b.classList.toggle('active',i===id);});
}
function toggleFloor(f){
  var fl=document.getElementById('floor'+f);
  var body=document.getElementById('fb'+f);
  var closing=!fl.classList.contains('closed');
  if(!closing) body.style.maxHeight=body.scrollHeight+'px';
  fl.classList.toggle('closed',closing);
  if(!closing) setTimeout(function(){body.style.maxHeight=body.scrollHeight+'px';},10);
}
function ping(){
  fetch('/status').then(function(){document.getElementById('sdot').className='sdot';}).catch(function(){document.getElementById('sdot').className='sdot off';});
}
setInterval(ping,6000);
</script>)rawliteral";

// ── Relay-Steuerung ───────────────────────────────────────────

void setRelay(int idx, bool on) {
  lightState[idx] = on;
  digitalWrite(LIGHTS[idx].pin, on ? LOW : HIGH);
}

// ── Seitenaufbau ──────────────────────────────────────────────

String buildPage() {
  // States-JSON
  String states = "[";
  for (int i = 0; i < NUM_LIGHTS; i++) {
    states += lightState[i] ? "true" : "false";
    if (i < NUM_LIGHTS - 1) states += ",";
  }
  states += "]";

  int totalOn = 0;
  for (int i = 0; i < NUM_LIGHTS; i++) if (lightState[i]) totalOn++;

  String p = "<!DOCTYPE html><html lang='de'><head><meta charset='UTF-8'>";
  p += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  p += "<title>Smart Haus</title>";
  p += String(FPSTR(PAGE_CSS));
  p += "</head><body>";

  // Header
  p += "<header>";
  p += "<div class='logo'>&#127968; Smart <span>Haus</span></div>";
  p += "<div class='hbadge'><b id='hon'>" + String(totalOn) + "</b> / " + String(NUM_LIGHTS) + " an</div>";
  p += "<div class='sdot' id='sdot'></div>";
  p += "</header>";

  // Szenen-Leiste
  p += "<div class='sbar'>";
  for (int i = 0; i < NUM_SCENES; i++) {
    p += "<button class='sbtn' onclick='scene(" + String(i) + ")'>";
    p += String(SCENES[i].icon) + " " + String(SCENES[i].name);
    p += "</button>";
  }
  p += "</div>";

  // Etagen
  p += "<main>";
  const char* floorColors[3] = {"#f59e0b","#3b82f6","#8b5cf6"};

  for (int f = 0; f < 3; f++) {
    const Floor& fl = FLOORS[f];
    int on = 0;
    for (int i = fl.start; i < fl.start + fl.count; i++) if (lightState[i]) on++;
    int pct = fl.count > 0 ? (on * 100 / fl.count) : 0;

    p += "<section class='floor' id='floor" + String(f) + "'>";

    // Header
    p += "<div class='fhdr' onclick='toggleFloor(" + String(f) + ")'>";
    p += "<span class='fpill' style='color:" + String(fl.color) + "'>" + fl.badge + "</span>";
    p += "<span class='ftitle'>" + fl.name + "</span>";
    p += "<span class='fcnt' id='fc" + String(f) + "'>" + String(on) + "/" + String(fl.count) + " an</span>";
    p += "<div class='fbtns'><button onclick='event.stopPropagation();floorAll(" + String(f) + ",true)'>Alle AN</button>";
    p += "<button onclick='event.stopPropagation();floorAll(" + String(f) + ",false)'>Alle AUS</button></div>";
    p += "<span class='chev'>&#9660;</span></div>";

    // Progress bar
    p += "<div class='fprog'><div class='fprogb' id='fp" + String(f) + "' style='width:" + String(pct) + "%;background:" + fl.color + "'></div></div>";

    // Karten
    p += "<div class='fbody' id='fb" + String(f) + "' style='max-height:1000px'><div class='cards'>";
    for (int i = fl.start; i < fl.start + fl.count; i++) {
      bool card_on = lightState[i];
      p += "<div class='card" + String(card_on ? " on" : "") + "' id='c" + String(i) + "' style='--fc:" + fl.color + "' onclick='document.getElementById(\"b" + String(i) + "\").click()'>";
      p += "<div class='cicon'>" + String(LIGHTS[i].icon) + "</div>";
      p += "<div class='cname'>" + String(LIGHTS[i].name) + "</div>";
      p += "<div class='cstat' id='s" + String(i) + "'>" + (card_on ? "An" : "Aus") + "</div>";
      p += "<label class='tgl' onclick='event.stopPropagation()'>";
      p += "<input type='checkbox' id='b" + String(i) + "'" + String(card_on ? " checked" : "") + " onchange='toggle(" + String(i) + ")'>";
      p += "<span class='tslider'></span></label></div>";
    }
    p += "</div></div></section>";
  }

  p += "</main><footer>ESP32 &bull; " + WiFi.softAPIP().toString() + "</footer>";

  String js = String(FPSTR(PAGE_JS));
  js.replace("%STATES%", states);
  p += js;
  p += "</body></html>";
  return p;
}

// ── HTTP Handler ──────────────────────────────────────────────

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildPage());
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
  if (!server.hasArg("id")) {
    server.send(400, "application/json", "{\"error\":\"id\"}"); return;
  }
  int id = server.arg("id").toInt();
  if (id < 0 || id >= NUM_SCENES) {
    server.send(400, "application/json", "{\"error\":\"range\"}"); return;
  }
  for (int i = 0; i < NUM_LIGHTS; i++) setRelay(i, SCENES[id].states[i]);

  String json = "{\"scene\":" + String(id) + ",\"states\":[";
  for (int i = 0; i < NUM_LIGHTS; i++) {
    json += lightState[i] ? "true" : "false";
    if (i < NUM_LIGHTS - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleStatus() {
  String json = "{\"lights\":[";
  for (int i = 0; i < NUM_LIGHTS; i++) {
    json += lightState[i] ? "true" : "false";
    if (i < NUM_LIGHTS - 1) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
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
  Serial.println("======================");

  server.on("/",      handleRoot);
  server.on("/set",   handleSet);
  server.on("/scene", handleScene);
  server.on("/status",handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
}
