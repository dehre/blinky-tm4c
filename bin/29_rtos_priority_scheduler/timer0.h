#ifndef TIMER0_H_INCLUDED
#define TIMER0_H_INCLUDED

void Timer0_Init1KHz(void (*periodicIntHandler)(void));
void Timer0_Enable(void);

#endif
