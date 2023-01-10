//************************************************************************************
//
// Connect to the Nokia 5110 LCD Display through SSI.
// Helpful resouce: https://www.youtube.com/watch?v=RAlZ1DHw03g
// Helpful resouce: https://mecrisp-stellaris-folkdoc.sourceforge.io/nokia-5110.html
// Nice tool for converting images to bmp: http://javl.github.io/image2cpp/
// Circuit's diagram available in this repository.
// Date: 11-07-2021
//
//************************************************************************************

//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "ascii.h"
#include "longhorn.h"

#define LCD_RST_PIN GPIO_PIN_7
#define LCD_DC_PIN GPIO_PIN_6
#define LCD_DIN_PIN GPIO_PIN_5
#define LCD_CE_PIN GPIO_PIN_3
#define LCD_CLK_PIN GPIO_PIN_2

static const uint8_t Lcd_WIDTH = 84;
static const uint8_t Lcd_ROWS = 6;

static void Lcd_Init(void);

enum writeType
{
    COMMAND,
    DATA
};

static void Lcd_Write(enum writeType type, uint32_t message);

static void Lcd_Flush(void);

static void Lcd_SetCursor(uint8_t newX, uint8_t newY);

static void Lcd_OutChar(char ch);

static void Lcd_OutString(const char *str);

static void Lcd_DrawBitmap(const uint8_t bmp[]);

int main(void)
{
    // Clock at 4MHz.
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Initialize the display.
    Lcd_Init();

    Lcd_Flush();
    Lcd_SetCursor(0, 2);
    Lcd_OutString("Hello, world!");
    SysCtlDelay(SysCtlClockGet());

    while (1)
    {
        Lcd_DrawBitmap(Longhorn);
        SysCtlDelay(SysCtlClockGet() / 3);
        Lcd_DrawBitmap(Longhorn2);
        SysCtlDelay(SysCtlClockGet() / 3);
    }
}

//
// Initialize Nokia 5110 48x84 LCD by sending the proper
// commands to the PCD8544 driver.
//
void Lcd_Init(void)
{
    // Enable  and configure peripherals.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
        ;

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, LCD_DC_PIN | LCD_RST_PIN);

    // Toggle the RST pin.
    GPIOPinWrite(GPIO_PORTA_BASE, LCD_RST_PIN, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, LCD_RST_PIN, LCD_RST_PIN);

    // Configure and enable the SSI port for TI master mode.  Use SSI0, system
    // clock supply, master mode, 1MHz SSI frequency, and 8-bit data.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5);
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);
    SSIEnable(SSI0_BASE);

    // Write initialization commands:
    Lcd_Write(COMMAND, 0x21); // LCD extended commands
    Lcd_Write(COMMAND, 0xB1); // LCD Vop (contrast)
    Lcd_Write(COMMAND, 0x04); // LCD temp coefficient
    Lcd_Write(COMMAND, 0x14); // LCD bias mode 1:48
    Lcd_Write(COMMAND, 0x20); // LCD basic commands
    Lcd_Write(COMMAND, 0x0C); // LCD normal video
}

//
// Send an 8-bit message to the LCD.
//
void Lcd_Write(enum writeType type, uint32_t message)
{
    // DC pin is low for commands, high for data
    (type == COMMAND)
        ? GPIOPinWrite(GPIO_PORTA_BASE, LCD_DC_PIN, 0)
        : GPIOPinWrite(GPIO_PORTA_BASE, LCD_DC_PIN, LCD_DC_PIN);

    // Transmit serial data and wait for transaction to complete
    SSIDataPut(SSI0_BASE, message);
    while (SSIBusy(SSI0_BASE))
        ;
}

//
// Move the cursor to the desired X- and Y-position.
// The next character will be printed here.
// No operation is performed if newX >= 14 or newY >= 6.
//
void Lcd_SetCursor(uint8_t newX, uint8_t newY)
{
    if (newX >= (Lcd_WIDTH / (ASCII_WIDTH + 1)) || newY >= Lcd_ROWS)
    {
        return;
    }
    // multiply newX by 6 because each character is 6 columns wide
    Lcd_Write(COMMAND, 0x80 | (newX * (ASCII_WIDTH + 1))); // setting bit 8 updates X-position
    Lcd_Write(COMMAND, 0x40 | newY);                       // setting bit 7 updates Y-position
}

//
// Flush the LCD by writing zeros to the entire screen and
// reset the cursor to the top-left corner.
//
void Lcd_Flush(void)
{
    uint32_t i;
    for (i = 0; i < (Lcd_WIDTH * Lcd_ROWS); i++)
    {
        Lcd_Write(DATA, 0x00);
    }
    Lcd_SetCursor(0, 0);
}

//
// Print a character at the current cursor position.
//
void Lcd_OutChar(char ch)
{
    int i;
    for (i = 0; i < ASCII_WIDTH; i++)
    {
        Lcd_Write(DATA, ASCII[ch - 0x20][i]);
    }
    Lcd_Write(DATA, 0x00);
}

//
// Print a string at the current cursor position.
//
void Lcd_OutString(const char *str)
{
    while (*str)
    {
        Lcd_OutChar(*str++);
    }
}

//
// Fill the whole screen by drawing a 48x84 bitmap image.
//! \param bmp pointer to a 504 byte bitmap.
//
void Lcd_DrawBitmap(const uint8_t bmp[])
{
    Lcd_SetCursor(0, 0);
    uint32_t i;
    for (i = 0; i < (Lcd_WIDTH * Lcd_ROWS); i++)
    {
        Lcd_Write(DATA, bmp[i]);
    }
}
