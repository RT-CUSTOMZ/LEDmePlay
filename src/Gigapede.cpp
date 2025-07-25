// Gigapede
// for the LEDmePlay
//
// www.mithotronic.de
//
// Version: 1.1.1
// Author: Michael Rosskopf (2013)
//
// Many thanks Thomas Laubach!
//
// Release Notes:
// V1.1.1: Support of LEDmePI (2021)
// V1.1.0: Support of LEDmePlay Joypad and LEDmePlayBoy (2018)
// V1.0.1: Smaller improvements (colors adapted for IDE 1.x, waiting in status screens can be skipped with fire button)
// V1.0.0: First release

#include <hardware_setup.hpp> // Initialization of hardware definitions
#include <Notes.hpp>    // Initialization of music note definitions
#include <LmP_base.hpp>

// Playfield
byte playfield_gigapede[32][32]; // 0 == empty, 1 == centipede, 2 == mushroom, 3 == shot, 4 == spider, 5 == bug, 6 == dragonfly

// Enemies

// Centipede
byte lengthOfCentipede = 16;
byte centipedeX[24];
byte centipedeY[24];
byte centipedeDirection[24]; // 0 == killed, 1 == right down, 2 == left down, 3 == right up, 4 == left up
byte segmentsKilled;
boolean centipedeKilled;

// Spider
int spiderX;
int spiderY;
byte spiderDirection; // 0 == absent, 1 == right down, 2 == left down, 3 == right up, 4 == left up

// Bug
int bugX;
int bugY;
boolean bugPresent;

// Dragonfly (first appears in level 3)
int dragonflyX;
int dragonflyY;
byte dragonflyDirection; // 0 == absent, 1 == right down, 2 == left down

// Guns and shots
byte numberOfPlayers;
byte gunX[2];
byte gunY[2];
byte shotX[2];
byte shotY[2];
boolean shotFired[2];

// Level settings
byte level_gigapede;
byte lives_gigapede[2];
int score_gigapede[2];
int extraLifeScore_gigapede[2];
int highScore_gigapede = 0;
int gameSpeed;
int enemyProbability;
boolean reset; // if true, return to title screen

// Colors
byte gun1R;
byte gun1G;
byte gun1B;
byte gun2R;
byte gun2G;
byte gun2B;
byte centipedeR;
byte centipedeG;
byte centipedeB;
byte mushroomR;
byte mushroomG;
byte mushroomB;
byte spiderR;
byte spiderG;
byte spiderB;
byte bugR;
byte bugG;
byte bugB;
byte colors[6];
byte newColor;
boolean newColorFound;

// Function declarations
void setupGame_gigapede();
void setupLevel_gigapede();
void setColor(byte color, byte &color_R, byte &color_G, byte &color_B);
void moveCentipede();
void moveSpider();
void moveBug();
void moveDragonfly();
void moveGun();
void moveShot_gigapede();
void drawMushrooms(int _numberOfMushrooms);
boolean collisionDetection();
void explosion_gigapede(byte _x, byte _y);
void wait(int _waitingTime);

void setup_Gigapede()
{
  hardware_init();

  // Mithotronic logo
  mithotronic();
  ledMePlay();
}

// Start screen and game options selection (difficulty and number of players)
void setupGame_gigapede()
{
  // Reset set to false
  reset = false;

  // Title screen
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(4, 0);
  matrix.setTextColor(matrix.Color333(0, 5, 0));
  matrix.println("Giga");
  matrix.setCursor(4, 8);
  matrix.setTextColor(matrix.Color333(0, 5, 0));
  matrix.println("pede");
  matrix.setCursor(1, 16);
  matrix.setTextColor(matrix.Color333(3, 3, 0));
  matrix.println("HiScr");
  matrix.setCursor(1, 24);
  matrix.println(highScore_gigapede);

  // Initialize level score etc.
  score_gigapede[0] = 0;
  score_gigapede[1] = 0;
  lives_gigapede[0] = 3;
  lives_gigapede[1] = 3;
  extraLifeScore_gigapede[0] = 0;
  extraLifeScore_gigapede[1] = 0;
  level_gigapede = 1;

  // Jingle on start screen
  tone(audioPacer, 523, 200);
  delay(200);
  tone(audioPacer, 659, 200);
  delay(200);
  tone(audioPacer, 784, 200);
  delay(200);
  tone(audioPacer, 1047, 400);
  delay(400);
  tone(audioPacer, 784, 200);
  delay(200);
  tone(audioPacer, 1047, 400);
  delay(400);

  // Wait until player 1 presses fire
  do
  {
  } while (!joy1Fire());
  tone(audioPacer, 1024, 20);
  delay(200);

  // Select difficulty (defined by length of centipede)
  matrix.fillRect(0, 16, 32, 16, matrix.Color333(0, 0, 0));
  matrix.setCursor(1, 16);
  matrix.setTextColor(matrix.Color333(3, 3, 0));
  matrix.println("Diff");
  matrix.setCursor(1, 24);
  delay(200);

  do
  {
    if (joy1Left())
    {
      if (lengthOfCentipede > 12)
      {
        lengthOfCentipede = lengthOfCentipede - 4;
        matrix.fillRect(0, 24, 32, 8, matrix.Color333(0, 0, 0));
        tone(audioPacer, 1024, 20);
        delay(200);
      }
    }
    if (joy1Right())
    {
      if (lengthOfCentipede < 24)
      {
        lengthOfCentipede = lengthOfCentipede + 4;
        matrix.fillRect(0, 24, 32, 8, matrix.Color333(0, 0, 0));
        tone(audioPacer, 1024, 20);
        delay(200);
      }
    }
    matrix.setCursor(1, 24);
    switch (lengthOfCentipede)
    {
    case 12:
      matrix.println("Light");
      break;

    case 16:
      matrix.println("Med");
      break;

    case 20:
      matrix.println("Hard");
      break;

    case 24:
      matrix.println("Ultra");
      break;
    }
  } while (!joy1Fire());
  tone(audioPacer, 1024, 20);
  delay(200);

  // Select number of players
  matrix.fillRect(0, 16, 32, 16, matrix.Color333(0, 0, 0));
  matrix.setCursor(1, 16);
  matrix.setTextColor(matrix.Color333(3, 3, 0));
  matrix.println("Plyrs");
  matrix.setCursor(1, 24);
  numberOfPlayers = 1;

  do
  {
    if (joy1Left() && numberOfPlayers != 1)
    {
      numberOfPlayers = 1;
      matrix.fillRect(0, 24, 32, 8, matrix.Color333(0, 0, 0));
      tone(audioPacer, 1024, 20);
      delay(200);
    }
    if (joy1Right() && numberOfPlayers != 2)
    {
      numberOfPlayers = 2;
      matrix.fillRect(0, 24, 32, 8, matrix.Color333(0, 0, 0));
      tone(audioPacer, 1024, 20);
      delay(200);
    }
    matrix.setCursor(14, 24);
    matrix.println(numberOfPlayers);
  } while (!joy1Fire());
  tone(audioPacer, 1024, 20);
  delay(200);
}

// Setup a new or retried level
void setupLevel_gigapede()
{
  // Set colors;
  if (level_gigapede == 1) // Standard colors in level 1
  {
    setColor(1, gun1R, gun1G, gun1B);
    setColor(2, mushroomR, mushroomG, mushroomB);
    setColor(3, gun2R, gun2G, gun2B);
    setColor(4, spiderR, spiderG, spiderB);
    setColor(5, bugR, bugG, bugB);
    setColor(6, centipedeR, centipedeG, centipedeB);
  }

  if ((level_gigapede != 1) && centipedeKilled) // Random colors after level 1
  {
    for (byte i = 0; i < 6; i++)
    {
      colors[i] = 0;
      do
      {
        newColor = byte(random(6) + 1);
        newColorFound = true;
        for (byte j = 0; j < i; j++)
        {
          if (colors[j] == newColor)
          {
            newColorFound = false;
          }
        }
      } while (!newColorFound);
      colors[i] = newColor;
    }
    setColor(colors[0], gun1R, gun1G, gun1B);
    setColor(colors[1], mushroomR, mushroomG, mushroomB);
    setColor(colors[2], gun2R, gun2G, gun2B);
    setColor(colors[3], spiderR, spiderG, spiderB);
    setColor(colors[4], bugR, bugG, bugB);
    setColor(colors[5], centipedeR, centipedeG, centipedeB);
  }

  // Show score
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(1, 0);
  matrix.setTextColor(matrix.Color333(3, 3, 3));
  matrix.println("Score");
  matrix.setCursor(1, 8);
  matrix.setTextColor(matrix.Color333(gun1R, gun1G, gun1B));
  matrix.println(score_gigapede[0]);
  if (numberOfPlayers == 2)
  {
    matrix.setCursor(1, 16);
    matrix.setTextColor(matrix.Color333(gun2R, gun2G, gun2B));
    matrix.println(score_gigapede[1]);
  }
  matrix.setTextColor(matrix.Color333(3, 3, 3));
  matrix.setCursor(1, 24);
  matrix.println(highScore_gigapede);
  wait(4000);

  // Extra life (every 2000 points)
  for (byte i = 0; i < numberOfPlayers; i++)
  {
    if (extraLifeScore_gigapede[i] > 2000)
    {
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      extraLifeScore_gigapede[i] = extraLifeScore_gigapede[i] - 2000;
      lives_gigapede[i]++;
      for (byte j = 100; j > 0; j--)
      {
        matrix.setTextColor(matrix.Color333(7 * (j % 2), 0, 0));
        matrix.setCursor(1, 0);
        matrix.println("EXTRA");
        matrix.setCursor(4, 8);
        matrix.println("LIFE");
        matrix.setCursor(4, 16);
        matrix.println("Plyr");
        matrix.setCursor(14, 24);
        matrix.print(i + 1);
        tone(audioPacer, j * 50, 10);
        delay(10);
      }
      matrix.setTextColor(matrix.Color333(7, 0, 0));
      matrix.setCursor(1, 0);
      matrix.println("EXTRA");
      matrix.setCursor(4, 8);
      matrix.println("LIFE");
      matrix.setCursor(4, 16);
      matrix.println("Plyr");
      matrix.setCursor(14, 24);
      matrix.print(i + 1);
      delay(1000);
    }
  }

  // Show level and remaining lives
  matrix.fillScreen(matrix.Color333(0, 0, 0));
  matrix.setCursor(1, 0);
  matrix.setTextColor(matrix.Color333(0, 0, 3));
  matrix.println("Level");
  matrix.setCursor(13, 8);
  matrix.println(level_gigapede);
  matrix.setCursor(1, 16);
  matrix.setTextColor(matrix.Color333(3, 0, 0));
  matrix.println("Lives");
  if (numberOfPlayers == 1)
  {
    matrix.setCursor(13, 24);
    matrix.println(lives_gigapede[0]);
  }
  else
  {
    matrix.setCursor(1, 24);
    matrix.println(lives_gigapede[0]);
    matrix.setCursor(25, 24);
    matrix.println(lives_gigapede[1]);
  }
  wait(4000);

  // Clear screen again
  matrix.fillScreen(matrix.Color333(0, 0, 0));

  // Initialize playfield
  for (byte i = 0; i < 32; i++)
  {
    for (byte j = 0; j < 32; j++)
    {
      playfield_gigapede[i][j] = 0;
    }
  }

  // Initialize centipede
  for (byte i = 0; i < lengthOfCentipede; i++)
  {
    centipedeX[i] = lengthOfCentipede - 1 - i;
    centipedeY[i] = 0;
    centipedeDirection[i] = 1;
  }
  segmentsKilled = 0;
  centipedeKilled = false;

  // Initialize spider;
  spiderDirection = 0;

  // Initialize bug;
  bugPresent = false;

  // Initialize dragonfly;
  dragonflyDirection = 0;

  // Initialize guns
  gunY[0] = 30;
  gunY[1] = 30;
  shotFired[0] = false;
  shotFired[1] = false;
  if (numberOfPlayers == 1)
  {
    gunX[0] = 15;
  }
  if (numberOfPlayers == 2)
  {
    gunX[0] = 10;
    gunX[1] = 21;
  }

  // Setup level specific game play (increasing difficulty)

  // Initialize mushrooms (number increases with level)
  drawMushrooms(16 + (level_gigapede * 4));
  delay(200);

  // Set game speed (speed increases with level)
  gameSpeed = 60 - ((level_gigapede - 1) * 2);
  if (gameSpeed < 30)
  {
    gameSpeed = 30;
  }

  // Set probability that spider, bug or dragonfly appears (1000 == 100%)
  enemyProbability = 5 + ((level_gigapede - 1) * 5);
  if (enemyProbability > 1000)
  {
    enemyProbability = 1000;
  }
}

// Method to set color values to referenced RGB values (specially used for random color assignment starting in level 2)
void setColor(byte color, byte &color_R, byte &color_G, byte &color_B)
{
  switch (color)
  {
  case 0:
    color_R = 0;
    color_G = 0;
    color_B = 0;
    break;

  case 1:
    color_R = 0;
    color_G = 0;
    color_B = 7;
    break;

  case 2:
    color_R = 0;
    color_G = 7;
    color_B = 0;
    break;

  case 3:
    color_R = 0;
    color_G = 7;
    color_B = 7;
    break;

  case 4:
    color_R = 7;
    color_G = 0;
    color_B = 0;
    break;

  case 5:
    color_R = 7;
    color_G = 0;
    color_B = 7;
    break;

  case 6:
    color_R = 7;
    color_G = 7;
    color_B = 0;
    break;

  case 7:
    color_R = 7;
    color_G = 7;
    color_B = 7;
    break;
  }
}

// Movement of the centipede
void moveCentipede()
{
  for (byte i = 0; i < lengthOfCentipede; i++)
  {
    // remove segment from old position
    if (centipedeDirection[i] > 0)
    {
      playfield_gigapede[centipedeX[i]][centipedeY[i]] = 0;
      matrix.drawPixel(centipedeX[i], centipedeY[i], matrix.Color333(0, 0, 0));
    }
    // Algorithm to change the direction if there is a mushroom or the screen border
    switch (centipedeDirection[i])
    {
    case 1: // right down
      if ((centipedeX[i] == 31) || (playfield_gigapede[centipedeX[i] + 1][centipedeY[i]] > 0))
      {
        if (centipedeY[i] == 31)
        {
          centipedeDirection[i] = 4;
          centipedeY[i]--;
        }
        else
        {
          centipedeDirection[i] = 2;
          centipedeY[i]++;
        }
      }
      else
      {
        centipedeX[i]++;
      }
      break;

    case 2: // left down
      if ((centipedeX[i] == 0) || (playfield_gigapede[centipedeX[i] - 1][centipedeY[i]] > 0))
      {
        if (centipedeY[i] == 31)
        {
          centipedeDirection[i] = 3;
          centipedeY[i]--;
        }
        else
        {
          centipedeDirection[i] = 1;
          centipedeY[i]++;
        }
      }
      else
      {
        centipedeX[i]--;
      }
      break;

    case 3: // right up
      if ((centipedeX[i] == 31) || (playfield_gigapede[centipedeX[i] + 1][centipedeY[i]] > 0))
      {
        if (centipedeY[i] == 0)
        {
          centipedeDirection[i] = 2;
          centipedeY[i]++;
        }
        else
        {
          centipedeDirection[i] = 4;
          centipedeY[i]--;
        }
      }
      else
      {
        centipedeX[i]++;
      }
      break;

    case 4: // left up
      if ((centipedeX[i] == 0) || (playfield_gigapede[centipedeX[i] - 1][centipedeY[i]] > 0))
      {
        if (centipedeY[i] == 0)
        {
          centipedeDirection[i] = 1;
          centipedeY[i]++;
        }
        else
        {
          centipedeDirection[i] = 3;
          centipedeY[i]--;
        }
      }
      else
      {
        centipedeX[i]--;
      }
      break;
    }

    // Draw segment at new position
    if (centipedeDirection[i] > 0)
    {
      playfield_gigapede[centipedeX[i]][centipedeY[i]] = 1;
      matrix.drawPixel(centipedeX[i], centipedeY[i], matrix.Color333(centipedeR, centipedeG, centipedeB));
    }
  }
}

// Movement of spider
void moveSpider()
{
  // Activate spider randomly
  if (spiderDirection == 0)
  {
    if (random(1000) < enemyProbability)
    {
      if (random(100) < 50)
      {
        spiderDirection = 1;
        spiderX = -10;
        spiderY = random(7) + 24;
      }
      else
      {
        spiderDirection = 2;
        spiderX = 39;
        spiderY = random(7) + 24;
      }
    }
  }
  else
  {
    // Remove spider at old position
    if ((spiderX > -2) && (spiderX < 31))
    {
      playfield_gigapede[spiderX + 1][spiderY] = 0;
      matrix.drawPixel(spiderX + 1, spiderY, matrix.Color333(0, 0, 0));
    }
    if ((spiderX > -1) && (spiderX < 32))
    {
      playfield_gigapede[spiderX][spiderY + 1] = 0;
      matrix.drawPixel(spiderX, spiderY + 1, matrix.Color333(0, 0, 0));
    }
    if ((spiderX > -3) && (spiderX < 30))
    {
      playfield_gigapede[spiderX + 2][spiderY + 1] = 0;
      matrix.drawPixel(spiderX + 2, spiderY + 1, matrix.Color333(0, 0, 0));
    }

    switch (spiderDirection)
    {
    case 1: // right down
      if (spiderY == 30)
      {
        spiderDirection = 3;
      }
      else
      {
        spiderX++;
        spiderY++;
      }
      break;

    case 2: // left down
      if (spiderY == 30)
      {
        spiderDirection = 4;
      }
      else
      {
        spiderX--;
        spiderY++;
      }
      break;

    case 3: // right up
      if (spiderY == 24)
      {
        spiderDirection = 1;
      }
      else
      {
        spiderX++;
        spiderY--;
      }
      break;

    case 4: // left up
      if (spiderY == 24)
      {
        spiderDirection = 2;
      }
      else
      {
        spiderX--;
        spiderY--;
      }
      break;
    }

    // Spider moves out of the screen
    if ((spiderX < -10 || spiderX > 39))
    {
      spiderDirection = 0;
    }
    else
    {
      // Draw spider at new position
      if ((spiderX > -2) && (spiderX < 31))
      {
        playfield_gigapede[spiderX + 1][spiderY] = 4;
        matrix.drawPixel(spiderX + 1, spiderY, matrix.Color333(spiderR, spiderG, spiderB));
      }
      if ((spiderX > -1) && (spiderX < 32))
      {
        playfield_gigapede[spiderX][spiderY + 1] = 4;
        matrix.drawPixel(spiderX, spiderY + 1, matrix.Color333(spiderR, spiderG, spiderB));
      }
      if ((spiderX > -3) && (spiderX < 30))
      {
        playfield_gigapede[spiderX + 2][spiderY + 1] = 4;
        matrix.drawPixel(spiderX + 2, spiderY + 1, matrix.Color333(spiderR, spiderG, spiderB));
      }
    }
  }
}

// Movement of bug
void moveBug()
{
  // Activate bug randomly
  if (bugPresent == false)
  {
    if (random(1000) < enemyProbability)
    {
      bugPresent = true;
      bugX = random(32);
      bugY = -1;
    }
  }
  else
  {
    // Remove bug at old position
    if (bugY > -1)
    {
      if (random(8) == 0)
      {
        // Bug randomly drops a mushroom
        playfield_gigapede[bugX][bugY] = 2;
        matrix.drawPixel(bugX, bugY, matrix.Color333(mushroomR, mushroomG, mushroomB));
      }
      else
      {
        playfield_gigapede[bugX][bugY] = 0;
        matrix.drawPixel(bugX, bugY, matrix.Color333(0, 0, 0));
      }
    }

    // Move bug
    bugY++;

    if (bugY > 31)
    {
      bugPresent = false;
    }
    else
    {
      // Draw bug at new position
      playfield_gigapede[bugX][bugY] = 5;
      matrix.drawPixel(bugX, bugY, matrix.Color333(bugR, bugG, bugB));
    }
  }
}

// Movement of dragonfly
void moveDragonfly()
{
  // Dragonfly appears randomly (in level 3 or higher)
  if (dragonflyDirection == 0)
  {
    if ((random(1000) < (enemyProbability) / 2) && level_gigapede > 2)
    {
      if (random(100) < 50)
      {
        dragonflyDirection = 1;
      }
      else
      {
        dragonflyDirection = 2;
      }
      dragonflyX = random(30);
      dragonflyY = -3;
    }
  }
  else
  {
    // Remove dragonfly at old position
    if ((dragonflyY > -1) && (dragonflyY < 32))
    {
      playfield_gigapede[dragonflyX + 1][dragonflyY] = 0;
      matrix.drawPixel(dragonflyX + 1, dragonflyY, matrix.Color333(0, 0, 0));
    }
    if ((dragonflyY > -2) && (dragonflyY < 31))
    {
      playfield_gigapede[dragonflyX + 1][dragonflyY + 1] = 0;
      matrix.drawPixel(dragonflyX + 1, dragonflyY + 1, matrix.Color333(0, 0, 0));
    }
    if ((dragonflyY > -3) && (dragonflyY < 30))
    {
      playfield_gigapede[dragonflyX][dragonflyY + 2] = 0;
      matrix.drawPixel(dragonflyX, dragonflyY + 2, matrix.Color333(0, 0, 0));
    }
    if ((dragonflyY > -3) && (dragonflyY < 30))
    {
      playfield_gigapede[dragonflyX + 1][dragonflyY + 2] = 0;
      matrix.drawPixel(dragonflyX + 1, dragonflyY + 2, matrix.Color333(0, 0, 0));
    }
    if ((dragonflyY > -3) && (dragonflyY < 30))
    {
      playfield_gigapede[dragonflyX + 2][dragonflyY + 2] = 0;
      matrix.drawPixel(dragonflyX + 2, dragonflyY + 2, matrix.Color333(0, 0, 0));
    }

    switch (dragonflyDirection)
    {
    case 1: // right down
      if (dragonflyX > 27)
      {
        dragonflyDirection = 2;
      }
      else
      {
        dragonflyX = dragonflyX + 2;
        dragonflyY++;
      }
      break;

    case 2: // left down
      if (dragonflyX < 2)
      {
        dragonflyDirection = 1;
      }
      else
      {
        dragonflyX = dragonflyX - 2;
        dragonflyY++;
      }
      break;
    }

    // Dragonfly moves out of the screen
    if ((dragonflyY > 31))
    {
      dragonflyDirection = 0;
    }
    else
    {
      // Draw dragonfly at new position
      if ((dragonflyY > -1) && (dragonflyY < 32))
      {
        playfield_gigapede[dragonflyX + 1][dragonflyY] = 6;
        matrix.drawPixel(dragonflyX + 1, dragonflyY, matrix.Color333(3, 3, 3));
      }
      if ((dragonflyY > -2) && (dragonflyY < 31))
      {
        playfield_gigapede[dragonflyX + 1][dragonflyY + 1] = 6;
        matrix.drawPixel(dragonflyX + 1, dragonflyY + 1, matrix.Color333(3, 3, 3));
      }
      if ((dragonflyY > -3) && (dragonflyY < 30))
      {
        playfield_gigapede[dragonflyX][dragonflyY + 2] = 6;
        matrix.drawPixel(dragonflyX, dragonflyY + 2, matrix.Color333(3, 3, 3));
      }
      if ((dragonflyY > -3) && (dragonflyY < 30))
      {
        playfield_gigapede[dragonflyX + 1][dragonflyY + 2] = 6;
        matrix.drawPixel(dragonflyX + 1, dragonflyY + 2, matrix.Color333(3, 0, 0));
      }
      if ((dragonflyY > -3) && (dragonflyY < 30))
      {
        playfield_gigapede[dragonflyX + 2][dragonflyY + 2] = 6;
        matrix.drawPixel(dragonflyX + 2, dragonflyY + 2, matrix.Color333(3, 3, 3));
      }
    }
  }
}

// Movement of guns
void moveGun()
{
  // Remove gun at old position
  matrix.drawPixel(gunX[0], gunY[0], matrix.Color333(0, 0, 0));
  matrix.drawPixel(gunX[0], gunY[0] + 1, matrix.Color333(0, 0, 0));

  // Check joystick 1 for player 1
  if (joy1Left() && (gunX[0] > 0))
  {
    gunX[0]--;
  }
  if (joy1Right() && (gunX[0] < 31))
  {
    gunX[0]++;
  }
  if (joy1Up() && (gunY[0] > 24))
  {
    gunY[0]--;
  }
  if (joy1Down() && (gunY[0] < 30))
  {
    gunY[0]++;
  }
  if (joy1Fire() && (shotFired[0] == false))
  {
    shotX[0] = gunX[0];
    shotY[0] = gunY[0];
    shotFired[0] = true;
    tone(audioPacer, 1024, 20);
  }

  // Draw gun at new position
  matrix.drawPixel(gunX[0], gunY[0], matrix.Color333(gun1R, gun1G, gun1B));
  matrix.drawPixel(gunX[0], gunY[0] + 1, matrix.Color333(gun1R, gun1G, gun1B));

  if (numberOfPlayers == 2)
  {
    // Remove gun at old position
    matrix.drawPixel(gunX[1], gunY[1], matrix.Color333(0, 0, 0));
    matrix.drawPixel(gunX[1], gunY[1] + 1, matrix.Color333(0, 0, 0));

    // Check joystick 2 for player 2
    if (joy2Left() && (gunX[1] > 0))
    {
      gunX[1]--;
    }
    if (joy2Right() && (gunX[1] < 31))
    {
      gunX[1]++;
    }
    if (joy2Up() && (gunY[1] > 24))
    {
      gunY[1]--;
    }
    if (joy2Down() && (gunY[1] < 30))
    {
      gunY[1]++;
    }
    if (joy2Fire() && (shotFired[1] == false))
    {
      shotX[1] = gunX[1];
      shotY[1] = gunY[1];
      shotFired[1] = true;
      tone(audioPacer, 1024, 20);
    }

    // Draw gun at new position
    matrix.drawPixel(gunX[1], gunY[1], matrix.Color333(gun2R, gun2G, gun2B));
    matrix.drawPixel(gunX[1], gunY[1] + 1, matrix.Color333(gun2R, gun2G, gun2B));
  }
}

// Movements of shots and hit detection
void moveShot_gigapede()
{
  for (byte i = 0; i < 3; i++)
  {
    for (byte j = 0; j < numberOfPlayers; j++)
    {

      if (shotFired[j])
      {
        // Remove shot at old position
        matrix.drawPixel(shotX[j], shotY[j], matrix.Color333(0, 0, 0));

        if (shotY[j] == 0)
        {
          shotFired[j] = false;
        }
        else if (playfield_gigapede[shotX[j]][shotY[j] - 1] == 0)
        {
          shotY[j]--;
          // Draw shot at new position
          matrix.drawPixel(shotX[j], shotY[j], matrix.Color333(7, 7, 7));
        }
        else
        {
          // Hit
          tone(audioPacer, 512, 20);
          switch (playfield_gigapede[shotX[j]][shotY[j] - 1])
          {

          case 1: // Centipede hit
            for (byte i = 0; i < lengthOfCentipede; i++)
            {
              if ((centipedeX[i] == shotX[j]) && (centipedeY[i] == shotY[j] - 1) && (centipedeDirection[i] != 0))
              {
                centipedeDirection[i] = 0;
                segmentsKilled++;
                // Turn hit segment into a mushroom
                playfield_gigapede[shotX[j]][shotY[j] - 1] = 2;
                matrix.drawPixel(shotX[j], shotY[j] - 1, matrix.Color333(mushroomR, mushroomG, mushroomB));
                shotFired[j] = false;
                score_gigapede[j] = score_gigapede[j] + 10;
                extraLifeScore_gigapede[j] = extraLifeScore_gigapede[j] + 10;
              }
            }
            break;

          case 2: // Mushroom hit
            playfield_gigapede[shotX[j]][shotY[j] - 1] = 0;
            matrix.drawPixel(shotX[j], shotY[j] - 1, matrix.Color333(0, 0, 0));
            shotFired[j] = false;
            score_gigapede[j] = score_gigapede[j] + 1;
            extraLifeScore_gigapede[j] = extraLifeScore_gigapede[j] + 1;
            break;

          case 4: // Spider hit
            spiderDirection = 0;
            if ((spiderX > -2) && (spiderX < 31))
            {
              playfield_gigapede[spiderX + 1][spiderY] = 0;
              matrix.drawPixel(spiderX + 1, spiderY, matrix.Color333(0, 0, 0));
            }
            if ((spiderX > -1) && (spiderX < 32))
            {
              playfield_gigapede[spiderX][spiderY + 1] = 0;
              matrix.drawPixel(spiderX, spiderY + 1, matrix.Color333(0, 0, 0));
            }
            if ((spiderX > -3) && (spiderX < 30))
            {
              playfield_gigapede[spiderX + 2][spiderY + 1] = 0;
              matrix.drawPixel(spiderX + 2, spiderY + 1, matrix.Color333(0, 0, 0));
            }
            shotFired[j] = false;
            score_gigapede[j] = score_gigapede[j] + 40;
            extraLifeScore_gigapede[j] = extraLifeScore_gigapede[j] + 40;
            break;

          case 5: // Bug hit
            bugPresent = false;
            playfield_gigapede[bugX][bugY] = 0;
            matrix.drawPixel(bugX, bugY, matrix.Color333(0, 0, 0));
            shotFired[j] = false;
            score_gigapede[j] = score_gigapede[j] + 20;
            extraLifeScore_gigapede[j] = extraLifeScore_gigapede[j] + 20;
            break;

          case 6: // Dragonfly hit
            dragonflyDirection = 0;
            if ((dragonflyY > -1) && (dragonflyY < 32))
            {
              playfield_gigapede[dragonflyX + 1][dragonflyY] = 0;
              matrix.drawPixel(dragonflyX + 1, dragonflyY, matrix.Color333(0, 0, 0));
            }
            if ((dragonflyY > -2) && (dragonflyY < 31))
            {
              playfield_gigapede[dragonflyX + 1][dragonflyY + 1] = 0;
              matrix.drawPixel(dragonflyX + 1, dragonflyY + 1, matrix.Color333(0, 0, 0));
            }
            if ((dragonflyY > -3) && (dragonflyY < 30))
            {
              playfield_gigapede[dragonflyX][dragonflyY + 2] = 0;
              matrix.drawPixel(dragonflyX, dragonflyY + 2, matrix.Color333(0, 0, 0));
            }
            if ((dragonflyY > -3) && (dragonflyY < 30))
            {
              playfield_gigapede[dragonflyX + 1][dragonflyY + 2] = 0;
              matrix.drawPixel(dragonflyX + 1, dragonflyY + 2, matrix.Color333(0, 0, 0));
            }
            if ((dragonflyY > -3) && (dragonflyY < 30))
            {
              playfield_gigapede[dragonflyX + 2][dragonflyY + 2] = 0;
              matrix.drawPixel(dragonflyX + 2, dragonflyY + 2, matrix.Color333(0, 0, 0));
            }
            shotFired[j] = false;
            score_gigapede[j] = score_gigapede[j] + 100;
            extraLifeScore_gigapede[j] = extraLifeScore_gigapede[j] + 100;
            break;
          }
        }
      }
    }
  }
}

// Randomly drop mushrooms on the screen (used before a new level starts)
void drawMushrooms(int _numberOfMushrooms)
{
  for (int i = 0; i < _numberOfMushrooms; i++)
  {
    byte mushroomX = random(32);
    byte mushroomY = random(23) + 1;
    playfield_gigapede[mushroomX][mushroomY] = 2;
    matrix.drawPixel(mushroomX, mushroomY, matrix.Color333(mushroomR, mushroomG, mushroomB));
  }
}

// Collision detection of players with enemies
boolean collisionDetection()
{
  boolean collision = false;

  for (byte i = 0; i < numberOfPlayers; i++)
  {
    if ((playfield_gigapede[gunX[i]][gunY[i]] || playfield_gigapede[gunX[i]][gunY[i] + 1]) > 0)
    {
      explosion_gigapede(gunX[i], gunY[i]);
      lives_gigapede[i]--;
      collision = true;
    }
  }
  if (collision)
  {
    return true;
  }
  return false;
}

// Creates an explosion if a player has been hit
void explosion_gigapede(byte _x, byte _y)
{
  matrix.fillCircle(_x, _y, 1, matrix.Color333(7, 7, 7));
  tone(audioPacer, 2048, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 2, matrix.Color333(7, 7, 5));
  tone(audioPacer, 1024, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 3, matrix.Color333(7, 7, 3));
  tone(audioPacer, 512, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 4, matrix.Color333(5, 5, 3));
  tone(audioPacer, 256, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 5, matrix.Color333(4, 4, 2));
  tone(audioPacer, 128, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 6, matrix.Color333(3, 3, 2));
  tone(audioPacer, 64, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 7, matrix.Color333(2, 2, 1));
  tone(audioPacer, 32, 40);
  delay(40);
  matrix.fillCircle(_x, _y, 8, matrix.Color333(1, 1, 0));
  tone(audioPacer, 16, 40);
  delay(40);
  for (byte i = 1; i < 9; i++)
  {
    matrix.fillCircle(_x, _y, i, matrix.Color333(0, 0, 0));
    delay(40);
  }
}

// Wait (which can be interrupted by pressing fire button)
void wait(int _waitingTime)
{
  int i;
  boolean firePressed = false;
  if (_waitingTime > 200)
  {
    i = _waitingTime - 200;
  }
  else
  {
    i = _waitingTime;
  }
  delay(200);

  do
  {
    i--;
    delay(1);
    if (joy1Fire() || joy2Fire())
    {
      firePressed = true;
    }
  } while (i > 0 && !firePressed);

  if (firePressed)
  {
    tone(audioPacer, 1024, 20);
    delay(20);
  }
}

// Main loop of the game
void loop_Gigapede()
{
  // Start screen
  setupGame_gigapede();
  do
  {
    // Start or retry a level
    setupLevel_gigapede();
    do
    {
      // Sounds depending on enemies on screen
      tone(audioPacer, 200, 20);
      if (bugPresent)
      {
        tone(audioPacer, 200, 40);
      }
      if (spiderDirection > 0)
      {
        tone(audioPacer, 400, 40);
      }
      if (dragonflyDirection > 0)
      {
        tone(audioPacer, 800, 40);
      }

      // Moving enemies
      moveCentipede();
      moveSpider();
      moveBug();
      moveDragonfly();

      // Moving guns and shots
      moveGun();
      moveShot_gigapede();

      // Delay depending on game speed
      delay(gameSpeed);

      // Killed
      if (collisionDetection())
      {
        break;
      }

      // Centipede killed
      if (segmentsKilled == lengthOfCentipede)
      {
        centipedeKilled = true;

        // Level completed sound
        for (int i = 0; i < 5; i++)
        {
          for (int j = 0; j < 100; j++)
          {
            tone(audioPacer, j * 50, 2);
            delay(2);
          }
        }

        // Add level bonus
        score_gigapede[0] = score_gigapede[0] + (level_gigapede * 50) * ((lengthOfCentipede / 4) - 2);
        extraLifeScore_gigapede[0] = extraLifeScore_gigapede[0] + (level_gigapede * 50) * ((lengthOfCentipede / 4) - 2);
        if (numberOfPlayers == 2)
        {
          score_gigapede[1] = score_gigapede[1] + (level_gigapede * 50) * ((lengthOfCentipede / 4) - 2);
          extraLifeScore_gigapede[1] = extraLifeScore_gigapede[1] + (level_gigapede * 50) * ((lengthOfCentipede / 4) - 2);
        }
        level_gigapede++; // Increase level

        break;
      }

      checkPause();

    } while (true);
    delay(1000);
  } while ((lives_gigapede[0] > 0) && (lives_gigapede[1] > 0) && !reset);

  if (!reset)
  {

    // Game over
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    for (int i = 0; i < 100; i++)
    {
      matrix.setCursor(4, 8);
      matrix.setTextColor(matrix.Color333(7 * (i % 2), 0, 0));
      matrix.println("GAME");
      matrix.setCursor(4, 16);
      matrix.println("OVER");
      tone(audioPacer, i * 10, 10);
      delay(10);
    }
    matrix.setCursor(4, 8);
    matrix.setTextColor(matrix.Color333(7, 0, 0));
    matrix.println("GAME");
    matrix.setCursor(4, 16);
    matrix.println("OVER");
    wait(3000);

    // Show winner (if two players)
    if ((numberOfPlayers == 2) && (score_gigapede[0] != score_gigapede[1]))
    {
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      matrix.setCursor(4, 4);
      matrix.setTextColor(matrix.Color333(0, 0, 7));
      matrix.println("Plyr");
      matrix.setCursor(14, 12);
      if (score_gigapede[0] > score_gigapede[1])
      {
        matrix.print(1);
      }
      else
      {
        matrix.print(2);
      }
      matrix.setCursor(2, 20);
      matrix.println("wins!");
      wait(4000);
    }

    // Set high score if reached
    if (score_gigapede[0] > highScore_gigapede)
    {
      highScore_gigapede = score_gigapede[0];
      matrix.fillScreen(matrix.Color333(0, 0, 0));
      for (byte i = 0; i < 5; i++)
      {
        explosion_gigapede(random(24) + 4, random(24) + 4);
      }
      matrix.fillScreen(matrix.Color333(0, 0, 0));

      for (byte i = 100; i > 0; i--)
      {
        matrix.setCursor(7, 8);
        matrix.setTextColor(matrix.Color333(7 * (i % 2), 7 * (i % 2), 7 * (i % 2)));
        matrix.println("NEW");
        matrix.setCursor(1, 16);
        matrix.println("HiScr");
        tone(audioPacer, i * 50, 10);
        delay(10);
      }
      matrix.setCursor(7, 8);
      matrix.setTextColor(matrix.Color333(7, 7, 7));
      matrix.println("NEW");
      matrix.setCursor(1, 16);
      matrix.println("HiScr");
      wait(4000);
    }

    // Show final score
    matrix.fillScreen(matrix.Color333(0, 0, 0));
    matrix.setCursor(1, 0);
    matrix.setTextColor(matrix.Color333(3, 3, 3));
    matrix.println("Score");
    matrix.setCursor(1, 8);
    matrix.setTextColor(matrix.Color333(gun1R, gun1G, gun1B));
    matrix.println(score_gigapede[0]);
    if (numberOfPlayers == 2)
    {
      matrix.setCursor(1, 16);
      matrix.setTextColor(matrix.Color333(gun2R, gun2G, gun2B));
      matrix.println(score_gigapede[1]);
    }
    matrix.setTextColor(matrix.Color333(3, 3, 3));
    matrix.setCursor(1, 24);
    matrix.println(highScore_gigapede);
    wait(4000);
  }
}
