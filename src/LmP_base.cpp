/** Here are the basic functions of the LEDmePlay-Framework located.*/
#include <hardware_setup.hpp>
#include <Notes.hpp>
#include <Joystick.hpp>

// Draw the M of the Mithotronic logo
void drawM(int x, int y)
{
  matrix.fillRect(x + 2, y + 2, 6, 1, matrix.Color333(0, 0, 0));
  matrix.fillRect(x, y + 3, 10, 1, matrix.Color333(0, 0, 0));

  matrix.fillRect(x, y + 4, 2, 6, matrix.Color333(3, 3, 3));
  matrix.fillRect(x + 2, y + 3, 2, 2, matrix.Color333(3, 3, 3));
  matrix.fillRect(x + 4, y + 4, 2, 6, matrix.Color333(3, 3, 3));
  matrix.fillRect(x + 6, y + 3, 2, 2, matrix.Color333(3, 3, 3));
  matrix.fillRect(x + 8, y + 4, 2, 6, matrix.Color333(3, 3, 3));
}

// Draw the T of the Mithotronic logo
void drawT(int x, int y)
{
  matrix.fillRect(x, y + 5, 6, 1, matrix.Color333(0, 0, 0));
  matrix.fillRect(x + 2, y + 10, 3, 1, matrix.Color333(0, 0, 0));

  matrix.fillRect(x, y + 3, 6, 2, matrix.Color333(0, 0, 7));
  matrix.fillRect(x + 2, y, 2, 10, matrix.Color333(0, 0, 7));
  matrix.fillRect(x + 4, y + 8, 1, 2, matrix.Color333(0, 0, 7));
}

// Draw the animated Mithotronic logo and play jingle
void mithotronic()
{
  int i = -10;
  do
  {
    drawM(7, i);
    drawT(19, 22 - i);
    i++;
    delay(50);
  } while (i <= 11);

  // Jingle on start screen
  tone(audioPacer, NOTE_C4, 200);
  delay(400 + 20);
  tone(audioPacer, NOTE_C4, 90);
  delay(200 - 20);
  tone(audioPacer, NOTE_G4, 140);
  delay(400 + 20);
  tone(audioPacer, NOTE_G4, 140);
  delay(200 - 20);
  tone(audioPacer, NOTE_C5, 450);
  delay(600);
  tone(audioPacer, NOTE_AS4, 140);
  delay(200 - 20);
  tone(audioPacer, NOTE_A4, 130);
  delay(200 - 10);
  tone(audioPacer, NOTE_F4, 120);
  delay(200);
  tone(audioPacer, NOTE_G4, 1000);
  delay(3000);

}

// Sets color for the next character to show the LEDmePLay logo
void setLEDMePlayColor(int i)
{
  switch (i % 9)
  {
  case 0:
    matrix.setTextColor(matrix.Color333(5, 0, 0));
    break;
  case 1:
    matrix.setTextColor(matrix.Color333(5, 2, 0));
    break;
  case 2:
    matrix.setTextColor(matrix.Color333(2, 5, 0));
    break;
  case 3:
    matrix.setTextColor(matrix.Color333(0, 5, 0));
    break;
  case 4:
    matrix.setTextColor(matrix.Color333(0, 5, 2));
    break;
  case 5:
    matrix.setTextColor(matrix.Color333(0, 2, 5));
    break;
  case 6:
    matrix.setTextColor(matrix.Color333(0, 0, 5));
    break;
  case 7:
    matrix.setTextColor(matrix.Color333(2, 0, 5));
    break;
  case 8:
    matrix.setTextColor(matrix.Color333(5, 0, 2));
    break;
  }
}

// Draw the LEDmePlay logo
void ledMePlay()
{
  // Clear screen
  matrix.fillScreen(matrix.Color333(0, 0, 0));

  int i = 0;
  do
  {
    // Write 'LEDmePlay'
    setLEDMePlayColor(i++);
    matrix.setCursor(7, 5);
    matrix.println("L");
    setLEDMePlayColor(i++);
    matrix.setCursor(13, 5);
    matrix.println("E");
    setLEDMePlayColor(i++);
    matrix.setCursor(19, 5);
    matrix.println("D");

    setLEDMePlayColor(i++);
    matrix.setCursor(10, 11);
    matrix.println("m");
    setLEDMePlayColor(i++);
    matrix.setCursor(16, 11);
    matrix.println("e");

    setLEDMePlayColor(i++);
    matrix.setCursor(4, 19);
    matrix.println("P");
    setLEDMePlayColor(i++);
    matrix.setCursor(10, 19);
    matrix.println("l");
    setLEDMePlayColor(i++);
    matrix.setCursor(16, 19);
    matrix.println("a");
    setLEDMePlayColor(i++);
    matrix.setCursor(22, 19);
    matrix.println("y");

    i++;
    if (i > 81)
    {
      i = 0;
    }

    int j = 0;
    do
    {
      j++;
      delay(1);
    } while (j < 250 && !joy1Fire() && !joy2Fire());
  } while (!joy1Fire() && !joy2Fire());
  tone(audioPacer, 1024, 20);
  delay(200);
  matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 0, 0));
}
