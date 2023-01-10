#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/debug.h>
#include "ascii-5x8.h"

#include "adafruit-ssd1306.h"

static void ssd1306_command1(uint8_t command);
static void ssd1306_commandList(const uint8_t *const commandList, uint8_t commandListLen);
static void ssd1306_displayBuffer(void);

static bool displayRotation = false;
static void (*i2cDataPut)(const uint8_t *const dataArr, uint32_t dataLen);

static const uint8_t ssd1306_commandByte = 0x00;
static const uint8_t ssd1306_dataByte = 0x40;

#define DISPLAY_BUFFER_LEN (DISPLAY_WIDTH * ((DISPLAY_HEIGHT + 7) / 8))
static uint8_t displayBuffer[DISPLAY_BUFFER_LEN] = {};

void Adafruit_SSD1306_begin(
    void (*i2cDataPutFnPtr)(const uint8_t *const dataArr, uint32_t dataLen),
    uint8_t vccSelection,
    bool rotation)
{
    i2cDataPut = i2cDataPutFnPtr;
    displayRotation = rotation;
    uint8_t vccstate = vccSelection;

    Adafruit_SSD1306_clearDisplay();

    static const uint8_t init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
                                    SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
                                    0x80,                       // the suggested ratio 0x80
                                    SSD1306_SETMULTIPLEX};      // 0xA8
    ssd1306_commandList(init1, sizeof(init1));
    ssd1306_command1(DISPLAY_HEIGHT - 1);

    static const uint8_t init2[] = {SSD1306_SETDISPLAYOFFSET,   // 0xD3
                                    0x0,                        // no offset
                                    SSD1306_SETSTARTLINE | 0x0, // line #0
                                    SSD1306_CHARGEPUMP};        // 0x8D
    ssd1306_commandList(init2, sizeof(init2));

    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    static const uint8_t init3[] = {SSD1306_MEMORYMODE, // 0x20
                                    0x00,               // Horizontal addressing mode
                                    SSD1306_SEGREMAP | 0x1,
                                    SSD1306_COMSCANDEC};
    ssd1306_commandList(init3, sizeof(init3));

    uint8_t comPins = 0x02;
    uint8_t contrast = 0x8F;
    if ((DISPLAY_WIDTH == 128) && (DISPLAY_HEIGHT == 32))
    {
        comPins = 0x02;
        contrast = 0x8F;
    }
    else if ((DISPLAY_WIDTH == 128) && (DISPLAY_HEIGHT == 64))
    {
        comPins = 0x12;
        contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x9F : 0xCF;
    }
    else if ((DISPLAY_WIDTH == 96) && (DISPLAY_HEIGHT == 16))
    {
        comPins = 0x2; // ada x12
        contrast = (vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0xAF;
    }
    else
    {
        // Other display varieties -- TBD
    }

    ssd1306_command1(SSD1306_SETCOMPINS);
    ssd1306_command1(comPins);
    ssd1306_command1(SSD1306_SETCONTRAST);
    ssd1306_command1(contrast);

    ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    static const uint8_t init5[] = {SSD1306_SETVCOMDETECT, // 0xDB
                                    0x40,
                                    SSD1306_DISPLAYALLON_RESUME, // 0xA4
                                    SSD1306_NORMALDISPLAY,       // 0xA6
                                    SSD1306_DEACTIVATE_SCROLL,
                                    SSD1306_DISPLAYON}; // Main display turn on
    ssd1306_commandList(init5, sizeof(init5));
}

void Adafruit_SSD1306_clearDisplay(void)
{
    memset(displayBuffer, 0, DISPLAY_BUFFER_LEN);
}

void Adafruit_SSD1306_display(void)
{
    static const uint8_t dlist1[] = {
        SSD1306_PAGEADDR,
        0,                      // Page start address
        0xFF,                   // Page end (not really, but works here)
        SSD1306_COLUMNADDR, 0}; // Column start address
    ssd1306_commandList(dlist1, sizeof(dlist1));
    ssd1306_command1(DISPLAY_WIDTH - 1); // Column end address

    ssd1306_displayBuffer();
}

void Adafruit_SSD1306_drawPixel(uint32_t x, uint32_t y)
{
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
        return;

    if (displayRotation)
    {
        x = DISPLAY_WIDTH - x - 1;
        y = DISPLAY_HEIGHT - y - 1;
    }

    displayBuffer[x + (y / 0x08) * DISPLAY_WIDTH] |= (1 << (y & 0x07));
}

void Adafruit_SSD1306_drawChar(char ch, uint32_t column, uint32_t row)
{
    static const uint32_t maxColumn = DISPLAY_WIDTH / (ASCII_5X8_WIDTH + 1);
    static const uint32_t maxRow = DISPLAY_HEIGHT / ASCII_5X8_HEIGHT;
    if (column >= maxColumn || row >= maxRow)
        return;

    uint32_t relativeX = column * (ASCII_5X8_WIDTH + 1);
    uint32_t relativeY = row * ASCII_5X8_HEIGHT;

    uint32_t idx;
    uint32_t jdx;
    for (idx = 0; idx < ASCII_5X8_WIDTH; idx++)
    {
        uint32_t aByteInChar = ASCII_5X8[ch - ASCII_5X8_MIN_VALUE][idx];

        // iterate each bit in byte
        for (jdx = 0; jdx < 8; jdx++)
        {
            if (aByteInChar & 0x01)
            {
                Adafruit_SSD1306_drawPixel(relativeX, relativeY);
            }
            aByteInChar >>= 1;
            relativeY++;
        }

        relativeY = row * ASCII_5X8_HEIGHT;
        relativeX++;
    }
}

void Adafruit_SSD1306_drawString(const char *const str, uint32_t column, uint32_t row)
{
    static const uint32_t maxColumn = DISPLAY_WIDTH / (ASCII_5X8_WIDTH + 1);
    static const uint32_t maxRow = DISPLAY_HEIGHT / ASCII_5X8_HEIGHT;
    if (column >= maxColumn || row >= maxRow)
        return;

    uint32_t currentColumn = column;
    uint32_t currentRow = row;
    uint32_t idx;
    for (idx = 0; str[idx]; idx++)
    {
        Adafruit_SSD1306_drawChar(str[idx], currentColumn, currentRow);
        currentColumn++;
        if (currentColumn >= maxColumn)
        {
            currentRow++;
            if (currentRow >= maxRow)
                return;
            else
                currentColumn = column;
        }
    }
}

void ssd1306_command1(uint8_t command)
{
    uint8_t buf[2] = {ssd1306_commandByte, command};
    i2cDataPut(buf, 2);
}

void ssd1306_commandList(const uint8_t *const commandList, uint8_t commandListLen)
{
    ASSERT(commandListLen < 10);
    static uint8_t buf[10];
    memset(buf, ssd1306_commandByte, 1);
    memcpy(buf + 1, commandList, sizeof(uint8_t) * commandListLen);
    i2cDataPut(buf, commandListLen + 1);
}

void ssd1306_displayBuffer(void)
{
    static uint8_t buf[DISPLAY_BUFFER_LEN + 1];
    memset(buf, ssd1306_dataByte, 1);
    memcpy(buf + 1, displayBuffer, sizeof(uint8_t) * DISPLAY_BUFFER_LEN);
    i2cDataPut(buf, DISPLAY_BUFFER_LEN + 1);
}
