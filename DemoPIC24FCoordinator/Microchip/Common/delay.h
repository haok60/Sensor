/*
 *	Delay functions for C18 version 2.40 on the PIC
 *
 *	Functions available:
 *		DelayUs(x)	Delay specified number of microseconds
 *		DelayMs(x)	Delay specified number of milliseconds
 *
 *	Note that there are range limits: x must not exceed 255 - for xtal
 *	frequencies > 12MHz the range for DelayUs is even smaller.
 *	To use DelayUs it is only necessary to include this file; to use
 *	DelayMs you must include delay.c in your project.
 *
 */


//#ifndef	XTAL_FREQ
#define	XTAL_FREQ 	16		/* Crystal frequency in MHz */
//#endif

#if	XTAL_FREQ >= 12

#define	DelayUs(x)	{ unsigned char _dcnt; \
			  _dcnt = (x)*((XTAL_FREQ)/(12)); \
			  while(--_dcnt != 0) \
				  continue; }
#else

#define	DelayUs(x)	{ unsigned char _dcnt; \
			  _dcnt = (x)/((12)/(XTAL_FREQ))|1; \
			  while(--_dcnt != 0) \
				  continue; }
#endif

extern void DelayMs(unsigned char);

