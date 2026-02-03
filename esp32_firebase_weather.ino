/*
 * ESP32 Weather Station with Firebase
 * Reads temperature and humidity from DHT sensor
 * Sends data to Firebase Realtime Database
 */

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <time.h>

// Provide the token generation process info
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID "oooo"
#define WIFI_PASSWORD "ashish0000"

// Firebase project API Key
#define API_KEY "AIzaSyD3yHD71PNoV7HiuM9GOOnYQjx-QSxRM9w"

// Firebase Realtime Database URL
#define DATABASE_URL "https://iotboranno-default-rtdb.firebaseio.com"

// DHT Sensor settings
#define DHTPIN 4          // GPIO pin connected to DHT sensor
#define DHTTYPE DHT22     // DHT 22 (AM2302) or DHT11
DHT dht(DHTPIN, DHTTYPE);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// Time settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;      // Adjust for your timezone (0 for GMT)
const int daylightOffset_sec = 0;  // Adjust if you have daylight saving

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  
  // Initialize time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // Sign up (anonymous)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup error: %s\n", config.signer.signupError.message.c_str());
  }
  
  // Assign the callback function for token generation task
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.println("Setup complete!");
}

void loop() {
  // Send data every 30 seconds
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 60000*30 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    // Read sensor data
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    // Check if readings are valid
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    // Get current time
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    
    // Format date and time
    char dateStr[11];
    char timeStr[9];
    char timestampStr[20];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    strftime(timestampStr, sizeof(timestampStr), "%Y%m%d%H%M%S", &timeinfo);
    
    Serial.println("--------------------");
    Serial.printf("Date: %s\n", dateStr);
    Serial.printf("Time: %s\n", timeStr);
    Serial.printf("Temperature: %.2f°C\n", temperature);
    Serial.printf("Humidity: %.2f%%\n", humidity);
    
    // Create JSON object for current reading
    FirebaseJson json;
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("date", String(dateStr));
    json.set("time", String(timeStr));
    json.set("timestamp", (int)time(NULL));
    
    // Store latest reading
    if (Firebase.RTDB.setJSON(&fbdo, "/weather/current", &json)) {
      Serial.println("Current data sent successfully");
    } else {
      Serial.println("Failed to send current data");
      Serial.println("Reason: " + fbdo.errorReason());
    }
    
    // Store historical data (for charts and daily stats)
    String historyPath = "/weather/history/" + String(dateStr) + "/" + String(timestampStr);
    if (Firebase.RTDB.setJSON(&fbdo, historyPath, &json)) {
      Serial.println("Historical data saved");
    } else {
      Serial.println("Failed to save historical data");
    }
    
    // Update daily statistics
    updateDailyStats(dateStr, temperature, humidity);
    
    Serial.println("--------------------");
  }
}

void updateDailyStats(String date, float temp, float humid) {
  String statsPath = "/weather/daily_stats/" + date;
  
  // Read current stats
  if (Firebase.RTDB.getJSON(&fbdo, statsPath)) {
    FirebaseJson &json = fbdo.jsonObject();
    FirebaseJsonData result;
    
    float minTemp = temp;
    float maxTemp = temp;
    float minHumid = humid;
    float maxHumid = humid;
    int count = 1;
    
    // Get existing values if they exist
    if (json.get(result, "min_temp")) {
      float existing = result.floatValue;
      minTemp = min(existing, temp);
    }
    if (json.get(result, "max_temp")) {
      float existing = result.floatValue;
      maxTemp = max(existing, temp);
    }
    if (json.get(result, "min_humidity")) {
      float existing = result.floatValue;
      minHumid = min(existing, humid);
    }
    if (json.get(result, "max_humidity")) {
      float existing = result.floatValue;
      maxHumid = max(existing, humid);
    }
    if (json.get(result, "count")) {
      count = result.intValue + 1;
    }
    
    // Create updated stats
    FirebaseJson statsJson;
    statsJson.set("date", date);
    statsJson.set("min_temp", minTemp);
    statsJson.set("max_temp", maxTemp);
    statsJson.set("min_humidity", minHumid);
    statsJson.set("max_humidity", maxHumid);
    statsJson.set("count", count);
    statsJson.set("last_update", (int)time(NULL));
    
    // Save updated stats
    if (Firebase.RTDB.setJSON(&fbdo, statsPath, &statsJson)) {
      Serial.println("Daily stats updated");
    } else {
      Serial.println("Failed to update daily stats");
    }
  } else {
    // First reading of the day - create new stats
    FirebaseJson statsJson;
    statsJson.set("date", date);
    statsJson.set("min_temp", temp);
    statsJson.set("max_temp", temp);
    statsJson.set("min_humidity", humid);
    statsJson.set("max_humidity", humid);
    statsJson.set("count", 1);
    statsJson.set("last_update", (int)time(NULL));
    
    Firebase.RTDB.setJSON(&fbdo, statsPath, &statsJson);
  }
}
