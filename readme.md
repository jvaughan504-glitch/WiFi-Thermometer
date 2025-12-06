ESP32 DS18B20 Temperature Monitor + OLED + Auto-Refreshing Web Server

This project uses an ESP32 DevKit V1, a DS18B20 digital temperature sensor, and a 128×64 I²C OLED display to create a standalone temperature monitor.
The device displays live temperature readings on the OLED while also hosting a web server that shows the temperature in both °C and °F with automatic updates every 2 seconds.

This is a great starter IoT project demonstrating sensors, displays, networking, JSON data endpoints, and dynamic web interfaces.

📌 Features
✔ DS18B20 Temperature Sensor

Accurate 1-Wire temperature sensor

Reads temperature every 1 second

Automatic error handling if sensor becomes disconnected

✔ 128×64 OLED Display (SSD1306)

Displays:

Current temperature (°C)

Wi-Fi connection status

ESP32 IP address

Easy to read, large text layout

✔ Wi-Fi Web Server

ESP32 connects to your Wi-Fi network

Serves a web dashboard at:
http://<ESP32-IP>/

Auto-refreshing UI using JavaScript (AJAX)

Fetches temperature from JSON endpoint (/temp) every 2 seconds

✔ Lightweight & Responsive

No additional frameworks needed

Runs on standard Arduino IDE ESP32 core

Works on desktop and mobile browsers

📡 How It Works
1. Sensor Reading

The DS18B20 is connected through the OneWire protocol on GPIO 4.
The ESP32 requests a temperature reading once per second using the DallasTemperature library.

2. OLED Display

Each new reading is written to the OLED display using the Adafruit SSD1306 driver.
The display shows:

TEMP
23.4 C

IP: 192.168.1.50

3. Web Server

The ESP32 runs a minimal HTTP server on port 80.

It serves:

/ → Main web dashboard (HTML + JavaScript)

/temp → JSON temperature data
Example response:

{ "tempC": 23.437, "tempF": 74.186 }


The dashboard uses JavaScript to fetch /temp every 2000 ms.
No refreshing required — values update automatically.

🧰 Required Components
Component	Description
ESP32 DevKit V1	Wi-Fi microcontroller
DS18B20 Temperature Sensor	Waterproof or TO-92 package
4.7kΩ – 5.1kΩ resistor	Pull-up resistor for data line
128×64 I²C OLED (SSD1306)	Uses I²C address 0x3C
Breadboard + jumper wires	For wiring
🔌 Wiring Diagram
DS18B20 → ESP32
DS18B20 Pin	ESP32 Pin
VCC	3.3V
GND	GND
DATA	GPIO 4

📌 Important: You must add a pull-up resistor (4.7kΩ or 5.1kΩ) between DATA and 3.3V

1-Wire Pull-Up
3.3V -----[5.1kΩ]----- DATA (GPIO 4)

OLED (SSD1306 I²C) → ESP32
OLED Pin	ESP32 Pin
VCC	3.3V
GND	GND
SDA	GPIO 21
SCL	GPIO 22

Most ESP32 boards use SDA = 21 and SCL = 22 by default.

📁 Project Structure
ESP32-Temp-Monitor/
│
├── src/
│   └── main.cpp          # Full sketch (ESP32 + DS18B20 + OLED + Web Server)
│
└── README.md             # You are reading this file

▶ How to Use
1. Install Libraries in Arduino IDE

Install these from Library Manager:

OneWire

DallasTemperature

Adafruit GFX

Adafruit SSD1306

2. Flash the Code

Select ESP32 DevKit V1 as your board

Enter your Wi-Fi SSID + password in the code

Upload the sketch

3. View Temperature

Check the OLED for temperature + IP address

Open your browser and go to:

http://<ESP32-IP>/


You will see a dynamic page that updates every 2 seconds.

📈 Example Web UI Output
ESP32 DS18B20 Temperature
23.4 °C
74.2 °F
Last update: 12:45:08


The page automatically refreshes the values using asynchronous JSON calls.

🛠 Notes & Tips

If the DS18B20 reads -127°C, the sensor is disconnected or wiring is incorrect.

The project works on both 2.4 GHz and dual-band routers (ESP32 Wi-Fi is 2.4 only).

The OLED must be I²C, not SPI (address 0x3C or occasionally 0x3D).

📜 License

This project is open-source.
Use, modify, repurpose, or build on it however you like.
