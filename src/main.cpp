
#include <hardware_setup.hpp>

void setup_Invaders();
void loop_Invaders();

void setup_Gigapede();
void loop_Gigapede();

void setup_joy();
void loop_joy();

void setup_Pacer();
void loop_Pacer();

uint8_t selectGame();

struct GameMethods myGames[] = {
    {"Invaders", &setup_Invaders, &loop_Invaders},
    {"Gentipede", &setup_Gigapede, &loop_Gigapede},
    {"Pacer", &setup_Pacer, &loop_Pacer},
    {"TestJoy", &setup_joy, &loop_joy},
    {"Carambolage", &setup_Invaders, &loop_Invaders},
    {"Lara Croft", &setup_Gigapede, &loop_Gigapede},
    {"Dortmund", &setup_joy, &loop_joy},
    {"Ende", NULL}};

u_int8_t ldm_game = 3;

void setup()
{
    hardware_init();
    ldm_game = selectGame();
    // Serial.println(ldm_game);
    // ldm_game=2;
    myGames[ldm_game].setup();
}

void loop()
{
    myGames[ldm_game].loop();
}