//***************************************************************************************
//
// Interface with the OLED display via I2C.
// I2C setup and display driver have been decoupled into different modules.
// The display driver mirrors closely the Adafruit library:
//     https://github.com/adafruit/Adafruit_SSD1306
//
// Circuit's diagram available in this repository.
// Date: 15-12-2021
//
//***************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/debug.h>
#include <driverlib/sysctl.h>

#include "i2c0pb23.h"
#include "adafruit-ssd1306.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    I2C0PB23_Init(.slaveAddress = SSD1306_I2C_ADDRESS, .isRead = false, .uartDebug = false);

    Adafruit_SSD1306_begin(I2C0PB23_BurstSend, SSD1306_SWITCHCAPVCC, true);

    Adafruit_SSD1306_drawPixel(0, 0);
    Adafruit_SSD1306_drawPixel(1, 1);
    Adafruit_SSD1306_drawPixel(2, 2);
    Adafruit_SSD1306_drawPixel(64, 16);
    Adafruit_SSD1306_drawPixel(125, 29);
    Adafruit_SSD1306_drawPixel(126, 30);
    Adafruit_SSD1306_drawPixel(127, 31);
    Adafruit_SSD1306_display();

    uint32_t delay = SysCtlClockGet() / 2;
    SysCtlDelay(delay);

    Adafruit_SSD1306_clearDisplay();
    Adafruit_SSD1306_drawString("Hello, world!", 4, 1);
    Adafruit_SSD1306_display();

    while (1)
    {
        SysCtlSleep();
    }
}
