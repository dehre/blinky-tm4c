//*****************************************************************************
//
// Traffic light controller.
// For reference: book "Introduction to ARM Cortex-M Microcontrollers" page 252
// Date: 23-06-2021
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "timer.h"

//
// The error routine that is called if the driver library encounters an error.
//
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line)
{
    while (1)
        ;
}
#endif

#define GPIO_PIN_0_1 (GPIO_PIN_0 | GPIO_PIN_1)
#define GPIO_PIN_0_1_2_3_4_5 (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)

enum Sensor
{
    AllSensors = GPIO_PIN_0_1,
    SensorEast = GPIO_PIN_0,
    SensorNorth = GPIO_PIN_1,
};

enum Light
{
    AllLights = GPIO_PIN_0_1_2_3_4_5,
    GreenNorth = GPIO_PIN_0,
    YellowNorth = GPIO_PIN_1,
    RedNorth = GPIO_PIN_2,
    GreenEast = GPIO_PIN_3,
    YellowEast = GPIO_PIN_4,
    RedEast = GPIO_PIN_5,
};

typedef struct State
{
    const uint32_t Out;
    const uint8_t Time;
    struct State const *Next[4];
} State_t;

#define goNorth (&states[0])
#define waitNorth (&states[1])
#define goEast (&states[2])
#define waitEast (&states[3])

static const State_t states[] = {
    {.Out = GreenNorth | RedEast,
     .Time = 5,
     .Next = {goNorth, waitNorth, goNorth, waitNorth}},
    {.Out = YellowNorth | RedEast,
     .Time = 1,
     {goEast, goEast, goEast, goEast}},
    {.Out = GreenEast | RedNorth,
     .Time = 5,
     .Next = {goEast, goEast, waitEast, waitEast}},
    {.Out = YellowEast | RedNorth,
     .Time = 1,
     .Next = {goNorth, goNorth, goNorth, goNorth}}};

int main(void)
{
    //
    // Setup the clock and the Systick timer.
    //
    TimerSystickSetup();

    //
    // Enable the GPIO ports.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB) || !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
    {
    }

    //
    // Enable the GPIO pins and set their direction.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0_1_2_3_4_5);

    //
    // Reset timer and define initial state.
    //
    const State_t *currentState = goNorth;
    int32_t sensorsRead = 0;
    TimerReset();

    //
    // Loop forever.
    //
    while (1)
    {
        GPIOPinWrite(GPIO_PORTB_BASE, AllLights, currentState->Out);
        TimerWait1s(currentState->Time);
        sensorsRead = GPIOPinRead(GPIO_PORTE_BASE, AllSensors);
        assert(sensorsRead >= 0 && sensorsRead <= 4);
        currentState = currentState->Next[sensorsRead];
    }
}
