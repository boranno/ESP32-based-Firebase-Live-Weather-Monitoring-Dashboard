# ESP32 based Firebase Live Weather Monitoring Dashboard

![ESP32 Live Weather logo](./logo.jpg)


Live Weather Monitoring — ESP32 + Firebase

A compact ESP32-based system that streams temperature, humidity sensor data to Firebase Realtime Database and provides a lightweight, real-time dashboard (`firebase_weather_dashboard.html`).

---

## 🔧 Repository

**Name:** ESP32-based-Firebase-Live-Weather-Monitoring-Dashboard

**Clone:**

```bash
git clone https://github.com/boranno/ESP32-based-Firebase-Live-Weather-Monitoring-Dashboard.git
```

**Deployed (GitHub Pages):**

https://boranno.github.io/ESP32-based-Firebase-Live-Weather-Monitoring-Dashboard/

---

## ✨ Overview

This project contains an Arduino sketch for an ESP32 that publishes sensor readings to Firebase Realtime Database and a small HTML dashboard for live monitoring. It's intended as a simple, extensible starting point for DIY IoT weather monitoring.

## ✅ Key features

- Real-time temperature, humidity publishing
- Lightweight dashboard for live visualization (works with GitHub Pages)
- Simple configuration for DHT sensors and ESP32 S3 boards
- Example Firebase security rules and data format

## 🛠️ Requirements

Hardware
- ESP32 development board
- DHT22/DHT11
- Micro-USB cable

Software/Libraries
- Arduino IDE or PlatformIO
- `WiFi.h`, `Firebase-ESP-Client` (or `FirebaseArduino`)
- `ArduinoJson` and appropriate sensor libraries (`DHT`, `Adafruit_BME280`)

---

## 🚀 Quick start

1. Clone the repository (see above).
2. Open `esp32_firebase_weather.ino` in the Arduino IDE or import into PlatformIO.
3. Install the required libraries.
4. Configure Wi‑Fi and Firebase credentials at the top of the sketch:

```cpp
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";
const char* FIREBASE_HOST = "your-project.firebaseio.com"; // or databaseURL
const char* FIREBASE_AUTH = "YOUR_FIREBASE_SECRET_OR_API_KEY";
```

5. Set your sensor type and pins, then upload to the ESP32.
6. Confirm successful Firebase writes via the serial monitor.
7. View the dashboard locally (`index.html`) or visit the live demo URL after your device starts publishing.

> Tip: If the dashboard doesn't load directly from a file due to browser security, serve it with a static server (e.g., `npx http-server`) or use GitHub Pages.

---

## 🔐 Example Firebase rules (for testing only)

```json
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
```

Remember to secure your database for production use.

## 📁 Example data structure

```json
/weather/{deviceId}:
  temperature: 21.5
  humidity: 48
  timestamp: 1620000000
```

---

## 🤝 Contributing

Contributions and feedback are welcome — open an issue or submit a pull request with a short description and test steps.

