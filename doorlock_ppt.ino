/****************************************************
 * ESP32 Door Sensor
 * - Uses GPIO4 with internal pull-up for a reed switch
 * - Sends "open"/"closed" JSON to a Flask server
 * - Includes optimized debounce and state stability checks
 ****************************************************/
#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "iPhone pro max";
const char* WIFI_PASSWORD = "11223344";

String serverURL = "http://172.20.10.2:5000/api/door_status";

const int reedSwitchPin = 4; // Using GPIO4
bool lastState = HIGH;       // Default state is HIGH (door closed with pull-up)
unsigned long lastStateChangeTime = 0; // Timestamp for last state change
const unsigned long debounceDelay = 50; // Reduced debounce delay to 50ms

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(reedSwitchPin, INPUT_PULLUP);
  lastState = digitalRead(reedSwitchPin); // Read initial state

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);
  connectToWiFi();
}

void loop() {
  bool currentState = digitalRead(reedSwitchPin); // Read reed switch state
  unsigned long currentTime = millis();

  if (currentState != lastState && (currentTime - lastStateChangeTime) > debounceDelay) {
    lastStateChangeTime = currentTime; // Update timestamp
    lastState = currentState;         // Update last state

    String doorStatus = (currentState == LOW) ? "closed" : "open";
    Serial.print("Door changed to: ");
    Serial.println(doorStatus);

    sendDoorStatus(doorStatus);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
    connectToWiFi();
  }
}

void sendDoorStatus(String doorStatus) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    Serial.println("Preparing HTTP POST to server...");
    http.begin(serverURL);
    http.setTimeout(5000); // Set timeout to 5 seconds
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"door_status\": \"" + doorStatus + "\"}";
    Serial.print("HTTP POST Payload: ");
    Serial.println(jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("Server response code: ");
      Serial.println(httpResponseCode);

      String response = http.getString();
      Serial.print("Server response body: ");
      Serial.println(response);
    } else {
      Serial.print("HTTP POST error: ");
      Serial.println(httpResponseCode);
      Serial.println("-> Check server IP, port, or network.");
    }
    http.end();
  } else {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    connectToWiFi();
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retryCount = 0;

  while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
    delay(500);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi. Restarting...");
    ESP.restart(); // Restart the ESP32 to attempt a fresh connection
  }
}