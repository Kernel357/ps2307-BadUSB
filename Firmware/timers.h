#ifndef _TIMERS_H_INCLUDED
#define _TIMERS_H_INCLUDED

void InitLED(void);
void SetLEDThreshold(int Threshold);
void LEDBlink(void);
void LEDOff(void);

void InitTicks();
BYTE GetTickCount(void);

#endif
