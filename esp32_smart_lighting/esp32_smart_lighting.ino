#include <WiFi.h>
#include <WebServer.h>

// ── WiFi Access Point ──────────────────────────────────────────
const char* SSID     = "SmartHaus";
const char* PASSWORD = "12345678";

// ── Relay-Pins (active LOW: LOW = AN, HIGH = AUS) ─────────────
const int RELAY_PIN[3] = {26, 27, 14};  // Licht 1, 2, 3

bool relayState[3] = {false, false, false};

WebServer server(80);

// ── HTML-Seite (im Flash gespeichert) ─────────────────────────
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart Haus</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: #1a1a2e;
      color: #eee;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    h1 {
      font-size: 2rem;
      color: #e94560;
      margin-bottom: 6px;
      letter-spacing: 1px;
    }
    .subtitle {
      color: #888;
      margin-bottom: 40px;
      font-size: 0.9rem;
    }
    .grid {
      display: flex;
      flex-direction: column;
      gap: 18px;
      width: 100%;
      max-width: 420px;
    }
    .card {
      background: #16213e;
      border-radius: 16px;
      padding: 22px 24px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      box-shadow: 0 4px 24px rgba(0,0,0,0.4);
      transition: transform 0.1s;
    }
    .card:hover { transform: translateY(-2px); }
    .card-left { display: flex; align-items: center; gap: 14px; }
    .icon { font-size: 2rem; line-height: 1; }
    .card-info h2 { font-size: 1.1rem; margin-bottom: 3px; }
    .status { font-size: 0.82rem; color: #666; }
    .status.on { color: #f5a623; font-weight: 600; }
    /* Toggle-Switch */
    .toggle { position: relative; width: 60px; height: 32px; flex-shrink: 0; }
    .toggle input { opacity: 0; width: 0; height: 0; }
    .slider {
      position: absolute;
      inset: 0;
      background: #333;
      border-radius: 32px;
      cursor: pointer;
      transition: background 0.3s;
    }
    .slider:before {
      content: '';
      position: absolute;
      width: 24px; height: 24px;
      left: 4px; top: 4px;
      background: #fff;
      border-radius: 50%;
      transition: transform 0.3s;
      box-shadow: 0 2px 6px rgba(0,0,0,0.3);
    }
    input:checked + .slider { background: #f5a623; }
    input:checked + .slider:before { transform: translateX(28px); }
    /* Ladeindikator */
    .loading { opacity: 0.5; pointer-events: none; }
    .footer {
      margin-top: 40px;
      color: #444;
      font-size: 0.75rem;
    }
  </style>
</head>
<body>
  <h1>&#127968; Smart Haus</h1>
  <p class="subtitle">Beleuchtungssteuerung</p>

  <div class="grid" id="grid">
    %CARDS%
  </div>

  <p class="footer">ESP32 &bull; IP: %IP%</p>

  <script>
    function toggle(relay, checkbox) {
      var grid = document.getElementById('grid');
      grid.classList.add('loading');
      fetch('/toggle?relay=' + relay)
        .then(function(r) { return r.json(); })
        .then(function() { location.reload(); })
        .catch(function() { location.reload(); });
    }
  </script>
</body>
</html>
)rawliteral";

// ── Hilfsfunktionen ───────────────────────────────────────────

void setRelay(int index, bool on) {
  relayState[index] = on;
  // Active-LOW: LOW = Relais AN, HIGH = Relais AUS
  digitalWrite(RELAY_PIN[index], on ? LOW : HIGH);
}

String buildCards() {
  String cards = "";
  for (int i = 0; i < 3; i++) {
    bool on = relayState[i];
    cards += "<div class='card'>";
    cards += "  <div class='card-left'>";
    cards += "    <span class='icon'>&#128161;</span>";
    cards += "    <div class='card-info'>";
    cards += "      <h2>Licht " + String(i + 1) + "</h2>";
    cards += "      <p class='status" + String(on ? " on" : "") + "'>";
    cards += on ? "An" : "Aus";
    cards += "</p></div></div>";
    cards += "  <label class='toggle'>";
    cards += "    <input type='checkbox'" + String(on ? " checked" : "");
    cards += " onchange='toggle(" + String(i + 1) + ", this)'>";
    cards += "    <span class='slider'></span>";
    cards += "  </label>";
    cards += "</div>";
  }
  return cards;
}

String buildPage() {
  String page = String(HTML_PAGE);
  page.replace("%CARDS%", buildCards());
  page.replace("%IP%", WiFi.softAPIP().toString());
  return page;
}

// ── HTTP Handler ──────────────────────────────────────────────

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", buildPage());
}

void handleToggle() {
  if (!server.hasArg("relay")) {
    server.send(400, "application/json", "{\"error\":\"missing relay\"}");
    return;
  }
  int relay = server.arg("relay").toInt();
  if (relay < 1 || relay > 3) {
    server.send(400, "application/json", "{\"error\":\"invalid relay\"}");
    return;
  }
  int idx = relay - 1;
  setRelay(idx, !relayState[idx]);

  String json = "{\"relay\":" + String(relay) +
                ",\"state\":" + (relayState[idx] ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

// ── Setup & Loop ──────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(500);

  // Relay-Pins: alle AUS beim Start
  for (int i = 0; i < 3; i++) {
    pinMode(RELAY_PIN[i], OUTPUT);
    setRelay(i, false);
  }

  // Access Point starten
  WiFi.softAP(SSID, PASSWORD);
  IPAddress ip = WiFi.softAPIP();

  Serial.println("\n===== Smart Haus =====");
  Serial.print("WLAN-Name:  "); Serial.println(SSID);
  Serial.print("Passwort:   "); Serial.println(PASSWORD);
  Serial.print("Website:    http://"); Serial.println(ip);
  Serial.println("======================\n");

  // Routen registrieren
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  server.begin();
  Serial.println("Web-Server gestartet.");
}

void loop() {
  server.handleClient();
}
