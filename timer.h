#ifndef TIMER_H
#define TIMER_H

#define TIME_FREQUENCY_SYSTEM 0x0
#define TIME_FREQUENCY_64 0x1
#define TIME_FREQUENCY_256 0x2
#define TIME_FREQUENCY_1024 0x3
#define TIME_OVERFLOW 0x4
#define TIME_ENABLE 0x80
#define TIME_IRQ_ENABLE 0x40

#endif

void Wait(int seconds)
{
	//Start the timer
	REG_TM3CNT = TIME_FREQUENCY_1024 | TIME_ENABLE;

	//zero the timer
	REG_TM3D = 0;

	while(seconds--)
	{
		while(REG_TM3D <= 16386){} //wait
		REG_TM3D = 0; //reset the timer
	}
}

void TimerStart()
{
	//Start the timer
	REG_TM3CNT = TIME_FREQUENCY_1024 | TIME_ENABLE;

	//zero the timer
	REG_TM3D = 0;
}

u8 Timer()
{
	if(REG_TM3D > 16386)
	{
		REG_TM3D = 0;
		return 1;
	}
	else 
		return 0;
}
