#include "defs.h"
#include "timers.h"

static BYTE	Timer0Count;
static BYTE	LedTicks;
static BYTE	LedTimer;
static BYTE	LedTickThreshold;
static BYTE Timer1Count;
static WORD	Timer1Reload;

void Timer1Isr(void) __interrupt (TMR1_VECT)
{
	TR1 = 0;
	TH1 = MSB(Timer1Reload);
	TL1 = LSB(Timer1Reload);
	Timer1Count++;
	TR1 = 1;
}

void InitTicks()
{
	if (XVAL(0xFA60) == 0x0F)
	{
		Timer1Reload = 0xF63C;
	}
	else
	{
		Timer1Reload = 0-(2500/(XVAL(0xFA60)+2));
	}

	Timer1Count = 0;
	TR1 = 0;
	ET1 = 1;
	TMOD = TMOD & 0x0F | 0x10;
}

BYTE GetTickCount(void)
{
	return Timer1Count;
}

void Timer0Isr(void) __interrupt (TMR0_VECT)
{
	//approx. 10 times per second
	TR0 = 0;
	TL0 = 0xE6;
	TH0 = 0x96;
	TR0 = 1;

	if ((GPIO0OUT & 2) == 0) //turned off
	{
		return;
	}

	Timer0Count++;
	LedTicks++;
	if (LedTicks < LedTickThreshold)
	{
		return;
	}

	LedTicks = 0;
	if (LedTimer >= 31)
	{
		GPIO0OUT = 1;
		LedTimer = 0;		
		return;
	}

	if (LedTimer >= 10)
	{
		GPIO0OUT = ~GPIO0OUT;
		LedTimer++;
		return;
	}

	if (LedTimer == 0)
	{
		return;
	}

	if (GPIO0OUT & 1)
	{
		GPIO0OUT &= 0xFE;
	}
	else
	{
		GPIO0OUT |= 1;
	}
}

void SetLEDThreshold(int Threshold)
{
	LedTickThreshold = Threshold;
}

void InitLED(void)
{
	LedTickThreshold = 100;
	Timer0Count = 0;
	GPIO0OUT = 3;
	LedTicks = 0;
	LedTimer = 0;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

void LEDBlink(void)
{
	GPIO0OUT = 2;
	LedTimer = 1;
}

void LEDOff(void)
{
	GPIO0OUT = 3;
	LedTimer = 0;
}
