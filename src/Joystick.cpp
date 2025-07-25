#include <Joystick.hpp>

int initX1, initY1;
int initX2, initY2;

// Macro to read the analog value from the joystick and subtract an offset.
//  Paramter: pin The analog pin number to read from.
//  return: The adjusted analog value.
#define JOY_READ(pin) analogRead(pin) - JOY_OFFSET

// Macro to check if the joystick is moved up.
#define TEST_UP j > JOY_THRESHOLD

// Macro to check if the joystick is moved down.
#define TEST_DOWN j < -JOY_THRESHOLD

// Macro to check if the joystick is moved left.
#define TEST_LEFT TEST_DOWN

// Macro to check if the joystick is moved right.
#define TEST_RIGHT TEST_UP

// Joystick 1
#define JOY_H1 34       //  Horizontal axis pin for Joystick 1
#define JOY_V1 36       //  Vertical axis pin for Joystick 1
#define JOY_FIRE_1 26   // Fire button pin for Joystick 1
#define JOY_FIRE_L_1 32 // Left firebutton for Joystick 1

// Joystick 2
#define JOY_H2 35       //  Horizontal axis pin for Joystick 2
#define JOY_V2 39       //  Vertical axis pin for Joystick 2
#define JOY_FIRE_2 27   // Fire button pin for Joystick 2
#define JOY_FIRE_L_2 33 // Left firebutton for Joystick 2

// Other buttons
#define GAME_RESET 13 // Full-Game-Reset

// Thresholds & Offsets
#define JOY_OFFSET 2048          ///< Offset to center joystick readings
#define JOY_THRESHOLD 1000       ///< Threshold for detecting joystick movement
#define JOY_ENABLE_THRESHOLD 500 ///< Threshold for enabling joystick detection

// This task implements the option to restart the console.
void IRAM_ATTR ResetWatcher()
{
    // Do a restart
    ESP.restart();
}

/// Tests the current state of the joysticks and prints the results.
void testJoystick()
{
    Serial.printf("Joy V1: %d\n", JOY_READ(JOY_V1));
    Serial.printf("Joy H1: %d\n", JOY_READ(JOY_H1));
    Serial.printf("Joy V2: %d\n", JOY_READ(JOY_V2));
    Serial.printf("Joy H2: %d\n", JOY_READ(JOY_H2));

    if (joy1Up())
        Serial.println("joy1Up");
    if (joy1Down())
        Serial.println("joy1Down");
    if (joy1Left())
        Serial.println("joy1Left");
    if (joy1Right())
        Serial.println("joy1Right");
    if (joy1Fire())
        Serial.println("joy1Fire");
    if (joy1FireL())
        Serial.println("joy1FireL");

    if (joy2Up())
        Serial.println("joy2Up");
    if (joy2Down())
        Serial.println("joy2Down");
    if (joy2Left())
        Serial.println("joy2Left");
    if (joy2Right())
        Serial.println("joy2Right");
    if (joy2Fire())
        Serial.println("joy2Fire");
    if (joy2FireL())
        Serial.println("joy2FireL");

    if (gameReset())
        Serial.println("gameReset");

    checkPause();
}

bool isJoy_1_alive, isJoy_2_alive;

// Initializes the joysticks and checks if they are functional.
// This includes checking if the joystick readings are within a valid range.
void initJoystick()
{
    if (abs(JOY_READ(JOY_V1)) < JOY_ENABLE_THRESHOLD && abs(JOY_READ(JOY_H1)) < JOY_ENABLE_THRESHOLD)
    {
        isJoy_1_alive = true;
        Serial.println("Joy1 usable.");
    }
    else
    {
        Serial.println("Joy1 killed.");
        isJoy_1_alive = false;
    }

    if (abs(JOY_READ(JOY_V2)) < JOY_ENABLE_THRESHOLD && abs(JOY_READ(JOY_H2)) < JOY_ENABLE_THRESHOLD)
    {
        isJoy_2_alive = true;
        Serial.println("Joy2 usable.");
    }
    else
    {
        Serial.println("Joy2 killed.");
        isJoy_2_alive = false;
    }

    // Initialize joysticks and  buttons
    pinMode(JOY_FIRE_1, INPUT_PULLUP);
    pinMode(JOY_FIRE_L_1, INPUT_PULLUP);
    pinMode(JOY_FIRE_2, INPUT_PULLUP);
    pinMode(JOY_FIRE_L_2, INPUT_PULLUP);

    initX1 = analogRead(JOY_V1);
    initY1 = analogRead(JOY_H1);
    initX2 = analogRead(JOY_V2);
    initY2 = analogRead(JOY_H2);

    pinMode(GAME_RESET, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(GAME_RESET), ResetWatcher, FALLING);
}

void checkPause()
{

    // Is a pausButton pressed?
    if (!joy1FireL() && !joy2FireL())
        return;

    // Wait for button release
    while (joy1FireL() || joy2FireL())
        delay(1);

    Serial.println("Pause!");

    // Pause
    while (!joy1FireL() && !joy2FireL())
        delay(1);

    // Wait for button release
    while (joy1FireL() || joy2FireL())
        delay(1);
}

int linearize(int aktVal, int mid)
{
    float leftFactor = 2048.0 / mid;
    float rightFactor = 2048.0 / (4096 - mid);

    int div = aktVal - mid;
    if (div < 0)
        return div * leftFactor;
    else
        return div * rightFactor;
}

int joy1X()
{
    if (!isJoy_1_alive)
        return 0;

    return linearize(analogRead(JOY_V1), initX1);
}

int joy1Y()
{
    if (!isJoy_1_alive)
        return 0;

    return linearize(analogRead(JOY_H1), initY1);
}

int joy2X()
{
    if (!isJoy_1_alive)
        return 0;

    return linearize(analogRead(JOY_V2), initX2);
}

int joy2Y()
{
    if (!isJoy_1_alive)
        return 0;

    return linearize(analogRead(JOY_H2), initY2);;
}

// Joystick 1
boolean joy1Up()
{
    if (!isJoy_1_alive)
        return false;

    int16_t j = JOY_READ(JOY_V1);
    return TEST_UP;
}
boolean joy1Down()
{
    if (!isJoy_1_alive)
        return false;

    int16_t j = JOY_READ(JOY_V1);
    return TEST_DOWN;
}
boolean joy1Left()
{
    if (!isJoy_1_alive)
        return false;

    int16_t j = JOY_READ(JOY_H1);
    return TEST_LEFT;
}
boolean joy1Right()
{
    if (!isJoy_1_alive)
        return false;

    int16_t j = JOY_READ(JOY_H1);
    return TEST_RIGHT;
}
boolean joy1Fire()
{
    if (!isJoy_1_alive)
        return false;

    return digitalRead(JOY_FIRE_1) == LOW;
}
boolean joy1FireL()
{
    return digitalRead(JOY_FIRE_L_1) == LOW;
}

// Joystick 2
boolean joy2Up()
{
    if (!isJoy_2_alive)
        return false;

    int16_t j = JOY_READ(JOY_V2);
    return TEST_UP;
}

boolean joy2Down()
{
    if (!isJoy_2_alive)
        return false;

    int16_t j = JOY_READ(JOY_V2);
    return TEST_DOWN;
}
boolean joy2Left()
{
    if (!isJoy_2_alive)
        return false;

    int16_t j = JOY_READ(JOY_H2);
    return TEST_LEFT;
}
boolean joy2Right()
{
    if (!isJoy_2_alive)
        return false;

    int16_t j = JOY_READ(JOY_H2);
    return TEST_RIGHT;
}
boolean joy2Fire()
{
    if (!isJoy_2_alive)
        return false;

    return digitalRead(JOY_FIRE_2) == LOW;
}
boolean joy2FireL()
{
    return digitalRead(JOY_FIRE_L_2) == LOW;
}
boolean gameReset()
{
    return digitalRead(GAME_RESET) == LOW;
}