//***************************************************************************************************
//
// A one-directional communication channel between two microcontrollers
//   using a handshaked protocol.
// This TM4C123 LaunchPad acts as Transmitter, the STM32F3 Discovery acts as Receiver.
// Press SW1 on the Transmitter to start sending the string.
//
// Pinout:
// * PD0           -> OUT - Ready
// * PD1           -> IN  - Ack
// * PE0..5 PA6..7 -> OUT - Data
//
// For the STM32F3 code, check the related repository at `<path-to-repo>/Bin/2_handshake/`.
//
// For reference: book "Real-Time Interfacing to ARM Cortex-M Microcontrollers" page 228.
// Circuit's diagram available in this repository.
// Date: 15-11-2021
//
//***************************************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "heart-beat.h"
#include "macro-utils.h"
#include "onboard-sw1.h"
#include "uart-init.h"
#include "utils/uartstdio.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

// One millisecond delay between setting the READY pin LOW and HIGH again,
//   to make sure the Receiver has time to detect the change.
#define SYNCHRONIZATION_DELAY (clockRate / 1000)

#define GPIO_DATA_PINS_PORT_E (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)
#define GPIO_DATA_PINS_PORT_A (GPIO_PIN_6 | GPIO_PIN_7)

static const char *OUTGOING_STR = "Hello, world";

static uint32_t clockRate;

static void GPIOReadyPin_Init(void);
static void GPIOAckPin_Init(void);
static void GPIODataPins_Init(void);

static void GPIOReadyPin_Write(bool data);
static bool GPIOAckPin_Read(void);
static void GPIODataPins_Write(uint8_t data);

static void TransmitChar(uint8_t data);

int main(void)
{
    HeartBeat_Init();
    OnboardSw1_Init();
    UART_Init();

    GPIOReadyPin_Init();
    GPIOAckPin_Init();
    GPIODataPins_Init();

    clockRate = SysCtlClockGet();
    UARTprintf("Transmitter application started\n");

    while (OnboardSw1_Read() == 0)
    {
        HeartBeat_Toggle();
        SysCtlDelay(clockRate / 4);
    }
    HeartBeat_Reset();

    unsigned int outgoingStrIdx;
    for (outgoingStrIdx = 0; outgoingStrIdx <= strlen(OUTGOING_STR); outgoingStrIdx++)
    {
        TransmitChar(OUTGOING_STR[outgoingStrIdx]);
    }
    UARTprintf("Done sending string\n");

    while (1)
        SysCtlSleep();
}

void GPIOReadyPin_Init(void)
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0);
}

void GPIOAckPin_Init(void)
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);
}

void GPIODataPins_Init(void)
{
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnableAndReady(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_DATA_PINS_PORT_E);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_DATA_PINS_PORT_A);
}

void GPIOReadyPin_Write(bool data)
{
    uint8_t writeValue = data ? GPIO_PIN_0 : 0;
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0, writeValue);
}

bool GPIOAckPin_Read(void)
{
    return GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1);
}

void GPIODataPins_Write(uint8_t data)
{
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_DATA_PINS_PORT_E, data & GPIO_DATA_PINS_PORT_E);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_DATA_PINS_PORT_A, data & GPIO_DATA_PINS_PORT_A);
}

void TransmitChar(uint8_t data)
{
    GPIODataPins_Write(data);
    GPIOReadyPin_Write(true);
    UARTprintf("Transmitting char: %c, hex: %x\n", data, data);
    while (GPIOAckPin_Read() == 1)
        ;
    while (GPIOAckPin_Read() == 0)
        ;
    GPIOReadyPin_Write(false);
    SysCtlDelay(SYNCHRONIZATION_DELAY);
}
