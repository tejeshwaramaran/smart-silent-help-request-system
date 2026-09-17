#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WebServer.h>          // <<< ADDED

// --- 1. WiFi & Firebase Credentials ---
#define WIFI_SSID "Name"
#define WIFI_PASSWORD "Password"
#define FIREBASE_HOST "N/A"
#define FIREBASE_AUTH "N/A"

// ======================
// Output Pins
// ======================
#define RED_LED    16
#define GREEN_LED  19
#define BUZZER     12

// Firebase Objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

// <<< ADDED — web server instance and shared state for dashboard
WebServer server(80);

bool redLedState   = false;
bool greenLedState = false;
bool buzzerState   = false;
int  lastButton    = 0;
int  lastMotion    = 0;

// ════════════════════════════════════════════════════════════════
// <<< ADDED — HTML Dashboard (styled same as previous project)
// ════════════════════════════════════════════════════════════════
String buildHTML() {
  return R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Safety Alert Monitor</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: #0d0d1a;
      color: #e8e8f0;
      min-height: 100vh;
      padding: 28px 16px;
    }

    header { text-align: center; margin-bottom: 32px; }
    header h1 {
      font-size: 1.7rem;
      color: #e94560;
      letter-spacing: 1px;
      margin-bottom: 4px;
    }
    header p { color: #666; font-size: 0.82rem; }

    .section-label {
      text-align: center;
      font-size: 0.7rem;
      text-transform: uppercase;
      letter-spacing: 2px;
      color: #555;
      margin-bottom: 12px;
    }

    .grid-1, .grid-2 {
      max-width: 620px;
      margin: 0 auto 28px;
      display: grid;
      gap: 14px;
    }
    .grid-1 { grid-template-columns: 1fr; }
    .grid-2 { grid-template-columns: 1fr 1fr; }

    .card {
      background: #12122a;
      border: 1px solid #1e1e40;
      border-radius: 14px;
      padding: 22px 20px;
      transition: border-color 0.3s;
    }
    .card.alert  { border-color: #e94560; }
    .card.active { border-color: #4ade80; }

    .card-label {
      font-size: 0.72rem;
      text-transform: uppercase;
      letter-spacing: 1.5px;
      color: #555;
      margin-bottom: 10px;
    }

    .sensor-value {
      font-size: 1.8rem;
      font-weight: 700;
      transition: color 0.3s;
    }

    .status-row {
      display: flex;
      align-items: center;
      gap: 8px;
      font-size: 0.95rem;
      font-weight: 600;
      margin-bottom: 16px;
      min-height: 22px;
    }

    .dot {
      width: 10px; height: 10px;
      border-radius: 50%;
      flex-shrink: 0;
      transition: background 0.3s, box-shadow 0.3s;
    }
    .dot.on      { background: #4ade80; box-shadow: 0 0 7px #4ade80; }
    .dot.off     { background: #333; }
    .dot.red-on  { background: #e94560; box-shadow: 0 0 7px #e94560; }
    .dot.buzz-on { background: #f97316; box-shadow: 0 0 7px #f97316; }

    /* ── Alert banner ── */
    .alert-banner {
      max-width: 620px;
      margin: 0 auto 28px;
      background: #2a0a0a;
      border: 2px solid #e94560;
      border-radius: 14px;
      padding: 20px;
      text-align: center;
      display: none;
    }
    .alert-banner.visible { display: block; }
    .alert-banner h2 {
      font-size: 1.3rem;
      color: #e94560;
      margin-bottom: 6px;
      letter-spacing: 1px;
    }
    .alert-banner p {
      font-size: 0.82rem;
      color: #aaa;
      margin-bottom: 0;
    }

    /* ── Buttons ── */
    .btn {
      width: 100%;
      padding: 11px 0;
      border-radius: 9px;
      border: none;
      font-size: 0.88rem;
      font-weight: 700;
      cursor: pointer;
      letter-spacing: 0.5px;
      transition: opacity 0.2s, transform 0.1s;
    }
    .btn:hover  { opacity: 0.85; }
    .btn:active { transform: scale(0.97); }
    .btn-reset  { background: #4ade80; color: #0d0d1a; }
    .btn-reset:disabled { background: #1e3a2a; color: #444; cursor: default; transform: none; }

    .hint {
      font-size: 0.68rem;
      color: #444;
      margin-top: 8px;
      text-align: center;
    }

    .badge {
      display: inline-block;
      background: #0f1a30;
      color: #60a5fa;
      font-size: 0.68rem;
      padding: 3px 9px;
      border-radius: 20px;
      margin-top: 4px;
    }

    footer {
      text-align: center;
      color: #333;
      font-size: 0.72rem;
      margin-top: 8px;
    }
  </style>
</head>
<body>

  <header>
    <h1>&#128680; Safety Alert Monitor</h1>
    <p>IoT Safety Alert System &mdash; ESP32 Live Dashboard</p>
  </header>

  <!-- Alert banner — only visible when full alert is active -->
  <div class="alert-banner" id="alertBanner">
    <h2>&#9888; ALERT TRIGGERED</h2>
    <p>Button pressed AND motion detected &mdash; Red LED and buzzer are active.</p>
  </div>

  <!-- Device A sensors -->
  <p class="section-label">Device A &mdash; Sensor Readings</p>
  <div class="grid-2">

    <div class="card" id="btnCard">
      <div class="card-label">Button (Device A)</div>
      <div class="sensor-value" id="btnVal">--</div>
    </div>

    <div class="card" id="pirCard">
      <div class="card-label">PIR Motion (Device A)</div>
      <div class="sensor-value" id="pirVal">--</div>
    </div>

  </div>

  <!-- Device B outputs -->
  <p class="section-label">Device B &mdash; Output Status</p>
  <div class="grid-2">

    <div class="card" id="redCard">
      <div class="card-label">Red LED</div>
      <div class="status-row">
        <div class="dot" id="redDot"></div>
        <span id="redText">--</span>
      </div>
      <div class="badge">ON when both sensors triggered</div>
    </div>

    <div class="card" id="greenCard">
      <div class="card-label">Green LED</div>
      <div class="status-row">
        <div class="dot" id="greenDot"></div>
        <span id="greenText">--</span>
      </div>
      <div class="badge">ON when only one sensor triggered</div>
    </div>

  </div>

  <div class="grid-1">
    <div class="card" id="buzzerCard">
      <div class="card-label">Buzzer</div>
      <div class="status-row">
        <div class="dot" id="buzzerDot"></div>
        <span id="buzzerText">--</span>
      </div>
      <div class="badge">ON when both sensors triggered</div>
    </div>
  </div>

  <!-- Device B reset control -->
  <p class="section-label">Device B &mdash; Control</p>
  <div class="grid-1">
    <div class="card">
      <div class="card-label">Reset Device B</div>
      <div class="status-row" style="margin-bottom:14px;">
        <div class="dot" id="resetDot"></div>
        <span id="resetText">--</span>
      </div>
      <button class="btn btn-reset" id="resetBtn" onclick="resetDevice()">Reset Device B</button>
      <p class="hint" id="resetHint">Press to clear the alert and turn off all outputs</p>
    </div>
  </div>

  <footer>Auto-refreshing every 2 seconds</footer>

  <script>
    async function fetchStatus() {
      try {
        const res = await fetch('/status');
        const d   = await res.json();

        // ── Button ──
        const btnVal  = document.getElementById('btnVal');
        const btnCard = document.getElementById('btnCard');
        btnVal.textContent = d.button ? 'PRESSED' : 'RELEASED';
        btnVal.style.color = d.button ? '#f97316' : '#4ade80';
        btnCard.classList.toggle('alert',  d.button);
        btnCard.classList.toggle('active', !d.button);

        // ── PIR ──
        const pirVal  = document.getElementById('pirVal');
        const pirCard = document.getElementById('pirCard');
        pirVal.textContent = d.motion ? 'DETECTED' : 'CLEAR';
        pirVal.style.color = d.motion ? '#f97316' : '#4ade80';
        pirCard.classList.toggle('alert',  d.motion);
        pirCard.classList.toggle('active', !d.motion);

        // ── Red LED ──
        document.getElementById('redDot').className  = 'dot ' + (d.redLed ? 'red-on' : 'off');
        document.getElementById('redText').textContent = 'Red LED is currently: ' + (d.redLed ? 'ON' : 'OFF');
        document.getElementById('redCard').classList.toggle('alert', d.redLed);

        // ── Green LED ──
        document.getElementById('greenDot').className  = 'dot ' + (d.greenLed ? 'on' : 'off');
        document.getElementById('greenText').textContent = 'Green LED is currently: ' + (d.greenLed ? 'ON' : 'OFF');
        document.getElementById('greenCard').classList.toggle('active', d.greenLed);

        // ── Buzzer ──
        document.getElementById('buzzerDot').className  = 'dot ' + (d.buzzer ? 'buzz-on' : 'off');
        document.getElementById('buzzerText').textContent = 'Buzzer is currently: ' + (d.buzzer ? 'SOUNDING' : 'SILENT');
        document.getElementById('buzzerCard').classList.toggle('alert', d.buzzer);

        // ── Alert banner — show only when full alert (both sensors + red LED) ──
        const fullAlert = d.button && d.motion && d.redLed;
        document.getElementById('alertBanner').classList.toggle('visible', fullAlert);

        // ── Reset button — only enabled during full alert ──
        const resetBtn  = document.getElementById('resetBtn');
        const resetDot  = document.getElementById('resetDot');
        const resetText = document.getElementById('resetText');
        const resetHint = document.getElementById('resetHint');

        if (d.redLed || d.greenLed || d.buzzer) {
          resetBtn.disabled = false;
          resetDot.className = 'dot red-on';
          resetText.textContent = 'Alert active — reset available';
          resetHint.textContent = 'Press to clear all outputs on Device B';
        } else {
          resetBtn.disabled = true;
          resetDot.className = 'dot on';
          resetText.textContent = 'All clear — no active alert';
          resetHint.textContent = 'Nothing to reset right now';
        }

      } catch (e) {
        console.error('Fetch error:', e);
      }
    }

    async function resetDevice() {
      await fetch('/reset');
      fetchStatus();
    }

    fetchStatus();
    setInterval(fetchStatus, 2000);
  </script>
</body>
</html>
)rawhtml";
}

// ════════════════════════════════════════════════════════════════
// <<< ADDED — Route handlers
// ════════════════════════════════════════════════════════════════

// Serve the dashboard page
void handleRoot() {
  server.send(200, "text/html", buildHTML());
}

// Return current state as JSON (called by webpage every 2 seconds)
void handleStatus() {
  String json = "{";
  json += "\"button\":"  + String(lastButton    ? "true" : "false") + ",";
  json += "\"motion\":"  + String(lastMotion    ? "true" : "false") + ",";
  json += "\"redLed\":"  + String(redLedState   ? "true" : "false") + ",";
  json += "\"greenLed\":" + String(greenLedState ? "true" : "false") + ",";
  json += "\"buzzer\":"  + String(buzzerState   ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// Reset Device B — turns off all outputs regardless of sensor state
void handleReset() {
  digitalWrite(RED_LED,   LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER,    LOW);
  redLedState   = false;
  greenLedState = false;
  buzzerState   = false;
  server.send(200, "text/plain", "RESET");
}

// ════════════════════════════════════════════════════════════════
//  SETUP
// ════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  pinMode(RED_LED,   OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER,    OUTPUT);

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected");

  // <<< ADDED — print dashboard URL to Serial Monitor
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // <<< ADDED — register web routes
  server.on("/",       handleRoot);
  server.on("/status", handleStatus);
  server.on("/reset",  handleReset);
  server.begin();
  Serial.println("Web server started.");
}

// ════════════════════════════════════════════════════════════════
//  LOOP — existing logic completely unchanged
// ════════════════════════════════════════════════════════════════
void loop() {
  server.handleClient();    // <<< ADDED — must be first in loop

  // Read Firebase (existing, unchanged)
  if (Firebase.getInt(firebaseData, "/deviceA/button")) {
    lastButton = firebaseData.intData();
  }
  if (Firebase.getInt(firebaseData, "/deviceA/motion")) {
    lastMotion = firebaseData.intData();
  }

  Serial.print("Button: ");
  Serial.print(lastButton);
  Serial.print(" | Motion: ");
  Serial.println(lastMotion);

  // ==================================
  // Alert Logic — existing, unchanged
  // ==================================
  if (lastButton == 1 && lastMotion == 1) {
    digitalWrite(RED_LED,   HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER,    HIGH);
    redLedState   = true;     // <<< ADDED — track state for dashboard
    greenLedState = false;
    buzzerState   = true;
  }
  else if ((lastButton == 1 && lastMotion == 0) ||
           (lastButton == 0 && lastMotion == 1)) {
    digitalWrite(RED_LED,   LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER,    LOW);
    redLedState   = false;    // <<< ADDED
    greenLedState = true;
    buzzerState   = false;
  }
  else {
    digitalWrite(RED_LED,   LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER,    LOW);
    redLedState   = false;    // <<< ADDED
    greenLedState = false;
    buzzerState   = false;
  }

  delay(500);
}
