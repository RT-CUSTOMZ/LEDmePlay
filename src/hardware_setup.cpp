#include <hardware_setup.hpp>

// R1, G1, B1, R2, G2, B2 pins
uint8_t rgbPins[] = {D_R1, D_G1, D_B1, D_R2, D_G2, D_B2};
RGBmatrixPanel matrix(D_A, D_B, D_C, D_D, D_CLK, D_LAT, D_OE, false, 32, rgbPins);

// Audio out
int const audioPacer = 25;

// Joystick 1
const int buttonU1 = 35;
const int buttonD1 = 34;
const int buttonL1 = 39;
const int buttonR1 = 33;
const int buttonFire1 = 32;
const int analogX1 = 8;
const int analogY1 = 9;

// Joystick 2
const int buttonU2 = 27;
const int buttonD2 = 26;
const int buttonL2 = 25;
const int buttonR2 = 12;
const int buttonFire2 = 14;
const int analogX2 = 10;
const int analogY2 = 11;

// Other buttons
const int buttonReset = 42;
const int buttonPause = 43;

// Sensitivity of analog thumb joysticks (of the LEDmePlay Joypad) in case of "digital usage" (detects movement if deviates from center position value of 512 by sensitivity value)
const int sensitivity = 192;

bool isInit = false;

void hardware_init()
{
    // Do the init only once
    if (isInit)
        return;

    isInit = true;

    // Initialize serial connection
    Serial.begin(115200);

    initJoystick();

    // Initialize matrix and define text mode
    matrix.begin();
    matrix.setTextSize(1);
    matrix.setTextWrap(false);
}

void clearScreen()
{
    // Delete screen
    matrix.fillScreen(0);
}