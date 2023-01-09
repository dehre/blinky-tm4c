//*****************************************************************************
//
// Interface with a 4x4 matrix keypad and transmit the pressed key to UART0.
// While a key is pressed:
//   - it's transmitted only once to UART;
//   - other keys pressed aren't transmitted to UART.
// Circuit's diagram available in this repository.
// Date: 30-07-2021
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define ROW_0 GPIO_PIN_0
#define ROW_1 GPIO_PIN_1
#define ROW_2 GPIO_PIN_2
#define ROW_3 GPIO_PIN_3
#define ROWS ROW_0 | ROW_1 | ROW_2 | ROW_3

#define COLUMN_0 GPIO_PIN_2
#define COLUMN_1 GPIO_PIN_3
#define COLUMN_2 GPIO_PIN_4
#define COLUMN_3 GPIO_PIN_5
#define COLUMNS COLUMN_0 | COLUMN_1 | COLUMN_2 | COLUMN_3

static const uint32_t rows[] = {ROW_0, ROW_1, ROW_2, ROW_3};

static const char KEYS[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    UART_Init();

    //
    // Configure Column 0..3 as inputs with internal weak pull-up resistors
    //       and Row 0..3 as open-drain outputs.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
        ;
    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, COLUMNS);
    GPIOPadConfigSet(GPIO_PORTA_BASE, COLUMNS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, ROWS);
    GPIOPadConfigSet(GPIO_PORTD_BASE, ROWS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

    char lastKeyDisplayed = '\0';
    while (1)
    {
        uint32_t rowIdx;
        for (rowIdx = 0; rowIdx < sizeof(rows) / sizeof(uint32_t); rowIdx++)
        {
            GPIOPinWrite(GPIO_PORTD_BASE, rows[rowIdx], 0);
            int32_t columns_read = GPIOPinRead(GPIO_PORTA_BASE, COLUMNS);
            // Columns 0..3 are connected to pins 2..5, hence the right shift
            columns_read >>= 2;

            uint32_t columnIdx;
            for (columnIdx = 0; columnIdx < 4; columnIdx++)
            {
                // column input-pins are pulled-up -> 0 if key pressed, 1 if key released
                char keyAtCurrentPosition = KEYS[rowIdx][columnIdx];
                bool keyAtCurrentPositionIsPressed = (~columns_read) & (1 << columnIdx);

                if (keyAtCurrentPositionIsPressed && lastKeyDisplayed == '\0')
                {
                    UARTprintf("%c ", keyAtCurrentPosition);
                    lastKeyDisplayed = keyAtCurrentPosition;
                }

                if (!keyAtCurrentPositionIsPressed && lastKeyDisplayed == keyAtCurrentPosition)
                {
                    lastKeyDisplayed = '\0';
                }
            }

            GPIOPinWrite(GPIO_PORTD_BASE, rows[rowIdx], rows[rowIdx]);
        }
    }
}

//
// -- PSEUDO-CODE --
//
// for each row
//     set it low
//     read all columns
//     for each column
//         lookup the row-column symbol
//         if key pressed (reads low) and not already printed
//             print the symbol to UART
//         if key not pressed and already printed
//             reset lastKeyDisplayed to '\0'
//     restore row to high
//
