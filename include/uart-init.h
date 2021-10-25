//*****************************************************************************
//
// Usage:
// ```c
// #include "uart-init.h"
// #include "utils/uartstdio.h"
//
// UART_Init();
// UARTprintf("hello, world\n");
// ```
//
//*****************************************************************************

#ifndef _UART_INIT_H_
#define _UART_INIT_H_

//
// Initialize the first UART for 9600 baud rate, 8-N-1 operation,
// connected to the USB debug virtual serial port on the evaluation board.
//
void UART_Init(void);

#endif
