/*
***********************************************************************************************
* SHT1x digital temperature and humidity sensor Driver
* All Rights Reserved
* File name	: SHT1x.c
* Programmer 	: John Leung, TechToys Co. Hong Kong
* Web presence  : www.TechToys.com.hk
* Note		: 
* Language	: C18 complier version 2.40, MPLAB v7.41
* Hardware	: PIC18F26K20
* Date		: 11 Oct 2006
* Version       : 1.0
* Apply by      : Lab411

***********************************************************************************************
* DESCRIPTION
*
* This module provides an interface to Sensirion SHT10 digital temperature & humidity sensor
*
* pinout function summarized as below
* ---SHT1x			MCU -----------------
* DATA - data		RA1
* SCK  - clock		RA2
***********************************************************************************************
*/

#ifndef SHT1X_H
#define SHT1X_H

#include "generic.h"
/*
***********************************************************************************************
*PORT DEFINITION
***********************************************************************************************
*/
#if defined (__18F26K20)
#define DATA_WR		LATAbits.LATA1
#define	DATA_RD   	PORTAbits.RA1
#define	SCK   		LATAbits.LATA2
#define DATA_TRIS	TRISAbits.TRISA1
#define SCK_TRIS	TRISAbits.TRISA2
#endif
#if defined (__PIC24F__)
#define DATA_WR		LATGbits.LATG3
#define	DATA_RD   	PORTGbits.RG3
#define	SCK   		LATGbits.LATG2
#define DATA_TRIS	TRISGbits.TRISG3
#define SCK_TRIS	TRISGbits.TRISG2
#endif


/*
***********************************************************************************************
* GLOBAL CONSTANTS
***********************************************************************************************
*/

enum {TEMP,HUMI};	

/*
***********************************************************************************************
* FUNCTION PROTOTYPES
***********************************************************************************************
*/
char s_softreset(void);
char s_measure(BYTE *p_value, BYTE *p_checksum, BYTE mode);
char s_write_statusreg(BYTE *p_value);
char s_read_statusreg(BYTE *p_value, BYTE *p_checksum);

/*
***********************************************************************************************
* FUNCTION PROTOTYPES
* HARDWARE SPECIFIC
***********************************************************************************************
*/

void s_transstart(void);
void s_connectionreset(void);
char s_read_byte(BYTE ack);
char s_write_byte(BYTE value);
#endif

