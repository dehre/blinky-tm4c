//*****************************************************************************
//
// Sound generation system consisting of a 6-bit binary-weighted DAC
// and a speaker interface.
// Seven input buttons, corresponding to the notes Do..Si, are used to
// trigger the waveform generation at different frequencies.
// Circuit's diagram available in this repository.
// Data sheet for the 3.5mm Audio Jack: https://www.mouser.de/datasheet/2/222/STX3120-334667.pdf
// For reference: book "Introduction to ARM Cortex-M Microcontrollers" page 382
// Date: 09-08-2021
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "heart-beat.h"

#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define KEYS_INPUT_PERIPH_B SYSCTL_PERIPH_GPIOB
#define KEYS_INPUT_PERIPH_C SYSCTL_PERIPH_GPIOC
#define KEYS_INPUT_PORT_B GPIO_PORTB_BASE
#define KEYS_INPUT_PORT_C GPIO_PORTC_BASE
#define KEYS_INPUT_PINS_PORT_B GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7
#define KEYS_INPUT_PINS_PORT_C GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7

#define DAC_OUTPUT_PERIPH SYSCTL_PERIPH_GPIOE
#define DAC_OUTPUT_PORT GPIO_PORTE_BASE
#define DAC_OUTPUT_PINS GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5

typedef uint8_t Keys;

typedef struct
{
    uint16_t frequency;
} Note;

static const uint8_t sineWave[] = {
    32, 35, 38, 41, 44, 47, 49, 52, 54, 56, 58, 59, 61, 62, 62, 63, 63, 63, 62, 62, 61, 59, 58,
    56, 54, 52, 49, 47, 44, 41, 38, 35, 32, 29, 26, 23, 20, 17, 15, 12, 10, 8, 6, 5, 3, 2, 2, 1,
    1, 1, 2, 2, 3, 5, 6, 8, 10, 12, 15, 17, 20, 23, 26, 29};

static void gpioPeripheralsInit(void);

static void sysTickCounterClear(void);

static void sysTickInterruptHandler(void);

static Keys keysRead(void);

static Note keysToNote(Keys keysInput);

static int main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    heartBeatInit();
    gpioPeripheralsInit();
    SysTickIntRegister(sysTickInterruptHandler);

    bool keyPressedBefore = false;
    while (1)
    {
        Keys keysPressed = keysRead();
        if (!keyPressedBefore && keysPressed > 0x00)
        {
            keyPressedBefore = true;
            Note notePressed = keysToNote(keysPressed);
            SysTickPeriodSet(SysCtlClockGet() / ((sizeof(sineWave) / sizeof(uint8_t)) * notePressed.frequency));
            sysTickCounterClear();
            SysTickEnable();
            heartBeatSet();
        }

        if (keyPressedBefore && keysPressed == 0x00)
        {
            keyPressedBefore = false;
            SysTickDisable();
            heartBeatReset();
        }
    }
}

void gpioPeripheralsInit(void)
{
    SysCtlPeripheralEnable(KEYS_INPUT_PERIPH_B);
    SysCtlPeripheralEnable(KEYS_INPUT_PERIPH_C);
    SysCtlPeripheralEnable(DAC_OUTPUT_PERIPH);
    while (
        !SysCtlPeripheralReady(KEYS_INPUT_PERIPH_B) ||
        !SysCtlPeripheralReady(KEYS_INPUT_PERIPH_C) ||
        !SysCtlPeripheralReady(DAC_OUTPUT_PERIPH))
        ;

    GPIOPinTypeGPIOInput(KEYS_INPUT_PORT_B, KEYS_INPUT_PINS_PORT_B);
    GPIOPinTypeGPIOInput(KEYS_INPUT_PORT_C, KEYS_INPUT_PINS_PORT_C);
    GPIOPinTypeGPIOOutput(DAC_OUTPUT_PORT, DAC_OUTPUT_PINS);
}

void sysTickCounterClear(void)
{
    // any write to this register clears the SysTickCounter value
    NVIC_ST_CURRENT_R = 0;
}

void sysTickInterruptHandler(void)
{
    static uint8_t waveIdx = 0;
    waveIdx = (waveIdx + 1) & 0x3F; // pick the first 6 bits
    GPIOPinWrite(DAC_OUTPUT_PORT, DAC_OUTPUT_PINS, sineWave[waveIdx]);
}

Keys keysRead(void)
{
    int32_t portBKeys = GPIOPinRead(KEYS_INPUT_PORT_B, KEYS_INPUT_PINS_PORT_B);
    int32_t portCKeys = GPIOPinRead(KEYS_INPUT_PORT_C, KEYS_INPUT_PINS_PORT_C);

    // store the values for Do..Si in order, from bit 0 (Do) to bit 7 (Si)
    uint8_t packedKeys = (portBKeys >> 4) | (portCKeys >> 1);
    return packedKeys;
}

Note keysToNote(uint8_t keysInput)
{
    Note Do = {
        .frequency = 523};
    Note Re = {
        .frequency = 587};
    Note Mi = {
        .frequency = 659};
    Note Fa = {
        .frequency = 698};
    Note Sol = {
        .frequency = 784};
    Note La = {
        .frequency = 880};
    Note Si = {
        .frequency = 988};

    if (keysInput & 0x01)
        return Do;
    if (keysInput & 0x02)
        return Re;
    if (keysInput & 0x04)
        return Mi;
    if (keysInput & 0x08)
        return Fa;
    if (keysInput & 0x10)
        return Sol;
    if (keysInput & 0x20)
        return La;
    return Si;
}
