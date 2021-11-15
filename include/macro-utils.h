//*****************************************************************************
//
// A collection of useful macros.
//
//*****************************************************************************

#ifndef _MACRO_UTILS_H_
#define _MACRO_UTILS_H_

#include "driverlib/gpio.h"

#define SysCtlPeripheralEnableAndReady(peripheral) \
    {                                              \
        SysCtlPeripheralEnable(peripheral);        \
        while (!SysCtlPeripheralReady(peripheral)) \
            ;                                      \
    }

#endif
