#include <Arduino.h>

#include "RiceWallCOntroller.cpp"
#include <esp_task_wdt.h>

RiceWallController* strip;

// Tasks
TaskHandle_t ledThread;
TaskHandle_t wifiThread;

void ledThreadFunc( void* pvParameters ) {
  
  while (true) { strip->tick(); } }

void wifiThreadFunc( void* pvParameters ) { 
  // esp_task_wdt_init(30, false);      // Maybe needed if this task takes too long

  // for (int i = 0; i < 21; i++) {
  //   int rand = random(3);
  //   if (rand == 0) {
  //     strip->fillBox(i, CHSV(190 + random(30), 255, 255));
  //   }
  // }

  while (true) {
    // delay(100);
    // strip->shiftBoxesDown();

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