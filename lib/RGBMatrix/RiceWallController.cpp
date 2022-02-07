#ifndef RGBSTRIPGRID_CPP
#define RGBSTRIPGRID_CPP
#endif

#include <FastLED.h>


/*  
    Box Matrix Mapping
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

    DEBUG: Demo of how to get h value
    Serial.println(boxBufferCHSV[i].h);
*/
// Box Matrix (BM) properties
#define BM_WIDTH            3
#define BM_HEIGHT           7
#define BM_COUNT            (BM_WIDTH * BM_HEIGHT)
#define MAX_PIXELS_PER_BOX  17

// FastLED Settings
#define FLED_DATA_PIN       13
#define FLED_LED_COUNT      200
#define FLED_LED_TYPE       WS2812B
#define FLED_COLOR_ORDER    GRB

class RiceWallController {

private:
    // -- Constants --
    // Box Spiral Sequence (Counter Clockwise)
    const int spiralSequence[BM_COUNT] = {0, 3, 6, 9, 12, 15, 18, 19, 20, 17, 14, 11, 8, 5, 2, 1, 4, 7, 10, 13, 16};
    // Maps individual pixels to their respective boxes
    const int pixelToBoxMap[BM_COUNT][MAX_PIXELS_PER_BOX] = {
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
    enum class animationModes : uint8_t { 
        experimental,
        fire,
        rain
    };

    enum class Direction : uint8_t {
        Up,
        Down,
        Left,
        Right
    };

    // -- Variables --
    // Buufer holding CHSV values for each box
    CHSV boxBufferCHSV[BM_COUNT];

    // FastLED properties
    CRGB* leds = new CRGB[FLED_LED_COUNT];
    int brightness = 255;

    // FPS Calculations
    unsigned long thisTime;
    unsigned long lastTime = 0;
    int fps = 7;

    // Render Settings
    bool renderBoxes = true;
    bool renderSegments = false;

    // State Variables
    bool strip_on;
    bool animation_on;
    animationModes animationMode;
    // Color State
    uint8_t global_hue, global_sat, global_val = 0;

    // For debugging
    bool skip_frame = true;
    
    // -- Methods --

    // Animations
    void animation_experimental() {
        shiftBoxes(Direction::Down);

        // Set whole top row to low color
        for (int i = 0; i < BM_WIDTH; i++) { boxBufferCHSV[i] = CHSV(global_hue, global_sat, 90); }

        // Loweer the random int to increase the chance of a box being lit
        int dropIndex = random(15);
        if (dropIndex < BM_WIDTH) { boxBufferCHSV[dropIndex] = CHSV(global_hue + random(40), global_sat, global_val); }
    }

    void animation_fire() {
        // TODO: !
    }

    void animation_rain() {

        shiftBoxes(Direction::Down);

        // Set whole top row to low color
        for (int i = 0; i < BM_WIDTH; i++) { boxBufferCHSV[i] = CHSV(global_hue, global_sat, 90); }

        // Loweer the random int to increase the chance of a box being lit
        int dropIndex = random(15);
        if (dropIndex < BM_WIDTH) { boxBufferCHSV[dropIndex] = CHSV(global_hue + random(40), global_sat, global_val); }
    }

public:
    RiceWallController() {
        // Setup FastLED
        FastLED.addLeds<WS2812B, FLED_DATA_PIN, GRB>(leds, FLED_LED_COUNT).setCorrection( Typical8mmPixel  );
        FastLED.setBrightness(brightness);

        // Default hue
        global_hue = 0;
        global_sat = 255;
        global_val = 255;

        strip_on = true;
        animation_on = true;

        // Default animation
        animationMode = animationModes::experimental;
        setStripColor(CHSV(0, 0, 0));

        // Init Box Buffer
        for (int i = 0; i < BM_COUNT; i++) {
            boxBufferCHSV[i] = CHSV(0, 0, 0);
        }
    }

    // Strip Methods
    void setStripColor(CHSV color) {
        for (int i = 0; i < FLED_LED_COUNT; i++) {
            leds[i] = color;
        }
    }
    
    // Box Methods
    void f_renderBoxes() {
        // For each box
        for (int boxIndex = 0; boxIndex < BM_COUNT; boxIndex++) {
            for (int boxPixelIndex = 0; boxPixelIndex < MAX_PIXELS_PER_BOX; boxPixelIndex++) {

                if (pixelToBoxMap[boxIndex][boxPixelIndex] == -1) { break; }

                leds[pixelToBoxMap[boxIndex][boxPixelIndex]] = boxBufferCHSV[boxIndex];
            }
        }
    }

    void fillBox(int boxIndex, CHSV color) { boxBufferCHSV[boxIndex] = color;  }

    void shiftBoxes(Direction dir) {

        if (dir == Direction::Down) {
            // Save the last row
            CHSV temp[BM_WIDTH];
            for (int i = BM_COUNT - BM_WIDTH; i < BM_COUNT; i++) {
                temp[ i - (BM_HEIGHT - 1) * BM_WIDTH] = boxBufferCHSV[i];
            }

            // Shift the rows down
            for (int i = BM_COUNT - 1; i >= 0; i--) {
                boxBufferCHSV[i] = boxBufferCHSV[i - BM_WIDTH];
            }

            // Fill the first row with the saved last row
            for (int i = 0; i < BM_WIDTH; i++) {
                boxBufferCHSV[i] = temp[i];
            }
        }
    }

    // Segment Methods
    void f_renderSegments() {
        // TODO: 
    }

    void tick() {
        thisTime = millis();
        if (thisTime - lastTime > 1000 / fps && strip_on) {

            // You were ready to quick?
            if (!skip_frame) { Serial.println("WARNING: Frame rate may be too high!"); }
            
            // Process Active Animation
            if (animation_on) {
                switch (animationMode) {
                    case animationModes::rain: { animation_rain(); break; }
                    case animationModes::experimental: { animation_experimental(); break; }
                    case animationModes::fire: { animation_fire(); break; }
                }
            }

            if (renderBoxes) { f_renderBoxes(); }
            if (renderSegments) { f_renderSegments(); }

            lastTime = thisTime;
            FastLED.show();
            skip_frame = false;

        } else { skip_frame = true; }
    }
};