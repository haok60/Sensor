/*
 *	Delay functions for C18 version 2.40
 *	See delay.h for details
 *
 *	Make sure this code is compiled with full optimization!!!
 *  Downloaded from www.microchipc.com - see site for more sample PIC C code
 */
//#include "pic24fj128ga306.h"
#include "delay.h"

void DelayMs(unsigned char cnt)
{
#if	XTAL_FREQ <= 2
	do {
		DelayUs(996);
	} while(--cnt);
#else
	unsigned char	i;
	do {
		i = 4;
		do {
			DelayUs(250);
		} while(--i);
	} while(--cnt);
#endif
}
