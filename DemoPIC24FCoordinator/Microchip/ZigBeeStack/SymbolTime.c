/*********************************************************************
 *
 *                  ZigBee Symbol Timer
 *
 *********************************************************************
 * FileName:        SymbolTime.c
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#include "zigbee.def"
#include "SymbolTime.h"
#include "Compiler.h"
#include "generic.h"

volatile BYTE timerExtension1;
volatile BYTE timerExtension2;

//re-programming by dat_a3cbq91 to timing
void InitSymbolTimer()
{
    DWORD timer = TIMER_UNIT;
    #if defined(__18CXX)
        T0CON = 0b00000000 | CLOCK_DIVIDER_SETTING;
        INTCON2bits.TMR0IP = 1;
        INTCONbits.TMR0IF = 0;
        INTCONbits.TMR0IE = 1;
        T0CONbits.TMR0ON = 1;

        timerExtension1 = 0;
        timerExtension2 = 0;
    #elif defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__)
        T2CON = 0b0000000000001000 | CLOCK_DIVIDER_SETTING;//configure Timer2/3 for 32-bit operation, prescale 1:64
        //for timer 5s
        TMR2 = 0x0000;
        TMR3 = 0x0000;
        PR3 = timer >> 16;
        PR2 = timer & 0xFFFF;

        //configure interrupt
        IPC2bits.T3IP = 0x06;
        IFS0bits.T3IF = 0;
        IEC0bits.T3IE = 1;

        T2CONbits.TON = 1;
    #else
        #error "Symbol timer implementation required for stack usage."
    #endif
}
//end by dat_a3cbq91

/* caution: this function should never be called when interrupts are disabled */
/* if interrupts are disabled when this is called then the timer might rollover */
/* and the byte extension would not get updated. */
TICK TickGet(void)
{
    TICK currentTime;

    #if defined(__18CXX)

        /* copy the byte extension */
        currentTime.byte.b2 = 0;
        currentTime.byte.b3 = 0;

        /* disable the timer to prevent roll over of the lower 16 bits while before/after reading of the extension */
        INTCONbits.TMR0IE = 0;

        /* read the timer value */
        currentTime.byte.b0 = TMR0L;
        currentTime.byte.b1 = TMR0H;

        //if an interrupt occured after IE = 0, then we need to figure out if it was
        //before or after we read TMR0L
        if(INTCONbits.TMR0IF)
        {
            if(currentTime.byte.b0<10)
            {
                //if we read TMR0L after the rollover that caused the interrupt flag then we need
                //to increment the 3rd byte
                currentTime.byte.b2++;  //increment the upper most
                if(timerExtension1 == 0xFF)
                {
                    currentTime.byte.b3++;
                }
            }
        }

        /* copy the byte extension */
        currentTime.byte.b2 += timerExtension1;
        currentTime.byte.b3 += timerExtension2;

        /* enable the timer*/
        INTCONbits.TMR0IE = 1;
    #elif defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__)
        currentTime.word.w0 = TMR2;
        currentTime.word.w1 = TMR3;
    #else
        #error "Symbol timer implementation required for stack usage."
    #endif
    return currentTime;
}
