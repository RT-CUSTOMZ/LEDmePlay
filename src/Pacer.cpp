// Pacer
// for the LEDmePlay
//
// www.mithotronic.de
//
// Version: 1.1.1
// Author: Thomas Laubach (2015)
//
// Many thanks to Michael Rosskopf
//
// Release Notes:
// V1.1.1: Support for LEDmePI (2021)
// V1.1.0: Support for LEDmePlay Joypad and LEDmePlayBoy (2018)
// V1.0.3: Colors improved
// V1.0.2: Missing wall bug with newer versions of Adafruit libraries removed
// V1.0.0: First release

// Include libraries for adafruit matrix
#include <Adafruit_GFX.h>     // Core graphics library
#include <RGBmatrixPanel.h>   // Hardware-specific library
#include <Timer.h>            // Enables timed events
#include <pgmspace.h>         // Necessary in order to maintain the maze data in program memory
#include <hardware_setup.hpp> // Initialization of hardware definitions
#include <Pacer_Data.hpp>
#include <Notes.hpp>
#include <LmP_base.hpp>

boolean fireSkipsStage = false;

// Audio out
// const int audioPacer = 2;
boolean audioIsProduced; // True if an arbitrary sound is being produced, false otherwise
int *melodyPointer;      // Pointer to an array that holds musical notes (frequencies) of a melody
int melodyPosition;
int melodyLength;

int additionalLifeMelody[] = {NOTE_A5, 0, NOTE_A5, NOTE_A5, NOTE_A5, 0, NOTE_A5, NOTE_A5, NOTE_A5, 0, NOTE_E5, 0, NOTE_FS5, 0, NOTE_E5, 0, NOTE_CS5, 0, NOTE_CS5, NOTE_CS5, NOTE_CS5, 0, NOTE_D5, 0, NOTE_E5, 0};
int bonusCollectedJingle[] = {NOTE_C5, 0, NOTE_C5, 0, 0, NOTE_C5, 0, NOTE_F5, 0, 0, NOTE_A5, 0, 0, 0};
int baddiesFleeMelody[] = {0, NOTE_G4, 0, NOTE_A4, 0, NOTE_B4, 0, NOTE_C5, 0, NOTE_C3, 0, NOTE_C5, 0, NOTE_C3, 0, NOTE_A4, 0, NOTE_G4, 0, NOTE_G2, 0, NOTE_G4, 0, NOTE_A4, 0, NOTE_G4, 0, NOTE_C5, 0, NOTE_B4, 0, NOTE_G2, 0, NOTE_G3, 0, NOTE_G2, 0, NOTE_G3, 0, NOTE_D5, 0, NOTE_D3, 0, NOTE_D5, 0, NOTE_D3, 0, NOTE_B4, 0, NOTE_G4, 0, NOTE_G2, 0, NOTE_G4, 0, NOTE_A4, 0, NOTE_G4, 0, NOTE_D5, 0, NOTE_E5, 0, NOTE_C2, 0, NOTE_C3, 0, NOTE_C2, 0, NOTE_C3, 0, NOTE_F5, 0, NOTE_F5, 0, NOTE_F3, 0, NOTE_E5, 0, NOTE_D5, 0, NOTE_D5, 0, NOTE_D5, 0, NOTE_D3, 0, NOTE_E5, 0, NOTE_E5, 0, NOTE_C3, 0, NOTE_D5, 0, NOTE_C5, 0, NOTE_C5, 0, NOTE_C5, 0, NOTE_C3, 0, NOTE_D5, 0, NOTE_D5, 0, NOTE_D3, 0, NOTE_D5, 0, NOTE_E5, 0, NOTE_D3, 0, NOTE_D5, 0, NOTE_D3, 0, NOTE_G5, 0, NOTE_G3, 0, NOTE_A5, 0, NOTE_G3, 0, NOTE_AS5, 0, NOTE_G3, 0, NOTE_B5, NOTE_G3, 0, 0};
int baddieCaughtJingle[] = {NOTE_E4, NOTE_B4, NOTE_E5, NOTE_B5, NOTE_E6, NOTE_B6, 0};

// Joystick directions
#define STILL 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4

// Colors
const int backgroundColor = matrix.Color333(0, 0, 0);
const int blinkingColor = matrix.Color444(2, 2, 2);
const int pillColor = matrix.Color444(3, 3, 2);        // With the old Adafruit library, used to be Color888(14, 14, 31);
const int superPillColor = matrix.Color888(40, 32, 0); // see above, used to be Color888(16, 16, 0);
const int pacerColor = matrix.Color333(3, 3, 0);
const int baddie1Color = matrix.Color333(0, 3, 0);
const int baddie2Color = matrix.Color333(3, 0, 0);
const int baddie3Color = matrix.Color333(3, 0, 3);
const int baddie4Color = matrix.Color888(127, 70, 0); // see above, used to be Color888(16, 10, 8)
const int baddieEyeColor = matrix.Color444(3, 3, 0);
const int fleeingBaddieColor = matrix.Color888(0, 64, 128);
const int baddieHomePositionColor = matrix.ColorHSV(0, 255, 66, true);
const int mazeColors[] = {matrix.Color888(0, 17, 0), matrix.Color888(0, 0, 20), matrix.Color888(0, 16, 20)};

// The maze is surrounded by a border
// A gap is inserted at the right position in the border if code 9 (PASSAGE) is encountered
// There may be multiple baddies of either type
// The baddies start at their locations as set in the map
byte labyrinth[100];
const byte numMazeColors = sizeof(mazeColors) / sizeof(mazeColors[0]); // Maze colors are defined above
int mazeColor;
byte mazeColorNum;
#define DRAW_MAZE 0
#define CLEAR_MAZE 1

// Playfield
byte playfield[38][38];
const byte o = 3; // Playfield has a three pixel-wide border. o is added to the playfield coordinates, i.e.,  (3, 3) is (0, 0) in matrix.drawPixel(0, 0, ...)
                  // pacerX/Y and baddieX/Y are also shifted by +o
                  // This allows the sprite to cross the border without the requirement to set their coordinates to negative values

// Pacer
#define PLAYER_SPEED 120 // The less, the faster
byte pacerX;
byte pacerY;
byte pacerX_old;
byte pacerY_old;
byte pacer_initialX;
byte pacer_initialY;
byte pacerStatus;
byte pacer_animPhase = 0;      // The player's sprite has two animation phases for each direction
byte currentDirection = STILL; // 0 == STILL, 1 == UP, 2 == RIGHT, 3 == DOWN, 4 == LEFT
byte intendedDirection;        // same encoding as for currentDirection
boolean blinkPacer;

// Baddies
#define BADDIE_SPEED 260 // The less, the faster
const byte NUM_BADDIES = 10;
byte baddieX[NUM_BADDIES];
byte baddieY[NUM_BADDIES];
byte baddieX_old[NUM_BADDIES];
byte baddieY_old[NUM_BADDIES];
byte baddie_initialX[NUM_BADDIES];
byte baddie_initialY[NUM_BADDIES];
boolean baddie_active[NUM_BADDIES] = {false, false, false, false, false, false, false, false, false, false};
boolean baddieGoingHome[NUM_BADDIES] = {false, false, false, false, false, false, false, false, false, false};
byte baddie_currentDirection[NUM_BADDIES];  // 0 == STILL, 1 == UP, 2 == RIGHT, 3 == DOWN, 4 == LEFT
byte baddie_intendedDirection[NUM_BADDIES]; // same encoding as for currentDirection
byte baddie_type[NUM_BADDIES];
byte baddieNum = 0; // The number of baddies in the current maze
byte baddie_animPhase;
byte animTimer = 0; // Is modified in moveBaddiesGoingHome()
int animCycle = 1;  // Needed to let the fleeing baddies blink

// stage settings
const byte NUM_LIVES = 5;
const byte NUM_STAGES = (sizeof(maze) / sizeof(maze[0])) / 100;
byte stage = 0;             // Game begins at stage (between 0 and NUM_STAGES)
byte lives = NUM_LIVES;     // Number of remaining lives
byte difficultyPacer = 0;   // difficulty of current game
int score = 0;              // Current score
byte pills;                 // Number of pills in current stage
byte numStagesFinished = 0; // Counts the number of stages the player has finished

// Event variables
boolean showTitleScreen = true;
boolean showMithotronic = true;
boolean gameBegins; // True at the beginning of the game
boolean resetPacer;
boolean isPause;         // if true, halt the game until the joystick is moved
boolean paintingAllowed; // True throughout the game, false at the title screen and the Game Over-Scene(prevents bonus items and pills from being redrawn)
boolean collision;
boolean playerInvincible;
boolean skipStage;
int invincibleTimer;
byte numInvincibleCycles = max(20, 60 - difficultyPacer * 5); // Time span in which the player's sprite is invincible, i. e., in which the baddies flee.

byte bonusInterval = min(255, 90 + difficultyPacer * 10); // Time span = {span in which bonus item is not shown} + {span in which bonus item is shown}.
int bonusTimer;                                           // A number within bonusInterval, is zeroed when the player picks up the bonus item
int timeAtWhichBonusAppears;                              // A number within bonusInterval, the time at which the bonus item appears on screen, is dependent on the difficultyPacer level
boolean bonusPresent;                                     // True when the bonus item is on screen and may be picked up by the player
byte bonusX, bonusY;
byte bonusItem;        // Number of the item currently present
byte numBoniCollected; // Every {five+difficultyPacer} collected boni, the player gets one additional life

// Helper (used for different things)
int i, j, k;
double x, y, z;

// Initialize Timer object
Timer timer;
int pacerMover;
int baddiesGoHomeMover;
int baddiesMover;
int melodyPlayer;

// Function declarations
void setLEDMePlayColor(int i);
void stopTimers();
void startTimers();
void initializePlayfield();
void drawMaze();
void clearPassages();
void drawInnerWalls(int col, byte mode);
void dissipateMazeWalls();
void blinkMazeWalls(byte times, int duration);
void redrawPills();
void learnPacerStartingPosition(byte *gx, byte *gy);
void movePacer();
void tryCurrentDirection_pacer();
boolean tryNewDirection_pacer(byte x, byte y, byte dir);
void repaintPacer();
void playDeathAnimation(byte pacerX, byte pacerY);
boolean tryCurrentDirection_baddie(byte baddieNum);
boolean tryCurrentDirection_baddie_invincible(byte baddieNum);
boolean tryNewDirection_baddie(byte x, byte y, byte dir);
boolean positionOverlapsWithBaddie(byte num);
void moveBaddies();
byte getAllDirections(byte x, byte y, boolean alternateWay[]);
byte getAllDirections_invincible(byte x, byte y, boolean alternateWay[]);
void paintOrdinaryBaddie(byte baddieNum);
void paintFleeingBaddie(byte baddieNum);
void repaintBaddies();
void restorePills();
void repaintBaddie(byte baddieNum);
void repaintBaddiesGoingHome();
void moveBaddiesGoingHome();
void paintBonusItem(boolean paintItem);
void determineBonusItemLocation();
void playMunchingNoise();
void playCaughtBaddieNoise();
void playSuperPillNoise();
void playOpeningJingle();
void playGameOverJingle();
void issueMelody(byte num);
void playMelody();
void showTitle();
int getNextMazeColor();
void consumeJoystickEvents();
void playLargeInterlude(byte xoffset, byte yoffset);
void playSmallInterlude(byte xoffset, byte yoffset);
void playGameOverScene();

void setup_Pacer()
{
  hardware_init();

  initializePlayfield();
  startTimers();

  // Enter the game loop
  gameBegins = true;
}

void stopTimers()
{
  timer.stop(pacerMover);
  timer.stop(baddiesGoHomeMover);
  timer.stop(baddiesMover);
  timer.stop(melodyPlayer);
}

void startTimers()
{
  pacerMover = timer.every(max(75, PLAYER_SPEED - difficultyPacer * 10), movePacer);
  baddiesGoHomeMover = timer.every(65, moveBaddiesGoingHome);
  baddiesMover = timer.every(max(85, BADDIE_SPEED - difficultyPacer * 8), moveBaddies);
  melodyPlayer = timer.every(75, playMelody);
  // int noisePlayer = timer.every(5000, makeNoise);
}

void initializePlayfield()
{
  // Initialize playfield structure
  for (byte i = 0; i < 38; i++)
  {
    for (byte j = 0; j < 38; j++)
    {
      playfield[i][j] = EMPTY;
    }
  }
  // Draw border on playfield
  for (i = 0; i < 32; i++)
  {
    playfield[i + o][0 + o] = WALL;
    playfield[i + o][31 + o] = WALL;
    playfield[0 + o][i + o] = WALL;
    playfield[31 + o][i + o] = WALL;
  }
}

// Reads the maze structure from Flash ROM (PROGMEM) and transfers it to data structure in RAM
// Sets the initial positions of the player's sprite (Pacer) and the baddies
// Draws the maze, the sprites, and pills
void drawMaze()
{
  // Draw border on screen
  matrix.drawRect(0, 0, 32, 32, mazeColor);
  initializePlayfield();

  // Initialize the labyrinth and playfield data structures
  for (byte i = 0; i < 100; i++)
  {
    labyrinth[i] = EMPTY;
  }
  // Copy the maze from program memory into data structure labyrinth
  for (byte k = 0; k < 100; k++)
  {
    labyrinth[k] = pgm_read_byte_near(maze + (stage * 100) + k);
  }

  // Draw pills and passages, and store the walls in the playfield
  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
    {
      if (labyrinth[(j * 10) + i] == PILL || labyrinth[(j * 10) + i] == PASSAGE) // Passage
      {
        // Draw pill (draw also a pill if position is 4 which encodes for pill close to a passage)
        matrix.drawPixel((i * 3) + 2, (j * 3) + 2, pillColor); // formerly (1, 1, 2)
        playfield[(i * 3) + 2 + o][(j * 3) + 2 + o] = PILL;
        pills++;

        // Draw passages
        if (labyrinth[(j * 10) + i] == PASSAGE)
        {
          if (i == 0)
          {
            matrix.drawLine(0, (j * 3) + 1, 0, (j * 3) + 3, backgroundColor);
            playfield[0 + o][(j * 3) + 1 + o] = EMPTY;
            playfield[0 + o][(j * 3) + 2 + o] = EMPTY;
            playfield[0 + o][(j * 3) + 3 + o] = EMPTY;
          }
          if (i == 9)
          {
            matrix.drawLine(31, (j * 3) + 1, 31, (j * 3) + 3, backgroundColor);
            playfield[31 + o][(j * 3) + 1 + o] = EMPTY;
            playfield[31 + o][(j * 3) + 2 + o] = EMPTY;
            playfield[31 + o][(j * 3) + 3 + o] = EMPTY;
          }
          if (j == 0)
          {
            matrix.drawLine((i * 3) + 1, 0, (i * 3) + 3, 0, backgroundColor);
            playfield[(i * 3) + 1 + o][0 + o] = EMPTY;
            playfield[(i * 3) + 2 + o][0 + o] = EMPTY;
            playfield[(i * 3) + 3 + o][0 + o] = EMPTY;
          }
          if (j == 9)
          {
            matrix.drawLine((i * 3) + 1, 31, (i * 3) + 3, 31, backgroundColor);
            playfield[(i * 3) + 1 + o][31 + o] = EMPTY;
            playfield[(i * 3) + 2 + o][31 + o] = EMPTY;
            playfield[(i * 3) + 3 + o][31 + o] = EMPTY;
          }
        }
      }
      else if (labyrinth[(j * 10) + i] == SUPER_PILL)
      {
        // Draw super pill
        matrix.drawPixel((i * 3) + 2, (j * 3) + 2, superPillColor);
        playfield[(i * 3) + 2 + o][(j * 3) + 2 + o] = SUPER_PILL;
        pills++;
      }
      else if (labyrinth[(j * 10) + i] == WALL) // Wall
      {
        // Store the wall element in the playfield
        playfield[(i * 3) + 1 + o][(j * 3) + 1 + o] = WALL;
        playfield[(i * 3) + 1 + o][(j * 3) + 3 + o] = WALL;
        playfield[(i * 3) + 3 + o][(j * 3) + 1 + o] = WALL;
        playfield[(i * 3) + 3 + o][(j * 3) + 3 + o] = WALL;
      }
      else if ((labyrinth[(j * 10) + i] >= BADDIE_TYPE1) && (labyrinth[(j * 10) + i] <= BADDIE_TYPE4)) // The baddies' locations
      {
        // Set the starting locations of the baddies
        baddie_type[baddieNum] = labyrinth[(j * 10) + i];
        baddie_active[baddieNum] = true;
        baddieX[baddieNum] = i * 3 + 1 + o;
        baddieY[baddieNum] = j * 3 + 1 + o;
        baddieX_old[baddieNum] = baddieX[baddieNum];
        baddieY_old[baddieNum] = baddieY[baddieNum];
        baddie_initialX[baddieNum] = baddieX[baddieNum];
        baddie_initialY[baddieNum] = baddieY[baddieNum];

        if (baddieNum < NUM_BADDIES)
        {
          baddieNum++;
        }; // Next baddie
      }
      else if (labyrinth[(j * 10) + i] == PACER) // The player's starting location
      {
        playfield[(i * 3) + 1 + o][(j * 3) + 1 + o] = EMPTY;
        playfield[(i * 3) + 1 + o][(j * 3) + 3 + o] = EMPTY;
        playfield[(i * 3) + 3 + o][(j * 3) + 1 + o] = EMPTY;
        playfield[(i * 3) + 3 + o][(j * 3) + 3 + o] = EMPTY;
        pacerX = i * 3 + 1 + o; // Offset o ensures that pacerX and pacerY never become lower than zero
        pacerY = j * 3 + 1 + o;
        pacer_initialX = pacerX;
        pacer_initialY = pacerY;
        pacerX_old = pacerX;
        pacerY_old = pacerY;
      }
    } // FOR j
  } // FOR i
  drawInnerWalls(mazeColor, DRAW_MAZE);
  // return;

  currentDirection = 0; // Player's sprite is not moving

  for (int baddieNum = 0; baddieNum < NUM_BADDIES; baddieNum++)
  {
    if (baddie_active[baddieNum])
    {
      byte x = baddieX[baddieNum] - o;
      byte y = baddieY[baddieNum] - o;
      // Assign an initial direction to every active baddie
      boolean assigned = false;
      byte selectedDirection = STILL;
      do
      {
        selectedDirection = random(1, 5); // Direction in [1..4]
        switch (selectedDirection)
        {
        case UP:
          if (!((playfield[x + o][y - 1 + o] == WALL) && (playfield[x + 1 + o][y - 1 + o] == WALL) && (playfield[x + 2 + o][y - 1 + o] == WALL)))
          {
            assigned = true;
            break;
          }
        case DOWN:
          if (!((playfield[x + o][y + 1 + o] == WALL) && (playfield[x + 1 + o][y + 1 + o] == WALL) && (playfield[x + 2 + o][y + 1 + o] == WALL)))
          {
            assigned = true;
            break;
          }
        case RIGHT:
          if (!((playfield[x + 1 + o][y - 1 + o] == WALL) && (playfield[x + 1 + o][y + o] == WALL) && (playfield[x + 1 + o][y + 2 + o] == WALL)))
          {
            assigned = true;
            break;
          }
        case LEFT:
          if (!((playfield[x - 1 + o][y - 1 + o] == WALL) && (playfield[x - 1 + o][y + o] == WALL) && (playfield[x - 1 + o][y + 2 + o] == WALL)))
          {
            assigned = true;
            break;
          }
        }
      } while (!assigned);
      baddie_currentDirection[baddieNum] = selectedDirection;
    } // baddie_active[baddieNum]
  }

  score = 0;
} // drawMaze

void clearPassages()
{
  // Draw passages
  if (labyrinth[(j * 10) + i] == PASSAGE)
  {
    if (i == 0)
    {
      matrix.drawLine(0, (j * 3) + 1, 0, (j * 3) + 3, backgroundColor);
    }
    if (i == 9)
    {
      matrix.drawLine(31, (j * 3) + 1, 31, (j * 3) + 3, backgroundColor);
    }
    if (j == 0)
    {
      matrix.drawLine((i * 3) + 1, 0, (i * 3) + 3, 0, backgroundColor);
    }
    if (j == 9)
    {
      matrix.drawLine((i * 3) + 1, 31, (i * 3) + 3, 31, backgroundColor);
    }
  }
}

// Draws the walls of the maze in four variations: filled/not filled with sharp/round corners
// Uses col as wall color
// mode 0 - clear maze, 1 - draw maze
void drawInnerWalls(int col, byte mode)
{
  if (((stage + 4) % 4 == 0) || ((stage + 4) % 4 == 2)) // Wall tiles not filled
  {
    // Draw the wall elements to the north, south, east, and west per wall tile
    for (i = 0; i < 10; i++)
    {
      for (j = 0; j < 10; j++)
      {
        if (playfield[i * 3 + 1 + o][j * 3 + 1 + o] == WALL) // Tile at (i, j) is a wall
        {
          matrix.fillRect((i * 3) + 1, (j * 3) + 1, 3, 3, backgroundColor); // Erase pills that stay in the background

          // North
          if (playfield[i * 3 + 1 + o][(j - 1) * 3 + 1 + o] <= SUPER_PILL) // Tile at (i, j-1) is not a wall
          {
            if (j > 0)
            {
              matrix.drawLine((i * 3) + 1, (j * 3) + 1, (i * 3) + 3, (j * 3) + 1, col);
            }
          }
          // South
          if (playfield[i * 3 + 1 + o][(j + 1) * 3 + 1 + o] <= SUPER_PILL)
          {
            if (j < 10)
            {
              matrix.drawLine((i * 3) + 1, (j * 3) + 3, (i * 3) + 3, (j * 3) + 3, col);
            }
          }
          // East
          if (playfield[(i + 1) * 3 + 1 + o][j * 3 + 1 + o] <= SUPER_PILL)
          {
            if (i < 10)
            {
              matrix.drawLine((i * 3) + 3, (j * 3) + 1, (i * 3) + 3, (j * 3) + 3, col);
            }
          }
          // West
          if (playfield[(i - 1) * 3 + 1 + o][j * 3 + 1 + o] <= SUPER_PILL)
          {
            if (i > 0)
            {
              matrix.drawLine((i * 3) + 1, (j * 3) + 1, (i * 3) + 1, (j * 3) + 3, col);
            }
          }
        } // tile (i, j) is a wall
      } // for j
    } // for i

    if ((stage + 4) % 4 == 0) // Wall tiles not filled, sharp corners
    {
      // Add corner points in color mazeColor
      for (i = 0; i < 10; i++)
      {
        for (j = 0; j < 10; j++)
        {
          if (playfield[i * 3 + 1 + o][j * 3 + 1 + o] == WALL) // Tile at (i, j) is a wall
          {
            // SE corner
            if ((playfield[(i + 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] == WALL) && (playfield[(i + 0) * 3 + 1 + o][(j + 1) * 3 + 1 + o] == WALL) &&
                (playfield[(i + 1) * 3 + 1 + o][(j + 1) * 3 + 1 + o] != WALL))
            {
              matrix.drawPixel(i * 3 + 3, j * 3 + 3, col);
            }

            // SW corner
            if ((playfield[(i - 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] == WALL) && (playfield[(i + 0) * 3 + 1 + o][(j + 1) * 3 + 1 + o] == WALL) &&
                (playfield[(i - 1) * 3 + 1 + o][(j + 1) * 3 + 1 + o] != WALL))
            {
              matrix.drawPixel(i * 3 + 1, j * 3 + 3, col);
            }

            // NE corner
            if ((playfield[(i + 0) * 3 + 1 + o][(j - 1) * 3 + 1 + o] == WALL) && (playfield[(i + 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] == WALL) &&
                (playfield[(i + 1) * 3 + 1 + o][(j - 1) * 3 + 1 + o] != WALL))
            {
              matrix.drawPixel(i * 3 + 3, j * 3 + 1, col);
            }

            // NW corner
            if ((playfield[(i + 0) * 3 + 1 + o][(j - 1) * 3 + 1 + o] == WALL) && (playfield[(i - 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] == WALL) &&
                (playfield[(i - 1) * 3 + 1 + o][(j - 1) * 3 + 1 + o] != WALL))
            {
              matrix.drawPixel(i * 3 + 1, j * 3 + 1, col);
            }
          } // tile is a wall
        } // for j
      } // for i
    } // sharp corners
    else if ((stage + 4) % 4 == 2) // Wall tiles not filled, round corners
    {
      for (i = 0; i < 10; i++)
      {
        for (j = 0; j < 10; j++)
        {
          if (playfield[i * 3 + 1 + o][j * 3 + 1 + o] != WALL) // Tile at (i, j) is not a wall
          {
            // SE corner
            if ((playfield[(i - 1) * 3 + 1 + o][(j - 1) * 3 + 1 + o] == WALL) && (playfield[(i + 0) * 3 + 1 + o][(j - 1) * 3 + 1 + o] != WALL) &&
                (playfield[(i - 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL))
            {
              if (((i - 1) * 3 + 3 != 31) && ((j - 1) * 3 + 3 != 31))
              {
                matrix.drawPixel((i - 1) * 3 + 3, (j - 1) * 3 + 3, backgroundColor);
              }
            }

            // SW corner
            if ((playfield[(i + 1) * 3 + 1 + o][(j - 1) * 3 + 1 + o] == WALL) && (playfield[(i + 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL) &&
                (playfield[(i + 0) * 3 + 1 + o][(j - 1) * 3 + 1 + o] != WALL))
            {
              if (((i + 1) * 3 + 1 != 31) && ((j - 1) * 3 + 3 != 31))
              {
                matrix.drawPixel((i + 1) * 3 + 1, (j - 1) * 3 + 3, backgroundColor);
              }
            }

            // NW corner
            if ((playfield[(i + 1) * 3 + 1 + o][(j + 1) * 3 + 1 + o] == WALL) && (playfield[(i + 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL) &&
                (playfield[(i + 0) * 3 + 1 + o][(j + 1) * 3 + 1 + o] != WALL))
            {
              if (((i + 1) * 3 + 1 != 31) && ((j + 1) * 3 + 1 != 31))
              {
                matrix.drawPixel((i + 1) * 3 + 1, (j + 1) * 3 + 1, backgroundColor);
              }
            }

            // NE corner
            if ((playfield[(i - 1) * 3 + 1 + o][(j + 1) * 3 + 1 + o] == WALL) && (playfield[(i - 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL) &&
                (playfield[(i + 0) * 3 + 1 + o][(j + 1) * 3 + 1 + o] != WALL))
            {
              if (((i - 1) * 3 + 3 != 31) && ((j + 1) * 3 + 1 != 31))
                matrix.drawPixel((i - 1) * 3 + 3, (j + 1) * 3 + 1, backgroundColor);
            }
          } // tile is a wall
        } // for j
      } // for i

      // Pixels in the corners of the screen
      matrix.drawPixel(0, 0, backgroundColor);
      matrix.drawPixel(31, 0, backgroundColor);
      matrix.drawPixel(0, 31, backgroundColor);
      matrix.drawPixel(31, 31, backgroundColor);
    } // round corners
  } // Wall tiles not filled
  else if (((stage + 4) % 4 == 1) || ((stage + 4) % 4 == 3)) // Wall tiles filled
  {
    // Sharp corners
    for (i = 0; i < 10; i++)
    {
      for (j = 0; j < 10; j++)
      {
        if (playfield[i * 3 + 1 + o][j * 3 + 1 + o] == WALL) // Tile at (i, j) is a wall
        {
          matrix.fillRect((i * 3) + 1, (j * 3) + 1, 3, 3, col);
        }
      }
    }

    if ((stage + 4) % 4 == 3) // Wall tiles filled, round corners
    {
      // Remove corner points
      for (i = 0; i < 10; i++)
      {
        for (j = 0; j < 10; j++)
        {
          if (playfield[i * 3 + 1 + o][j * 3 + 1 + o] != WALL) // Tile at (i, j) is not a wall
          {
            // SE corner
            if ((playfield[(i - 1) * 3 + 1 + o][(j - 1) * 3 + 1 + o] == WALL) && (playfield[(i + 0) * 3 + 1 + o][(j - 1) * 3 + 1 + o] != WALL) &&
                (playfield[(i - 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL))
            {
              if (((i - 1) * 3 + 3 != 31) && ((j - 1) * 3 + 3 != 31))
              {
                matrix.drawPixel((i - 1) * 3 + 3, (j - 1) * 3 + 3, backgroundColor);
              }
            }

            // SW corner
            if ((playfield[(i + 1) * 3 + 1 + o][(j - 1) * 3 + 1 + o] == WALL) && (playfield[(i + 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL) &&
                (playfield[(i + 0) * 3 + 1 + o][(j - 1) * 3 + 1 + o] != WALL))
            {
              if (((i + 1) * 3 + 1 != 31) && ((j - 1) * 3 + 3 != 31))
              {
                matrix.drawPixel((i + 1) * 3 + 1, (j - 1) * 3 + 3, backgroundColor);
              }
            }

            // NW corner
            if ((playfield[(i + 1) * 3 + 1 + o][(j + 1) * 3 + 1 + o] == WALL) && (playfield[(i + 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL) &&
                (playfield[(i + 0) * 3 + 1 + o][(j + 1) * 3 + 1 + o] != WALL))
            {
              if (((i + 1) * 3 + 1 != 31) && ((j + 1) * 3 + 1 != 31))
              {
                matrix.drawPixel((i + 1) * 3 + 1, (j + 1) * 3 + 1, backgroundColor);
              }
            }

            // NE corner
            if ((playfield[(i - 1) * 3 + 1 + o][(j + 1) * 3 + 1 + o] == WALL) && (playfield[(i - 1) * 3 + 1 + o][(j + 0) * 3 + 1 + o] != WALL) &&
                (playfield[(i + 0) * 3 + 1 + o][(j + 1) * 3 + 1 + o] != WALL))
            {
              if (((i - 1) * 3 + 3 != 31) && ((j + 1) * 3 + 1 != 31))
              {
                matrix.drawPixel((i - 1) * 3 + 3, (j + 1) * 3 + 1, backgroundColor);
              }
            }
          } // tile is a wall
        } // for j
      } // for i
    } // Round corners
  } // Wall tiles filled

  // Clear or draw the outer border, omit the passages
  for (byte x = 0; x < 32; x++)
  {
    if (playfield[x + o][0 + o] == WALL)
    {
      matrix.drawPixel(x, 0, col);
    }
    if (playfield[x + o][31 + o] == WALL)
    {
      matrix.drawPixel(x, 31, col);
    }
  }
  for (byte y = 0; y < 32; y++)
  {
    if (playfield[0 + o][y + o] == WALL)
    {
      matrix.drawPixel(0, y, col);
    }
    if (playfield[31 + o][y + o] == WALL)
    {
      matrix.drawPixel(31, y, col);
    }
  }
  // If necessary, clear the corner pixels in the border
  if ((((stage + 4) % 4) == 2) || (((stage + 4) % 4) == 3))
  {
    matrix.drawPixel(0, 0, backgroundColor);
    matrix.drawPixel(31, 0, backgroundColor);
    matrix.drawPixel(0, 31, backgroundColor);
    matrix.drawPixel(31, 31, backgroundColor);
  }
} // drawInnerWalls

// Realizes the collapsing walls effect that occurs when the player has cleared a stage
// This is executed only for mazes with wall icons that have round corners
void dissipateMazeWalls()
{
  for (byte i = 31; i > 0; i--)
  {
    repaintPacer();
    matrix.drawPixel(i, 0, backgroundColor);
    matrix.drawPixel(31 - i, 31, backgroundColor);
    matrix.drawPixel(0, i, backgroundColor);
    matrix.drawPixel(31, 31 - i, backgroundColor);
    delay(60);
  }
  matrix.drawPixel(0, 0, backgroundColor);
  matrix.drawPixel(31, 0, backgroundColor);
  matrix.drawPixel(0, 31, backgroundColor);
  matrix.drawPixel(31, 31, backgroundColor);

  // Delete the maze tiles in a meandering sequence
  byte i = 9, j = 0;
  do
  {
    do
    {
      matrix.fillRect(i * 3 + 1, j * 3 + 1, 3, 3, backgroundColor);
      repaintPacer();
      delay(20);
      i--;
    } while (i > 0);
    matrix.fillRect(1, j * 3 + 1, 3, 3, backgroundColor);
    repaintPacer();
    delay(20);
    matrix.fillRect(1, j * 3 + 2, 3, 3, backgroundColor);
    repaintPacer();
    delay(20);
    matrix.fillRect(1, j * 3 + 3, 3, 3, backgroundColor);
    repaintPacer();
    delay(20);
    j++;
    do
    {
      matrix.fillRect(i * 3 + 1, j * 3 + 1, 3, 3, backgroundColor);
      repaintPacer();
      delay(20);
      i++;
    } while (i < 9);
    matrix.fillRect(28, j * 3 + 1, 3, 3, backgroundColor);
    repaintPacer();
    delay(20);
    matrix.fillRect(28, j * 3 + 2, 3, 3, backgroundColor);
    repaintPacer();
    delay(20);
    matrix.fillRect(28, j * 3 + 3, 3, 3, backgroundColor);
    repaintPacer();
    delay(20);
    j++;
  } while (j < 9);
}

// Realizes the blinking walls effect that occurs when the player has cleared a stage
// This is executed only for mazes with wall icons that have sharp corners
void blinkMazeWalls(byte times, int duration)
{
  for (byte i = 0; i < times; i++)
  {
    drawInnerWalls(mazeColor, DRAW_MAZE);
    delay(duration);
    drawInnerWalls(blinkingColor, CLEAR_MAZE);
    delay(duration);
  }
} // blinkMazeWalls(times, duration)

void redrawPills()
{
  // Draw pills and passages, and store the walls in the playfield
  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
    {
      if (labyrinth[(j * 10) + i] == PILL || labyrinth[(j * 10) + i] == PASSAGE) // Passage
      {
        // Draw pill (draw also a pill if position is 4 which encodes for pill close to a passage)
        matrix.drawPixel((i * 3) + 2, (j * 3) + 2, pillColor); // matrix.Color333(1, 1, 2));
        playfield[(i * 3) + 2 + o][(j * 3) + 2 + o] = PILL;
        pills++;
      }
      else if (labyrinth[(j * 10) + i] == SUPER_PILL)
      {
        // Draw super pill
        matrix.drawPixel((i * 3) + 2, (j * 3) + 2, superPillColor);
        playfield[(i * 3) + 2 + o][(j * 3) + 2 + o] = SUPER_PILL;
        pills++;
      }
    } // FOR j
  } // FOR i
}

// ********************************* PLAYER'S SPRITE ******************************************************

void learnPacerStartingPosition(byte *gx, byte *gy)
{
  byte lab[100];

  // Initialize the labyrinth and playfield data structures
  for (byte i = 0; i < 100; i++)
  {
    lab[i] = EMPTY;
  }
  // Copy the maze from program memory into data structure labyrinth
  for (byte k = 0; k < 100; k++)
  {
    // At this moment, stage is already the one to be drawn
    lab[k] = pgm_read_byte_near(maze + (stage * 100) + k); // Preaccess the data for the next stage
  }

  // Find the player's starting location in the next stage
  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 10; j++)
    {
      if (lab[(j * 10) + i] == PACER)
      {
        *gx = i * 3 + 1;
        *gy = j * 3 + 1;
        return;
      }
    }
  }
} // learnPacerStartingPosition

// This subroutine is called from within a timer
void movePacer()
{
  boolean ignoreJoystick = false;
  if (pacer_animPhase == 1)
  {
    pacer_animPhase = 0;
  }
  else
  {
    pacer_animPhase = 1;
  }

  pacerX_old = pacerX;
  pacerY_old = pacerY;
  i = pacerX;
  j = pacerY;
  byte i_incr = 0;
  byte j_incr = 0;
  byte intendedDirection = 0; // 0 == Not moving, 1 == North, 2 == East, 3 == South, 4 == West

  // If the player's sprite moves outside the playfield, ignore all joystick commands
  if (((pacerX < 1 + o) || (pacerX > 29 + o)) && (currentDirection == LEFT))
  {
    pacerX--;
    ignoreJoystick = true;
    repaintPacer();
  }
  else if (((pacerX > 28 + o) || (pacerX < 1 + o)) && (currentDirection == RIGHT))
  {
    pacerX++;
    ignoreJoystick = true;
    repaintPacer();
  }
  else if (((pacerY < 1 + o) || (pacerY > 29 + o)) && (currentDirection == UP))
  {
    pacerY--;
    ignoreJoystick = true;
    repaintPacer();
  }
  else if (((pacerY > 28 + o) || (pacerY < 1 + o)) && (currentDirection == DOWN))
  {
    pacerY++;
    ignoreJoystick = true;
    repaintPacer();
  }

  if (!ignoreJoystick)
  {
    // Check joystick 1 for player 1
    // Player indicates a new direction
    if (joy1Left() && !joy1Up() && !joy1Down() && (currentDirection != LEFT)) // LEFT
    {
      intendedDirection = LEFT;
      if (tryNewDirection_pacer(i - o - 1, j - o, intendedDirection))
      {
        pacerX = i--;
        currentDirection = intendedDirection;
        repaintPacer();
      }
      else
      {
        tryCurrentDirection_pacer();
      }
    }
    else if (joy1Right() && !joy1Up() && !joy1Down() && (currentDirection != RIGHT)) // RIGHT
    {
      intendedDirection = RIGHT;
      if (tryNewDirection_pacer(i - o + 1, j - o, intendedDirection))
      {
        pacerX = i++;
        currentDirection = intendedDirection;
        repaintPacer();
      }
      else
      {
        tryCurrentDirection_pacer();
      }
    }
    else if (joy1Up() && !joy1Left() && !joy1Right() && (currentDirection != UP)) // UP
    {
      intendedDirection = UP;
      if (tryNewDirection_pacer(i - o, j - o - 1, intendedDirection))
      {
        pacerY = j--;
        currentDirection = intendedDirection;
        repaintPacer();
      }
      else
      {
        tryCurrentDirection_pacer();
      }
    }
    else if (joy1Down() && !joy1Left() && !joy1Right() && (currentDirection != DOWN)) // DOWN
    {
      intendedDirection = DOWN;
      if (tryNewDirection_pacer(i - o, j - o + 1, intendedDirection))
      {
        pacerY = j++;
        currentDirection = intendedDirection;
        repaintPacer();
      }
      else
      {
        tryCurrentDirection_pacer();
      }
    }
    else // Try to move into the current direction
    {
      tryCurrentDirection_pacer();
    }

    if (joy1Fire())
    {
      if (fireSkipsStage)
      {
        skipStage = true;
      }
    }

    // Check for pills
    byte cellX = (pacerX - o) / 3;
    byte cellY = (pacerY - o) / 3;
    if (playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] == PILL)
    {
      playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] = EMPTY;
      score++;
      pills--;
      playMunchingNoise();
    }
    else if (playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] == SUPER_PILL) // Player picked up a super pill
    {
      playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] = EMPTY;
      score = score + 10;
      pills--;
      playerInvincible = true;
      invincibleTimer = numInvincibleCycles;
      playSuperPillNoise();
      issueMelody(BADDIESFLEE_MELODY);

      // Change the baddie's direction in a way that it moves away from the player's sprite
      for (int baddieNum = 0; baddieNum < NUM_BADDIES; baddieNum++)
      {
        if (baddie_active[baddieNum])
        {
          int diffX = (baddieX[baddieNum] - o) - (pacerX - o);
          int diffY = (baddieY[baddieNum] - o) - (pacerY - o);
          if (diffX > 0)
          {
            baddie_currentDirection[baddieNum] = RIGHT;
          }
          else
          {
            baddie_currentDirection[baddieNum] = LEFT;
          }
          if (diffY > 0)
          {
            baddie_currentDirection[baddieNum] = DOWN;
          }
          else
          {
            baddie_currentDirection[baddieNum] = UP;
          }
        } // baddie_active
      } // for baddieNum
    } // SUPER_PILL

    // Has the player picked up the bonus item?
    if ((abs(pacerX - o - bonusX) < 3) && (abs(pacerY - o - bonusY) < 3) && bonusPresent)
    {
      score = score + 100;
      numBoniCollected++;
      if ((numBoniCollected > 0) && ((numBoniCollected + 5 + difficultyPacer) % (5 + difficultyPacer) == 0))
      {
        issueMelody(ADDITIONALLIFE_JINGLE);
        lives++;
      }
      else
      {
        issueMelody(BONUSITEM_JINGLE);
      }
      matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);

      bonusPresent = false;
      bonusTimer = 0;
      paintBonusItem(false);
    }
  } // !ignoreJoystick

  // Did the player steer his/her sprite across the screen boundary?
  if ((pacerX == 32 + o) && (currentDirection == RIGHT))
  {
    pacerX_old = pacerX;
    pacerX = -2 + o;
  }
  else if ((pacerX == -3 + o) && (currentDirection == LEFT))
  {
    pacerX_old = pacerX;
    pacerX = 30 + o;
  }
  else if ((pacerY == -3 + o) && (currentDirection == UP))
  {
    pacerY_old = pacerY;
    pacerY = 30 + o;
  }
  else if ((pacerY == 32 + o) && (currentDirection == DOWN))
  {
    pacerY_old = pacerY;
    pacerY = -2 + o;
  }

  // Abuse this method to redraw the bonus item
  if (bonusPresent)
  {
    paintBonusItem(true);
  }

  repaintPacer();

} // movePacer

void tryCurrentDirection_pacer()
{
  intendedDirection = currentDirection;
  byte i_incr = 0;
  byte j_incr = 0;
  switch (currentDirection)
  {
  case STILL:
    i_incr = 0;
    j_incr = 0;
    break; // Standing still
  case UP:
    i_incr = 0;
    j_incr = -1;
    break; // Up
  case RIGHT:
    i_incr = +1;
    j_incr = 0;
    break; // Right
  case DOWN:
    i_incr = 0;
    j_incr = +1;
    break; // Down
  case LEFT:
    i_incr = -1;
    j_incr = 0;
    break; // Left
  }

  if (tryNewDirection_pacer(i - o + i_incr, j - o + j_incr, intendedDirection))
  {
    pacerX = pacerX + i_incr;
    pacerY = pacerY + j_incr;
    currentDirection = intendedDirection;
    repaintPacer();
  }
} // tryCurrentDirection

boolean tryNewDirection_pacer(byte x, byte y, byte dir)
{
  // Check for walls
  if ((playfield[x + o][y + o] == WALL) || (playfield[x + 1 + o][y + o] == WALL) || (playfield[x + 2 + o][y + o] == WALL) ||
      (playfield[x + o][y + 1 + o] == WALL) || (playfield[x + 2 + o][y + 1 + o] == WALL) || (playfield[x + o][y + 2 + o] == WALL) ||
      (playfield[x + 1 + o][y + 2 + o] == WALL) || (playfield[x + 2 + o][y + 2 + o] == WALL))
  {
    // Wall detected;
    return false;
  }
  return true;
} // tryNewDirection_pacer

void repaintPacer()
{
  if (blinkPacer)
  {
    switch (pacer_animPhase)
    {
    case 0:
      matrix.drawPixel(pacerX + 0 - o, pacerY + 0 - o, matrix.Color444(4, 4, 4)); // Eye
      matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
      matrix.drawPixel(pacerX + 2 - o, pacerY + 0 - o, pacerColor);
      matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
      matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
      matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
      matrix.drawPixel(pacerX + 2 - o, pacerY + 2 - o, pacerColor);
      break;
    case 1:
      matrix.fillRect(pacerX_old - o, pacerY_old - o, 3, 3, backgroundColor);
      break;
    }
  }
  else
  {
    // Clear pacer at old position
    matrix.fillRect(pacerX_old - o, pacerY_old - o, 3, 3, backgroundColor);
    // Draw pacer at new position
    switch (currentDirection)
    {
    case STILL:
      matrix.drawPixel(pacerX + 0 - o, pacerY + 0 - o, matrix.Color444(4, 4, 4)); // Eye
      matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
      matrix.drawPixel(pacerX + 2 - o, pacerY + 0 - o, pacerColor);
      matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
      matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
      matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
      matrix.drawPixel(pacerX + 2 - o, pacerY + 2 - o, pacerColor);
      break;

    case RIGHT:
      matrix.drawPixel(pacerX + 0 - o, pacerY + 0 - o, matrix.Color444(4, 4, 4)); // Eye

      switch (pacer_animPhase)
      {
      case 0:
        matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 2 - o, pacerColor);
        break;
      case 1:
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 2 - o, pacerColor); // May be omitted
        break;
      }
      break; // Direction RIGHT

    case LEFT:
      matrix.drawPixel(pacerX + 2 - o, pacerY + 0 - o, matrix.Color444(4, 4, 4)); // Eye

      switch (pacer_animPhase)
      {
      case 0:
        matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 2 - o, pacerColor);
        break;
      case 1:
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 2 - o, pacerColor); // May be omitted
        break;
      }
      break; // Direction LEFT

    case UP:
      matrix.drawPixel(pacerX + 0 - o, pacerY + 2 - o, matrix.Color444(4, 4, 4)); // Eye

      switch (pacer_animPhase)
      {
      case 0:
        matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 0 - o, pacerColor);
        break;
      case 1:
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 0 - o, pacerColor); // May be omitted
        break;
      }
      break; // Direction UP

    case DOWN:
      matrix.drawPixel(pacerX + 0 - o, pacerY + 0 - o, matrix.Color444(4, 4, 4)); // Eye

      switch (pacer_animPhase)
      {
      case 0:
        matrix.drawPixel(pacerX + 0 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 0 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 2 - o, pacerColor);
        break;
      case 1:
        matrix.drawPixel(pacerX + 2 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 2 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 0 - o, pacerColor);
        matrix.drawPixel(pacerX + 1 - o, pacerY + 1 - o, pacerColor);
        matrix.drawPixel(pacerX + 2 - o, pacerY + 2 - o, pacerColor); // May be omitted
        break;
      }
      break; // Direction DOWN
    } // switch currentDirection
  } // Pacer does not blink (we are not in the starting phase of the game)
} // repaintPacer

void playDeathAnimation(byte pacerX, byte pacerY)
{
  for (int i = 0; i < 600; i = i + 2)
  {
    tone(audioPacer, 1250 - i * 2, 2);
  }

  // delay(500);

  for (byte i = 0; i < 2; i++)
  {
    matrix.fillRect(pacerX, pacerY, 3, 3, backgroundColor);
    matrix.drawPixel(pacerX + 0, pacerY + 0, matrix.Color444(4, 4, 4)); // Eye
    matrix.drawPixel(pacerX + 1, pacerY + 0, pacerColor);
    matrix.drawPixel(pacerX + 2, pacerY + 0, pacerColor);
    matrix.drawPixel(pacerX + 0, pacerY + 1, pacerColor);
    matrix.drawPixel(pacerX + 1, pacerY + 2, pacerColor);
    matrix.drawPixel(pacerX + 1, pacerY + 2, pacerColor);
    matrix.drawPixel(pacerX + 2, pacerY + 2, pacerColor);
    delay(500);

    matrix.fillRect(pacerX, pacerY, 3, 3, backgroundColor);
    matrix.drawPixel(pacerX + 0, pacerY + 0, matrix.Color444(4, 4, 4)); // Eye
    matrix.drawPixel(pacerX + 1, pacerY + 2, pacerColor);
    matrix.drawPixel(pacerX + 2, pacerY + 1, pacerColor);
    matrix.drawPixel(pacerX + 0, pacerY + 1, pacerColor);
    matrix.drawPixel(pacerX + 1, pacerY + 1, pacerColor);
    matrix.drawPixel(pacerX + 2, pacerY + 2, pacerColor);
    delay(500);
  }

  matrix.drawPixel(pacerX + 0, pacerY + 0, matrix.Color444(4, 4, 4)); // Eye
  matrix.drawPixel(pacerX + 1, pacerY + 0, pacerColor);
  matrix.drawPixel(pacerX + 2, pacerY + 0, pacerColor);
  matrix.drawPixel(pacerX + 0, pacerY + 1, pacerColor);
  matrix.drawPixel(pacerX + 1, pacerY + 1, backgroundColor);
  matrix.drawPixel(pacerX + 2, pacerY + 1, backgroundColor);
  matrix.drawPixel(pacerX + 0, pacerY + 2, backgroundColor);
  matrix.drawPixel(pacerX + 1, pacerY + 2, pacerColor);
  matrix.drawPixel(pacerX + 2, pacerY + 2, pacerColor);
  delay(500);
  matrix.fillRect(pacerX, pacerY, 3, 3, backgroundColor);
}

// ************************************* BADDIES ***************************************************

boolean tryCurrentDirection_baddie(byte baddieNum)
{
  byte i_incr = 0;
  byte j_incr = 0;

  switch (baddie_currentDirection[baddieNum])
  {
  case STILL:
    i_incr = 0;
    j_incr = 0;
    break; // Standing still
  case UP:
    i_incr = 0;
    j_incr = -1;
    break; // Up
  case RIGHT:
    i_incr = +1;
    j_incr = 0;
    break; // Right
  case DOWN:
    i_incr = 0;
    j_incr = +1;
    break; // Down
  case LEFT:
    i_incr = -1;
    j_incr = 0;
    break; // Left
  }

  // if (positionOverlapsWithBaddie[baddieNum]) { return false; }

  if (tryNewDirection_baddie(baddieX[baddieNum] - o + i_incr, baddieY[baddieNum] - o + j_incr, baddie_currentDirection[baddieNum]))
  {
    baddieX[baddieNum] = baddieX[baddieNum] + i_incr;
    baddieY[baddieNum] = baddieY[baddieNum] + j_incr;

    return true;
  }
  return false;
} // tryCurrentDirection_baddie

// Same as above with an exception:
// When in the first or last row or column, the baddie is forbidden to cross the border
boolean tryCurrentDirection_baddie_invincible(byte baddieNum)
{
  if ((baddie_currentDirection[baddieNum] == LEFT) && (baddieX[baddieNum] == 1 + o))
  {
    return false;
  }
  else if ((baddie_currentDirection[baddieNum] == RIGHT) && (baddieX[baddieNum] == 28 + o))
  {
    return false;
  }
  else if ((baddie_currentDirection[baddieNum] == UP) && (baddieY[baddieNum] == 1 + o))
  {
    return false;
  }
  else if ((baddie_currentDirection[baddieNum] == DOWN) && (baddieY[baddieNum] == 28 + o))
  {
    return false;
  }

  byte i_incr = 0;
  byte j_incr = 0;

  switch (baddie_currentDirection[baddieNum])
  {
  case STILL:
    i_incr = 0;
    j_incr = 0;
    break; // Standing still
  case UP:
    i_incr = 0;
    j_incr = -1;
    break; // Up
  case RIGHT:
    i_incr = +1;
    j_incr = 0;
    break; // Right
  case DOWN:
    i_incr = 0;
    j_incr = +1;
    break; // Down
  case LEFT:
    i_incr = -1;
    j_incr = 0;
    break; // Left
  }

  if (tryNewDirection_baddie(baddieX[baddieNum] - o + i_incr, baddieY[baddieNum] - o + j_incr, baddie_currentDirection[baddieNum]))
  {
    baddieX[baddieNum] = baddieX[baddieNum] + i_incr;
    baddieY[baddieNum] = baddieY[baddieNum] + j_incr;

    return true;
  }
  return false;
} // tryCurrentDirection_baddie_invincible

boolean tryNewDirection_baddie(byte x, byte y, byte dir)
{
  // Check for walls
  if ((playfield[x + 0 + o][y + 0 + o] == WALL) || (playfield[x + 1 + o][y + 0 + o] == WALL) || (playfield[x + 2 + o][y + 0 + o] == WALL) ||
      (playfield[x + 0 + o][y + 1 + o] == WALL) || (playfield[x + 2 + o][y + 1 + o] == WALL) || (playfield[x + 0 + o][y + 2 + o] == WALL) ||
      (playfield[x + 1 + o][y + 2 + o] == WALL) || (playfield[x + 2 + o][y + 2 + o] == WALL))
  {
    // Wall detected;
    return false;
  }
  return true;
} // tryNewDirection_baddie

boolean positionOverlapsWithBaddie(byte num)
{
  for (int baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
  {
    if ((baddie_active[baddieNum]) && (baddieNum != num))
    {
      if ((abs(baddieX[num] - baddieX[baddieNum]) < 4) && (abs(baddieY[num] - baddieY[baddieNum]) < 4))
      {
        return true;
      }
    }
  }
  return false;
}

void moveBaddies()
{
  // Do not move the baddies at the opening of a stage. Instead, blink the player's sprite
  if (blinkPacer)
  {
    return;
  }

  if (bonusTimer == bonusInterval) // Player has not picked up the bonus item: make it disappear from screen
  {
    bonusTimer = 0; // resetPacer to start of bonusInterval
    paintBonusItem(false);
    bonusPresent = false;
  }
  else if (bonusTimer == timeAtWhichBonusAppears)
  {
    bonusItem = random(0, 6);
    paintBonusItem(true);
    bonusPresent = true;
    bonusTimer++;
  }
  else
  {
    bonusTimer++;
  }

  if (baddie_animPhase == 4)
  {
    baddie_animPhase = 0;
  }
  else
  {
    baddie_animPhase++;
  }
  if (invincibleTimer > 0)
  {
    invincibleTimer--;
  }
  else
  {
    playerInvincible = false;
  }

  for (int baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
  {
    if ((baddie_active[baddieNum]) && (!baddieGoingHome[baddieNum]))
    {
      // Save old location
      baddieX_old[baddieNum] = baddieX[baddieNum];
      baddieY_old[baddieNum] = baddieY[baddieNum];

      boolean goingThroughBorder = false;
      // If the baddie moves outside the playfield, guide it through it
      if (((baddieX[baddieNum] < 1 + o) || (baddieX[baddieNum] > 29 + o)) && (baddie_currentDirection[baddieNum] == LEFT))
      {
        baddieX[baddieNum]--;
        goingThroughBorder = true;
        repaintBaddie(baddieNum);
      }
      else if (((baddieX[baddieNum] > 28 + o) || (baddieX[baddieNum] < 1 + o)) && (baddie_currentDirection[baddieNum] == RIGHT))
      {
        baddieX[baddieNum]++;
        goingThroughBorder = true;
        repaintBaddie(baddieNum);
      }
      else if (((baddieY[baddieNum] < 1 + o) || (baddieY[baddieNum] > 29 + o)) && (baddie_currentDirection[baddieNum] == UP))
      {
        baddieY[baddieNum]--;
        goingThroughBorder = true;
        repaintBaddie(baddieNum);
      }
      else if (((baddieY[baddieNum] > 28 + o) || (baddieY[baddieNum] < 1 + o)) && (baddie_currentDirection[baddieNum] == DOWN))
      {
        baddieY[baddieNum]++;
        goingThroughBorder = true;
        repaintBaddie(baddieNum);
      }

      if (!goingThroughBorder)
      {
        // possibleDirections: 0 - STILL, 1 - UP, 2 - RIGHT, 3 - DOWN, 4 - LEFT
        boolean directionPossible[5] = {false, false, false, false, false};

        if (!playerInvincible) // Ordinary game play
        {
          switch (baddie_type[baddieNum])
          {
          case BADDIE_TYPE1: // This baddie's behavior is incalculable as it moves nearly randomly without preference

            // Check directions emerging from baddie's position
            if (getAllDirections(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) > 2) // Baddie is at a forking (not in a corridor)
            {
              boolean assigned = false;
              byte selectedDirection = STILL;
              do
              {
                selectedDirection = random(1, 5); // Direction in [1..4]
                switch (baddie_currentDirection[baddieNum])
                {
                case UP:
                  if ((selectedDirection != DOWN) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case RIGHT:
                  if ((selectedDirection != LEFT) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case DOWN:
                  if ((selectedDirection != UP) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case LEFT:
                  if ((selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                }
              } while (!assigned);
              baddie_currentDirection[baddieNum] = selectedDirection;
              tryCurrentDirection_baddie(baddieNum);
            }
            else if (getAllDirections(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) == 2) // Baddie is either in a corridor (stick with the current direction (if the player's sprite is not in sight) ), or in a corner (try another direction)
            {
              if (!tryCurrentDirection_baddie(baddieNum)) // Baddie has hit a wall within a corner
              {
                boolean assigned = false;
                byte selectedDirection = STILL;
                do
                {
                  selectedDirection = random(1, 5); // Direction in [1..4]

                  // When at a corner, do not move back
                  switch (baddie_currentDirection[baddieNum])
                  {
                  case UP:
                    if ((selectedDirection != DOWN) && (selectedDirection != UP) && (directionPossible[selectedDirection]))
                    {
                      assigned = true;
                    };
                    break;
                  case RIGHT:
                    if ((selectedDirection != LEFT) && (selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
                    {
                      assigned = true;
                    };
                    break;
                  case DOWN:
                    if ((selectedDirection != UP) && (selectedDirection != DOWN) && (directionPossible[selectedDirection]))
                    {
                      assigned = true;
                    };
                    break;
                  case LEFT:
                    if ((selectedDirection != RIGHT) && (selectedDirection != LEFT) && (directionPossible[selectedDirection]))
                    {
                      assigned = true;
                    };
                    break;
                  }
                } while (!assigned);
                baddie_currentDirection[baddieNum] = selectedDirection;
                tryCurrentDirection_baddie(baddieNum);
              }
            }
            else // Baddie is in a dead end - move in opposite direction
            {
              switch (baddie_currentDirection[baddieNum])
              {
              case UP:
                baddie_currentDirection[baddieNum] = DOWN;
                break;
              case RIGHT:
                baddie_currentDirection[baddieNum] = LEFT;
                break;
              case DOWN:
                baddie_currentDirection[baddieNum] = UP;
                break;
              case LEFT:
                baddie_currentDirection[baddieNum] = RIGHT;
                break;
              }

              if (!tryCurrentDirection_baddie(baddieNum))
              {
                boolean assigned = false;
                byte selectedDirection = STILL;
                do
                {
                  selectedDirection = random(1, 5); // Direction in [1..4]
                  if (directionPossible[selectedDirection])
                  {
                    assigned = true;
                  };
                  break;
                } while (!assigned);
                baddie_currentDirection[baddieNum] = selectedDirection;
                tryCurrentDirection_baddie(baddieNum);
              }
            } // Baddie is in a dead end

            break; // BADDIE_TYPE1

          case BADDIE_TYPE2: // If possible, preferably maintains its direction, seldom crosses the screen border

            // Save old location
            // baddieX_old[baddieNum] = baddieX[baddieNum];
            //
            // Check directions emerging from baddie's position
            if (getAllDirections(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) > 2) // Baddie is at a forking (not in a corridor)
            {
              boolean assigned = false;
              byte selectedDirection = STILL;
              do
              {
                byte randomNum = random(1, 11);
                if (randomNum < 8)
                {
                  selectedDirection = baddie_currentDirection[baddieNum];
                }
                else
                {
                  selectedDirection = random(1, 5);
                } // Direction in [1..4]
                switch (baddie_currentDirection[baddieNum])
                {
                //
                case UP:
                  if ((selectedDirection != DOWN) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case RIGHT:
                  if ((selectedDirection != LEFT) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case DOWN:
                  if ((selectedDirection != UP) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case LEFT:
                  if ((selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                }
              } while (!assigned);
              baddie_currentDirection[baddieNum] = selectedDirection;
              tryCurrentDirection_baddie(baddieNum);
            }
            else // Baddie is either in a corridor (stick with the current direction (if the player's sprite is not in sight) ), or in a corner (try another direction)
            {
              if (!tryCurrentDirection_baddie(baddieNum)) // Baddie has hit a wall within a corner
              {
                boolean assigned = false;
                byte selectedDirection = STILL;
                do
                {
                  selectedDirection = random(1, 5); // Direction in [1..4]
                  if (directionPossible[selectedDirection])
                  {
                    assigned = true;
                  };
                  break;
                } while (!assigned);
                baddie_currentDirection[baddieNum] = selectedDirection;
                tryCurrentDirection_baddie(baddieNum);
              }
            } // in a corridor or in a corner

            break; // BADDIE_TYPE2

          case BADDIE_TYPE3: // With a high probability moves towards the player

            // Save old location
            // baddieX_old[baddieNum] = baddieX[baddieNum];
            // baddieY_old[baddieNum] = baddieY[baddieNum];

            // Check directions emerging from baddie's position
            if (getAllDirections(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) > 2) // Baddie is at a forking (not in a corridor)
            {
              byte selectedDirection = STILL;
              int diffX = (baddieX[baddieNum] - o) - (pacerX - o);
              int diffY = (baddieY[baddieNum] - o) - (pacerY - o);

              if ((diffX > 0) && (directionPossible[LEFT]))
              {
                baddie_currentDirection[baddieNum] = LEFT;
              }
              else if ((diffX < 0) && (directionPossible[RIGHT]))
              {
                baddie_currentDirection[baddieNum] = RIGHT;
              }
              else if ((diffY > 0) && (directionPossible[UP]))
              {
                baddie_currentDirection[baddieNum] = UP;
              }
              else if ((diffY < 0) && (directionPossible[DOWN]))
              {
                baddie_currentDirection[baddieNum] = DOWN;
              }

              if (!tryCurrentDirection_baddie(baddieNum)) // Baddie cannot move towards the player. Choose a random direction
              {
                boolean assigned = false;
                byte selectedDirection = STILL;
                do
                {
                  selectedDirection = random(1, 5); // Direction in [1..4]
                  if (directionPossible[selectedDirection])
                  {
                    assigned = true;
                  };
                  break;
                } while (!assigned);

                baddie_currentDirection[baddieNum] = selectedDirection;
                tryCurrentDirection_baddie(baddieNum);
              }
            } // allDirections > 2
            else // Baddie is either in a corridor (stick with the current direction (if the player's sprite is not in sight) ), or in a corner (try another direction)
            {
              if (!tryCurrentDirection_baddie(baddieNum)) // Baddie has hit a wall within a corner
              {
                byte selectedDirection = STILL;
                int diffX = (baddieX[baddieNum] - o) - (pacerX - o);
                int diffY = (baddieY[baddieNum] - o) - (pacerY - o);
                boolean assigned = false;
                byte randomNum = random(1, 3);
                if (randomNum == 2) // Shorten the distance to the player either horizontally or vertically with equal probability
                {
                  if (diffX > 0)
                  {
                    selectedDirection = LEFT;
                  }
                  else
                  {
                    selectedDirection = RIGHT;
                  }
                }
                else
                {
                  if (diffY > 0)
                  {
                    selectedDirection = UP;
                  }
                  else
                  {
                    selectedDirection = DOWN;
                  }
                }
                baddie_currentDirection[baddieNum] = selectedDirection;

                if (!tryCurrentDirection_baddie(baddieNum))
                {
                  do
                  {
                    selectedDirection = random(1, 5); // Direction in [1..4]
                    if (directionPossible[selectedDirection])
                    {
                      assigned = true;
                    };
                    break;
                  } while (!assigned);

                  baddie_currentDirection[baddieNum] = selectedDirection;
                  tryCurrentDirection_baddie(baddieNum);
                }
              } // hit a wall within a corner
            } // Baddie is either in a corridor or in a corner

            break; // BADDIE_TYPE3

          case BADDIE_TYPE4: // Baddie keeps on running from wall to wall. It cannot cross the screen border

            // Check directions emerging from baddie's position
            if (getAllDirections(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) > 2) // Baddie is at a forking (not in a corridor)
            {
              if (!tryCurrentDirection_baddie(baddieNum)) // Baddie can no longer move in its current direction
              {
                // Try the directions clockwise
                boolean assigned = false;
                byte selectedDirection = STILL;

                do
                {
                  switch (baddie_currentDirection[baddieNum]) // Turn clockwise
                  {
                  case UP:
                    selectedDirection = RIGHT;
                    break;
                  case RIGHT:
                    selectedDirection = DOWN;
                    break;
                  case DOWN:
                    selectedDirection = LEFT;
                    break;
                  case LEFT:
                    selectedDirection = UP;
                    break;
                  case STILL:
                    selectedDirection = random(1, 5);
                    break;
                  }
                  if (directionPossible[selectedDirection])
                  {
                    baddie_currentDirection[baddieNum] = selectedDirection;
                    assigned = true;
                  }
                } while (!assigned);
                tryCurrentDirection_baddie(baddieNum);
              } // baddie cannot move in its current direction
            } // at least three directions possible
            else if (getAllDirections(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) == 2) // Baddie is either in a corridor (stick with the current direction (if the player's sprite is not in sight) ), or in a corner (try another direction)
            {
              if (!tryCurrentDirection_baddie(baddieNum)) // Baddie could not move in current direction
              {
                byte selectedDirection;
                switch (baddie_currentDirection[baddieNum])
                {
                case UP:
                  selectedDirection = DOWN;
                  break;
                case RIGHT:
                  selectedDirection = LEFT;
                  break;
                case DOWN:
                  selectedDirection = UP;
                  break;
                case LEFT:
                  selectedDirection = RIGHT;
                  break;
                }
                baddie_currentDirection[baddieNum] = selectedDirection;
                if (!tryCurrentDirection_baddie(baddieNum))
                {
                  boolean assigned = false;
                  byte selectedDirection;
                  do
                  {
                    selectedDirection = random(1, 5); // Direction in [1..4]
                    switch (baddie_currentDirection[baddieNum])
                    {

                    case UP:
                      if ((selectedDirection != DOWN) && (selectedDirection != UP) && (directionPossible[selectedDirection]))
                      {
                        assigned = true;
                      };
                      break;
                    case RIGHT:
                      if ((selectedDirection != LEFT) && (selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
                      {
                        assigned = true;
                      };
                      break;
                    case DOWN:
                      if ((selectedDirection != UP) && (selectedDirection != DOWN) && (directionPossible[selectedDirection]))
                      {
                        assigned = true;
                      };
                      break;
                    case LEFT:
                      if ((selectedDirection != RIGHT) && (selectedDirection != LEFT) && (directionPossible[selectedDirection]))
                      {
                        assigned = true;
                      };
                      break;
                    }
                  } while (!assigned);
                  baddie_currentDirection[baddieNum] = selectedDirection;
                  tryCurrentDirection_baddie(baddieNum);
                }
              } // could not move in current direction
            } // either at a corridor or in a corner
            else // Baddie is in a dead end - move in opposite direction if possible,...
            {
              switch (baddie_currentDirection[baddieNum])
              {
              case UP:
                baddie_currentDirection[baddieNum] = DOWN;
                break;
              case RIGHT:
                baddie_currentDirection[baddieNum] = LEFT;
                break;
              case DOWN:
                baddie_currentDirection[baddieNum] = UP;
                break;
              case LEFT:
                baddie_currentDirection[baddieNum] = RIGHT;
                break;
              }

              if (!tryCurrentDirection_baddie(baddieNum)) // ...otherwise, in a random direction
              {
                boolean assigned = false;
                byte selectedDirection = STILL;
                do
                {
                  selectedDirection = random(1, 5); // Direction in [1..4]
                  if (directionPossible[selectedDirection])
                  {
                    assigned = true;
                  }
                } while (!assigned);
                baddie_currentDirection[baddieNum] = selectedDirection;
                tryCurrentDirection_baddie(baddieNum);
              }
            } // baddie is in a dead end

            break; // BADDIE_TYPE4

          } // switch baddie_type

          // Did the baddie collide with the player?
          if ((abs((pacerX - o) - (baddieX[baddieNum] - o)) < 3) && (abs((pacerY - o) - (baddieY[baddieNum] - o)) < 3))
          {
            collision = true;
          }

        } // ordinary game play: player's sprite is not invincible
        else // player's sprite is invincible: the baddies flee
        {
          // Save old location
          baddieX_old[baddieNum] = baddieX[baddieNum];
          baddieY_old[baddieNum] = baddieY[baddieNum];

          // Has the player caught the baddie?
          if ((abs((pacerX - o) - (baddieX[baddieNum] - o)) < 3) && (abs((pacerY - o) - (baddieY[baddieNum] - o)) < 3))
          {
            // playCaughtBaddieNoise();
            issueMelody(BADDIECAUGHT_JINGLE);
            baddieGoingHome[baddieNum] = true;
            // Ensure that the background is kept clean
            matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, backgroundColor);
            restorePills();
            /*
            // Restore a pill or a super pill at the old position of the baddie
            byte cellX = (baddieX[baddieNum]-o) / 3;
            byte cellY = (baddieY[baddieNum]-o) / 3;
            if       (playfield[cellX*3+2 +o][cellY*3+2 +o] == PILL)        { matrix.drawPixel(cellX*3+2, cellY*3+2, pillColor); }
            else if  (playfield[cellX*3+2 +o][cellY*3+2 +o] == SUPER_PILL)  { matrix.drawPixel(cellX*3+2, cellY*3+2, superPillColor); }
            */
          }

          // Check directions emerging from baddie's position
          if (getAllDirections_invincible(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) > 2) // Baddie is at a forking (not in a corridor)
          {
            // Steer the baddie away from the player's sprite
            int diffX = (baddieX[baddieNum] - o) - (pacerX - o);
            int diffY = (baddieY[baddieNum] - o) - (pacerY - o);
            if (diffX > 0)
            {
              baddie_currentDirection[baddieNum] = RIGHT;
            }
            else
            {
              baddie_currentDirection[baddieNum] = LEFT;
            }
            if (diffY > 0)
            {
              baddie_currentDirection[baddieNum] = DOWN;
            }
            else
            {
              baddie_currentDirection[baddieNum] = UP;
            }

            if (!tryCurrentDirection_baddie_invincible(baddieNum)) // Baddie could not move into the opposite direction
            {
              boolean assigned = false;
              byte selectedDirection = STILL;
              do
              {
                selectedDirection = random(1, 5); // Direction in [1..4]

                // When at a corner, do not move back
                switch (baddie_currentDirection[baddieNum])
                {
                case UP:
                  if ((selectedDirection != DOWN) && (selectedDirection != UP) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case RIGHT:
                  if ((selectedDirection != LEFT) && (selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case DOWN:
                  if ((selectedDirection != UP) && (selectedDirection != DOWN) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                case LEFT:
                  if ((selectedDirection != RIGHT) && (selectedDirection != LEFT) && (directionPossible[selectedDirection]))
                  {
                    assigned = true;
                  };
                  break;
                }
              } while (!assigned);
              baddie_currentDirection[baddieNum] = selectedDirection;
              tryCurrentDirection_baddie_invincible(baddieNum);
            }
          } // possible directions > 2
          else // Baddie is either in a passage or in a corner
          {
            if (!tryCurrentDirection_baddie_invincible(baddieNum)) // Baddie could not move any longer into the current direction
            {
              boolean assigned = false;
              byte selectedDirection = STILL;
              do
              {
                selectedDirection = random(1, 5); // Direction in [1..4]
                if (directionPossible[selectedDirection])
                {
                  assigned = true;
                }
              } while (!assigned);
              baddie_currentDirection[baddieNum] = selectedDirection;
              tryCurrentDirection_baddie_invincible(baddieNum);
            } // Baddie could no longer move into the current direction
          } // baddie is in a passage or within a corner
        } // player is invincible, baddie flees (player has picked up a super pill)
      } // baddie is not going through the border
      else
      {
        // ADDED on 22.01.15: Did the baddie collide with the player within the passage?
        if (baddie_active[baddieNum] && (!playerInvincible)) // Ordinary game play
        {
          if ((abs((pacerX - o) - (baddieX[baddieNum] - o)) < 3) && (abs((pacerY - o) - (baddieY[baddieNum] - o)) < 3))
          {
            collision = true;
          }
        }
      } // check whether baddie has met the player within a passage
    } // baddie active and not going home

    // Is the baddie moving at the screen boundary?
    if ((baddieX[baddieNum] == 32 + o) && (baddie_currentDirection[baddieNum] == RIGHT))
    {
      baddieX_old[baddieNum] = baddieX[baddieNum];
      baddieX[baddieNum] = -2 + o;
    }
    else if ((baddieX[baddieNum] == -3 + o) && (baddie_currentDirection[baddieNum] == LEFT))
    {
      baddieX_old[baddieNum] = baddieX[baddieNum];
      baddieX[baddieNum] = 30 + o;
    }
    else if ((baddieY[baddieNum] == -3 + o) && (baddie_currentDirection[baddieNum] == UP))
    {
      baddieY_old[baddieNum] = baddieY[baddieNum];
      baddieY[baddieNum] = 30 + o;
    }
    else if ((baddieY[baddieNum] == 32 + o) && (baddie_currentDirection[baddieNum] == DOWN))
    {
      baddieY_old[baddieNum] = baddieY[baddieNum];
      baddieY[baddieNum] = -2 + o;
    }
  } // for each baddieNum

  repaintBaddies();
} // moveBaddies

// Returns the number of directions when baddie is at a forking.
byte getAllDirections(byte x, byte y, boolean alternateWay[])
{
  byte dirs = 0;
  // UP
  if ((playfield[x + 0 + o][y - 1 + o] != WALL) && (playfield[x + 1 + o][y - 1 + o] != WALL) && (playfield[x + 2 + o][y - 1 + o] != WALL))
  {
    alternateWay[UP] = true;
    dirs++;
  }
  // RIGHT
  if ((playfield[x + 3 + o][y + 0 + o] != WALL) && (playfield[x + 3 + o][y + 1 + o] != WALL) && (playfield[x + 3 + o][y + 2 + o] != WALL))
  {
    alternateWay[RIGHT] = true;
    dirs++;
  }
  // DOWN
  if ((playfield[x + 0 + o][y + 3 + o] != WALL) && (playfield[x + 1 + o][y + 3 + o] != WALL) && (playfield[x + 2 + o][y + 3 + o] != WALL))
  {
    alternateWay[DOWN] = true;
    dirs++;
  }
  // LEFT
  if ((playfield[x - 1 + o][y + 0 + o] != WALL) && (playfield[x - 1 + o][y + 1 + o] != WALL) && (playfield[x - 1 + o][y + 2 + o] != WALL))
  {
    alternateWay[LEFT] = true;
    dirs++;
  }
  return dirs;
}

// When the player is invincible, the baddies may not cross the border
// Returns the directions and their count when baddie is at a forking, but ignores the entrances to the border.
byte getAllDirections_invincible(byte x, byte y, boolean alternateWay[])
{
  byte dirs = 0;
  // UP
  if (y > 1)
  {
    if ((playfield[x + 0 + o][y - 1 + o] != WALL) && (playfield[x + 1 + o][y - 1 + o] != WALL) && (playfield[x + 2 + o][y - 1 + o] != WALL))
    {
      alternateWay[UP] = true;
      dirs++;
    }
  }
  // RIGHT
  if (x < 28)
  {
    if ((playfield[x + 3 + o][y + 0 + o] != WALL) && (playfield[x + 3 + o][y + 1 + o] != WALL) && (playfield[x + 3 + o][y + 2 + o] != WALL))
    {
      alternateWay[RIGHT] = true;
      dirs++;
    }
  }
  // DOWN
  if (y < 28)
  {
    if ((playfield[x + 0 + o][y + 3 + o] != WALL) && (playfield[x + 1 + o][y + 3 + o] != WALL) && (playfield[x + 2 + o][y + 3 + o] != WALL))
    {
      alternateWay[DOWN] = true;
      dirs++;
    }
  }
  // LEFT
  if (x > 1)
  {
    if ((playfield[x - 1 + o][y + 0 + o] != WALL) && (playfield[x - 1 + o][y + 1 + o] != WALL) && (playfield[x - 1 + o][y + 2 + o] != WALL))
    {
      alternateWay[LEFT] = true;
      dirs++;
    }
  }
  return dirs;
}

void paintOrdinaryBaddie(byte baddieNum)
{
  switch (baddie_type[baddieNum])
  {
  case BADDIE_TYPE1:
    matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, baddie1Color);
    switch (baddie_animPhase)
    {
    case 0:
      matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 1:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 2:
      matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 3:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    }
    break;

  case BADDIE_TYPE2:
    matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, baddie2Color);
    switch ((baddie_animPhase + 1) % 4)
    {
    case 0:
      matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 1:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 2:
      matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 3:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    }
    break;

  case BADDIE_TYPE3:
    matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, baddie3Color);
    switch ((baddie_animPhase + 2) % 4)
    {
    case 0:
      matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 1:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 2:
      matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 3:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    }
    break;

  case BADDIE_TYPE4:
    matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, baddie4Color); // (12, 8, 7), (8, 5, 4)
    switch ((baddie_animPhase + 3) % 4)
    {
    case 0:
      matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 1:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 2:
      matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    case 3:
      matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
      break;
    }
    break;
  }

  // Baddie's eyes
  matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o, matrix.Color444(5, 5, 5));
  matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o, matrix.Color444(5, 5, 5));
}

void paintFleeingBaddie(byte baddieNum)
{
  // matrix.fillRect(baddieX[baddieNum]-o, baddieY[baddieNum]-o, 3, 3, baddieHomePositionColor);
  // matrix.fillRect(baddieX[baddieNum]-o, baddieY[baddieNum]-o, 3, 3, matrix.Color888(30, 30, 48));
  matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, fleeingBaddieColor);
  switch (baddie_animPhase)
  {
  case 0:
    matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o + 2, backgroundColor);
    break;
  case 1:
    matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
    break;
  case 2:
    matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o + 2, backgroundColor);
    break;
  case 3:
    matrix.drawPixel(baddieX[baddieNum] - o + 1, baddieY[baddieNum] - o + 2, backgroundColor);
    break;
  }
  // Baddie's eyes
  int col;
  // With the old Adafruit library, used to be if ((animTimer + baddieNum) % 3 == 0) { col = matrix.Color888(8, 8, 32); } else { col = matrix.Color444(2, 2, 4); }
  if ((animTimer + baddieNum) % 3 == 0)
  {
    col = matrix.Color888(16, 16, 64);
  }
  else
  {
    col = matrix.Color444(2, 2, 4);
  }
  matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o, col);
  matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o, col);
}

void repaintBaddies()
{
  animCycle = -animCycle;

  // Clear baddies at old positions and draw them at their new positions
  for (baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
  {
    if ((baddie_active[baddieNum]) && (!baddieGoingHome[baddieNum]))
    {
      matrix.fillRect(baddieX_old[baddieNum] - o, baddieY_old[baddieNum] - o, 3, 3, backgroundColor);
      restorePills();
      /*
      // Restore a pill or a super pill or a bonus item at the old position of the baddie
      byte cellX = (baddieX_old[baddieNum]-o) / 3;
      byte cellY = (baddieY_old[baddieNum]-o) / 3;
      if       (playfield[cellX*3+2 +o][cellY*3+2 +o] == PILL)        { matrix.drawPixel(cellX*3+2, cellY*3+2, pillColor); }//matrix.Color333(1, 1, 2)); } // matrix.drawLine(0, 27, 31, 27, pillColor);
      else if  (playfield[cellX*3+2 +o][cellY*3+2 +o] == SUPER_PILL)  { matrix.drawPixel(cellX*3+2, cellY*3+2, superPillColor); }
      */
      // if (((cellX*3 == bonusX) && (cellY*3 == bonusY) && bonusPresent)) { paintBonusItem(true); }

      if (playerInvincible) // Use color blue for all baddies
      {
        if (invincibleTimer > 20) // Plenty of time for the player to catch the baddies
        {
          paintFleeingBaddie(baddieNum);
        }
        else // Last moments in which the baddies are fleeing: make them blink
        {
          if (animCycle == 1)
          {
            paintFleeingBaddie(baddieNum);
          }
          else
          {
            paintOrdinaryBaddie(baddieNum);
          }
        } // Last moments
      }
      else // Normal situation: the player's sprite is not invincible (i.e., has not picked up a super pill)
      {
        paintOrdinaryBaddie(baddieNum);
      } // not invincible
    } // baddie_active
  } // for baddieNum
} // repaintBaddies

void restorePills()
{
  if (!paintingAllowed)
  {
    return;
  }
  // Restore a pill or a super pill at the position of the baddie
  byte cellX = (baddieX_old[baddieNum] - o) / 3;
  byte cellY = (baddieY_old[baddieNum] - o) / 3;
  if (playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] == PILL)
  {
    matrix.drawPixel(cellX * 3 + 2, cellY * 3 + 2, pillColor);
  }
  else if (playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] == SUPER_PILL)
  {
    matrix.drawPixel(cellX * 3 + 2, cellY * 3 + 2, superPillColor);
  }
}

void repaintBaddie(byte baddieNum)
{
  animCycle = -animCycle;

  if ((baddie_active[baddieNum]) && (!baddieGoingHome[baddieNum]))
  {
    matrix.fillRect(baddieX_old[baddieNum] - o, baddieY_old[baddieNum] - o, 3, 3, backgroundColor);
    restorePills();
    /*
    // Restore a pill or a super pill at the old position of the baddie
    byte cellX = (baddieX_old[baddieNum]-o) / 3;
    byte cellY = (baddieY_old[baddieNum]-o) / 3;
    if       (playfield[cellX*3+2 +o][cellY*3+2 +o] == PILL)        { matrix.drawPixel(cellX*3+2, cellY*3+2, pillColor); }// matrix.Color333(1, 1, 2)); }
    else if  (playfield[cellX*3+2 +o][cellY*3+2 +o] == SUPER_PILL)  { matrix.drawPixel(cellX*3+2, cellY*3+2, superPillColor); }
    */
    // if (((cellX*3 == bonusX) && (cellY*3 == bonusY) && bonusPresent)) { paintBonusItem(true); }

    if (playerInvincible) // Use color blue for all baddies
    {
      if (invincibleTimer > 20) // Plenty of time for the player to catch the baddies
      {
        paintFleeingBaddie(baddieNum);
      }
      else // Last moments in which the baddies are fleeing: make them blink
      {
        if (animCycle == 1)
        {
          paintFleeingBaddie(baddieNum);
        }
        else
        {
          paintOrdinaryBaddie(baddieNum);
        }
      } // Last moments
    }
    else // Normal situation: the player's sprite is not invincible (i.e., has not picked up a super pill)
    {
      paintOrdinaryBaddie(baddieNum);
    } // not invincible
  } // baddie_active
} // repaintBaddie(baddieNum)

void repaintBaddiesGoingHome()
{
  // Clear baddies at old positions and draw them at their new positions
  for (baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
  {
    if ((baddie_active[baddieNum]) && (baddieGoingHome[baddieNum]))
    {
      // Clear
      matrix.fillRect(baddieX_old[baddieNum] - o, baddieY_old[baddieNum] - o, 3, 3, backgroundColor);
      restorePills();
      /*
      // Restore a pill or a super pill at the old position of the baddie
      byte cellX = (baddieX_old[baddieNum]-o) / 3;
      byte cellY = (baddieY_old[baddieNum]-o) / 3;
      if       (playfield[cellX*3+2 +o][cellY*3+2 +o] == PILL)        { matrix.drawPixel(cellX*3+2, cellY*3+2, pillColor); }
      else if  (playfield[cellX*3+2 +o][cellY*3+2 +o] == SUPER_PILL)  { matrix.drawPixel(cellX*3+2, cellY*3+2, superPillColor); }
      */
      // if (((cellX*3 == bonusX) && (cellY*3 == bonusY) && bonusPresent)) { paintBonusItem(true); }

      // Draw baddie's eyes only
      matrix.drawPixel(baddieX[baddieNum] - o + 0, baddieY[baddieNum] - o, baddieEyeColor);
      matrix.drawPixel(baddieX[baddieNum] - o + 2, baddieY[baddieNum] - o, baddieEyeColor);

      // Animate baddie's home position
      switch (animTimer)
      {
      case 0:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 0, baddie_initialY[baddieNum] - o + 1, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 0, baddie_initialY[baddieNum] - o + 0, baddieHomePositionColor);
        break;
      case 1:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 0, baddie_initialY[baddieNum] - o + 0, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 1, baddie_initialY[baddieNum] - o + 0, baddieHomePositionColor);
        break;
      case 2:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 1, baddie_initialY[baddieNum] - o + 0, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 2, baddie_initialY[baddieNum] - o + 0, baddieHomePositionColor);
        break;
      case 3:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 2, baddie_initialY[baddieNum] - o + 0, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 2, baddie_initialY[baddieNum] - o + 1, baddieHomePositionColor);
        break;
      case 4:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 2, baddie_initialY[baddieNum] - o + 1, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 2, baddie_initialY[baddieNum] - o + 2, baddieHomePositionColor);
        break;
      case 5:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 2, baddie_initialY[baddieNum] - o + 2, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 1, baddie_initialY[baddieNum] - o + 2, baddieHomePositionColor);
        break;
      case 6:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 1, baddie_initialY[baddieNum] - o + 2, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 0, baddie_initialY[baddieNum] - o + 2, baddieHomePositionColor);
        break;
      case 7:
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 0, baddie_initialY[baddieNum] - o + 2, backgroundColor);
        matrix.drawPixel(baddie_initialX[baddieNum] - o + 0, baddie_initialY[baddieNum] - o + 1, baddieHomePositionColor);
        break;
      }
    } // baddie_active
  } // for baddieNum
} // repaintBaddiesGoingHome

void moveBaddiesGoingHome() // Called from a Timer
{
  if (animTimer == 8)
  {
    animTimer = 0;
  }
  else
  {
    animTimer++;
  }

  for (int baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
  {
    // if (baddie_active[baddieNum] && baddieGoingHome[baddieNum])
    if (baddieGoingHome[baddieNum])
    {
      // Repaint bonus items that this baddie has just erased
      // if ((abs(baddieX[baddieNum]-o - bonusX) < 4) && (abs(baddieY[baddieNum]-o - bonusY) < 4) && bonusPresent) { paintBonusItem(true); } // If necessary, repaint the bonus item

      // possibleDirections: 0 - STILL, 1 - UP, 2 - RIGHT, 3 - DOWN, 4 - LEFT
      boolean directionPossible[5] = {false, false, false, false, false};

      // Save old location
      baddieX_old[baddieNum] = baddieX[baddieNum];
      baddieY_old[baddieNum] = baddieY[baddieNum];

      // Check directions emerging from baddie's position
      if (getAllDirections_invincible(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) > 2) // Baddie is at a forking (not in a corridor)
      {
        boolean assigned = false;
        byte selectedDirection = STILL;
        do
        {
          selectedDirection = random(1, 5); // Direction in [1..4]
          switch (baddie_currentDirection[baddieNum])
          {
          case UP:
            if ((selectedDirection != DOWN) && (directionPossible[selectedDirection]))
            {
              assigned = true;
            };
            break;
          case RIGHT:
            if ((selectedDirection != LEFT) && (directionPossible[selectedDirection]))
            {
              assigned = true;
            };
            break;
          case DOWN:
            if ((selectedDirection != UP) && (directionPossible[selectedDirection]))
            {
              assigned = true;
            };
            break;
          case LEFT:
            if ((selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
            {
              assigned = true;
            };
            break;
          }
        } while (!assigned);
        baddie_currentDirection[baddieNum] = selectedDirection;
        tryCurrentDirection_baddie_invincible(baddieNum);
      }
      else if (getAllDirections_invincible(baddieX[baddieNum] - o, baddieY[baddieNum] - o, directionPossible) == 2) // Baddie is either in a corridor (stick with the current direction (if the player's sprite is not in sight) ), or in a corner (try another direction)
      {
        if (!tryCurrentDirection_baddie_invincible(baddieNum)) // Baddie has hit a wall within a corner
        {
          boolean assigned = false;
          byte selectedDirection = STILL;
          do
          {
            selectedDirection = random(1, 5); // Direction in [1..4]

            // When at a corner, do not move back
            switch (baddie_currentDirection[baddieNum])
            {
            case UP:
              if ((selectedDirection != DOWN) && (selectedDirection != UP) && (directionPossible[selectedDirection]))
              {
                assigned = true;
              };
              break;
            case RIGHT:
              if ((selectedDirection != LEFT) && (selectedDirection != RIGHT) && (directionPossible[selectedDirection]))
              {
                assigned = true;
              };
              break;
            case DOWN:
              if ((selectedDirection != UP) && (selectedDirection != DOWN) && (directionPossible[selectedDirection]))
              {
                assigned = true;
              };
              break;
            case LEFT:
              if ((selectedDirection != RIGHT) && (selectedDirection != LEFT) && (directionPossible[selectedDirection]))
              {
                assigned = true;
              };
              break;
            }
          } while (!assigned);
          baddie_currentDirection[baddieNum] = selectedDirection;
          tryCurrentDirection_baddie_invincible(baddieNum);
        }
      }
      else // Baddie is in a dead end - move in opposite direction
      {
        switch (baddie_currentDirection[baddieNum])
        {
        case UP:
          baddie_currentDirection[baddieNum] = DOWN;
          break;
        case RIGHT:
          baddie_currentDirection[baddieNum] = LEFT;
          break;
        case DOWN:
          baddie_currentDirection[baddieNum] = UP;
          break;
        case LEFT:
          baddie_currentDirection[baddieNum] = RIGHT;
          break;
        }
        tryCurrentDirection_baddie_invincible(baddieNum);
      }

      // Has the baddie arrived at its home position (i. e., from where it started)?
      if ((baddieX[baddieNum] == baddie_initialX[baddieNum]) && (baddieY[baddieNum] == baddie_initialY[baddieNum]))
      {
        baddieGoingHome[baddieNum] = false;
        matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, backgroundColor);
        matrix.fillRect(baddieX_old[baddieNum] - o, baddieY_old[baddieNum] - o, 3, 3, backgroundColor);
      }
    } // baddie is active and going home
  } // for each baddie

  repaintBaddiesGoingHome();
} // moveBaddiesGoingHome

// ***********************************  PAINT BONI *********************************************

void paintBonusItem(boolean paintItem)
{
  if (!paintingAllowed)
  {
    return;
  }

  byte x = bonusX;
  byte y = bonusY;

  if (paintItem) // Paint item
  {
    matrix.fillRect(x, y, 3, 3, backgroundColor);

    if (animCycle == -1)
    {
      switch (bonusItem)
      {
      case 0:
        //?
        matrix.fillRect(x + 0, y + 1, 2, 2, matrix.Color333(0, 2, 0));
        matrix.drawLine(x + 1, y + 1, x + 2, y + 0, matrix.Color333(2, 1, 0));
        break;

      case 1:
        // Ice cream
        matrix.drawPixel(x + 0, y + 1, matrix.Color444(3, 3, 5));
        matrix.drawPixel(x + 0, y + 2, matrix.Color444(3, 3, 5));
        matrix.drawPixel(x + 1, y + 2, matrix.Color444(3, 3, 5));
        matrix.drawPixel(x + 2, y + 2, matrix.Color444(3, 3, 5));
        matrix.drawPixel(x + 2, y + 1, matrix.Color444(3, 3, 5));

        matrix.drawPixel(x + 1, y + 1, matrix.Color444(5, 2, 0));
        matrix.drawPixel(x + 2, y + 0, matrix.Color444(5, 2, 0));
        break;

      case 2:
        // Apple
        matrix.drawPixel(x + 0, y + 0, matrix.Color444(4, 0, 0));
        matrix.drawPixel(x + 1, y + 0, matrix.Color444(12, 0, 0));
        matrix.drawPixel(x + 1, y + 1, matrix.Color444(12, 0, 0));
        matrix.drawPixel(x + 1, y + 2, matrix.Color444(12, 0, 0));
        matrix.drawPixel(x + 2, y + 0, matrix.Color444(4, 0, 0));
        matrix.drawPixel(x + 0, y + 2, matrix.Color444(4, 0, 0));
        matrix.drawPixel(x + 2, y + 1, matrix.Color444(8, 0, 0));
        matrix.drawPixel(x + 0, y + 1, matrix.Color444(4, 4, 4));
        break;

      case 3:
        // Flower
        matrix.drawPixel(x + 1, y + 2, matrix.Color333(2, 1, 0));
        matrix.drawPixel(x + 1, y + 1, matrix.Color333(2, 1, 0));
        matrix.drawPixel(x + 1, y + 0, matrix.Color333(1, 0, 3));
        matrix.drawPixel(x + 2, y + 0, matrix.Color333(3, 0, 1));
        matrix.drawPixel(x + 2, y + 1, matrix.Color333(1, 0, 3));
        matrix.drawPixel(x + 0, y + 1, matrix.Color333(0, 2, 0));
        break;

      case 4:
        // Banana
        matrix.drawPixel(x + 2, y + 0, baddieEyeColor);
        matrix.drawPixel(x + 1, y + 0, matrix.Color444(4, 3, 0));
        matrix.drawPixel(x + 0, y + 1, matrix.Color444(3, 4, 0));
        matrix.drawPixel(x + 0, y + 2, matrix.Color444(4, 4, 0));
        break;

      case 5:
        // Teddy Bear
        matrix.drawPixel(x + 0, y + 0, matrix.Color444(6, 5, 0));
        matrix.drawPixel(x + 1, y + 0, matrix.Color444(3, 2, 0));
        matrix.drawPixel(x + 0, y + 1, matrix.Color444(2, 1, 0));
        matrix.drawPixel(x + 1, y + 1, matrix.Color444(4, 3, 0));
        matrix.drawPixel(x + 2, y + 1, matrix.Color444(3, 2, 0));
        matrix.drawPixel(x + 1, y + 2, matrix.Color444(3, 1, 0));
        break;
      } // switch bonus item
    }
    else // Does not yet work
    {
      matrix.fillRect(x, y, 3, 3, backgroundColor);
    }
  }
  else // Remove item
  {
    matrix.fillRect(x, y, 3, 3, backgroundColor);
    // Restore a pill or a super pill at the old position of the baddie
    byte cellX = x / 3;
    byte cellY = y / 3;
    if (playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] == PILL)
    {
      matrix.drawPixel(cellX * 3 + 2, cellY * 3 + 2, pillColor);
    } // matrix.Color333(1, 1, 2)); }
    else if (playfield[cellX * 3 + 2 + o][cellY * 3 + 2 + o] == SUPER_PILL)
    {
      matrix.drawPixel(cellX * 3 + 2, cellY * 3 + 2, superPillColor);
    }
  }
}

void determineBonusItemLocation()
{
  boolean positionFound = false;
  byte i;
  byte j;

  while (!positionFound)
  {
    i = random(0, 10);
    j = random(0, 10);

    if (labyrinth[(j * 10) + i] < WALL)
    {
      bonusX = i * 3 + 1;
      bonusY = j * 3 + 1;
      if (((bonusX - 1) != (pacer_initialX - o)) && ((bonusY - 1) != (pacer_initialY - o)))
      {
        positionFound = true;
      }
      for (byte baddieNum = 0; baddieNum < NUM_BADDIES; baddieNum++)
      {
        if (((bonusX - 1) == (baddie_initialX[baddieNum] - o)) && ((bonusY - 1) == (baddie_initialY[baddieNum] - o)))
        {
          positionFound = false;
        }
      }
    }
  }
}

// *********************** NOISES AND SOUNDS ******************************

void playMunchingNoise()
{
  tone(audioPacer, NOTE_G4, 40);
  tone(audioPacer, NOTE_C3, 30);
  tone(audioPacer, NOTE_G5, 20);
  tone(audioPacer, NOTE_C2, 40);
  tone(audioPacer, NOTE_E3, 60);
  tone(audioPacer, NOTE_B4, 20);
}

void playCaughtBaddieNoise()
{
  for (byte t = 0; t < 20; t++)
  {
    tone(audioPacer, random(800, 1000), 2);
    delay(2);
  }
  for (byte t = 0; t < 20; t++)
  {
    tone(audioPacer, random(300, 400), 2);
    delay(2);
  }
}

void playSuperPillNoise()
{
  for (int i = 0; i < 120; i++)
  {
    tone(audioPacer, 500 + i * 3, 1);
  }
  for (int i = 40; i > 0; i--)
  {
    tone(audioPacer, 1500 + i * 2, 1);
  }
  for (int i = 0; i < 60; i++)
  {
    tone(audioPacer, 750 + i * 2, 1);
  }
}

void playOpeningJingle()
{
  byte dur = 90;
  tone(audioPacer, NOTE_C4, 0.9 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_G4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_C5, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_G4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_E4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_C5, 2.0 * dur);
  delay(4 * dur);
  //
  tone(audioPacer, NOTE_D4, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_D4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_D5, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_A4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_FS4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_D5, 2.0 * dur);
  delay(4 * dur);
  //
  tone(audioPacer, NOTE_C4, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_C4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_C5, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_G4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_E4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_C5, 2.0 * dur);
  delay(4 * dur);

  tone(audioPacer, NOTE_G4, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_G4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_G4, 0.5 * dur);
  delay(1 * dur);
  tone(audioPacer, NOTE_GS4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_A4, 1.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_AS4, 2.0 * dur);
  delay(2 * dur);
  tone(audioPacer, NOTE_B4, 2.0 * dur);
  delay(2 * dur);
}

void playGameOverJingle()
{
  int freq1 = NOTE_AS4;
  int freq2 = NOTE_A4;
  int freq3 = NOTE_GS4;
  int freq4 = NOTE_G4;
  int freq5 = NOTE_FS4;

  for (int f = freq1; f > freq2; f--)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq2; f < freq1; f++)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq2; f > freq3; f--)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq3; f < freq2; f++)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq3; f > freq4; f--)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq4; f < freq3; f++)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq4; f > freq5; f--)
  {
    tone(audioPacer, f, 5);
  }
  for (int f = freq5; f < freq4; f++)
  {
    tone(audioPacer, f, 5);
  }
  tone(audioPacer, freq5, 100);
  delay(1000);
}

void issueMelody(byte num)
{
  if (!audioIsProduced)
  {
    switch (num)
    {
    case BADDIESFLEE_MELODY:
      melodyPointer = baddiesFleeMelody;
      melodyPosition = 0;
      melodyLength = 134;
      audioIsProduced = true;
      break;
    case BONUSITEM_JINGLE:
      melodyPointer = bonusCollectedJingle;
      melodyPosition = 0;
      melodyLength = 13;
      audioIsProduced = true;
      break;
    case ADDITIONALLIFE_JINGLE:
      melodyPointer = additionalLifeMelody;
      melodyPosition = 0;
      melodyLength = 26;
      audioIsProduced = true;
      break;
    case BADDIECAUGHT_JINGLE:
      melodyPointer = baddieCaughtJingle;
      melodyPosition = 0;
      melodyLength = 7;
      audioIsProduced = true;
      break;
    default:
      melodyLength = 0;
      melodyPosition = 0;
      audioIsProduced = false;
      break;
    }
  }
  return;
}

void playMelody()
{
  if (audioIsProduced)
  {
    tone(audioPacer, *(melodyPointer + melodyPosition), 70);
    if (melodyPosition < melodyLength)
    {
      melodyPosition++;
    }
    else
    {
      melodyPosition = 0;
      audioIsProduced = false;
    }
  }
  return;
}

// ************************** TITLE SCREEN ******************************

void showTitle()
{
  paintingAllowed = false;
  int col;
  byte p;
  byte img[32 * 13];
  matrix.fillScreen(backgroundColor); // Clear screen
  for (int k = 0; k < 13 * 32; k++)
  {
    img[k] = pgm_read_byte_near(titleImg + k);
  }

  for (byte x = 0; x < 32; x++)
  {
    for (byte y = 0; y < 13; y++)
    {
      p = img[y * 32 + x];
      switch (p)
      {
      // With the old Adafruit library, used to be case 0: col = matrix.Color888(8, 4, 0); break;  // orange
      case 0:
        col = matrix.Color888(92, 26, 0);
        break; // orange
      case 1:
        col = matrix.Color888(16, 0, 0);
        break; // red
      case 2:
        col = backgroundColor;
        break; // black
      case 3:
        col = matrix.Color444(4, 4, 4);
        break; // white
      case 4:
        col = matrix.Color888(160, 216, 0); // yellow
      }
      matrix.drawPixel(x, y, col);
    }
  }

  // Paint Pacer
  pacerX_old = 40;
  pacerY_old = 40; // This avoids the sprites to be erased from the background at their old position
  for (int baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
  {
    baddieX_old[baddieNum] = 40;
    baddieY_old[baddieNum] = 40;
  }
  pacerX = 10;
  pacerY = 20;
  repaintPacer();

  // Paint baddies
  for (int i = 0; i < NUM_BADDIES - 1; i++)
  {
    baddie_active[i] = false;
  }
  baddieX[0] = -1 + o;
  baddieY[0] = 32;
  baddieGoingHome[0] = false;
  baddie_active[0] = true;
  baddie_type[0] = BADDIE_TYPE1;
  baddieX_old[0] = baddieX[0];
  baddieY_old[0] = baddieY[0];
  baddieX[1] = 3 + o;
  baddieY[1] = 32;
  baddieGoingHome[1] = false;
  baddie_active[1] = true;
  baddie_type[1] = BADDIE_TYPE2;
  baddieX_old[1] = baddieX[1];
  baddieY_old[1] = baddieY[1];
  baddieX[2] = 24 + o;
  baddieY[2] = 20;
  baddieGoingHome[2] = false;
  baddie_active[2] = true;
  baddie_type[2] = BADDIE_TYPE3;
  baddieX_old[2] = baddieX[2];
  baddieY_old[2] = baddieY[2];
  baddieX[3] = 29 + o;
  baddieY[3] = 32;
  baddieGoingHome[3] = false;
  baddie_active[3] = true;
  baddie_type[3] = BADDIE_TYPE4;
  baddieX_old[3] = baddieX[3];
  baddieY_old[3] = baddieY[3];
  repaintBaddies();

  // Paint pills
  matrix.drawPixel(11, 18, pillColor); // matrix.Color333(1, 1, 2));
  matrix.drawPixel(14, 18, pillColor);
  matrix.drawPixel(17, 18, superPillColor);
  matrix.drawPixel(2, 29, backgroundColor); // Sometimes, there is a dot at this position (?!)

  // Draw walls
  matrix.drawLine(7, 20, 28, 20, matrix.Color444(0, 0, 6));
  matrix.drawLine(28, 20, 28, 28, matrix.Color444(0, 0, 6));
  matrix.drawLine(28, 28, 0, 28, matrix.Color444(0, 0, 6));

  consumeJoystickEvents();
  while (!joy1Fire() && !joy1Left() && !joy1Right() && !joy1Up() && !joy1Down())
  {
  } // Wait for joystick operation

  matrix.fillScreen(backgroundColor); // Clear screen
  paintingAllowed = true;
}

// ************************ UTILITIES *****************************

int getNextMazeColor()
{
  if (mazeColorNum == numMazeColors - 1)
  {
    mazeColorNum = 0;
  }
  else
  {
    mazeColorNum++;
  }
  return mazeColors[mazeColorNum];
}

void debugPlayfield()
{
  for (byte i = 0; i < 38; i++)
  {
    Serial.print("\n");
    for (byte j = 0; j < 38; j++)
    {
      Serial.print(playfield[i][j]);
    }
  }
}

// Waits until the joystick is left alone
void consumeJoystickEvents()
{
  do
  {
  } while (joy1Left() || joy1Right() || joy1Up() || joy1Down() || joy1Fire());
}

void playLargeInterlude(byte xoffset, byte yoffset)
{
  byte gx = 18;
  byte gy = 16;

  pacerX_old = 40;
  pacerY_old = 40; // This avoids the pacer to be cleared from the background at its old position
  pacerX = xoffset;
  pacerY = yoffset;

  // First, let pacer move to a new position
  if (pacerX - o > gx)
  {
    currentDirection = LEFT;
  }
  else
  {
    currentDirection = RIGHT;
  }
  do
  {
    pacerX_old = pacerX;
    pacerY_old = pacerY;
    if (pacer_animPhase == 1)
    {
      pacer_animPhase = 0;
    }
    else
    {
      pacer_animPhase = 1;
    }
    if (pacerX - o > gx)
    {
      pacerX--;
    }
    else
    {
      pacerX++;
    }
    repaintPacer();
    delay(100);
  } while (pacerX - o != gx);
  if (pacerY - o > gy)
  {
    currentDirection = UP;
  }
  else
  {
    currentDirection = DOWN;
  }
  do
  {
    pacerX_old = pacerX;
    pacerY_old = pacerY;
    if (pacer_animPhase == 1)
    {
      pacer_animPhase = 0;
    }
    else
    {
      pacer_animPhase = 1;
    }
    if (pacerY - o > gy)
    {
      pacerY--;
    }
    else
    {
      pacerY++;
    }
    repaintPacer();
    delay(100);
  } while (pacerY - o != gy);

  // Pacer pauses for a moment...
  currentDirection = LEFT;
  matrix.fillRect(pacerX - o, pacerY - o, 3, 3, matrix.Color444(0, 0, 0));
  repaintPacer();

  // Move in a platform from the left. Pacer walks to the left...
  baddieX[0] = -45 + o;
  baddieY[0] = gy + o;
  baddieX[1] = -42 + o;
  baddieY[1] = gy + o;
  baddieX[2] = -39 + o;
  baddieY[2] = gy + o;
  baddieX[3] = -36 + o;
  baddieY[3] = gy + o;
  baddie_type[0] = BADDIE_TYPE1;
  baddie_type[1] = BADDIE_TYPE2;
  baddie_type[2] = BADDIE_TYPE3;
  baddie_type[3] = BADDIE_TYPE4;

  for (int x1 = -45; x1 < 1; x1++)
  // for (int x1 = -45; x1 < 0; x1++)
  {
    // Paint platform
    matrix.drawLine(x1, gy + 3, x1 + 32, gy + 3, matrix.Color444(0, 0, 8));
    matrix.drawLine(x1 + 36, gy + 3, x1 + 44, gy + 3, matrix.Color444(0, 0, 8));
    matrix.drawPixel(x1 + 44, gy + 3, matrix.Color888(50, 50, 50)); // White glow

    // Paint baddies
    paintOrdinaryBaddie(0);
    paintOrdinaryBaddie(1);
    paintOrdinaryBaddie(2);
    paintOrdinaryBaddie(3);
    baddieX[0]++;
    baddieX[1]++;
    baddieX[2]++;
    baddieX[3]++;
    // Paint Pacer
    pacerX_old = pacerX;
    pacerY_old = pacerY;
    repaintPacer();
    if (pacer_animPhase == 1)
    {
      pacer_animPhase = 0;
    }
    else
    {
      pacer_animPhase = 1;
    }
    delay(100);
    repaintPacer();
    matrix.drawLine(0, gy + 3, 31, gy + 3, matrix.Color444(0, 0, 0)); // Clear platform
  }
  matrix.drawLine(0, gy + 3, 31, gy + 3, matrix.Color444(0, 0, 8));
  // Baddie stops short, then turns right
  delay(1000);
  currentDirection = RIGHT;
  repaintPacer();
  delay(500);

  // Pacer flees
  int jump0[11] = {0, 3, 3, 2, 1, 0, -1, -2, -3, -3, 0};
  int jump1[9] = {0, 3, 2, 1, 0, -1, -2, -3, 0};
  int jump2[14] = {0, 3, 3, 2, 2, 1, 0, 0, -1, -2, -2, -3, -3, 0};
  int jump3[7] = {0, 3, 1, 0, -1, -3, 0};
  byte c0 = 0;
  byte c1 = 0;
  byte c2 = 0;
  byte c3 = 0;
  do
  {
    if (pacer_animPhase == 1)
    {
      pacer_animPhase = 0;
    }
    else
    {
      pacer_animPhase = 1;
    }
    repaintPacer();
    pacerX_old = pacerX;
    pacerX = pacerX + 2;
    // Baddies follow
    if (pacerX > 24) // The pacer has a head start
    {
      if (baddie_animPhase == 4)
      {
        baddie_animPhase = 0;
      }
      else
      {
        baddie_animPhase++;
      }
      matrix.fillRect(baddieX[0] - o, baddieY[0] - o, 3, 3, matrix.Color444(0, 0, 0));
      matrix.fillRect(baddieX[1] - o, baddieY[1] - o, 3, 3, matrix.Color444(0, 0, 0));
      matrix.fillRect(baddieX[2] - o, baddieY[2] - o, 3, 3, matrix.Color444(0, 0, 0));
      matrix.fillRect(baddieX[3] - o, baddieY[3] - o, 3, 3, matrix.Color444(0, 0, 0));
      if (pacerX > 24)
      {
        baddieX[0]++;
      }
      if (pacerX > 39)
      {
        baddieX[1]++;
      }
      if (pacerX > 28)
      {
        baddieX[2]++;
      }
      if (pacerX > 26)
      {
        baddieX[3]++;
      }
      if ((pacerX > 26) && (c0 < 11))
      {
        baddieY[0] = baddieY[0] - jump0[c0];
        c0++;
      }
      if ((pacerX > 40) && (c1 < 9))
      {
        baddieY[1] = baddieY[1] - jump1[c1];
        c1++;
      }
      if ((pacerX > 32) && (c2 < 14))
      {
        baddieY[2] = baddieY[2] - jump2[c2];
        c2++;
      }
      if ((pacerX > 28) && (c3 < 7))
      {
        baddieY[3] = baddieY[3] - jump3[c3];
        c3++;
      }
      paintOrdinaryBaddie(0);
      paintOrdinaryBaddie(1);
      paintOrdinaryBaddie(2);
      paintOrdinaryBaddie(3);
    }
    matrix.drawLine(0, 10, 0, 18, backgroundColor); // Check if this deletes the visual artifact on the first column
    delay(100);
  } while (baddieX[0] < 42);

  // Let the platform disintegrate
  for (byte x = 31; x > 0; x--)
  {
    matrix.drawPixel(x, gy + 3, matrix.Color888(50, 50, 50));
    delay(15);
    matrix.drawPixel(x, gy + 3, backgroundColor);
    delay(35);
  }
  matrix.fillRect(0, 0, 32, 32, matrix.Color444(0, 0, 0));
} // PlayLargeInterlude

void playSmallInterlude(byte xoffset, byte yoffset)
{
  delay(500);

  // Learn the pacer's new starting position in the next maze
  byte gx;
  byte gy;
  learnPacerStartingPosition(&gx, &gy);

  pacerX = xoffset;
  pacerY = yoffset;

  // Draw the passage from the Pacer's current position to the new position
  int phase = 1;
  int col;
  pacer_animPhase = 0;
  if ((abs(pacerX - o - gx) > 2) && (abs(pacerY - o - gy) > 2)) // Ensure that the passage is reasonably long
  {
    for (byte r = 0; r < 4; r++)
    {
      phase = -phase;
      if (phase == 1)
      {
        col = matrix.Color444(0, 0, 8);
      }
      else
      {
        col = matrix.Color888(50, 50, 50);
      }
      if (pacerY - o < gy)
      {
        if (pacerX - o < gx)
        {
          currentDirection = RIGHT;
          matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
          repaintPacer();
          matrix.drawLine(pacerX - o, pacerY - 1 - o, gx + 3, pacerY - 1 - o, col);
          matrix.drawLine(pacerX - o, pacerY + 3 - o, gx - 1, pacerY + 3 - o, col);
          matrix.drawLine(gx + 3, pacerY - 1 - o, gx + 3, gy + 2, col);
          matrix.drawLine(gx - 1, pacerY + 3 - o, gx - 1, gy + 2, col);
        }
        else
        {
          currentDirection = LEFT;
          matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
          repaintPacer();
          matrix.drawLine(pacerX + 2 - o, pacerY - 1 - o, gx - 1, pacerY - 1 - o, col);
          matrix.drawLine(pacerX + 2 - o, pacerY + 3 - o, gx + 3, pacerY + 3 - o, col);
          matrix.drawLine(gx - 1, pacerY - 1 - o, gx - 1, gy + 2, col);
          matrix.drawLine(gx + 3, pacerY + 3 - o, gx + 3, gy + 2, col);
        }
      }
      else
      {
        if (pacerX - o < gx)
        {
          currentDirection = RIGHT;
          matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
          repaintPacer();
          matrix.drawLine(pacerX - o, pacerY - 1 - o, gx - 1, pacerY - 1 - o, col);
          matrix.drawLine(pacerX - o, pacerY + 3 - o, gx + 3, pacerY + 3 - o, col);
          matrix.drawLine(gx - 1, pacerY - 1 - o, gx - 1, gy - 1, col);
          matrix.drawLine(gx + 3, pacerY + 3 - o, gx + 3, gy - 1, col);
        }
        else
        {
          currentDirection = LEFT;
          matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
          repaintPacer();
          matrix.drawLine(pacerX + 3 - o, pacerY - 1 - o, gx + 3, pacerY - 1 - o, col);
          matrix.drawLine(pacerX + 3 - o, pacerY + 3 - o, gx - 1, pacerY + 3 - o, col);
          matrix.drawLine(gx + 3, pacerY - 1 - o, gx + 3, gy - 1, col);
          matrix.drawLine(gx - 1, pacerY + 3 - o, gx - 1, gy - 1, col);
        }
      }
      delay(50);
    }
  } // for r
  delay(500);

  // First, let pacer move to the middle of the screen
  if (pacerX - o > gx)
  {
    currentDirection = LEFT;
  }
  else if (pacerX == gx)
  {
    currentDirection = STILL;
  }
  else
  {
    currentDirection = RIGHT;
  }
  if (pacerX - o != gx) // Do not move the Pacer to the left or right, if not necessary
  {
    do
    {
      pacerX_old = pacerX;
      pacerY_old = pacerY;
      if (pacer_animPhase == 1)
      {
        pacer_animPhase = 0;
      }
      else
      {
        pacer_animPhase = 1;
      }
      if (pacerX - o > gx)
      {
        pacerX--;
      }
      else
      {
        pacerX++;
      }
      repaintPacer();
      delay(100);
    } while (pacerX - o != gx);
  }

  if (pacerY - o != gy) // Do not move the Pacer up or down, if not necessary
  {
    if (pacerY - o > gy)
    {
      currentDirection = UP;
    }
    else
    {
      currentDirection = DOWN;
    }
    do
    {
      pacerX_old = pacerX;
      pacerY_old = pacerY;
      if (pacer_animPhase == 1)
      {
        pacer_animPhase = 0;
      }
      else
      {
        pacer_animPhase = 1;
      }
      if (pacerY - o > gy)
      {
        pacerY--;
      }
      else
      {
        pacerY++;
      }
      repaintPacer();
      delay(100);
    } while (pacerY - o != gy);
  }

  matrix.fillRect(pacerX - o, pacerY - o, 3, 3, matrix.Color444(0, 0, 0));
  pacer_animPhase = 0;
  currentDirection = RIGHT;
  repaintPacer();
  matrix.fillRect(0, 0, 32, 32, backgroundColor);
  matrix.fillRect(pacerX - o, pacerY - o, 3, 3, matrix.Color444(0, 0, 0));
  currentDirection = RIGHT;
  pacer_animPhase = 0;
  repaintPacer();
  delay(1000);
} // PlaySmallInterlude

void playGameOverScene()
{
  paintingAllowed = false;
  /*
  int col = matrix.Color444(5, 5, 5);
  for (byte x = 0; x < 15; x++)
  {
    for (byte y = 0; y < 15; y++)
    {
      matrix.drawLine(x, y, 31-x, y, col);
      matrix.drawLine(x, y, x, 31-y, col);
      matrix.drawLine(x, 31-y, x+31, 31-y, col);
      matrix.drawLine(31-x, y, 31-x, 31-y, col);
    }
    delay(100);
  }
  delay(100);
  */

  /*
  // Clear the pixels linewise in a spiraling path, starting from the upper left and going right
  byte v = 0;
  int col = backgroundColor;
  do
  {
    matrix.drawLine(v, v, 31-v, v, col);
    delay(60);
    matrix.drawLine(31-v, v, 31-v, 31-v, col);
    delay(60);
    matrix.drawLine(31-v, 31-v, v, 31-v, col);
    delay(60);
    matrix.drawLine(v, 31-v, v, v+1, col);
    v++;
    delay(60);
  }
  while (v < 16);
  */

  // Clear the tiles in a spiraling path, starting from the upper left and going right
  // Would have been funny: as soon as the "delete marker" "hits" a baddie or the pacer, it jumps away down the screen
  matrix.drawRect(0, 0, 32, 32, backgroundColor); // Remove the border

  // Draw a spiral
  byte v = 0;
  byte i = 1;
  byte j = 1;
  byte ox = 1; // Upper left starting point of the spiral
  byte oy = 1;
  do
  {
    // From upper left to upper right
    do
    {
      matrix.fillRect(i, j, 3, 3, backgroundColor);
      repaintPacer();
      delay(35);
      i = i + 3;
    } while (i < 28 - v);
    // From upper right to lower right
    do
    {
      matrix.fillRect(i, j, 3, 3, backgroundColor);
      repaintPacer();
      delay(35);
      j = j + 3;
    } while (j < 28 - v);
    // From lower right to lower left
    do
    {
      matrix.fillRect(i, j, 3, 3, backgroundColor);
      repaintPacer();
      delay(35);
      i = i - 3;
    } while (i > ox + v);
    // From lower left to upper left
    do
    {
      matrix.fillRect(i, j, 3, 3, backgroundColor);
      repaintPacer();
      delay(35);
      j = j - 3;
    } while (j > oy + v + 3);
    v = v + 3;
  } while (v < 18);

  // Let the Pacer move into the middle of the screen
  byte gx = 17 - o;
  byte gy = 18 - o;
  // First, let pacer move to the middle of the screen
  if (pacerX - o > gx)
  {
    currentDirection = LEFT;
  }
  else
  {
    currentDirection = RIGHT;
  }
  do
  {
    pacerX_old = pacerX;
    pacerY_old = pacerY;
    if (pacer_animPhase == 1)
    {
      pacer_animPhase = 0;
    }
    else
    {
      pacer_animPhase = 1;
    }
    if (pacerX - o > gx)
    {
      pacerX--;
    }
    else
    {
      pacerX++;
    }
    repaintPacer();
    delay(100);
  } while (pacerX - o != gx);
  if (pacerY - o > gy)
  {
    currentDirection = UP;
  }
  else
  {
    currentDirection = DOWN;
  }
  do
  {
    pacerX_old = pacerX;
    pacerY_old = pacerY;
    if (pacer_animPhase == 1)
    {
      pacer_animPhase = 0;
    }
    else
    {
      pacer_animPhase = 1;
    }
    if (pacerY - o > gy)
    {
      pacerY--;
    }
    else
    {
      pacerY++;
    }
    repaintPacer();
    delay(100);
  } while (pacerY - o != gy);

  matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
  currentDirection = RIGHT;
  pacer_animPhase = 0;
  repaintPacer();
  delay(1500);

  // What about Pacer?
  for (byte baddieNum = 0; baddieNum < 4; baddieNum++)
  {
    baddie_active[baddieNum] = true;
    baddieGoingHome[baddieNum] = false;
  }
  for (byte baddieNum = 4; baddieNum < 9; baddieNum++)
  {
    baddieX[baddieNum] = 40;
    baddie_active[baddieNum] = false;
    baddieGoingHome[baddieNum] = false;
  }
  baddie_type[0] = BADDIE_TYPE1;
  baddie_type[1] = BADDIE_TYPE2;
  baddie_type[2] = BADDIE_TYPE3;
  baddie_type[3] = BADDIE_TYPE4;
  baddieX[0] = 6;
  baddieY[0] = 14;
  baddieX[1] = 24;
  baddieY[1] = 11;
  baddieX[2] = 25;
  baddieY[2] = 15;
  baddieX[3] = 23;
  baddieY[3] = 27;
  repaintBaddies();
  repaintPacer();
  delay(1500);
  for (byte y = 0; y < 32; y++)
  {
    matrix.drawLine(0, y, 31, y, backgroundColor);
    delay(20);
  }
  repaintPacer();
  baddieX[0] = 7;
  baddieY[0] = 12;
  baddieX[1] = 18;
  baddieY[1] = 11;
  baddieX[2] = 23;
  baddieY[2] = 17;
  baddieX[3] = 19;
  baddieY[3] = 25;
  repaintBaddies();
  repaintPacer();
  delay(1500);
  for (byte x = 31; x > 0; x--)
  {
    matrix.drawLine(x, 0, x, 31, backgroundColor);
    delay(20);
  }
  // matrix.fillRect(0, 0, 32, 32, backgroundColor);
  repaintPacer();
  baddieX[0] = 10;
  baddieY[0] = 14;
  baddieX[1] = 14;
  baddieY[1] = 11;
  baddieX[2] = 22;
  baddieY[2] = 15;
  baddieX[3] = 15;
  baddieY[3] = 21;
  repaintBaddies();
  repaintPacer();
  delay(1500);
  for (byte y = 31; y > 0; y--)
  {
    matrix.drawLine(0, y, 31, y, backgroundColor);
    delay(20);
  }
  // matrix.fillRect(0, 0, 32, 32, backgroundColor);
  repaintPacer();
  baddieX[0] = 14;
  baddieY[0] = 16;
  baddieX[1] = 15;
  baddieY[1] = 14;
  baddieX[2] = 19;
  baddieY[2] = 15;
  baddieX[3] = 15;
  baddieY[3] = 20;
  repaintBaddies();
  repaintPacer();
  delay(1500);
  for (byte x = 0; x < 32; x++)
  {
    matrix.drawLine(x, 0, x, 31, backgroundColor);
    delay(20);
  }
  delay(2000);

  paintingAllowed = true;
}

// **************************************************************************************************************

// Main loop of the game
void loop_Pacer()
{
  audioIsProduced = false;
  if (showMithotronic)
  {
    mithotronic();
    ledMePlay();
    showMithotronic = false;
  }
  if (showTitleScreen)
  {
    showTitle();
    showTitleScreen = false;
  }
  pills = 0;
  baddieNum = 0;
  for (byte i = 0; i < NUM_BADDIES; i++)
  {
    baddie_active[i] = false;
    baddieGoingHome[baddieNum] = false;
  }

  mazeColor = getNextMazeColor();
  drawMaze();
  determineBonusItemLocation();
  timeAtWhichBonusAppears = min(bonusInterval - 10, bonusInterval - 30 + difficultyPacer * 3);
  boolean stageCleared = false;
  boolean gameOver = false;
  playerInvincible = false;
  skipStage = false;
  currentDirection = STILL;

  // Paint the sprites at their initial positions
  repaintPacer();
  repaintBaddies();
  blinkPacer = true;

  if (gameBegins)
  {
    delay(500);
    playOpeningJingle();
  } // game begins
  else
  {
    // Wait for a joystick operation
    consumeJoystickEvents();
    while (!joy1Fire() && !joy1Left() && !joy1Right() && !joy1Up() && !joy1Down())
    {
      timer.update();
    }; // Wait for joystick operation
    gameBegins = false;
  }
  currentDirection = STILL;
  repaintPacer();
  blinkPacer = false;

  do
  {
    timer.update();

    if (joy1FireL())
    {
      resetPacer = true;
    }

    if (digitalRead(buttonPause) == LOW)
    {
      isPause = true;
      digitalWrite(buttonPause, HIGH);
      while ((!joy1Fire() && !joy1Left() && !joy1Right() && !joy1Up() && !joy1Down()) && (!digitalRead(buttonPause) == LOW))
      {
      }; // Wait for joystick operation or yet another pause button press
      isPause = false;
    }

    if (pills == 0)
    {
      stageCleared = true;
    }
    if (lives == 0)
    {
      gameOver = true;
    }

    if (collision) // A baddie collided with the player's sprite
    {
      issueMelody(STOP);
      collision = false;
      lives--;

      // bonusPresent = false;
      // bonusTimer = 0;
      // paintBonusItem(false);

      // Hide the baddies
      for (byte baddieNum = 0; baddieNum < NUM_BADDIES; baddieNum++)
      {
        if (baddie_active[baddieNum])
        {
          matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, backgroundColor);
        }
      }

      currentDirection = STILL;
      repaintPacer();
      matrix.drawPixel(pacerX - o, pacerY + 2 - o, backgroundColor); // Sometimes, the pixel is yellow (why?)
      delay(250);                                                    // Play a jingle and show a "death animation"
      // Death animation
      playDeathAnimation(pacerX - o, pacerY - o);
      matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);

      /*
      if (difficultyPacer > 4) // If not desired, this condition can be safely suppressed
      {
        // Redraw the pills and count them again
        pills = 0;
        redrawPills();
      }
      */

      // Place the baddies at their initial positions
      for (byte baddieNum = 0; baddieNum < NUM_BADDIES; baddieNum++)
      {
        if (baddie_active[baddieNum])
        {
          // Clear
          matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, backgroundColor);
          restorePills();
          baddieX_old[baddieNum] = baddieX[baddieNum];
          baddieY_old[baddieNum] = baddieY[baddieNum];
          baddieX[baddieNum] = baddie_initialX[baddieNum];
          baddieY[baddieNum] = baddie_initialY[baddieNum];
          baddieGoingHome[baddieNum] = false;
          if (baddie_currentDirection[baddieNum] == STILL)
          {
            baddie_currentDirection[baddieNum] = random(1, 5);
          } // Baddies are still when set at their starting position
        } // baddie active
      } // for baddieNum
      consumeJoystickEvents();

      // If all lives are used up, hide the baddies from view in order to prepare the GameOver-scene
      if (lives == 0)
      {
        for (byte baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
        {
          baddieX[baddieNum] = 40;
        }
        gameOver = true;
      }
      else
      {
        // Place the player's sprite at his/her initial position
        pacerX = pacer_initialX;
        pacerY = pacer_initialY;
        pacerX_old = pacerX;
        pacerY_old = pacerY;
        currentDirection = STILL;
      }
    } // A baddie collided with the player's sprite
  } while ((!stageCleared) && (!gameOver) && (!resetPacer) && (!skipStage));

  bonusPresent = false;
  bonusTimer = 0;
  paintBonusItem(false);

  // Player cleared current stage
  if (stageCleared)
  {
    numStagesFinished++;
    gameBegins = false;
    issueMelody(STOP);

    // Hide the baddies
    matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor); // Hide the last pill the player has eaten
    currentDirection = STILL;
    repaintPacer();
    // Hide the baddies
    for (byte baddieNum = 0; baddieNum < NUM_BADDIES; baddieNum++)
    {
      if (baddie_active[baddieNum])
      {
        matrix.fillRect(baddieX[baddieNum] - o, baddieY[baddieNum] - o, 3, 3, backgroundColor);
        matrix.fillRect(baddie_initialX[baddieNum] - o, baddie_initialY[baddieNum] - o, 3, 3, backgroundColor);
        baddieGoingHome[baddieNum] = false;
        baddie_active[baddieNum] = false;
        repaintPacer();
      }
    }
    repaintPacer();
    playerInvincible = false;

    // Display a blinking effect for maze walls with sharp corners
    // Dissipate the walls, otherwise
    if ((((stage + 4) % 4) == 0) || (((stage + 4) % 4) == 1))
    {
      blinkMazeWalls(4, 300);
    }
    else
    {
      dissipateMazeWalls();
    }

    matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
    currentDirection = STILL;
    repaintPacer();

    if (stage < NUM_STAGES - 1)
    {
      stage++;
    }
    else // Increase the difficultyPacer level
    {
      stage = 0;
      difficultyPacer++;
      numInvincibleCycles = max(20, 50 - difficultyPacer * 5);
      bonusInterval = min(255, 90 + difficultyPacer * 10);
      timeAtWhichBonusAppears = min(bonusInterval - 10, bonusInterval - 30 + difficultyPacer * 3);
    }

    // Play a nice interlude here...
    matrix.fillScreen(backgroundColor); // Clear screen
    repaintPacer();
    if (numStagesFinished % 3 == 0)
    {
      playLargeInterlude(pacerX, pacerY);
    } // Play a large interlude every 3 stages
    else
    {
      playSmallInterlude(pacerX, pacerY);
    }

    determineBonusItemLocation();
    mazeColor = getNextMazeColor();
    showTitleScreen = false;
    consumeJoystickEvents();
    currentDirection = STILL;

    matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
  } // stage cleared
  else if (skipStage)
  {
    matrix.fillRect(0, 0, 31, 31, backgroundColor);
    issueMelody(STOP);
    gameBegins = false;

    for (byte baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
    {
      baddieGoingHome[baddieNum] = false;
      matrix.fillRect(baddie_initialX[baddieNum] - o, baddie_initialY[baddieNum] - o, 3, 3, backgroundColor);
    }
    matrix.fillRect(pacerX - o, pacerY - o, 3, 3, backgroundColor);
    matrix.fillRect(pacerX_old - o, pacerY_old - o, 3, 3, backgroundColor);
    pacerX_old = pacerX;
    pacerY_old = pacerY;

    if (stage < NUM_STAGES - 1)
    {
      stage++;
    }
    else // Increase the difficultyPacer level
    {
      stage = 0;
      difficultyPacer++;
      numInvincibleCycles = max(20, 50 - difficultyPacer * 5);
      bonusInterval = min(255, 90 + difficultyPacer * 10);
      timeAtWhichBonusAppears = min(bonusInterval - 10, bonusInterval - 30 + difficultyPacer * 3);
    }

    bonusPresent = false;
    bonusTimer = 0;
    paintBonusItem(false);
    pills = 0;
    determineBonusItemLocation();
    mazeColor = getNextMazeColor();
    showTitleScreen = false;
    consumeJoystickEvents();
    skipStage = false;
  }
  else if (gameOver)
  {
    issueMelody(STOP);
    gameBegins = true;

    for (byte baddieNum = 0; baddieNum < NUM_BADDIES - 1; baddieNum++)
    {
      baddieGoingHome[baddieNum] = false;
    }

    bonusPresent = false;
    bonusTimer = 0;
    paintBonusItem(false);
    mazeColorNum = 0;
    lives = NUM_LIVES;
    stage = 0;
    playerInvincible = false;
    numStagesFinished = 0;
    skipStage = false;
    // makeNoise(GAMEOVER_NOISE);

    playGameOverScene();
    matrix.fillScreen(backgroundColor); // Clear screen
    showTitleScreen = true;
    consumeJoystickEvents();
  } // gameOver
  else if (resetPacer)
  {
    issueMelody(STOP);
    bonusPresent = false;
    bonusTimer = 0;
    paintBonusItem(false);

    gameBegins = true;
    showMithotronic = true;
    showTitleScreen = true;
    numStagesFinished = 0;
    resetPacer = false;
    currentDirection = STILL;
    mazeColorNum = 0;
    difficultyPacer = 0;
    lives = NUM_LIVES;
    stage = 0;
    playerInvincible = false;
    matrix.fillScreen(backgroundColor); // Clear screen
    consumeJoystickEvents();
  } // resetPacer
} // loop
