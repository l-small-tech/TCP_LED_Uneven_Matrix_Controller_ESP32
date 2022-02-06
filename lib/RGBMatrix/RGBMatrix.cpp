#ifndef RGBMATRIX_CPP
#define RGBMATRIX_CPP
#endif

/*
    White High density Strip: BTF-LIGHTING RGBW RGBNW Natural White SK6812 (Similar WS2812B)
    Black Lower Density Strip: SK6812 RGBW RGB Warm RGBNW Led Strip

    On the Wemos D1 Mini, Use D7 as the Data Pin. D7 == GPIO 13 == MOSI
    Use 430-470 Ohm with the data pin
*/

#include <FastLED.h>
#include <Arduino.h>

// ---- Parameters ----
#define DATA_PIN    13
#define NUM_LEDS    200
#define BRIGHTNESS  150
#define FPS         5
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB



class RGBMatrix {
    
private:
    CRGB* leds = new CRGB[NUM_LEDS];
    CRGB* leds_bkp = new CRGB[NUM_LEDS];

    int fps = FPS;
    int brightness = BRIGHTNESS;

    enum class matrixModes : uint8_t { m_static, flash };
    matrixModes mode = matrixModes::m_static;
    
    int r, g, b = 0;

    unsigned long thisTime;
    unsigned long lastTime = 0;

public:

    RGBMatrix() {
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
        FastLED.setBrightness(brightness);
    }
    
    void off() {
        // Save the state of the strip
        for (int i = 0; i < NUM_LEDS; i++) {
            leds_bkp[i] = leds[i];
        }

        // Turn all the lights off
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB::Black;
        }
    }

    void on() {
        // Set all LEDs to the state they had before beign turned off        
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = leds_bkp[i];
        }
    }

    void push(int r, int g, int b) {
        // Set first led to color
        leds_bkp[0] = CRGB(r, g, b);

        // Shift all the leds one place down the strip
        for (int i = 0; i < NUM_LEDS - 1; i++) {
            leds_bkp[i + 1] = leds[i];
        }

        // Copy the new shifted data to the strip
        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = leds_bkp[i];
        }
    }
    
    void setBrightness(int b) {

        this->brightness = b;
        FastLED.setBrightness(b);
    }

    void setFPS(int fps) {
        this->fps = fps;
    }

    void setMode(uint8_t m) { this->mode = (matrixModes)m; }

    void setStripColor(int r, int g, int b) {

        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB(r, g, b);
        }
    }

    void tick() {
        thisTime = millis();
        if (thisTime - lastTime > 1000 / fps) {
            
            // Run mode code
            switch (mode) {
                // Satic Mode: Lights are on and they don't move
                case matrixModes::m_static: {
                    Serial.println("DEBUG: Static mode Tick");
                    break;
                }

                // Flash that silly goose
                case matrixModes::flash: {
                    Serial.println("Flash Tick");
                    break;
                }
            }

            lastTime = thisTime;
            FastLED.show();
        }
    }
};

    // // Function Pointer Array Demo
    // typedef void (*farray)();
    // farray modes[5] = {NULL};

    // // modes[0] = &test;
    // modes[0] = []() {
    //     cout << "Test anon" << endl;
    // };

    // modes[1] = []() {
    //     cout << "Anon #2" << endl;
    // };

    // modes[1]();