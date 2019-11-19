/*********************************************************************
 *
 *                  ZigBee Symbol Timer Header File
 *
 *********************************************************************
 * FileName:        SymbolTime.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v3.00 or higher
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
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED ?AS IS? WITHOUT WARRANTY OF ANY
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
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#ifndef SYMBOLTIME_H
#define SYMBOLTIME_H

#include "zigbee.def"
#include "zigbee.h"
#include "generic.h"


#if defined(__18CXX)
/* this section is based on the Timer 0 module of the PIC18 family */
#if(FREQUENCY_BAND == 2400)
    #if(CLOCK_FREQ <= 250000)
        #define CLOCK_DIVIDER 1
        #define CLOCK_DIVIDER_SETTING 0x08 /* no prescalar */
        #define SYMBOL_TO_TICK_RATE 250000
    #elif(CLOCK_FREQ <= 500000)
        #define CLOCK_DIVIDER 2
        #define CLOCK_DIVIDER_SETTING 0x00
        #define SYMBOL_TO_TICK_RATE 500000
    #elif(CLOCK_FREQ <= 1000000)
        #define CLOCK_DIVIDER 4
        #define CLOCK_DIVIDER_SETTING 0x01
        #define SYMBOL_TO_TICK_RATE 1000000
    #elif(CLOCK_FREQ <= 2000000)
        #define CLOCK_DIVIDER 8
        #define CLOCK_DIVIDER_SETTING 0x02
        #define SYMBOL_TO_TICK_RATE 2000000
    #elif(CLOCK_FREQ <= 4000000)
        #define CLOCK_DIVIDER 16
        #define CLOCK_DIVIDER_SETTING 0x03
        #define SYMBOL_TO_TICK_RATE 4000000
    #elif(CLOCK_FREQ <= 8000000)
        #define CLOCK_DIVIDER 32
        #define CLOCK_DIVIDER_SETTING 0x04
        #define SYMBOL_TO_TICK_RATE 8000000
    #elif(CLOCK_FREQ <= 16000000)
        #define CLOCK_DIVIDER 64
        #define CLOCK_DIVIDER_SETTING 0x05
        #define SYMBOL_TO_TICK_RATE 16000000
    #elif(CLOCK_FREQ <= 3200000)
        #define CLOCK_DIVIDER 128
        #define CLOCK_DIVIDER_SETTING 0x06
        #define SYMBOL_TO_TICK_RATE 32000000
    #else
        #define CLOCK_DIVIDER 256
        #define CLOCK_DIVIDER_SETTING 0x07
        #define SYMBOL_TO_TICK_RATE 32000000
    #endif

//  Time calculations changed so overflow does not occur with
//  high clock frequencies

//    #define ONE_SECOND ((CLOCK_FREQ/1000 * 62500) / (SYMBOL_TO_TICK_RATE / 1000))
    #define ONE_SECOND (((((CLOCK_FREQ * 10ul) + (5ul * SYMBOL_TO_TICK_RATE))/SYMBOL_TO_TICK_RATE)*62500ul)/10ul)

    /* SYMBOLS_TO_TICKS to only be used with input (a) as a constant, otherwise you will blow up the code size */
//    #define SYMBOLS_TO_TICKS(a) ((CLOCK_FREQ/1000 * a) / (SYMBOL_TO_TICK_RATE / 1000))
    #define SYMBOLS_TO_TICKS(a) (((((CLOCK_FREQ * 10ul) + (5ul * SYMBOL_TO_TICK_RATE))/SYMBOL_TO_TICK_RATE)*a)/10ul)
#endif
#elif defined(__C30__)
    #if(FREQUENCY_BAND == 2400)
        #if(CLOCK_FREQ <= 125000)
            #define CLOCK_DIVIDER 1
            #define CLOCK_DIVIDER_SETTING 0x0000 /* no prescalar */
            #define SYMBOL_TO_TICK_RATE 125000
        #elif(CLOCK_FREQ <= 1000000)
            #define CLOCK_DIVIDER 8
            #define CLOCK_DIVIDER_SETTING 0x0010
            #define SYMBOL_TO_TICK_RATE 1000000
        #elif(CLOCK_FREQ <= 8000000)
            #define CLOCK_DIVIDER 64
            #define CLOCK_DIVIDER_SETTING 0x0020
            #define SYMBOL_TO_TICK_RATE 8000000
        #else
            #define CLOCK_DIVIDER 256
            #define CLOCK_DIVIDER_SETTING 0x0030
            #define SYMBOL_TO_TICK_RATE 32000000
        #endif

        #define ONE_SECOND ((CLOCK_FREQ/1000 ) / (SYMBOL_TO_TICK_RATE / 1000)) * 62500
        /* SYMBOLS_TO_TICKS to only be used with input (a) as a constant, otherwise you will blow up the code */
        #define SYMBOLS_TO_TICKS(a) (((DWORD)CLOCK_FREQ/1000) / (SYMBOL_TO_TICK_RATE / 1000)) * a
    #endif
#endif

#define TickGetDiff(a,b) (a.Val - b.Val)

typedef union _TICK
{
    DWORD Val;
    struct _TICK_bytes
    {
        BYTE b0;
        BYTE b1;
        BYTE b2;
        BYTE b3;
    } byte;
    BYTE v[4];
    struct _TICK_words
    {
        WORD w0;
        WORD w1;
    } word;
} TICK;

void InitSymbolTimer(void);
TICK TickGet(void);

extern volatile BYTE timerExtension1;
extern volatile BYTE timerExtension2;

#endif
