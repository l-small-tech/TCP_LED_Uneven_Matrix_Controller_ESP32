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

#include "PixelMapping.h"

// ---- Parameters ----
#define DATA_PIN    13
#define NUM_LEDS    200
#define BRIGHTNESS  150
#define FPS         2         // Try no to go over 165
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

class RGBMatrix {
    
private:
    CRGB* leds = new CRGB[NUM_LEDS];
    CRGB* leds_bkp = new CRGB[NUM_LEDS];

    int fps = FPS;
    int brightness = BRIGHTNESS;

    enum class matrixModes : uint8_t { experimental,
                                       rain,
                                       rainbow_row_down,
                                       rainbow_boxes,
                                       spiral,
                                       solid_color
                                     };
    // Default mode
    matrixModes mode = matrixModes::rain;

    // For debugging frame rate
    bool skip_frame = true;
    
    int r0, g0, b0 = 0;
    int h0, s0, v0 = 254;
    bool is_on = true;
    int boxIndex = 0;
    int index = 0;

    unsigned long thisTime;
    unsigned long lastTime = 0;

public:

    RGBMatrix() {
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( Typical8mmPixel  );
        FastLED.setBrightness(brightness);
        setStripColor(0, 0, 0);
    }

    void mode_rainbow_row_down() {
        setBoxColor(index, CHSV(h0, s0, v0));
        setBoxColor(index + 1, CHSV(h0, s0, v0));
        setBoxColor(index + 2, CHSV(h0, s0, v0));

        index += 3;
        if (index >= NUM_BOXES - 2) {
            index = 0;
            h0 = (h0 + 15) % 255;
        }
    }

    void mode_experimental() {

    }
    
    void mode_rain() {
        // Shift everythign down
        shiftBoxesDown();

        // Set whole top row low blue
        for (int i = 0; i < BOX_WIDTH; i++) {
            boxCHSV[i][0] = 150;
            boxCHSV[i][1] = 200;
            boxCHSV[i][2] = 90;
        }


        // Randomly set one of the top rows of boxes
        boxCHSV[random(3)][2] = 254;

        // Render it!
        renderBoxHues();
    }

    void mode_rainbow_boxes() {
        setBoxColor(boxIndex, CHSV(h0, 254, 254));

        boxIndex = (boxIndex + 1) % NUM_BOXES;
        h0 = (h0 + 40) % 255;
    }

    void mode_spiral() {

        // Lead dog
        boxCHSV[ spiralOrder[ (boxIndex + 1) % NUM_BOXES ] ][0] = h0;
        boxCHSV[ spiralOrder[ (boxIndex + 1) % NUM_BOXES ] ][1] = 254;
        boxCHSV[ spiralOrder[ (boxIndex + 1) % NUM_BOXES ] ][2] = 254;

        // Lagger
        boxCHSV[ spiralOrder[boxIndex] ][0] = h0;
        boxCHSV[ spiralOrder[boxIndex] ][1] = 0;
        boxCHSV[ spiralOrder[boxIndex] ][2] = 0;

        // Render and increment
        renderBoxHues();
        boxIndex = (boxIndex + 1) % NUM_BOXES;
        h0 = (h0 + 10) % 255;
    }

    void mode_solid_color() {
        setStripColor(r0, g0, b0);
    }
    
    void off() { is_on = false; }

    void on() { is_on = true; }

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
    
    void renderBoxHues() {
        for (int i = 0; i < NUM_BOXES; i++) {
            setBoxColor(i, CHSV(boxCHSV[i][0], boxCHSV[i][1], boxCHSV[i][2]));
        }
    }

    void setBoxColor(int boxIndex, CHSV color) {
            
        for (int i = 0; i < MAX_PIXELS_PER_BOX; i++) {

            if (pixelMap_boxes[boxIndex][i] == -1) { break; }

            leds[pixelMap_boxes[boxIndex][i]] = color;
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

    void shiftBoxesDown() {
        for (int i = 0; i <= NUM_BOXES - BOX_WIDTH; i++) {
            boxCHSV[i + BOX_WIDTH][0] = boxCHSV[i][0];
            boxCHSV[i + BOX_WIDTH][1] = boxCHSV[i][1];
            boxCHSV[i + BOX_WIDTH][2] = boxCHSV[i][2];
        }
    }

    void tick() {
        thisTime = millis();
        if (thisTime - lastTime > 1000 / fps && is_on) {

            // You were ready to quick?
            if (!skip_frame) {
                Serial.println("WARNING: Frame rate may be too high!");
            }
            
            // Run mode code
            switch (mode) {
                case matrixModes::experimental: { mode_experimental(); break; }
                case matrixModes::rain: { mode_rain(); break; }
                case matrixModes::rainbow_row_down: { mode_rainbow_row_down(); break; }
                case matrixModes::rainbow_boxes: { mode_rainbow_boxes(); break; }
                case matrixModes::spiral: { mode_spiral(); break; }
                case matrixModes::solid_color: { mode_solid_color(); break; }
            }

            lastTime = thisTime;
            FastLED.show();
            skip_frame = false;
        } else {
            skip_frame = true;
        }
    }
};