#include <hardware_setup.hpp>

int16_t bounds_x1;
int16_t bounds_y1;
uint16_t bounds_w = 0;
uint16_t bounds_h = 0;
uint8_t numberOfGames = 0;

uint8_t currentSelection = 0;

/**
 * Find the dimensions of the text field in pixels
 * Parameters:
 * x Position of the left/up corner of the text field in pixels
 * y Position of the left/up corner of the text field in pixels
 *
 * The result is written to the global variables bounds_w and bounds_h.
 */
void findBounds(String text, int x, int y)
{
    int16_t akt_x1;
    int16_t akt_y1;
    uint16_t akt_w;
    uint16_t akt_h;

    matrix.getTextBounds(text, x, y, &akt_x1, &akt_y1, &akt_w, &akt_h);

    if (akt_w > bounds_w)
        bounds_w = akt_w;

    akt_h += akt_y1;

    if (akt_h > bounds_h)
        bounds_h = akt_h;
}

void writeText(int x, int y, boolean isBoundCheck)
{
    clearScreen();

    matrix.setCursor(x, y);

    for (int i = 0; myGames[i].setup != NULL; i++)
    {
        int aktY = matrix.getCursorY();
        matrix.setCursor(x, aktY);

        if (isBoundCheck)
        {
            findBounds(myGames[i].name, x, aktY);
            numberOfGames++;
        }

        if ((aktY + 4) < (16 + 5) && (aktY + 4) > (16 - 4))
        {
            currentSelection = i;
            matrix.setTextColor(matrix.Color333(7, 7, 7));
        }
        else
            matrix.setTextColor(matrix.Color333(7, 0, 0));

        matrix.println(myGames[i].name);
    }

    matrix.updateDisplay();
}

uint8_t selectGame()
{
    u_int16_t currentY = 0;
    int8_t dirY = 0;
    uint8_t countDir = 0;

    matrix.begin();

    Serial.println("Setup");
    matrix.setTextColor(matrix.Color333(7, 7, 7));

    // Read the bounds of the textfield
    bounds_w = 0;
    bounds_h = 0;
    numberOfGames = 0;
    writeText(0, 0, true);

    // writeText(0, -8 + 20, false);
    // writeText(-bounds_w + 32, 0, false);
    // Serial.printf("W:%d H:%d\n", bounds_w, bounds_h);

    do
    {

        int jy = joy1Y();

        if (dirY == 0)
        {
            Serial.printf("%d<=%d && %d>=0\n", currentSelection, numberOfGames, currentSelection);
            if (currentSelection < (numberOfGames-1) && joy1Up())
                dirY = -1;
            if (currentSelection > 0 && joy1Down())
                dirY = 1;

            countDir = 0;
        }
        else
        {
            countDir++;
            currentY += dirY;
            if (countDir == 8)
            {
                dirY = 0;
            }
        }

        int x = 1.0 * (bounds_w) / 4096 * jy;

        // int y = 1.0 * jx * (bounds_h - 8) / 4096 - bounds_h / 2 + 16;

        writeText(x, currentY + 12, false);

        delay(100);
    } while (!joy1Fire());

    clearScreen();

    return currentSelection;
}
