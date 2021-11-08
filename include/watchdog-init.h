//**************************************************************************
//
// Utility function for quickly adding a watchdog timer to the application.
//
// Usage:
// ```c
// #include "driverlib/sysctl.h"
// #include "heart-beat.h"
// #include "watchdog-init.h"
//
// HeartBeat_Init();
// Watchdog_Init(SysCtlClockGet(), HeartBeat_Toggle);
// ```
//
//**************************************************************************

#ifndef _WATCHDOG_INIT_H_
#define _WATCHDOG_INIT_H_

#include <stdint.h>

//*****************************************************************************
//
//! Initializes and enables the watchdog timer.
//!
//! \param loadValue is the load value for the watchdog timer.
//!
//! \param recurringFn is a user-defined function called when the
//! watchdog timer interrupt occurs. It's not this function's
//! responsibility to clear the interrupt flag and/or to decide
//! on feeding the dog.
//!
//! \return None.
//
//*****************************************************************************
void Watchdog_Init(uint32_t loadValue, void (*recurringFn)(void));

#endif
