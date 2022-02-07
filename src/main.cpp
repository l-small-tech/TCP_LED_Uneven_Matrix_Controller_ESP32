#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "RiceWallCOntroller.cpp"
#include <esp_task_wdt.h>
#include "secrets.h"

#define TCP_BUFF_SIZE 2048

RiceWallController* strip;

// Tasks
TaskHandle_t ledThread;
TaskHandle_t wifiThread;

// LED Controller thread
void ledThreadFunc( void* pvParameters ) {
  
  while (true) { strip->tick(); } }

// Wireless radio thread. Test with "netcat 10.1.1.82 8888"
void wifiThreadFunc( void* pvParameters ) {
  	
  WiFiServer wifiServer(8888);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Create and clear message buffer
  char msgBuff[TCP_BUFF_SIZE];

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
 
  Serial.println("\nConnected to the WiFi network");
  Serial.println(WiFi.localIP());
 
  wifiServer.begin();

  while (true) {
    // CLear message buffer
    for (int i = 0; i < TCP_BUFF_SIZE; i++) { msgBuff[i] = '\0'; }
    WiFiClient client = wifiServer.available();
  
    if (client) {

      if(client.connected()) { Serial.println("Client Connected"); }
      
      int i = 0;
      while (client.connected()) {
        while (client.available()>0) {
          msgBuff[i++] = client.read(); 
        }
        delay(1);
      }
  
      Serial.println("Client disconnected, MSG:");
      Serial.print(msgBuff);
      Serial.println();

      StaticJsonDocument<TCP_BUFF_SIZE> doc;
      DeserializationError error = deserializeJson(doc, msgBuff);

      if (error) {

        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      if (doc.containsKey("hue")) {
        strip->setHue(doc["hue"]);
        Serial.print("Hue set to ");
        Serial.println((int)doc["hue"]);
      }

      client.stop();
    }
  }
}

void setup() {
  Serial.begin(115200);
  strip = new RiceWallController();
  
  xTaskCreatePinnedToCore(
      ledThreadFunc,  /* Function to implement the task */
      "LED Task",        /* Name of the task */
      10000,          /* Stack size in words */
      NULL,           /* Task input parameter */
      1,              /* Priority of the task */
      &ledThread,     /* Task handle. */
      0);             /* Core where the task should run */

  xTaskCreatePinnedToCore(
      wifiThreadFunc,  /* Function to implement the task */
      "WiFi Task",        /* Name of the task */
      10000,          /* Stack size in words */
      NULL,           /* Task input parameter */
      1,              /* Priority of the task */
      &wifiThread,     /* Task handle. */
      1);             /* Core where the task should run */
}

void loop() {}