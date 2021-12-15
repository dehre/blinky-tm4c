//******************************************************************************
//
// Setup a GPIO pin for triggering a debugging instrument,
//   eg. oscilloscope or logic analyzer.
//
// USAGE 1: expand the macro directly in the file to debug.
//
// ```c
// #include "instrument-trigger.h"
// InstrumentTrigger_Create(B, 0);
//
// InstrumentTriggerPB0_Init();
// InstrumentTriggerPB0_Toggle();
// ```
//
//
// USAGE 2: expand the macro in a new file, then include
//   the function definitions in the file to debug.
//
// *newfile.c*
// ```c
// #include "instrument-trigger.h"
// InstrumentTrigger_Create(B, 0);
// ```
//
// *main.c*
// ```c
// #include "instrument-trigger.h"
// InstrumentTrigger_Include(B, 0);
//
// InstrumentTriggerPB0_Init();
// InstrumentTriggerPB0_Toggle();
// ```
//******************************************************************************

#ifndef INSTRUMENT_TRIGGER_H_INCLUDED
#define INSTRUMENT_TRIGGER_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define InstrumentTrigger_Include(PORT, PIN)           \
    void InstrumentTriggerP##PORT##PIN##_Init(void);   \
    void InstrumentTriggerP##PORT##PIN##_Set(void);    \
    void InstrumentTriggerP##PORT##PIN##_Reset(void);  \
    void InstrumentTriggerP##PORT##PIN##_Toggle(void); \
    bool InstrumentTriggerP##PORT##PIN##_GetPinValue(void);

#define InstrumentTrigger_Create(PORT, PIN)                            \
    static bool InstrumentTriggerP##PORT##PIN##_pinValue = false;      \
    static inline void InstrumentTriggerP##PORT##PIN##_write(void)     \
    {                                                                  \
        GPIOPinWrite(                                                  \
            GPIO_PORT##PORT##_BASE,                                    \
            GPIO_PIN_##PIN,                                            \
            InstrumentTriggerP##PORT##PIN##_pinValue                   \
                ? GPIO_PIN_##PIN                                       \
                : 0);                                                  \
    }                                                                  \
    void InstrumentTriggerP##PORT##PIN##_Init(void)                    \
    {                                                                  \
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIO##PORT);              \
        while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIO##PORT))       \
            ;                                                          \
        GPIOPinTypeGPIOOutput(GPIO_PORT##PORT##_BASE, GPIO_PIN_##PIN); \
    }                                                                  \
    void InstrumentTriggerP##PORT##PIN##_Set(void)                     \
    {                                                                  \
        InstrumentTriggerP##PORT##PIN##_pinValue = true;               \
        InstrumentTriggerP##PORT##PIN##_write();                       \
    }                                                                  \
    void InstrumentTriggerP##PORT##PIN##_Reset(void)                   \
    {                                                                  \
        InstrumentTriggerP##PORT##PIN##_pinValue = false;              \
        InstrumentTriggerP##PORT##PIN##_write();                       \
    }                                                                  \
    void InstrumentTriggerP##PORT##PIN##_Toggle(void)                  \
    {                                                                  \
        InstrumentTriggerP##PORT##PIN##_pinValue =                     \
            !InstrumentTriggerP##PORT##PIN##_pinValue;                 \
        InstrumentTriggerP##PORT##PIN##_write();                       \
    }                                                                  \
    bool InstrumentTriggerP##PORT##PIN##_GetPinValue(void)             \
    {                                                                  \
        return InstrumentTriggerP##PORT##PIN##_pinValue;               \
    }

#endif
