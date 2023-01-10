//*****************************************************************************
//
// Use the semaphore to debounce a positive logic switch on PB5.
//
// Usage:
// ```c
// #include "switch-debounce.h"
//
// void onTouch(void) {}
// void onRelease(void) {}
// void emptyThread(void) {}
//
// int main(void)
// {
//     SwitchDebouncePB5_Init(onTouch, onRelease);
//     OS_Init(THREADFREQ, SwitchDebouncePB5_Task);
//     OS_ERRCHECK(OS_ThreadCreate(emptyThread));
//     OS_Launch();
// }
// ```
//
//*****************************************************************************

#ifndef SWITCH_DEBOUNCE_H_INCLUDED
#define SWITCH_DEBOUNCE_H_INCLUDED

void SwitchDebouncePB5_Init(void (*callbackOnTouch)(void), void (*callbackOnRelease)(void));
void SwitchDebouncePB5_Task(void);

#endif
