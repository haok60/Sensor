/*********************************************************************
 *
 *                  Compiler specific defs.
 *

 This file was originally intended to help compatibility between different
 C compilers.  However, these definitions have the effect that standard
 Microchip syntax, such as PORTAbits.RA2, is illegal.

 This file has been modified such that standard Microchip syntax is now
 used for all file register access.  Translations for other compilers
 may be added in the future.

 *********************************************************************
 * FileName:        Compiler.h
 * Dependencies:    None
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v1.00.50 or higher
 *                  MCC30 v2.05 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (c) 2004-2008 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, copy and distribute Software 
 * only when embedded on a Microchip microcontroller or digital signal 
 * controller and used with a Microchip radio frequency transceiver, which 
 * are integrated into your product or third party product (pursuant to the 
 * sublicense terms in the accompanying license agreement).  You may NOT 
 * modify or create derivative works of the Software.  
 *
 * If you intend to use this Software in the development of a product for 
 * sale, you must be a member of the ZigBee Alliance.  For more information, 
 * go to www.zigbee.org.
 *
 * You should refer to the license agreement accompanying this Software for 
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY 
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR 
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED 
 * UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF 
 * WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR 
 * EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, 
 * PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF 
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY 
 * THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER 
 * SIMILAR COSTS.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     7/12/04     Rel 0.9
 * Nilesh Rajbharti     11/1/04     Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7 
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#ifndef COMPILER_H
#define COMPILER_H

#if !defined(_WIN32) && !defined(__C30__)
    #define MCHP_C18
#endif

#if defined(MCHP_C18) && defined(HITECH_C18)
    #error "Invalid Compiler selection."
#endif

#if !defined(MCHP_C18) && !defined(__C30__)
    #error "Compiler not supported."
#endif

#if defined(MCHP_C18)
    #include <p18cxxx.h>    // p18cxxx.h must have current processor
                            // defined.
	#include <stdlib.h>                            

#elif defined(__PIC24F__)	// Microchip C30 compiler
	// PIC24F processor
	#include <p24Fxxxx.h>
#elif defined(__PIC24H__)	// Microchip C30 compiler
	// PIC24H processor
	#include <p24Hxxxx.h>
#elif defined(__dsPIC33F__)	// Microchip C30 compiler
	// dsPIC33F processor
	#include <p33Fxxxx.h>
#else
	#error Unknown processor or compiler.  See Compiler.h
#endif




#if defined(MCHP_C18)
    #define ROM                 rom

    #define NOP()               Nop()
    #define CLRWDT()            ClrWdt()
    #define RESET()             Reset()
    #define SLEEP()             Sleep()
    #define DISABLE_WDT()       (WDTCONbits.SWDTEN = 0)
    #define ENABLE_WDT()        (WDTCONbits.SWDTEN = 1)
    #define TBLWTPOSTINC()      _asm tblwtpostinc _endasm



#elif defined(__C30__)
	#define ROM					const
	#define memcmppgm2ram(a,b,c)	memcmp(a,b,c)
	#define memcpypgm2ram(a,b,c)	memcpy(a,b,c)
	#define strcpypgm2ram(a, b)		strcpy(a,b)
	#define Reset()					asm("reset")
	#define SLEEP()					Sleep()
	#define CLRWDT()				ClrWdt()
	#define NOP()					Nop()
	#define DISABLE_WDT()			(RCONbits.SWDTEN = 0)
	#define ENABLE_WDT()			(RCONbits.SWDTEN = 1)

#endif        

#ifdef MCHP_C18
	
	#define TMRL		TMR0L
	#define TMRH		TMR0H
	
#elif defined(__C30__)
	
	#define TMRL		TMR2
	#define TMRH		TMR3
	
#endif	

#endif
