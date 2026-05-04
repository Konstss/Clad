#include <WiFi.h>
#include <WebServer.h>

const char* SSID     = "SmartHaus";
const char* PASSWORD = "12345678";

// ── Licht-Definitionen ────────────────────────────────────────
#define NUM_LIGHTS 20

struct Light { const char* name; const char* icon; int pin; };

// GPIO 0 (letzter Pin) Hinweis: nicht waehrend des Bootens LOW halten
const Light LIGHTS[NUM_LIGHTS] = {
  // EG  (Index 0–6)
  {"Wohnzimmer",   "&#128715;", 2 },
  {"K&uuml;che",   "&#127859;", 4 },
  {"Esszimmer",    "&#127869;", 5 },
  {"Flur EG",      "&#128682;", 12},
  {"Bad EG",       "&#128703;", 13},
  {"Garage",       "&#128663;", 14},
  {"Garten",       "&#127807;", 15},
  // 1.OG (Index 7–13)
  {"Schlafzimmer",   "&#128717;", 16},
  {"Kinderzimmer 1", "&#129528;", 17},
  {"Kinderzimmer 2", "&#129528;", 18},
  {"B&uuml;ro",      "&#128187;", 19},
  {"Bad 1.OG",       "&#128703;", 21},
  {"Ankleide",       "&#128131;", 22},
  {"Flur 1.OG",      "&#128682;", 23},
  // 2.OG (Index 14–19)
  {"G&auml;stezimmer", "&#128717;", 25},
  {"Fitness",          "&#127947;", 26},
  {"Bad 2.OG",         "&#128703;", 27},
  {"Dachboden",        "&#128230;", 32},
  {"Terrasse",         "&#9728;",   33},
  {"Flur 2.OG",        "&#128682;", 0 },
};

struct Floor { int start; int count; const char* name; const char* badge; const char* color; };
const Floor FLOORS[3] = {
  {0,  7, "Erdgeschoss",     "EG",    "#f59e0b"},
  {7,  7, "1. Obergeschoss", "1.OG",  "#3b82f6"},
  {14, 6, "2. Obergeschoss", "2.OG",  "#8b5cf6"},
};

bool lightState[NUM_LIGHTS] = {};
WebServer server(80);

// ── CSS + JS (im Flash) ───────────────────────────────────────
const char PAGE_HEAD[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="de"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Haus</title>
<style>
:root{--eg:#f59e0b;--og1:#3b82f6;--og2:#8b5cf6}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',Arial,sans-serif;background:#060614;color:#e2e8f0;min-height:100vh;padding-bottom:40px}
/* Background blobs */
body::before,body::after{content:'';position:fixed;border-radius:50%;filter:blur(120px);z-index:0;pointer-events:none}
body::before{width:600px;height:600px;background:rgba(59,130,246,.07);top:-200px;left:-200px}
body::after{width:500px;height:500px;background:rgba(139,92,246,.07);bottom:-100px;right:-100px}
/* Header */
header{position:sticky;top:0;z-index:100;backdrop-filter:blur(20px);background:rgba(6,6,20,.8);border-bottom:1px solid rgba(255,255,255,.07);padding:16px 24px;display:flex;align-items:center;justify-content:space-between}
.logo{font-size:1.4rem;font-weight:700;letter-spacing:.5px;color:#fff}
.logo span{color:#f59e0b}
.badge{background:rgba(255,255,255,.08);border:1px solid rgba(255,255,255,.12);border-radius:20px;padding:5px 14px;font-size:.8rem;color:#94a3b8}
.badge b{color:#fff}
/* Main */
main{position:relative;z-index:1;max-width:900px;margin:0 auto;padding:28px 16px;display:flex;flex-direction:column;gap:28px}
/* Floor section */
.floor{border-radius:20px;overflow:hidden;border:1px solid rgba(255,255,255,.06);background:rgba(255,255,255,.025)}
.floor-header{display:flex;align-items:center;gap:12px;padding:16px 20px;border-bottom:1px solid rgba(255,255,255,.06)}
.floor-pill{font-size:.7rem;font-weight:700;padding:3px 10px;border-radius:20px;letter-spacing:.5px;border:1px solid currentColor;flex-shrink:0}
.floor-title{font-size:1rem;font-weight:600;color:#cbd5e1;flex:1}
.floor-count{font-size:.78rem;color:#64748b;flex-shrink:0}
.floor-btns{display:flex;gap:6px;flex-shrink:0}
.floor-btns button{font-size:.72rem;padding:5px 10px;border-radius:8px;border:1px solid rgba(255,255,255,.1);background:rgba(255,255,255,.05);color:#94a3b8;cursor:pointer;transition:.2s}
.floor-btns button:hover{background:rgba(255,255,255,.1);color:#fff}
/* Cards grid */
.cards{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:12px;padding:16px}
/* Single card */
.card{border-radius:14px;padding:16px 14px;background:rgba(255,255,255,.03);border:1px solid rgba(255,255,255,.07);display:flex;flex-direction:column;align-items:center;gap:8px;transition:all .3s;cursor:default}
.card.on{border-color:var(--fc,#f59e0b);box-shadow:0 0 24px rgba(var(--fc-rgb,245,158,11),.18)}
.card-icon{font-size:2rem;transition:filter .3s}
.card.on .card-icon{filter:drop-shadow(0 0 10px var(--fc,#f59e0b))}
.card-name{font-size:.78rem;color:#94a3b8;text-align:center;font-weight:500}
.card-status{font-size:.7rem;padding:2px 10px;border-radius:10px;background:rgba(255,255,255,.05);color:#475569;transition:all .3s}
.card.on .card-status{background:rgba(var(--fc-rgb,245,158,11),.15);color:var(--fc,#f59e0b);font-weight:600}
/* Toggle */
.toggle{position:relative;width:46px;height:26px;flex-shrink:0;margin-top:4px}
.toggle input{opacity:0;width:0;height:0}
.slider{position:absolute;inset:0;background:#1e2a3a;border-radius:26px;cursor:pointer;transition:.3s;border:1px solid rgba(255,255,255,.1)}
.slider:before{content:'';position:absolute;width:18px;height:18px;left:3px;top:3px;background:#475569;border-radius:50%;transition:.3s}
input:checked+.slider{background:var(--fc,#f59e0b);border-color:var(--fc,#f59e0b)}
input:checked+.slider:before{transform:translateX(20px);background:#fff}
/* Footer */
footer{text-align:center;color:#1e293b;font-size:.72rem;margin-top:32px;position:relative;z-index:1}
@media(max-width:480px){.floor-header{flex-wrap:wrap}.floor-btns{width:100%}.floor-btns button{flex:1}.cards{grid-template-columns:repeat(2,1fr)}}
</style>
</head><body>)rawliteral";

const char PAGE_FOOT[] PROGMEM = R"rawliteral(
<script>
var states=%STATES%;
function updateCard(id,on){
  var card=document.getElementById('c'+id);
  var st=document.getElementById('s'+id);
  var cb=document.getElementById('b'+id);
  states[id]=on;
  cb.checked=on;
  card.classList.toggle('on',on);
  st.textContent=on?'An':'Aus';
  updateCounts();
}
function updateCounts(){
  var total=0;
  for(var i=0;i<states.length;i++) if(states[i]) total++;
  document.getElementById('totalOn').textContent=total;
  [0,1,2].forEach(function(f){
    var starts=[0,7,14],counts=[7,7,6];
    var on=0;
    for(var i=starts[f];i<starts[f]+counts[f];i++) if(states[i]) on++;
    document.getElementById('fc'+f).textContent=on+'/'+counts[f]+' an';
  });
}
function toggle(id){
  var cb=document.getElementById('b'+id);
  var on=cb.checked;
  updateCard(id,on);
  fetch('/set?relay='+id+'&state='+(on?1:0))
    .catch(function(){updateCard(id,!on)});
}
function floorAll(floor,state){
  var starts=[0,7,14],counts=[7,7,6];
  var s=starts[floor],c=counts[floor];
  var promises=[];
  for(var i=s;i<s+c;i++){
    updateCard(i,state);
    promises.push(fetch('/set?relay='+i+'&state='+(state?1:0)));
  }
}
</script>
</body></html>)rawliteral";

// ── Relay steuern ─────────────────────────────────────────────

void setRelay(int idx, bool on) {
  lightState[idx] = on;
  digitalWrite(LIGHTS[idx].pin, on ? LOW : HIGH);
}

// ── Seitenaufbau ──────────────────────────────────────────────

String buildPage() {
  String page = String(FPSTR(PAGE_HEAD));

  // Active count badge
  int totalOn = 0;
  for (int i = 0; i < NUM_LIGHTS; i++) if (lightState[i]) totalOn++;

  page += "<header><div class='logo'>&#127968; Smart <span>Haus</span></div>";
  page += "<div class='badge'><b id='totalOn'>" + String(totalOn) + "</b> / " + String(NUM_LIGHTS) + " Lichter an</div></header>";
  page += "<main>";

  const char* floorColors[3]    = {"#f59e0b","#3b82f6","#8b5cf6"};
  const char* floorColorRgb[3]  = {"245,158,11","59,130,246","139,92,246"};

  for (int f = 0; f < 3; f++) {
    const Floor& fl = FLOORS[f];
    int floorOn = 0;
    for (int i = fl.start; i < fl.start + fl.count; i++) if (lightState[i]) floorOn++;

    page += "<section class='floor'>";
    page += "<div class='floor-header'>";
    page += "<span class='floor-pill' style='color:" + String(fl.color) + "'>" + fl.badge + "</span>";
    page += "<span class='floor-title'>" + String(fl.name) + "</span>";
    page += "<span class='floor-count' id='fc" + String(f) + "'>" + String(floorOn) + "/" + String(fl.count) + " an</span>";
    page += "<div class='floor-btns'>";
    page += "<button onclick='floorAll(" + String(f) + ",true)'>Alle AN</button>";
    page += "<button onclick='floorAll(" + String(f) + ",false)'>Alle AUS</button>";
    page += "</div></div>";

    page += "<div class='cards'>";
    for (int i = fl.start; i < fl.start + fl.count; i++) {
      bool on = lightState[i];
      String onClass = on ? " on" : "";
      page += "<div class='card" + onClass + "' id='c" + String(i) + "' style='--fc:" + String(fl.color) + ";--fc-rgb:" + String(floorColorRgb[f]) + "'>";
      page += "<div class='card-icon'>" + String(LIGHTS[i].icon) + "</div>";
      page += "<div class='card-name'>" + String(LIGHTS[i].name) + "</div>";
      page += "<div class='card-status' id='s" + String(i) + "'>" + (on ? "An" : "Aus") + "</div>";
      page += "<label class='toggle'>";
      page += "<input type='checkbox' id='b" + String(i) + "'" + String(on ? " checked" : "") + " onchange='toggle(" + String(i) + ")'>";
      page += "<span class='slider'></span></label>";
      page += "</div>";
    }
    page += "</div></section>";
  }

  page += "</main>";
  page += "<footer>ESP32 &bull; " + WiFi.softAPIP().toString() + "</footer>";

  // Build states JSON array
  String states = "[";
  for (int i = 0; i < NUM_LIGHTS; i++) {
    states += (lightState[i] ? "true" : "false");
    if (i < NUM_LIGHTS - 1) states += ",";
  }
  states += "]";

  String foot = String(FPSTR(PAGE_FOOT));
  foot.replace("%STATES%", states);
  page += foot;
  return page;
}

// ── HTTP Handler ──────────────────────────────────────────────

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildPage());
}

void handleSet() {
  if (!server.hasArg("relay") || !server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"missing args\"}");
    return;
  }
  int idx   = server.arg("relay").toInt();
  int state = server.arg("state").toInt();
  if (idx < 0 || idx >= NUM_LIGHTS) {
    server.send(400, "application/json", "{\"error\":\"invalid relay\"}");
    return;
  }
  setRelay(idx, state != 0);
  server.send(200, "application/json",
    "{\"relay\":" + String(idx) + ",\"state\":" + (lightState[idx] ? "true" : "false") + "}");
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

  Serial.println("\n╔══════════════════════╗");
  Serial.println("║     Smart Haus       ║");
  Serial.println("╠══════════════════════╣");
  Serial.print("║ WLAN:  "); Serial.println(SSID);
  Serial.print("║ PW:    "); Serial.println(PASSWORD);
  Serial.print("║ URL:   http://"); Serial.println(ip);
  Serial.println("╚══════════════════════╝");

  server.on("/",    handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

void loop() {
  server.handleClient();
}
