#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "RiceWallCOntroller.cpp"
#include <esp_task_wdt.h>
#include "secrets.h"


RiceWallController* strip;

// Tasks
TaskHandle_t ledThread;
TaskHandle_t wifiThread;

void ledThreadFunc( void* pvParameters ) {
  
  while (true) { strip->tick(); } }

void wifiThreadFunc( void* pvParameters ) {
  	
  WiFiServer wifiServer(8888);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
 
  Serial.println("\nConnected to the WiFi network");
  Serial.println(WiFi.localIP());
 
  wifiServer.begin();

  while (true) {
    WiFiClient client = wifiServer.available();
  
    if (client) {
  
      while (client.connected()) {
        
        Serial.println("RX: ");
        while (client.available()>0) {
          char c = client.read();
          Serial.print(c);
        }
  
        delay(10);
      }
      Serial.println("End of RX");
  
      client.stop();
      Serial.println("Client disconnected");
  
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