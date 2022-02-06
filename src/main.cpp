#include <Arduino.h>

#include "RGBMatrix.cpp"

RGBMatrix* strip;

// Tasks
TaskHandle_t ledThread;
TaskHandle_t wifiThread;

void ledThreadFunc( void* pvParameters ) {  while (true) { strip->tick(); } }

void wifiThreadFunc( void* pvParameters ) { 
  while (true) {
    strip->setMode(0);
    delay(2000);
    strip->setMode(1);
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  strip = new RGBMatrix();
  
  xTaskCreatePinnedToCore(
      ledThreadFunc,  /* Function to implement the task */
      "Task1",        /* Name of the task */
      10000,          /* Stack size in words */
      NULL,           /* Task input parameter */
      1,              /* Priority of the task */
      &ledThread,     /* Task handle. */
      0);             /* Core where the task should run */

  xTaskCreatePinnedToCore(
      wifiThreadFunc,  /* Function to implement the task */
      "Task1",        /* Name of the task */
      10000,          /* Stack size in words */
      NULL,           /* Task input parameter */
      1,              /* Priority of the task */
      &wifiThread,     /* Task handle. */
      1);             /* Core where the task should run */
}

void loop() {}