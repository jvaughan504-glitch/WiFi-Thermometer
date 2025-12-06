/*
  ESP32 + DS18B20 + 128x64 OLED + Simple Auto-Refreshing Web Server
  - DS18B20 data pin on GPIO 4 (D4)
  - I2C OLED (SSD1306 128x64) on default ESP32 I2C pins:
      SDA -> GPIO 21
      SCL -> GPIO 22
*/

#include <WiFi.h>
#include <WebServer.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- USER CONFIG ----------
const char* ssid     = "BELL728";
const char* password = "9134EC94D365";
// --------------------------------

// DS18B20 on GPIO 4 (D4 on many dev boards)
#define ONE_WIRE_BUS 4

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Reset pin (or -1 if shared / not used)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// OneWire + DallasTemperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Web server on port 80
WebServer server(80);

// Global temperature value
float temperatureC = NAN;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL_MS = 1000; // 1 second

// ---------- Function Prototypes ----------
void handleRoot();
void handleTempJSON();
void updateOLED();
void readTemperature();
// ----------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Common I2C address is 0x3C
    Serial.println("SSD1306 allocation failed");
    // Continue anyway; just no display
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Booting...");
    display.display();
  }

  // Init DS18B20
  sensors.begin();
  Serial.println("DS18B20 initialized.");

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (display.width() > 0) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting to WiFi:");
    display.println(ssid);
    display.display();
  }

  Serial.printf("Connecting to %s", ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Show IP on OLED
  if (display.width() > 0) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi connected");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(1500);
  }

  // Initial temperature read
  readTemperature();
  updateOLED();

  // Configure web server routes
  server.on("/", handleRoot);         // main HTML page
  server.on("/temp", handleTempJSON); // JSON endpoint
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // Handle incoming HTTP clients
  server.handleClient();

  // Periodically read the temperature
  unsigned long now = millis();
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;
    readTemperature();
    updateOLED();
  }
}

// Read temperature from DS18B20
void readTemperature() {
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor disconnected!");
  } else {
    Serial.print(temperatureC);
    Serial.println(" C");
  }
}

// Update the OLED screen with the latest temperature
void updateOLED() {
  if (display.width() == 0) return; // Display not initialized or failed

  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("TEMP");

  display.setTextSize(3);
  display.setCursor(0, 24);

  if (temperatureC == DEVICE_DISCONNECTED_C || isnan(temperatureC)) {
    display.println("ERR");
  } else {
    // Show Celsius with 1 decimal place
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f C", temperatureC);
    display.println(buf);
  }

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("IP: ");
  display.print(WiFi.localIP());

  display.display();
}

// HTTP handler for "/" – serves auto-refreshing web UI
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>ESP32 Temperature</title>";

  html += "<style>";

  // -------------------------
  // LIGHT MODE
  // -------------------------
  html += "@media (prefers-color-scheme: light) {";
  html += "body{margin:0;font-family:system-ui;background:#f5f5f5;color:#111;"
          "display:flex;justify-content:center;align-items:center;min-height:100vh;}";
  html += ".card{background:white;border-radius:16px;padding:24px;box-shadow:0 8px 20px rgba(0,0,0,0.15);"
          "max-width:380px;width:100%;box-sizing:border-box;border:1px solid #ddd;}";
  html += ".title{font-size:1.1rem;font-weight:600;margin-bottom:4px;color:#222;}";
  html += ".subtitle{font-size:0.85rem;color:#666;margin-top:0;margin-bottom:16px;}";
  html += ".temp-main{font-size:3rem;font-weight:600;margin:8px 0;color:#d9460f;text-align:center;}";
  html += ".temp-sub{font-size:1.3rem;color:#333;text-align:center;margin:0 0 4px 0;}";
  html += ".pill-row{display:flex;gap:8px;margin-top:10px;flex-wrap:wrap;}";
  html += ".pill{font-size:0.75rem;padding:4px 8px;border-radius:999px;background:#fff;"
          "border:1px solid #ccc;color:#333;}";
  html += ".badge-dot{display:inline-block;width:8px;height:8px;border-radius:999px;margin-right:6px;"
          "background:#16a34a;vertical-align:middle;}";
  html += ".status{font-size:0.8rem;color:#555;margin-top:14px;text-align:left;}";
  html += "}";

  // -------------------------
  // DARK MODE
  // -------------------------
  html += "@media (prefers-color-scheme: dark) {";
  html += "body{margin:0;font-family:system-ui;background:#0b1120;color:#e5e7eb;"
          "display:flex;justify-content:center;align-items:center;min-height:100vh;}";
  html += ".card{background:#020617;border-radius:16px;padding:24px;box-shadow:0 20px 40px rgba(0,0,0,0.6);"
          "max-width:380px;width:100%;box-sizing:border-box;border:1px solid rgba(148,163,184,0.28);}";
  html += ".title{font-size:1.1rem;font-weight:600;color:#f9fafb;margin-bottom:4px;}";
  html += ".subtitle{font-size:0.78rem;color:#9ca3af;margin-top:0;margin-bottom:16px;}";
  html += ".temp-main{font-size:3rem;font-weight:600;margin:8px 0;color:#f97316;text-align:center;}";
  html += ".temp-sub{font-size:1.3rem;color:#e5e7eb;text-align:center;margin:0 0 4px 0;opacity:0.9;}";
  html += ".pill-row{display:flex;gap:8px;margin-top:10px;flex-wrap:wrap;}";
  html += ".pill{font-size:0.75rem;padding:4px 8px;border-radius:999px;"
          "background:rgba(15,23,42,0.9);border:1px solid rgba(148,163,184,0.4);color:#e5e7eb;}";
  html += ".badge-dot{display:inline-block;width:8px;height:8px;border-radius:999px;margin-right:6px;"
          "background:#22c55e;box-shadow:0 0 8px rgba(34,197,94,0.9);vertical-align:middle;}";
  html += ".status{font-size:0.78rem;color:#9ca3af;margin-top:14px;text-align:left;}";
  html += "}";

  html += "</style></head><body>";

  html += "<div class='card'>";
  html += "  <h1 class='title'>ESP32 DS18B20</h1>";
  html += "  <p class='subtitle'>Live temperature monitor</p>";
  html += "  <div id='tempC' class='temp-main'>--.- &deg;C</div>";
  html += "  <div id='tempF' class='temp-sub'>--.- &deg;F</div>";

  html += "  <div class='pill-row'>";
  html += "    <div class='pill'><span class='badge-dot'></span><span id='netStatus'>Online</span></div>";
  html += "    <div class='pill'>Update: <span id='updateTime'>--:--:--</span></div>";
  html += "  </div>";

  html += "  <p id='status' class='status'>Connecting to sensor...</p>";
  html += "</div>";

  // JavaScript: Auto-refresh the temperature every 2 seconds
  html += "<script>";
  html += "async function fetchTemp(){";
  html += "  const statusEl = document.getElementById('status');";
  html += "  const netStatusEl = document.getElementById('netStatus');";
  html += "  const updateTimeEl = document.getElementById('updateTime');";
  html += "  try {";
  html += "    const r = await fetch('/temp',{cache:'no-store'});";
  html += "    const d = await r.json();";
  html += "    if(d.error){";
  html += "      document.getElementById('tempC').innerHTML = 'ERR';";
  html += "      document.getElementById('tempF').innerHTML = d.error;";
  html += "      statusEl.textContent = 'Sensor error: ' + d.error;";
  html += "    } else {";
  html += "      document.getElementById('tempC').innerHTML = d.tempC.toFixed(1) + ' &deg;C';";
  html += "      document.getElementById('tempF').innerHTML = d.tempF.toFixed(1) + ' &deg;F';";
  html += "      const now = new Date();";
  html += "      const t = now.toLocaleTimeString();";
  html += "      statusEl.textContent = 'Last update at ' + t;";
  html += "      updateTimeEl.textContent = t;";
  html += "      netStatusEl.textContent = 'Online';";
  html += "    }";
  html += "  } catch(e) {";
  html += "    statusEl.textContent = 'Web request failed';";
  html += "    netStatusEl.textContent = 'Offline';";
  html += "  }";
  html += "}";
  html += "fetchTemp();";
  html += "setInterval(fetchTemp, 2000);";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// JSON endpoint for temperature
void handleTempJSON() {
  String json;
  if (temperatureC == DEVICE_DISCONNECTED_C || isnan(temperatureC)) {
    json = "{\"error\":\"Sensor disconnected or not found\"}";
  } else {
    float tempF = temperatureC * 9.0 / 5.0 + 32.0;
    json = "{";
    json += "\"tempC\":" + String(temperatureC, 3) + ",";
    json += "\"tempF\":" + String(tempF, 3);
    json += "}";
  }
  server.send(200, "application/json", json);
}
