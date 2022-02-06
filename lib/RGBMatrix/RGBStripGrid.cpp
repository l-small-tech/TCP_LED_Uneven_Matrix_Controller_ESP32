#ifndef RGBSTRIPGRID_CPP
#define RGBSTRIPGRID_CPP
#endif

#include <FastLED.h>

/*
    TODO: Convert RGBMatrix.cpp to this class
    
    Rice Paper LED Grid Pixel Mapping:
        ---- 3 Wide ----
        +----+----+----+
        | 0  | 1  | 2  |
        +----+----+----+
        | 3  | 4  | 5  |
        +----+----+----+ |
        | 6  | 7  | 8  | |
        +----+----+----+ 7 High
        | 9  | 10 | 11 | |
        +----+----+----+ |
        | 12 | 13 | 14 |
        +----+----+----+
        | 15 | 16 | 17 |
        +----+----+----+
        | 18 | 19 | 20 |
        +----+----+----+
*/
// Box properties
#define BOX_WIDTH   3
#define BOX_HEIGHT  7
#define NUM_BOXES  (BOX_WIDTH * BOX_HEIGHT)
#define MAX_PIXELS_PER_BOX 17

// FastLED Settings
#define DATA_PIN    13
#define NUM_LEDS    200
#define BRIGHTNESS  150
#define FPS         5         // Try no to go over 165
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

class RGBStripGrid {

private:
    // -- Constants --
    // Box Spiral Sequence (Counter Clockwise)
    const int spiralSequence[NUM_BOXES] = {0, 3, 6, 9, 12, 15, 18, 19, 20, 17, 14, 11, 8, 5, 2, 1, 4, 7, 10, 13, 16};
    // Maps individual pixels to their respective boxes
    const int pixelToBoxMap[NUM_BOXES][MAX_PIXELS_PER_BOX] = {
        {146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, -1},                // 0
        {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 138, 139, 140, 141, 142, 143, 144},// 1
        {42, 43, 44, 45, 49, 47, 48, 49, -1},                                       // 2

        {157, 158, 159, 160, 161, 162, -1},                                         // 3
        {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 136, 137, -1},                     // 4
        {35, 36, 37, 38, 39, 40, 41, -1},                                           // 5

        {163, 164, 165, 166, 167, 168, 169, 170, -1},                               // 6
        {70, 71, 72, 73, 74, 75, 75, 77, 78, 79, 80, 133, 134, -1},                 // 7
        {29, 30, 31, 32, 33, 34, -1},                                               // 8

        {171, 172, 173, 174, 175, 176, 177, 178, 179, -1},                          // 9
        {81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 130, 131, 132, -1},                // 10
        {18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, -1},                           // 11

        {180, 181, 182 , 183, 184, 185, 186, 187, -1},                              // 12
        {91, 92, 93, 94, 95, 96, 97, 98, 00, 100, 101, 128, 129, -1},               // 13
        {13, 14, 15, 16, 17, -1},                                                   // 14

        {188, 189, 190, 191, 192, 193, -1},                                         // 15
        {102,  103, 104, 105, 106, 107, 108, 109, 110, 111, 125, 126, 127, -1},     // 16
        {6, 7, 8, 9, 10, 11, 12, -1},                                               // 17

        {194, 195, 196, 197, 198, 199, -1},                                         // 18
        {112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, -1},      // 19 
        {0, 1, 2, 3, 4, 5, -1}                                                      // 20
    };

    // -- Enumerations --
    enum class matrixModes : uint8_t { experimental,
                                    rain,
                                    rainbow_row_down,
                                    rainbow_boxes,
                                    spiral,
                                    solid_color
                                    };

    // -- Variables --
    // Buufer holding CHSV values for each box
    int boxBufferCHSV[NUM_BOXES][3];

    // FastLED properties
    CRGB* leds = new CRGB[NUM_LEDS];
    CRGB* leds_bkp = new CRGB[NUM_LEDS];
    int brightness = BRIGHTNESS;

    // FPS Calculations
    unsigned long thisTime;
    unsigned long lastTime = 0;
    int fps = FPS;
    bool is_on = true;

    // For debugging
    bool skip_frame = true;

    // Matrix properties
    matrixModes mode;


    // -- Methods --
    // Actions
    // Files a boxe with one color
    void action_fillBox(int index, CHSV color) {
        for (int i = 0; i < MAX_PIXELS_PER_BOX; i++) {

            if (pixelToBoxMap[index][i] == -1) { break; }

            leds[pixelToBoxMap[index][i]] = color;
        }

        // Update the memory
    }

    void action_render() {
        for (int i = 0; i < NUM_BOXES; i++) {
            for (int j = 0; j < MAX_PIXELS_PER_BOX; j++) {

                if (pixelToBoxMap[i][j] == -1) { break; }

                leds[pixelToBoxMap[i][j]] = CHSV(boxBufferCHSV[i][0], boxBufferCHSV[i][1], boxBufferCHSV[i][2]);
            }
        }
    }

    void action_shiftBoxesDown() {
        // TODO: Save the last row and use it to fill the first row
        // TODO: Enumerate directions and add them to this method
        for (int i = NUM_BOXES; i >= 0 + BOX_WIDTH; i--) {
            boxBufferCHSV[i][0] = boxBufferCHSV[i - BOX_WIDTH][0];
            boxBufferCHSV[i][1] = boxBufferCHSV[i - BOX_WIDTH][1];
            boxBufferCHSV[i][2] = boxBufferCHSV[i - BOX_WIDTH][2];
        }
    }

    // Modes
    void mode_experimental() {

    }

    void mode_rain() {

    }

public:
    RGBStripGrid() {
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( Typical8mmPixel  );
        FastLED.setBrightness(brightness);

        mode = matrixModes::rain;
        // TODO: Set eveything to 0
        // setStripColor(0, 0, 0);
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
            }

            lastTime = thisTime;
            FastLED.show();
            skip_frame = false;
        } else {
            skip_frame = true;
        }
    }
};