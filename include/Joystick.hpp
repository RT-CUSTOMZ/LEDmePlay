#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

// Function prototypes
void initJoystick();
void testJoystick();

// Joystick 1
boolean joy1Up();
boolean joy1Down();
boolean joy1Left();
boolean joy1Right();
boolean joy1Fire();
boolean joy1FireL();
int joy1X();
int joy1Y();

// Joystick 2
boolean joy2Up();
boolean joy2Down();
boolean joy2Left();
boolean joy2Right();
boolean joy2Fire();
boolean joy2FireL();
int joy2X();
int joy2Y();

// Reset-Button
boolean gameReset();

void checkPause();



#endif
