#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "MY_WIFI_NAME";
const char* password = "MY_WIFI_PASSWORD";

// Firebase settings
const char* fcmServerKey = "MY_FIREBASE_SERVER_KEY";
const char* fcmEndpoint = "https://fcm.googleapis.com/fcm/send";
const char* deviceToken = "MY_DEVICE_REGISTRATION_TOKEN";

// Button settings
const int buttonPin = 4;
int lastButtonState = HIGH;
int currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);

  // Initialize button pin
  pinMode(buttonPin, INPUT_PULLUP);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read button state
  int reading = digitalRead(buttonPin);

  // Check if button state changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Button pressed 
      if (currentButtonState == LOW) {
        Serial.println("Button pressed! Sending notification...");
        sendPushNotification();
      }
    }
  }

  lastButtonState = reading;
  delay(10);
}

void sendPushNotification() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(fcmEndpoint);

    // Set headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "key=" + String(fcmServerKey));

    // Create JSON payload
    DynamicJsonDocument doc(1024);
    doc["to"] = deviceToken;

    // Notification payload
    JsonObject notification = doc.createNestedObject("notification");
    notification["title"] = "ESP32 Alert";
    notification["body"] = "Button was pressed!";
    notification["sound"] = "default";

    // Data payload (optional)
    JsonObject data = doc.createNestedObject("data");
    data["timestamp"] = String(millis());
    data["sensor"] = "button_pin_4";

    // Convert JSON to string
    String payload;
    serializeJson(doc, payload);

    Serial.println("Sending payload: " + payload);

    // Send POST request
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Response: " + response);

      if (httpResponseCode == 200) {
        Serial.println("Push notification sent successfully!");
      } else {
        Serial.println("Failed to send push notification");
      }
    } else {
      Serial.println("Error in HTTP request: " + String(httpResponseCode));
    }

    http.end();
  } 
  
  else {
    Serial.println("WiFi not connected");
  }
}// send push notification

// Function to get current timestamp (optional)
String getTimestamp() {
  return String(millis() / 1000) + "s";
}
