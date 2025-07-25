// Invaders
// for the LEDmePlay
//
// www.mithotronic.de
//
// Version: 1.1.1
// Authors: Michael Rosskopf and Thomas Laubach (2016)
//
// Release Notes:
// V1.1.1: Support of LEDmePI (2021)
// V1.1.0: Support for LEDmePlay Joypad and LEDmePlayBoy (2018)
// V1.0.0: First release

// Include libraries for adafruit matrix

#include <hardware_setup.hpp> // Initialization of hardware definitions
#include <Notes.hpp>    // Initialization of music note definitions
#include <LmP_base.hpp>

// Playfield
byte playfield_invaders[32][32]; // 0 = empty, 1 = shield, 2 = mothership, 3 = boss, 4 ... 23 = enemy 0 - 19, 255 = bombs

// Player´s base
byte lives_invaders;
byte baseX;
byte level_invaders;
byte difficulty;
int waitTimerPlayer;
int currentWaitTimerPlayer;
byte playerShotX;
byte playerShotY;
byte playerShotStatus; // 0 = not fired, 1 = fired
int waitTimerPlayerShot;
int currentWaitTimerPlayerShot;
int score_invaders;
int extraLifeScore_invaders; // Extralife every 2000 points
int highscore_invaders;
int extraLifeCoughtTimer;
boolean baseDestroyed;
int waitCounterBaseDestroyed;

// Alien formation
int formationX;
int formationY;
boolean formationLanded; // true, if formation is so deep that player cannot win (level is restarted)
byte formationWidth = 5;
byte formationHeight = 4;
byte columnOffsetLeft;
byte columnOffsetRight;
byte animationPhase; // also used by bosses
int waitTimerFormation;
int currentWaitTimerFormation;
byte remainingAliens;
byte movementStatus;  // 0 = right, 1 = down, 2 = left, 3 = up
byte alienType[64];   // Variants of aliens (1 - 5)
byte alienStatus[64]; // 0 = absent, 1 = present, 2 = hit

// Alien bombs
int bombingRate;
int waitTimerBombs;
int currentWaitTimerBombs;
byte bombPointer;
float bombX[16];
byte bombY[16];
float bombDrift[16];
byte bombStatus[16]; // 0 = absent, 1 = present

// Alien mothership
int mothershipX;
byte mothershipStatus; // 0 = absent, 1 = present, 2 = hit
int mothershipRate;
int waitTimerMothership;
int currentWaitTimerMothership;

// Bosses
boolean bossActive;
byte bossType;
byte bossShield;
byte bossWidth;
byte bossX;
byte bossY;
byte bossMovementStatus;
int waitTimerBoss;
int currentWaitTimerBoss;
byte invader1[128] = {
    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 2, 1, 1, 2, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 1,
    0, 1, 0, 0, 0, 0, 1, 0,

    0, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 2, 1, 1, 2, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 0, 0, 1, 0, 0,
    0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 1, 0, 0, 1, 0, 1};

byte invader2[176] = {
    0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 2, 1, 1, 1, 2, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1,
    0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0,

    0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
    1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1,
    1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0};

byte invader3[192] = {
    0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,

    0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};

// Explosions
int waitTimerExplosions;
int currentWaitTimerExplosions;
byte explosionPointer;
byte explosionX[16];
byte explosionY[16];
byte explosionStatus[16]; // 0 = No explosion, 1 - 5 = Animation phases

// Title
byte title[150] = {
    1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1,
    1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0,
    1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1,
    1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1};

#define RIGHT 0
#define DOWN 1
#define LEFT 2
#define UP 3

// Function declarations
void setup();

void setupGame_invaders();
void setupLevel_invaders();
void titleScreen();
void drawShields();
void moveFormation();
void dropBomb(byte _x, byte _y, float _drift);
void moveBombs();
void drawAlien(byte type, byte phase, byte index, byte x, byte y);
void removeAlien(byte x, byte y);
void moveMothership();
void moveBoss();
void drawBoss(byte type, byte phase, int x, int y, byte bossWidth, boolean draw);
void moveBase();
void fire();
void moveShot_invaders();
boolean columnEmpty(byte i);
byte getLowestRow();
boolean alienBelow(byte k);
void explosion_invaders(byte _x, byte _y);
void animateExplosions();
boolean checkForHit();
void gameOver();
void showLives(byte _lives);

void setup_Invaders()
{
  hardware_init();

  // Logos
  mithotronic();
  ledMePlay();

  // Initialize scores;
  highscore_invaders = 0;

  // Initialize difficulty;
  difficulty = 2;

  // Draw title screen
  titleScreen();

  // Setup a new game
  setupGame_invaders();
}

void titleScreen()
{
  // Delete screen
  matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));

  // Draw text "INVADERS"
  for (byte j = 0; j < 5; j++)
  {
    for (byte i = 0; i < 30; i++)
    {
      if (title[(j * 30) + i] == 1)
      {
        matrix.drawPixel(i + 1, j + 3, matrix.Color333(1, 1, 0));
      }
    }
  }

  // Draw shield
  drawShields();

  // Wait for fire button to be pressed
  int timer = 2000;
  do
  {
    // Draw boss invader
    if (timer == 2000)
    {
      drawBoss(1, 1, 12, 12, 8, true);
    }
    if (timer == 1000)
    {
      drawBoss(1, 2, 12, 12, 8, true);
    }

    timer--;
    if (timer == 0)
    {
      timer = 2000;
    }
    delay(1);
  } while (!joy1Fire());
  tone(audioPacer, 1024, 20);
  delay(100);

  // Delete screen
  matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));

  // Show highscore
  matrix.setCursor(1, 0);
  matrix.setTextColor(matrix.Color333(3, 3, 0));
  matrix.println("HiScr");
  matrix.setCursor(1, 8);
  matrix.println(highscore_invaders);
  delay(100);

  // Select difficulty
  int i = 0;
  do
  {
    matrix.setTextColor(matrix.Color888(i, i, 0));
    matrix.setCursor(1, 16);
    matrix.println("<Sel>");

    matrix.setTextColor(matrix.Color333(3, 3, 0));
    matrix.setCursor(4, 24);
    if (difficulty == 1)
    {
      matrix.println("Easy");
    }
    else if (difficulty == 2)
    {
      matrix.println("Norm");
    }
    else if (difficulty == 3)
    {
      matrix.println("Hard");
    }
    else
    {
      matrix.println("Ultr");
    }

    i = i + 2;
    if (i == 256)
    {
      i = 0;
    }

    if (joy1Left())
    {
      difficulty--;
      if (difficulty == 0)
      {
        difficulty = 4;
      }
      matrix.fillRect(0, 24, 32, 8, matrix.Color333(0, 0, 0));
      delay(200);
    }
    else if (joy1Right())
    {
      difficulty++;
      if (difficulty == 5)
      {
        difficulty = 1;
      }
      matrix.fillRect(0, 24, 32, 8, matrix.Color333(0, 0, 0));
      delay(200);
    }
  } while (!joy1Fire());
  tone(audioPacer, 1024, 20);
  delay(100);

  if (difficulty == 4)
  {
    // Delete screen
    matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));
    matrix.setTextColor(matrix.Color333(3, 3, 0));
    matrix.setCursor(1, 4);
    matrix.println("Sorry");
    matrix.setCursor(10, 12);
    matrix.println("no");
    matrix.setCursor(1, 20);
    matrix.println("cover");
    delay(2000);
  }
}

void setupGame_invaders()
{
  lives_invaders = 3;
  score_invaders = 0;
  extraLifeScore_invaders = 0;
  level_invaders = 1;
}

void setupLevel_invaders()
{
  for (byte i = 0; i < 32; i++)
  {
    for (byte j = 0; j < 32; j++)
    {
      playfield_invaders[i][j] = 0;
    }
  }

  // Delete screen
  matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));

  // Draw shields
  // Difficulty specific definition: No shield with difficulty "Ultra"
  if (difficulty < 4)
  {
    drawShields();
  }

  // Setup alien formation
  for (byte i = 0; i < formationWidth * formationHeight; i++)
  {
    if (level_invaders > 7)
    {
      alienType[i] = random(5) + 1;
    }
    else
    {
      alienType[i] = (formationHeight - (i / formationWidth) + level_invaders) / 2;
      if (alienType[i] > 5)
      {
        alienType[i] = 5;
      }
    }
    alienStatus[i] = 1;
  }
  formationX = 0;
  formationY = 4;
  formationLanded = false;
  movementStatus = 0;
  columnOffsetLeft = 0;
  columnOffsetRight = 0;
  animationPhase = 1;
  remainingAliens = formationWidth * formationHeight;

  // Every second wave contains an extra invader (boss)
  if (level_invaders % 2 == 0)
  {
    remainingAliens = remainingAliens + 1;
  }

  // Level specific definition: Formation speed
  waitTimerFormation = remainingAliens * (50 - (level_invaders - 1) * 2);
  if (waitTimerFormation < 10)
  {
    waitTimerFormation = 10;
  }

  // Setup mothership
  mothershipStatus = 0;
  mothershipRate = 10;

  // Level specific definition: Mothership speed
  waitTimerMothership = 160 - (level_invaders * 10);
  if (waitTimerMothership < 10)
  {
    waitTimerMothership = 10;
  }

  // Bosses (approaching at the end of every second wave)
  bossActive = false;

  // Level specific definition: Boss speed
  waitTimerBoss = 110 - (5 * level_invaders);
  if (waitTimerBoss < 10)
  {
    waitTimerBoss = 10;
  }

  // Level specific definition: Boss shield
  bossShield = 6 + (level_invaders * 2);

  // Initialize bombs
  for (byte i = 0; i < 16; i++)
  {
    bombStatus[i] = 0;
    bombX[i] = 0.0;
    bombY[i] = 0;
    bombDrift[i] = 0;
  }
  // Level specific definition: Bombing rate
  bombingRate = 165 + (level_invaders * 15);
  if (bombingRate > 800)
  {
    bombingRate = 800;
  }

  // Difficulty specific definition: Speed of bombs
  waitTimerBombs = 60 - (difficulty * 10);
  bombPointer = 0;

  // Initialize explosions
  for (byte i = 0; i < 16; i++)
  {
    explosionStatus[i] = 0;
    explosionX[i] = 0;
    explosionY[i] = 0;
  }
  waitTimerExplosions = 40;
  explosionPointer = 0;

  // Initialize player
  baseX = 14;
  baseDestroyed = false;
  waitTimerPlayer = 100;
  waitTimerPlayerShot = 20;
  extraLifeCoughtTimer = 0;
  playerShotStatus = 0;
}

// Draws the shields on the screen and into the playfield
void drawShields()
{
  for (byte i = 0; i < 4; i++)
  {
    // draw into playfield
    playfield_invaders[4 + i * 7][25] = 1;
    playfield_invaders[5 + i * 7][25] = 1;
    playfield_invaders[6 + i * 7][25] = 1;
    playfield_invaders[3 + i * 7][26] = 1;
    playfield_invaders[4 + i * 7][26] = 1;
    playfield_invaders[5 + i * 7][26] = 1;
    playfield_invaders[6 + i * 7][26] = 1;
    playfield_invaders[7 + i * 7][26] = 1;
    playfield_invaders[3 + i * 7][27] = 1;
    playfield_invaders[4 + i * 7][27] = 1;
    playfield_invaders[5 + i * 7][27] = 1;
    playfield_invaders[6 + i * 7][27] = 1;
    playfield_invaders[7 + i * 7][27] = 1;
    playfield_invaders[3 + i * 7][28] = 1;
    playfield_invaders[7 + i * 7][28] = 1;
  }

  // Draw shield animation on screen
  for (byte i = 0; i < 4; i++)
  {
    matrix.drawPixel(7 + i * 7, 28, matrix.Color333(0, 1, 0));
    matrix.drawPixel(3 + i * 7, 28, matrix.Color333(0, 1, 0));
  }
  delay(100);
  for (byte i = 0; i < 4; i++)
  {
    matrix.drawLine(3 + i * 7, 27, 7 + i * 7, 27, matrix.Color333(0, 3, 0));
  }
  delay(100);
  for (byte i = 0; i < 4; i++)
  {
    matrix.drawLine(3 + i * 7, 26, 7 + i * 7, 26, matrix.Color333(0, 4, 0));
  }
  delay(100);
  for (byte i = 0; i < 4; i++)
  {
    matrix.drawLine(4 + i * 7, 25, 6 + i * 7, 25, matrix.Color333(0, 5, 0));
  }
  delay(100);
}

// Moves the complete formation of aliens
void moveFormation()
{
  byte k = 0; // Alien number in formation
  // Iterate over all columns and lines of formation
  for (byte j = 0; j < formationHeight; j++)
  {
    for (byte i = 0; i < formationWidth; i++)
    {
      // Draw aliens which are active (not destroyed)
      if (alienStatus[k] == 1)
      {
        // Remove a specific alien
        removeAlien((i * 5) + formationX, (j * 4) + formationY);

        // Draw a specific alien at a new position (depending on current movement direction)
        if (baseDestroyed) // No movement after base has been destroyed
        {
          drawAlien(alienType[k], animationPhase, k, (i * 5) + formationX, (j * 4) + formationY);
        }
        else if (movementStatus == RIGHT) // formation moves right
        {
          drawAlien(alienType[k], animationPhase, k, (i * 5) + formationX + 1, (j * 4) + formationY);
          if (bombingRate > random(1000) && !alienBelow(k))
          {
            dropBomb((i * 5) + formationX + 2 + random(2), (j * 4) + formationY + 3, 0.0);
          }
        }
        else if (movementStatus == DOWN) // formation moves down
        {
          drawAlien(alienType[k], animationPhase, k, (i * 5) + formationX, (j * 4) + formationY + 1);
          if (bombingRate > random(1000) && !alienBelow(k))
          {
            dropBomb((i * 5) + formationX + 1 + random(2), (j * 4) + formationY + 4, 0.0);
          }
        }
        else if (movementStatus == LEFT) // formation moves left
        {
          drawAlien(alienType[k], animationPhase, k, (i * 5) + formationX - 1, (j * 4) + formationY);
          if (bombingRate > random(1000) && !alienBelow(k))
          {
            dropBomb((i * 5) + formationX + random(2), (j * 4) + formationY + 3, 0.0);
          }
        }
        else // formation moves up
        {
          drawAlien(alienType[k], animationPhase, k, (i * 5) + formationX, (j * 4) + formationY - 1);
          if (bombingRate > random(1000) && !alienBelow(k))
          {
            dropBomb((i * 5) + formationX + 1 + random(2), (j * 4) + formationY + 2, 0.0);
          }
        }
      }
      k++;
    }
  }
  // Update formation position after movement of all aliens
  if (baseDestroyed)
  {
    // No movement after base has been destroyed
  }
  else if (movementStatus == RIGHT)
  {
    formationX++;
    // Change movement direction to down
    if (formationX >= 32 - (formationWidth * 5) + 1 + (columnOffsetRight * 5))
    {
      movementStatus = DOWN;
    }
  }
  else if (movementStatus == LEFT)
  {
    formationX--;
    // Change movement direction to down
    if (formationX <= 0 - (columnOffsetLeft * 5))
    {
      movementStatus = DOWN;
    }
  }
  else if (movementStatus == DOWN)
  {
    formationY++;
    // Change movement direction to right
    if (formationX <= 0 - (columnOffsetLeft * 5))
    {
      movementStatus = RIGHT;
    }
    // Change movement direction to left
    else
    {
      movementStatus = LEFT;
    }
  }
  if (animationPhase == 1)
  {
    animationPhase = 2;
  }
  else
  {
    animationPhase = 1;
  }
}

// Alien drops a bomb
void dropBomb(byte _x, byte _y, float _drift)
{
  // After base has been destroyed, no new bomb is thrown
  if (bombStatus[bombPointer] == 0 && !baseDestroyed)
  {
    bombStatus[bombPointer] = 1;
    bombX[bombPointer] = float(_x);
    bombY[bombPointer] = _y;
    bombDrift[bombPointer] = _drift;
    bombPointer++;
    if (bombPointer == 16)
    {
      bombPointer = 0;
    }
    // SOUND: Drop bomb
    tone(audioPacer, 400, 20);
  }
}

// Move falling bombs
void moveBombs()
{
  // Iterate over all possible bombs
  for (byte i = 0; i < 16; i++)
  {
    // Move those bombs which are active
    if (bombStatus[i] == 1)
    {
      // Remove bomb at old position
      matrix.drawPixel(int(bombX[i]), bombY[i], matrix.Color333(0, 0, 0));
      playfield_invaders[int(bombX[i])][bombY[i]] = 0;

      // Move bomb
      bombY[i]++;

      // Bomb drift
      bombX[i] = float(bombX[i]) + float(bombDrift[i]);
      if (int(bombX[i]) < 0)
      {
        bombX[i] = 0.0;
      }
      if (int(bombX[i]) > 31)
      {
        bombX[i] = 31.0;
      }
      if (bombDrift[i] < 0.0)
      {
        bombDrift[i] = bombDrift[i] + 0.2;
      }
      else if (bombDrift[i] > 0.0)
      {
        bombDrift[i] = bombDrift[i] - 0.2;
      }

      // Bomb hits shield
      if (playfield_invaders[int(bombX[i])][bombY[i]] == 1)
      {
        bombStatus[i] = 0;

        // Remove part of shield
        matrix.drawPixel(int(bombX[i]), bombY[i], matrix.Color333(0, 0, 0));
        playfield_invaders[int(bombX[i])][bombY[i]] = 0;

        // SOUND: Shield hit
        tone(audioPacer, 200, 20);
      }
      if (bombY[i] == 32)
      {
        bombStatus[i] = 0;
      }

      // Draw bomb
      if (bombStatus[i] == 1)
      {
        matrix.drawPixel(int(bombX[i]), bombY[i], matrix.Color333(1, 1, 1));
        playfield_invaders[int(bombX[i])][bombY[i]] = 255;
      }
    }
  }
}

// Draws alien of a specific type with a specifc index in a defined movement phase at position (x, y)
void drawAlien(byte type, byte phase, byte index, byte x, byte y)
{
  if (type == 1)
  {
    matrix.drawPixel(x, y, matrix.Color333(1, 1, 1));
    matrix.drawPixel(x + 3, y, matrix.Color333(1, 1, 1));
    matrix.drawPixel(x, y + 1, matrix.Color333(3, 3, 3));
    matrix.drawPixel(x + 3, y + 1, matrix.Color333(3, 3, 3));
    matrix.drawPixel(x + 1, y + 2, matrix.Color333(1, 1, 1));
    matrix.drawPixel(x + 2, y + 2, matrix.Color333(1, 1, 1));
    playfield_invaders[x][y] = index + 4;
    playfield_invaders[x + 3][y] = index + 4;
    playfield_invaders[x][y + 1] = index + 4;
    playfield_invaders[x + 1][y + 1] = index + 4;
    playfield_invaders[x + 2][y + 1] = index + 4;
    playfield_invaders[x + 3][y + 1] = index + 4;
    playfield_invaders[x + 1][y + 2] = index + 4;
    playfield_invaders[x + 2][y + 2] = index + 4;
    if (phase == 1)
    {
      matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 0, 7));
      matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 0, 1));
    }
    else
    {
      matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 0, 1));
      matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 0, 7));
    }
  }
  else if (type == 2)
  {
    matrix.drawPixel(x + 1, y, matrix.Color333(0, 0, 3));
    matrix.drawPixel(x + 2, y, matrix.Color333(0, 0, 3));
    matrix.drawPixel(x + 1, y + 1, matrix.Color333(3, 0, 0));
    matrix.drawPixel(x + 2, y + 1, matrix.Color333(3, 0, 0));
    playfield_invaders[x + 1][y] = index + 4;
    playfield_invaders[x + 2][y] = index + 4;
    playfield_invaders[x + 1][y + 1] = index + 4;
    playfield_invaders[x + 2][y + 1] = index + 4;
    if (phase == 1)
    {
      matrix.drawPixel(x, y + 1, matrix.Color333(1, 0, 0));
      matrix.drawPixel(x + 3, y + 1, matrix.Color333(1, 0, 0));
      playfield_invaders[x][y + 1] = index + 4;
      playfield_invaders[x + 3][y + 1] = index + 4;
    }
    else
    {
      matrix.drawPixel(x, y + 2, matrix.Color333(1, 0, 0));
      matrix.drawPixel(x + 3, y + 2, matrix.Color333(1, 0, 0));
      playfield_invaders[x][y + 2] = index + 4;
      playfield_invaders[x + 3][y + 2] = index + 4;
    }
  }
  else if (type == 3)
  {
    matrix.drawPixel(x + 1, y, matrix.Color333(0, 1, 1));
    matrix.drawPixel(x + 2, y, matrix.Color333(0, 1, 1));
    matrix.drawPixel(x + 1, y + 2, matrix.Color333(0, 1, 1));
    matrix.drawPixel(x + 2, y + 2, matrix.Color333(0, 1, 1));
    playfield_invaders[x + 1][y] = index + 4;
    playfield_invaders[x + 2][y] = index + 4;
    playfield_invaders[x][y + 1] = index + 4;
    playfield_invaders[x + 1][y + 1] = index + 4;
    playfield_invaders[x + 2][y + 1] = index + 4;
    playfield_invaders[x + 3][y + 1] = index + 4;
    playfield_invaders[x + 1][y + 2] = index + 4;
    playfield_invaders[x + 2][y + 2] = index + 4;
    if (phase == 1)
    {
      matrix.drawPixel(x, y + 1, matrix.Color333(0, 1, 1));
      matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 0, 3));
      matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 0, 3));
      matrix.drawPixel(x + 3, y + 1, matrix.Color333(0, 1, 1));
    }
    else
    {
      matrix.drawPixel(x, y + 1, matrix.Color333(0, 0, 3));
      matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 1, 1));
      matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 1, 1));
      matrix.drawPixel(x + 3, y + 1, matrix.Color333(0, 0, 3));
    }
  }
  else if (type == 4)
  {
    matrix.drawPixel(x, y, matrix.Color333(1, 1, 0));
    matrix.drawPixel(x + 1, y, matrix.Color333(3, 3, 0));
    matrix.drawPixel(x + 2, y, matrix.Color333(3, 3, 0));
    matrix.drawPixel(x + 3, y, matrix.Color333(1, 1, 0));
    playfield_invaders[x][y] = index + 4;
    playfield_invaders[x + 1][y] = index + 4;
    playfield_invaders[x + 2][y] = index + 4;
    playfield_invaders[x + 3][y] = index + 4;
    if (phase == 1)
    {
      matrix.drawPixel(x, y + 1, matrix.Color333(1, 1, 0));
      matrix.drawPixel(x + 1, y + 1, matrix.Color333(3, 3, 0));
      matrix.drawPixel(x + 2, y + 1, matrix.Color333(3, 3, 0));
      matrix.drawPixel(x + 3, y + 1, matrix.Color333(1, 1, 0));
      playfield_invaders[x][y + 1] = index + 4;
      playfield_invaders[x + 1][y + 1] = index + 4;
      playfield_invaders[x + 2][y + 1] = index + 4;
      playfield_invaders[x + 3][y + 1] = index + 4;
    }
    else
    {
      matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 0, 3));
      matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 0, 3));
      matrix.drawPixel(x, y + 2, matrix.Color333(1, 1, 0));
      matrix.drawPixel(x + 1, y + 2, matrix.Color333(3, 3, 0));
      matrix.drawPixel(x + 2, y + 2, matrix.Color333(3, 3, 0));
      matrix.drawPixel(x + 3, y + 2, matrix.Color333(1, 1, 0));
      playfield_invaders[x + 1][y + 1] = index + 4;
      playfield_invaders[x + 2][y + 1] = index + 4;
      playfield_invaders[x][y + 2] = index + 4;
      playfield_invaders[x + 1][y + 2] = index + 4;
      playfield_invaders[x + 2][y + 2] = index + 4;
      playfield_invaders[x + 3][y + 2] = index + 4;
    }
  }
  else
  {
    matrix.drawPixel(x, y, matrix.Color333(1, 0, 1));
    matrix.drawPixel(x + 1, y, matrix.Color333(3, 0, 3));
    matrix.drawPixel(x + 2, y, matrix.Color333(3, 0, 3));
    matrix.drawPixel(x + 3, y, matrix.Color333(1, 0, 1));
    matrix.drawPixel(x, y + 1, matrix.Color333(3, 0, 3));
    matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 0, 3));
    matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 0, 3));
    matrix.drawPixel(x + 3, y + 1, matrix.Color333(3, 0, 3));
    playfield_invaders[x][y] = index + 4;
    playfield_invaders[x + 1][y] = index + 4;
    playfield_invaders[x + 2][y] = index + 4;
    playfield_invaders[x + 3][y] = index + 4;
    playfield_invaders[x][y + 1] = index + 4;
    playfield_invaders[x + 1][y + 1] = index + 4;
    playfield_invaders[x + 2][y + 1] = index + 4;
    playfield_invaders[x + 3][y + 1] = index + 4;
    if (phase == 1)
    {
      matrix.drawPixel(x + 1, y + 2, matrix.Color333(1, 0, 1));
      matrix.drawPixel(x + 2, y + 2, matrix.Color333(1, 0, 1));
      playfield_invaders[x + 1][y + 2] = index + 4;
      playfield_invaders[x + 2][y + 2] = index + 4;
    }
    else
    {
      matrix.drawPixel(x, y + 2, matrix.Color333(1, 0, 1));
      matrix.drawPixel(x + 3, y + 2, matrix.Color333(1, 0, 1));
      playfield_invaders[x][y + 2] = index + 4;
      playfield_invaders[x + 3][y + 2] = index + 4;
    }
  }
}

void removeAlien(byte x, byte y)
{
  matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 1, y, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 2, y, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 3, y, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x, y + 1, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 1, y + 1, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 2, y + 1, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 3, y + 1, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x, y + 2, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 1, y + 2, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 2, y + 2, matrix.Color333(0, 0, 0));
  matrix.drawPixel(x + 3, y + 2, matrix.Color333(0, 0, 0));
  playfield_invaders[x][y] = 0;
  playfield_invaders[x + 1][y] = 0;
  playfield_invaders[x + 2][y] = 0;
  playfield_invaders[x + 3][y] = 0;
  playfield_invaders[x][y + 1] = 0;
  playfield_invaders[x + 1][y + 1] = 0;
  playfield_invaders[x + 2][y + 1] = 0;
  playfield_invaders[x + 3][y + 1] = 0;
  playfield_invaders[x][y + 2] = 0;
  playfield_invaders[x + 1][y + 2] = 0;
  playfield_invaders[x + 2][y + 2] = 0;
  playfield_invaders[x + 3][y + 2] = 0;
}

// Moves the mothership;
void moveMothership()
{
  // Mothership appears randomly
  if (mothershipStatus == 0 && !bossActive)
  {
    if (mothershipRate > random(1000))
    {
      mothershipStatus = 1;
      mothershipX = -5;
    }
  }
  if (mothershipStatus > 0)
  {
    // Remove mothership at old position
    matrix.drawPixel(mothershipX + 2, 0, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 3, 0, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 4, 0, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 1, 1, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 2, 1, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 3, 1, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 4, 1, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 5, 1, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX, 2, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 1, 2, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 3, 2, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 5, 2, matrix.Color333(0, 0, 0));
    matrix.drawPixel(mothershipX + 6, 2, matrix.Color333(0, 0, 0));
    if (mothershipX >= 0 && mothershipX < 32)
    {
      playfield_invaders[mothershipX][2] = 0;
    }
    if (mothershipX >= -1 && mothershipX < 31)
    {
      playfield_invaders[mothershipX + 1][2] = 0;
      playfield_invaders[mothershipX + 1][1] = 0;
    }
    if (mothershipX >= -2 && mothershipX < 30)
    {
      playfield_invaders[mothershipX + 2][0] = 0;
      playfield_invaders[mothershipX + 2][1] = 0;
    }
    if (mothershipX >= -3 && mothershipX < 29)
    {
      playfield_invaders[mothershipX + 3][0] = 0;
      playfield_invaders[mothershipX + 3][1] = 0;
      playfield_invaders[mothershipX + 3][2] = 0;
    }
    if (mothershipX >= -4 && mothershipX < 28)
    {
      playfield_invaders[mothershipX + 4][0] = 0;
      playfield_invaders[mothershipX + 4][1] = 0;
    }
    if (mothershipX >= -5 && mothershipX < 27)
    {
      playfield_invaders[mothershipX + 5][1] = 0;
      playfield_invaders[mothershipX + 5][2] = 0;
    }
    if (mothershipX >= -6 && mothershipX < 26)
    {
      playfield_invaders[mothershipX + 6][2] = 0;
    }

    // Move mothership
    mothershipX++;
    if (mothershipX > 31 || mothershipStatus == 2)
    {
      mothershipStatus = 0;
    }

    // Draw mothership at new position
    if (mothershipStatus == 1)
    {
      matrix.drawPixel(mothershipX + 2, 0, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 3, 0, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 4, 0, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 2, 1, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 4, 1, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX, 2, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 1, 2, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 3, 2, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 5, 2, matrix.Color333(1, 1, 1));
      matrix.drawPixel(mothershipX + 6, 2, matrix.Color333(1, 1, 1));
      if (animationPhase == 1)
      {
        matrix.drawPixel(mothershipX + 1, 1, matrix.Color333(0, 0, 7));
        matrix.drawPixel(mothershipX + 3, 1, matrix.Color333(0, 0, 1));
        matrix.drawPixel(mothershipX + 5, 1, matrix.Color333(0, 0, 7));
      }
      else
      {
        matrix.drawPixel(mothershipX + 1, 1, matrix.Color333(0, 0, 1));
        matrix.drawPixel(mothershipX + 3, 1, matrix.Color333(0, 0, 7));
        matrix.drawPixel(mothershipX + 5, 1, matrix.Color333(0, 0, 1));
      }

      if (mothershipX >= 0 && mothershipX < 32)
      {
        playfield_invaders[mothershipX][2] = 2;
      }
      if (mothershipX >= -1 && mothershipX < 31)
      {
        playfield_invaders[mothershipX + 1][2] = 2;
        playfield_invaders[mothershipX + 1][1] = 2;
      }
      if (mothershipX >= -2 && mothershipX < 30)
      {
        playfield_invaders[mothershipX + 2][0] = 2;
        playfield_invaders[mothershipX + 2][1] = 2;
      }
      if (mothershipX >= -3 && mothershipX < 29)
      {
        playfield_invaders[mothershipX + 3][0] = 2;
        playfield_invaders[mothershipX + 3][1] = 2;
        playfield_invaders[mothershipX + 3][2] = 2;
      }
      if (mothershipX >= -4 && mothershipX < 28)
      {
        playfield_invaders[mothershipX + 4][0] = 2;
        playfield_invaders[mothershipX + 4][1] = 2;
      }
      if (mothershipX >= -5 && mothershipX < 27)
      {
        playfield_invaders[mothershipX + 5][1] = 2;
        playfield_invaders[mothershipX + 5][2] = 2;
      }
      if (mothershipX >= -6 && mothershipX < 26)
      {
        playfield_invaders[mothershipX + 6][2] = 2;
      }
    }
  }
}

// Move boss
void moveBoss()
{
  // Remove boss at old position
  matrix.fillRect(bossX, bossY, bossWidth, 8, matrix.Color333(0, 0, 0));
  for (byte j = 0; j < 8; j++)
  {
    for (byte i = 0; i < bossWidth; i++)
    {
      playfield_invaders[bossX + i][bossY + j] = 0;
    }
  }

  // Move boss
  if (bossType == 1)
  {
    if (bossMovementStatus == RIGHT)
    {
      if (bossX == 32 - bossWidth)
      {
        bossMovementStatus = DOWN;
      }
      else
      {
        bossX++;
      }
    }
    else if (bossMovementStatus == LEFT)
    {
      if (bossX == 0)
      {
        bossMovementStatus = DOWN;
      }
      else
      {
        bossX--;
      }
    }
    else if (bossMovementStatus == DOWN)
    {
      if (bossY == 17)
      {
        bossMovementStatus = UP;
      }
      else
      {
        if (bossX == 0)
        {
          bossY++;
          if (bossY % 2 == 0)
          {
            bossMovementStatus = RIGHT;
          }
        }
        else
        {
          bossY++;
          if (bossY % 2 == 0)
          {
            bossMovementStatus = LEFT;
          }
        }
      }
    }
    else if (bossMovementStatus == UP)
    {
      if (bossY == 3)
      {
        bossMovementStatus = RIGHT;
      }
      else
      {
        bossY--;
      }
    }
  }
  else if (bossType == 2)
  {
    if (bossMovementStatus == RIGHT)
    {
      if (bossX == 32 - bossWidth)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        bossX++;
      }
    }
    else if (bossMovementStatus == DOWN)
    {
      if (bossY == 13)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        bossY++;
      }
    }
    else if (bossMovementStatus == LEFT)
    {
      if (bossX == 0)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        bossX--;
      }
    }
    else if (bossMovementStatus == UP)
    {
      if (bossY == 3)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        bossY--;
      }
    }
  }
  else
  {
    if (bossMovementStatus == RIGHT)
    {
      if (bossX == 32 - bossWidth)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        if (random(20) < 1)
        {
          bossMovementStatus = random(4);
        }
        else
        {
          bossX++;
        }
      }
    }
    else if (bossMovementStatus == DOWN)
    {
      if (bossY == 17)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        if (random(20) < 1)
        {
          bossMovementStatus = random(4);
        }
        else
        {
          bossY++;
        }
      }
    }
    else if (bossMovementStatus == LEFT)
    {
      if (bossX == 0)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        if (random(20) < 1)
        {
          bossMovementStatus = random(4);
        }
        else
        {
          bossX--;
        }
      }
    }
    else if (bossMovementStatus == UP)
    {
      if (bossY == 3)
      {
        bossMovementStatus = random(4);
      }
      else
      {
        if (random(20) < 1)
        {
          bossMovementStatus = random(4);
        }
        else
        {
          bossY--;
        }
      }
    }
  }
  // Drop bombs
  if (bombingRate > random(1000))
  {
    dropBomb(bossX + 3 + random(2), bossY + 6, (random(1 + (2 * level_invaders)) - level_invaders) / level_invaders);
  }

  // Draw boss at new position
  drawBoss(bossType, animationPhase, bossX, bossY, bossWidth, true);

  // Update animation phase
  if (animationPhase == 1)
  {
    animationPhase = 2;
  }
  else
  {
    animationPhase = 1;
  }
}

// Draw boss
void drawBoss(byte type, byte phase, int x, int y, byte bossWidth, boolean draw)
{
  byte pixel;
  for (byte j = 0; j < 8; j++)
  {
    for (byte i = 0; i < bossWidth; i++)
    {
      // Draw invader picture depending on type and phase
      if (type == 1)
      {
        pixel = invader1[(j * bossWidth) + i + (phase - 1) * 64];
      }
      else if (type == 2)
      {
        pixel = invader2[(j * bossWidth) + i + (phase - 1) * 88];
      }
      else if (type == 3)
      {
        pixel = invader3[(j * bossWidth) + i + (phase - 1) * 96];
      }

      if (draw)
      {
        if (pixel == 0)
        {
          matrix.drawPixel(x + i, y + j, matrix.Color333(0, 0, 0));
          playfield_invaders[x + i][y + j] = 0;
        }
        else if (pixel == 1)
        {
          matrix.drawPixel(x + i, y + j, matrix.Color333(1, 1, 1));
          playfield_invaders[x + i][y + j] = 3;
        }
        else if (pixel == 2)
        {
          matrix.drawPixel(x + i, y + j, matrix.Color333(0, 0, 7));
          playfield_invaders[x + i][y + j] = 3;
        }
      }
      else
      {
        if (pixel > 0)
        {
          matrix.drawPixel(x + i, y + j, matrix.Color333(0, 0, 0));
          playfield_invaders[x + i][y + j] = 0;
        }
      }
    }
  }
}

// Moves the base (player action)
void moveBase()
{
  // Move to the left
  if (joy1Left() && baseX > 0)
  {
    // Remove base at old postion
    matrix.drawPixel(baseX + 1, 30, matrix.Color333(0, 0, 0));
    matrix.drawPixel(baseX, 31, matrix.Color333(0, 0, 0));
    matrix.drawPixel(baseX + 1, 31, matrix.Color333(0, 0, 0));
    matrix.drawPixel(baseX + 2, 31, matrix.Color333(0, 0, 0));
    baseX--;
  }
  // Move to the right
  else if (joy1Right() && baseX < 29)
  {
    // Remove base at old postion
    matrix.drawPixel(baseX + 1, 30, matrix.Color333(0, 0, 0));
    matrix.drawPixel(baseX, 31, matrix.Color333(0, 0, 0));
    matrix.drawPixel(baseX + 1, 31, matrix.Color333(0, 0, 0));
    matrix.drawPixel(baseX + 2, 31, matrix.Color333(0, 0, 0));
    baseX++;
  }

  // Draw base at new position
  matrix.drawPixel(baseX + 1, 30, matrix.Color333(1, 1, 1));
  matrix.drawPixel(baseX, 31, matrix.Color333(1, 1, 1));
  matrix.drawPixel(baseX + 1, 31, matrix.Color333(3, 3, 3));
  matrix.drawPixel(baseX + 2, 31, matrix.Color333(1, 1, 1));
}

// Fires a new shot (player action)
void fire()
{
  if (joy1Fire() && playerShotStatus == 0)
  {
    playerShotStatus = 1;
    playerShotX = baseX + 1;
    playerShotY = 29;
    matrix.drawPixel(playerShotX, playerShotY, matrix.Color333(7, 7, 7));
    currentWaitTimerPlayerShot = waitTimerPlayerShot;

    // SOUND: Fire
    tone(audioPacer, 300, 20);
  }
}

// Moves a fired shot and checks whether it hits something
void moveShot_invaders()
{
  // Remove shot at old position
  matrix.drawPixel(playerShotX, playerShotY, matrix.Color333(0, 0, 0));

  // Move shot
  playerShotY--;
  if (playerShotY == 255)
  {
    playerShotStatus = 0;
  }

  if (playerShotStatus == 1)
  {
    // Check for hit (checkForHit method procedure manages everything else)
    if (checkForHit())
    {
      // Inactivates shot after hit (not drawn again)
      playerShotStatus = 0;
    }
    else
    {
      // Draw shot at new position;
      matrix.drawPixel(playerShotX, playerShotY, matrix.Color333(7, 7, 7));
    }
  }
}

// Returns true if all aliens in column i of the formation have been destroyed
// (required to update left and right movement limits)
boolean columnEmpty(byte i)
{
  for (byte j = 0; j < formationHeight; j++)
  {
    if (alienStatus[(j * formationWidth) + i] != 0)
    {
      return false;
    }
  }
  return true;
}

// Returns lowest row which is not empty
byte getLowestRow()
{
  for (int j = formationHeight - 1; j > -1; j--)
  {
    for (int i = 0; i < formationWidth; i++)
    {
      if (alienStatus[(j * formationWidth) + i] != 0)
      {
        return j;
      }
    }
  }
  return 0;
}

// Returns true if there are aliens below alien k (so that k cannot drop a bomb)
boolean alienBelow(byte k)
{
  byte index = k + formationWidth;
  while (index < formationWidth * formationHeight)
  {
    if (alienStatus[index] != 0)
    {
      return true;
    }
    index = index + formationWidth;
  };
  return false;
}

// Creates a new explosion at position (_x, _y)
void explosion_invaders(byte _x, byte _y)
{
  // Check if a new explosion is possible (according to resources)
  if (explosionStatus[explosionPointer] == 0)
  {
    // Define new explosion
    explosionStatus[explosionPointer] = 1;
    explosionX[explosionPointer] = _x;
    explosionY[explosionPointer] = _y;
    explosionPointer++;

    // Update explosion pointer
    if (explosionPointer == 16)
    {
      explosionPointer = 0;
    }
  }
}

// Animates all existing explosion and
void animateExplosions()
{
  // Iterate over all possible explosion
  for (byte i = 0; i < 16; i++)
  {
    // Animate explosion if in an animation phase (explosionStatus[i] between 1 and 5)
    if (explosionStatus[i] > 0)
    {
      if (explosionStatus[i] == 1)
      {
        matrix.drawPixel(explosionX[i] + 1, explosionY[i] + 1, matrix.Color333(7, 7, 3));
        matrix.drawPixel(explosionX[i] + 2, explosionY[i] + 1, matrix.Color333(7, 7, 3));
        // SOUND: Explosion
        tone(audioPacer, 800, 40);
      }
      else if (explosionStatus[i] == 2)
      {
        matrix.drawPixel(explosionX[i] + 1, explosionY[i], matrix.Color333(3, 3, 0));
        matrix.drawPixel(explosionX[i] + 2, explosionY[i], matrix.Color333(3, 3, 0));
        matrix.drawPixel(explosionX[i], explosionY[i] + 1, matrix.Color333(3, 3, 0));
        matrix.drawPixel(explosionX[i] + 1, explosionY[i] + 1, matrix.Color333(3, 1, 0));
        matrix.drawPixel(explosionX[i] + 2, explosionY[i] + 1, matrix.Color333(3, 1, 0));
        matrix.drawPixel(explosionX[i] + 3, explosionY[i] + 1, matrix.Color333(3, 3, 0));
        matrix.drawPixel(explosionX[i] + 1, explosionY[i] + 2, matrix.Color333(3, 3, 0));
        matrix.drawPixel(explosionX[i] + 2, explosionY[i] + 2, matrix.Color333(3, 3, 0));
        // SOUND: Explosion
        tone(audioPacer, 400, 40);
      }
      else if (explosionStatus[i] == 3)
      {
        matrix.drawPixel(explosionX[i] + 1, explosionY[i] + 1, matrix.Color333(0, 0, 0));
        matrix.drawPixel(explosionX[i] + 2, explosionY[i] + 1, matrix.Color333(0, 0, 0));
        // SOUND: Explosion
        tone(audioPacer, 200, 40);
      }
      else if (explosionStatus[i] == 4)
      {
        matrix.fillRect(explosionX[i], explosionY[i], 4, 3, matrix.Color333(0, 0, 0));
        matrix.drawPixel(explosionX[i], explosionY[i], matrix.Color333(1, 1, 0));
        matrix.drawPixel(explosionX[i] + 3, explosionY[i], matrix.Color333(1, 1, 0));
        matrix.drawPixel(explosionX[i], explosionY[i] + 2, matrix.Color333(1, 1, 0));
        matrix.drawPixel(explosionX[i] + 3, explosionY[i] + 2, matrix.Color333(1, 1, 0));
        // SOUND: Explosion
        tone(audioPacer, 100, 40);
      }
      else if (explosionStatus[i] == 5)
      {
        matrix.fillRect(explosionX[i], explosionY[i], 4, 3, matrix.Color333(0, 0, 0));
        // SOUND: Explosion
        tone(audioPacer, 50, 80);
      }
      explosionStatus[i]++;

      // Explosion ends
      if (explosionStatus[i] == 6)
      {
        explosionStatus[i] = 0;
      }
    }
  }
}

// Returns true if an alien from the formation has been hit
// If so, a destroyed alien is removed and left and right column of the formation are updated
boolean checkForHit()
{
  // Check if there is a hit (bombs cannot be hit)
  if (playfield_invaders[playerShotX][playerShotY] > 0 && playfield_invaders[playerShotX][playerShotY] != 255)
  {
    // SOUND: Hit
    tone(audioPacer, 200, 40);

    // Shield hit
    if (playfield_invaders[playerShotX][playerShotY] == 1)
    {
      playfield_invaders[playerShotX][playerShotY] = 0;
      matrix.drawPixel(playerShotX, playerShotY, matrix.Color333(0, 0, 0));
    }
    // Mothership hit
    else if (playfield_invaders[playerShotX][playerShotY] == 2)
    {
      score_invaders = score_invaders + 200;
      extraLifeScore_invaders = extraLifeScore_invaders + 200;
      mothershipStatus = 2;

      // Mothership explosion;
      explosion_invaders(mothershipX - 1, 0);
      explosion_invaders(mothershipX + 1, 0);
      explosion_invaders(mothershipX + 2, 0);
      explosion_invaders(mothershipX + 4, 0);
    }
    // Boss hit
    else if (playfield_invaders[playerShotX][playerShotY] == 3)
    {
      bossShield--;
      if (bossShield == 0)
      {
        score_invaders = score_invaders + 500;
        extraLifeScore_invaders = extraLifeScore_invaders + 500;
        currentWaitTimerBoss = 1000;

        // Remove boss
        matrix.fillRect(bossX, bossY, bossWidth, 8, matrix.Color333(0, 0, 0));
        for (byte j = 0; j < 8; j++)
        {
          for (byte i = 0; i < bossWidth; i++)
          {
            playfield_invaders[bossX + i][bossY + j] = 0;
          }
        }

        // Boss explosion
        for (byte i = 0; i < 16; i++)
        {
          explosion_invaders(bossX + random(bossWidth - 3), bossY + random(6));
        }
      }
    }
    // Alien hit
    else
    {
      // Get the hit alien and its position in formation
      byte alienIndex = playfield_invaders[playerShotX][playerShotY] - 4;
      alienStatus[alienIndex] = 0;
      byte i = alienIndex % formationWidth;
      byte j = alienIndex / formationWidth;

      // Remove the alien from screen
      removeAlien((i * 5) + formationX, (j * 4) + formationY);
      waitTimerFormation = waitTimerFormation - (50 - (level_invaders - 1) * 2); // Formation speeds up with every loss
      if (waitTimerFormation < 0)
      {
        waitTimerFormation = 0;
      }
      remainingAliens--;

      // Update score
      switch (alienType[alienIndex])
      {
      case 1:
        score_invaders = score_invaders + 10;
        extraLifeScore_invaders = extraLifeScore_invaders + 10;
        break;
      case 2:
        score_invaders = score_invaders + 20;
        extraLifeScore_invaders = extraLifeScore_invaders + 20;
        break;
      case 3:
        score_invaders = score_invaders + 30;
        extraLifeScore_invaders = extraLifeScore_invaders + 30;
        break;
      case 4:
        score_invaders = score_invaders + 40;
        extraLifeScore_invaders = extraLifeScore_invaders + 40;
        break;
      case 5:
        score_invaders = score_invaders + 50;
        extraLifeScore_invaders = extraLifeScore_invaders + 50;
        break;
      default:
        break;
      }

      // Create explosion
      explosion_invaders((i * 5) + formationX, (j * 4) + formationY);

      // Update left and right column of formation in case the most left or most right column has been destroyed completely
      byte k;
      k = i;
      while (columnEmpty(k))
      {
        if (k == columnOffsetLeft)
        {
          columnOffsetLeft++;
          k++;
        }
        else
        {
          break;
        }
      };
      k = i;
      while (columnEmpty(k))
      {
        if (k == formationWidth - 1 - columnOffsetRight)
        {
          columnOffsetRight++;
          k--;
        }
        else
        {
          break;
        }
      };
    }
    return true;
  }
  else
  {
    return false;
  }
}

// Show the Game Over screen
void gameOver()
{
  // Game over
  matrix.fillRect(0, 0, 32, 25, matrix.Color333(0, 0, 0));
  for (int i = 0; i < 50; i++)
  {
    matrix.setCursor(4, 4);
    matrix.setTextColor(matrix.Color333(7 * (i % 2), 7 * (i % 2), 0));
    matrix.println("Game");
    matrix.setCursor(4, 12);
    matrix.println("Over");

    // SOUND: Game Over
    tone(audioPacer, i * 10, 5);
    delay(10);
  }
  matrix.setCursor(4, 4);
  matrix.setTextColor(matrix.Color333(3, 3, 0));
  matrix.println("Game");
  matrix.setCursor(4, 12);
  matrix.println("Over");
  delay(3000);

  // Check for new highscore
  if (score_invaders > highscore_invaders)
  {
    highscore_invaders = score_invaders;
    matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));
    for (int i = 0; i < 50; i++)
    {
      matrix.setCursor(7, 4);
      matrix.setTextColor(matrix.Color333(7 * (i % 2), 7 * (i % 2), 0));
      matrix.println("New");
      matrix.setCursor(1, 12);
      matrix.println("HiScr");

      // SOUND: New High Score
      tone(audioPacer, 500 + i * 40, 5);
      delay(10);
    }
    matrix.setCursor(7, 4);
    matrix.setTextColor(matrix.Color333(3, 3, 0));
    matrix.println("New");
    matrix.setCursor(1, 12);
    matrix.println("HiScr");
    delay(3000);
  }

  // Show final score
  matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));
  matrix.setCursor(1, 4);
  matrix.setTextColor(matrix.Color333(3, 3, 0));
  matrix.println("Score");
  matrix.setCursor(1, 12);
  matrix.println(score_invaders);
  delay(2000);
}

// Show the number of remaining bases at the bottom of the screen
void showLives(byte _lives)
{
  matrix.fillRect(0, 30, 32, 2, matrix.Color333(0, 0, 0));
  for (byte i = 0; i < _lives; i++)
  {
    if (i < 4)
    {
      matrix.drawPixel(4 + (i * 7) + 1, 30, matrix.Color333(1, 1, 0));
      matrix.drawPixel(4 + (i * 7), 31, matrix.Color333(1, 1, 0));
      matrix.drawPixel(4 + (i * 7) + 1, 31, matrix.Color333(3, 3, 0));
      matrix.drawPixel(4 + (i * 7) + 2, 31, matrix.Color333(1, 1, 0));
    }
  }
}

void loop_Invaders()
{

  // Loop over levels
  do
  {
    Serial.println("Start loop");
    // Initialize all variables for a new level
    setupLevel_invaders();

    // Show score
    matrix.setCursor(1, 4);
    matrix.setTextColor(matrix.Color333(3, 3, 0));
    matrix.println("Score");
    matrix.setCursor(1, 12);
    matrix.println(score_invaders);
    delay(2000);

    // Clear screen from score
    matrix.fillRect(0, 0, 32, 24, matrix.Color333(0, 0, 0));

    // Show level
    matrix.setCursor(4, 4);
    matrix.setTextColor(matrix.Color333(3, 3, 0));
    matrix.println("Wave");
    matrix.setCursor(13, 12);
    matrix.println(level_invaders);

    // Show remaining bases
    showLives(lives_invaders);
    delay(2000);

    // Clear screen from level and base visualization
    matrix.fillRect(0, 30, 32, 2, matrix.Color333(0, 0, 0));
    matrix.fillRect(0, 4, 32, 16, matrix.Color333(0, 0, 0));

    // Set timers
    currentWaitTimerBombs = waitTimerBombs;
    currentWaitTimerExplosions = waitTimerExplosions;
    currentWaitTimerFormation = waitTimerFormation;
    currentWaitTimerMothership = waitTimerMothership;
    currentWaitTimerBoss = waitTimerBoss;
    currentWaitTimerPlayerShot = waitTimerPlayerShot;
    currentWaitTimerPlayer = waitTimerPlayer;

    // Main game loop
    do
    {
      // Serial.println("Start Game-loop");

      // Move falling bombs immediately after timer has elapsed
      currentWaitTimerBombs--;
      if (currentWaitTimerBombs == 0)
      {
        moveBombs();
        currentWaitTimerBombs = waitTimerBombs;
      }

      // Animate explosions immediately after timer has elapsed
      currentWaitTimerExplosions--;
      if (currentWaitTimerExplosions == 0)
      {
        animateExplosions();
        currentWaitTimerExplosions = waitTimerExplosions;
      }

      // Move formation immediately after timer has elapsed
      currentWaitTimerFormation--;
      if (currentWaitTimerFormation == 0)
      {
        // SOUND: Move formation (only if formation is moving)
        if (!baseDestroyed && !bossActive)
        {
          tone(audioPacer, 50 + (animationPhase * 50), 40);
        }

        moveFormation();
        currentWaitTimerFormation = waitTimerFormation;
      }

      // Move mothership immediately after timer has elapsed
      currentWaitTimerMothership--;
      if (currentWaitTimerMothership == 0)
      {
        // SOUND: Move mothership (only if formation is moving)
        if (mothershipStatus != 0 && !baseDestroyed && !bossActive)
        {
          tone(audioPacer, 200 + (animationPhase * 200), 40);
        }

        moveMothership();
        currentWaitTimerMothership = waitTimerMothership;
      }

      // Move boss immediately after timer has elapsed
      if (bossActive)
      {
        currentWaitTimerBoss--;
        if (currentWaitTimerBoss == 0)
        {
          // SOUND: Move formation (only if formation is moving)
          if (!baseDestroyed)
          {
            tone(audioPacer, 100 + (animationPhase * 100), 40);
          }
          if (bossShield > 0)
          {
            moveBoss();
          }
          else
          {
            bossActive = false;
            remainingAliens--;
          }
          currentWaitTimerBoss = waitTimerBoss;
        }
      }

      // Move shots by player immediately after timer has elapsed
      currentWaitTimerPlayerShot--;
      if (currentWaitTimerPlayerShot == 0)
      {
        if (playerShotStatus == 1)
        {
          moveShot_invaders();
        }
        currentWaitTimerPlayerShot = waitTimerPlayerShot;
      }

      // Allow player actions after timer has elapsed
      if (currentWaitTimerPlayer > 0)
      {
        currentWaitTimerPlayer--;
      }
      if (currentWaitTimerPlayer == 0)
      {
        if (!baseDestroyed)
        {
          moveBase();
          fire();
        }
        else
        {
          // Procedure after a hit
          waitCounterBaseDestroyed--;
          if (waitCounterBaseDestroyed == 40)
          {
            // Show remaining lives
            showLives(lives_invaders - 1);
          }
          else if (waitCounterBaseDestroyed == 20)
          {
            // Remove display of remaining lives
            matrix.fillRect(0, 30, 32, 2, matrix.Color333(0, 0, 0));
          }
          else if (waitCounterBaseDestroyed < 1)
          {
            // Decrease number of lives and go back to normal mode
            lives_invaders--;
            baseDestroyed = false;
            baseX = 14;
          }
        }
        currentWaitTimerPlayer = waitTimerPlayer;

        // Show extra life
        if (extraLifeCoughtTimer > 0)
        {
          if (extraLifeCoughtTimer > 1)
          {
            matrix.drawPixel(15, 1, matrix.Color333(1, 1, 0));
            matrix.drawPixel(14, 2, matrix.Color333(1, 1, 0));
            matrix.drawPixel(15, 2, matrix.Color333(3, 3, 0));
            matrix.drawPixel(16, 2, matrix.Color333(1, 1, 0));

            // SOUND: Extra life
            tone(audioPacer, 100 + (extraLifeCoughtTimer * 10), 20);
          }
          else
          {
            matrix.drawPixel(15, 1, matrix.Color333(0, 0, 0));
            matrix.drawPixel(14, 2, matrix.Color333(0, 0, 0));
            matrix.drawPixel(15, 2, matrix.Color333(0, 0, 0));
            matrix.drawPixel(16, 2, matrix.Color333(0, 0, 0));
          }
          extraLifeCoughtTimer--;
        }
      }

      // Check whether base has been hit
      if (playfield_invaders[baseX + 1][30] != 0 || playfield_invaders[baseX][31] || playfield_invaders[baseX + 1][31] || playfield_invaders[baseX + 2][31])
      {
        if (!baseDestroyed)
        {
          baseDestroyed = true;
          waitCounterBaseDestroyed = 60;

          // Base explodes
          explosion_invaders(baseX - 1, 29);
          explosion_invaders(baseX + 1, 29);
        }
      }

      // Check whether formation is so deep, that player cannot win (level is restarted in this case)
      if ((formationY + (getLowestRow() * 4)) > 27)
      {
        formationLanded = true;
      }

      // Check for extra life (every 2000 points)
      if (extraLifeScore_invaders > 2000)
      {
        extraLifeScore_invaders = extraLifeScore_invaders - 2000;
        lives_invaders++;
        extraLifeCoughtTimer = 20;
      }

      // Activate boss
      if ((level_invaders % 2 == 0) && remainingAliens == 1 && !bossActive)
      {
        bossActive = true;
        currentWaitTimerBoss = 500;

        // Boss type depends on level
        if ((level_invaders / 2) % 3 == 1)
        {
          bossType = 1;
          bossWidth = 8;
        }
        if ((level_invaders / 2) % 3 == 2)
        {
          bossType = 2;
          bossWidth = 11;
        }
        if ((level_invaders / 2) % 3 == 0)
        {
          bossType = 3;
          bossWidth = 12;
        }

        bossX = 16 - (bossWidth / 2);
        bossY = 3;
        bossMovementStatus = RIGHT;
      }

      delay(1);
    } while (lives_invaders > 0 && remainingAliens > 0 && !formationLanded);


    // Increase Level
    if (remainingAliens == 0)
    {
      level_invaders++;
    }
  } while (lives_invaders > 0);

  // Game Over
  gameOver();
}
