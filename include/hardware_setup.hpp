#ifndef HARDWARE_SETUP_HPP
#define HARDWARE_SETUP_HPP

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Joystick.hpp>

struct GameMethods
{
    String name;
    void (*setup)();
    void (*loop)();
};

extern struct GameMethods myGames[];

// Setup adafruit matrix
#define D_R1 23
#define D_G1 22
#define D_B1 21
#define D_R2 19
#define D_G2 18
#define D_B2 5
#define D_A 17
#define D_B 16
#define D_C 4
#define D_D 0
#define D_CLK 2
#define D_LAT 15
#define D_OE 14

extern uint8_t rgbPins[];
extern RGBmatrixPanel matrix;

extern const int audioPacer;

// Joystick 1
extern const int buttonU1;
extern const int buttonD1;
extern const int buttonL1;
extern const int buttonR1;
extern const int buttonFire1;
extern const int analogX1;
extern const int analogY1;

// Joystick 2
extern const int buttonU2;
extern const int buttonD2;
extern const int buttonL2;
extern const int buttonR2;
extern const int buttonFire2;
extern const int analogX2;
extern const int analogY2;

// Other buttons
extern const int buttonReset;
extern const int buttonPause;

// Sensitivity of analog thumb joysticks (of the LEDmePlay Joypad) in case of "digital usage" (detects movement if deviates from center position value of 512 by sensitivity value)
extern const int sensitivity;

/** Initialize the hardware */
void hardware_init();
void clearScreen();

#endif /* HARDWARE_SETUP_HPP */
