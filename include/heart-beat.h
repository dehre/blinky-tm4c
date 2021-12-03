//*****************************************************************************
//
// Utility functions for toggling the LED on PF2 for debugging purposes.
//
//*****************************************************************************

#ifndef _HEART_BEAT_H_
#define _HEART_BEAT_H_

void HeartBeat_Init(void);

void HeartBeat_Set(void);

void HeartBeat_Reset(void);

void HeartBeat_Toggle(void);

bool HeartBeat_GetPinValue(void);

#endif
