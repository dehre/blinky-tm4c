//***************************************************************************************************
//
// Connect to the DSD-Tech HM-19 Bluetooth module via UART1.
//
// The user can change the state of the blue-LED onboard with the following commands:
//   * "set."
//   * "reset."
//   * "toggle."
// User commands are case-insensitive and end with dot "."
// A wrong command blinks the red-LED onboard.
//
// Useful links and resources:
// https://deepbluembedded.com/stm32-hc-05-bluetooth-module-examples/
// https://components101.com/sites/default/files/component_datasheet/HC-05%20Datasheet.pdf
//
// Circuit's diagram available in this repository.
// Date: 03-12-2021
//
//***************************************************************************************************

//
// Make VSCode happy.
//
#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "heart-beat.h"
#include "macro-utils.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define UART1_RX_BUFFER_LEN 10

static void UART1_StringPut(const char *str);

static void OnboardRedLED_Init(void);
static void OnboardRedLED_DisplayError(void);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    HeartBeat_Init();
    OnboardRedLED_Init();
    UART_Init(); // UART0, virtual COM port connected to the PC via USB

    // Configure UART1 for 9600 baud rate, 8-N-1 operation.
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    UARTEnable(UART1_BASE);

    UART1_StringPut("\nAvailable commands: \n");
    UART1_StringPut("  * set.\n");
    UART1_StringPut("  * reset.\n");
    UART1_StringPut("  * toggle.\n");
    UART1_StringPut("Commands are case-insensitive and end with dot \".\"\n");
    UARTprintf("Listening for commands...\n");

    char rxChar;
    char rxBuffer[UART1_RX_BUFFER_LEN];
    uint32_t rxBufferIdx = 0;
    while (1)
    {
        // casting int32_t to char, so `strcmp` works as expected
        rxChar = UARTCharGet(UART1_BASE);
        if (rxChar != '.')
        {
            rxBuffer[rxBufferIdx] = tolower(rxChar);
            rxBufferIdx = (rxBufferIdx + 1) % UART1_RX_BUFFER_LEN;
            continue;
        }

        rxBuffer[rxBufferIdx] = '\0';
        UARTprintf("Received command: %s\n", rxBuffer);

        if (strcmp("set", rxBuffer) == 0)
            HeartBeat_Set();
        else if (strcmp("reset", rxBuffer) == 0)
            HeartBeat_Reset();
        else if (strcmp("toggle", rxBuffer) == 0)
            HeartBeat_Toggle();
        else
            OnboardRedLED_DisplayError();

        rxBufferIdx = 0;
    }
}

void UART1_StringPut(const char *outStr)
{
    uint32_t outStrIdx = 0;
    while (outStr[outStrIdx])
    {
        UARTCharPut(UART1_BASE, outStr[outStrIdx]);
        outStrIdx++;
    }
}

void OnboardRedLED_Init()
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
}

void OnboardRedLED_DisplayError(void)
{
    bool heartBeatValue = HeartBeat_GetPinValue();
    HeartBeat_Reset();

    uint32_t clockRate = SysCtlClockGet();
    uint32_t pinValue = 0;
    uint32_t idx;
    for (idx = 0; idx < 7; idx++)
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, pinValue);
        pinValue ^= GPIO_PIN_1;
        SysCtlDelay(clockRate / 20);
    }

    heartBeatValue ? HeartBeat_Set() : HeartBeat_Reset();
}
