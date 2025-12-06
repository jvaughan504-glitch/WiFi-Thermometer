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
const char* ssid     = "YourSSID";
const char* password = "YourPass";
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
  server.on("/", handleRoot);        // main HTML page
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
  html += "body{font-family:Arial, sans-serif; text-align:center; padding:20px;}";
  html += ".temp{font-size:3em; margin:10px 0;}";
  html += ".sub{font-size:1.5em; color:#555;}";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>ESP32 DS18B20 Temperature</h1>";
  html += "<div id='tempC' class='temp'>--.- &deg;C</div>";
  html += "<div id='tempF' class='sub'>--.- &deg;F</div>";
  html += "<p id='status'>Connecting...</p>";
  html += "<script>";
  html += "async function fetchTemp(){";
  html += "  try {";
  html += "    const r = await fetch('/temp');";
  html += "    if(!r.ok){ throw new Error('HTTP '+r.status); }";
  html += "    const d = await r.json();";
  html += "    if(d.error){";
  html += "      document.getElementById('tempC').textContent = 'ERR';";
  html += "      document.getElementById('tempF').textContent = d.error;";
  html += "      document.getElementById('status').textContent = 'Sensor error';";
  html += "    } else {";
  html += "      document.getElementById('tempC').innerHTML = d.tempC.toFixed(1) + ' &deg;C';";
  html += "      document.getElementById('tempF').innerHTML = d.tempF.toFixed(1) + ' &deg;F';";
  html += "      document.getElementById('status').textContent = 'Last update: ' + new Date().toLocaleTimeString();";
  html += "    }";
  html += "  } catch(e) {";
  html += "    document.getElementById('status').textContent = 'Error: ' + e.message;";
  html += "  }";
  html += "}";
  html += "fetchTemp();";
  html += "setInterval(fetchTemp, 2000);";  // update every 2 seconds
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
