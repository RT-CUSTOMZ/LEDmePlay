#include <Joystick.hpp>
#include <hardware_setup.hpp>
void setup_joy()
{
    hardware_init();
    Serial.println("Hello Joy");
    initJoystick();
}
void loop_joy()
{
    testJoystick();
    delay(2000);
}
