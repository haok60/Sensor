/*********************************************************************
 *
 *                  Console Routines
 *
 *********************************************************************
 * FileName:        Console.c
 * Dependencies:
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     10/15/04    Original
 * Nilesh Rajbharti     11/1/04 Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                09/08/05 Microchip ZigBee Stack v1.0-3.3
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
// Uncomment ENABLE_DEBUG line to enable debug mode for this file.
// Or you may also globally enable debug by defining this macro
// in zigbee.def file or from compiler command-line.
#ifndef ENABLE_DEBUG
//#define ENABLE_DEBUG
#endif

#include "Console.h"
#include "zigbee.h"
#include "zigbee.def"
#include "Compiler.h"
//#include <uart.h>

#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__)

    /* Writen by dat_a3cbq91 */
    void ConsoleInit(void)
    {
        U2BRG  = (CLOCK_FREQ/2/4)/BAUD_RATE-1;
        U2MODE = 0x8008;//enable UART2
                        //U2TX and U2RX pins are enabled and used
                        //Standard Speed mode (16 BRG clock cycles per bit)
                        //8-bit data, no parity
                        //One Stop bit

        U2STA = 0x2400;//Transmit is enabled, U2TX pin is controlled by UART2
        #if defined (ROUTER_EMB) || defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM)
            IEC1bits.U2RXIE = 1;
            IFS1bits.U2RXIF = 0;         /*clear interrupt flag*/
        #endif
    }
    /* End by dat_a3cbq91 */

    void ConsolePutROMString(ROM char* str)
    {
        BYTE c;
        while( (c = *str++) != 0 )
        ConsolePut(c);
    }

    void ConsolePut(BYTE c)
    {
        while(U2STAbits.TRMT == 0);
        U2TXREG = c;
    }

    ROM unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    void PrintChar(BYTE toPrint)
    {
        BYTE PRINT_VAR;
        PRINT_VAR = toPrint;
        toPrint = (toPrint>>4)&0x0F;
        ConsolePut(CharacterArray[toPrint]);
        toPrint = (PRINT_VAR)&0x0F;
        ConsolePut(CharacterArray[toPrint]);
        return;
    }

    //programming by dat_a3cbq91
    void PrintWord(WORD toPrint)
    {
        BYTE MSB,LSB;
        MSB = toPrint >> 8;
        LSB = toPrint & 0x00FF;
        PrintChar(MSB);
        PrintChar(LSB);
    }
    //end by dat_a3cbq91

    BYTE ConsoleGet(void)
    {
        char Temp;

//        while(IFS1bits.U2RXIF == 0);
        while(U2STAbits.URXDA == 0);
        Temp = U2RXREG;
//        IFS1bits.U2RXIF = 0;
        return Temp;
    }

    /* Writen by Hung Nguyen */
    //#ifdef USE_MINI_RADAR
        void UART1Init(void)
        {
            U1BRG  = (CLOCK_FREQ/2/4)/BAUD_RATE_UART1-1;

            IEC0bits.U1RXIE = 1;
            IFS0bits.U1RXIF = 0;//enable UART1 interrupt

            U1MODE = 0x8008;//enable UART1
                            //U1TX and U1RX pins are enabled and used
                            //Standard Speed mode (16 BRG clock cycles per bit)
                            //8-bit data, no parity
                            //One Stop bit
            U1STA = 0x0400;//Transmit is enabled, U1TX pin is controlled by UART1
        }

        void UART1Put(BYTE c)
        {
            while(U1STAbits.TRMT == 0);
            U1TXREG = c;
        }

        void UART1PutROMString(ROM char* str)
        {
            BYTE c;

            while( (c = *str++) != 0 )
            UART1Put(c);
        }


//	    ROM unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
        void UART1PrintChar(BYTE toPrint)
        {
            BYTE PRINT_VAR;
            PRINT_VAR = toPrint;
            toPrint = (toPrint>>4)&0x0F;
            UART1Put(CharacterArray[toPrint]);
            toPrint = (PRINT_VAR)&0x0F;
            UART1Put(CharacterArray[toPrint]);
            return;
        }

        void UART1PrintWord(WORD toPrint)
        {
            BYTE MSB,LSB;
            MSB = toPrint >> 8;
            LSB = toPrint & 0x00FF;
            UART1PrintChar(MSB);
            UART1PrintChar(LSB);
        }

        BYTE UART1ConsoleGet(void)
        {
            char Temp;

            while(IFS0bits.U1RXIF == 0);

            Temp = U1RXREG;
            IFS0bits.U1RXIF = 0;
            return Temp;
        }
   // #endif

    /* End by Hung Nguyen*/

#elif defined(__18CXX)
// This is only until they fix the include file...
#ifdef __18F87J10
    #define RCREG   RCREG1
    #define BAUDCON BAUDCON1
#endif


#define USART_USE_BRGH_HIGH

#if defined(USART_USE_BRGH_LOW)
    #define SPBRG_VAL   ( ((CLOCK_FREQ/BAUD_RATE)/64) - 1)
#else
   	#define SPBRG_VAL   ( ((CLOCK_FREQ/BAUD_RATE)/16) - 1)
#endif



#if SPBRG_VAL > 255
    #error "Calculated SPBRG value is out of range for currnet CLOCK_FREQ."
#endif

void ConsoleInit(void)
{

    RCSTAbits.SPEN = 1;   // (RCSTA<7>) must be set (= 1),
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;

#if defined(USART_USE_BRGH_HIGH)
    TXSTA = 0x24;
#else
    TXSTA = 0x20;
#endif

    RCSTA = 0x90; // 0b10010000;
    SPBRG = SPBRG_VAL;
    BAUDCON = 0x40;
}

void ConsolePutROMString(ROM char* str)
{
    BYTE c;

    while( c = *str++ )
        ConsolePut(c);

    // Since this function is mostly for debug, we'll block here to make sure
    // the last character gets out, in case we have a breakpoint on the
    // statement after the function call.
    while( !ConsoleIsPutReady() );
}


BYTE ConsoleGetString(char *buffer, BYTE bufferLen)
{
    BYTE v;
    BYTE count;

    count = 0;
    do
    {
        if ( bufferLen-- == 0 )
            break;

        while( !ConsoleIsGetReady() );

        v = RCREG;

        if ( v == '\r' || v == '\n' )
            break;

        count++;
        *buffer++ = v;
        *buffer = '\0';
    } while(1);
    return count;
}

void ConsolePut(BYTE c)
{
    while( !ConsoleIsPutReady() );
    TXREG = c;
}


void ConsolePutString(BYTE *s)
{
    BYTE c;

    while( (c = *s++) )
        ConsolePut(c);
    while( !ConsoleIsPutReady() );
}


BYTE ConsoleGet(void)
{
    // Clear overrun error if it has occured
    // New bytes cannot be received if the error occurs and isn't cleared
    if(RCSTAbits.OERR)
    {
        RCSTAbits.CREN = 0;   // Disable UART receiver
        RCSTAbits.CREN = 1;   // Enable UART receiver
    }

    return RCREG;
}

#if 1
//TODO: remove test code
//<test code>
ROM unsigned char CharacterArray[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void PrintChar(BYTE toPrint)
{
    BYTE PRINT_VAR;
    PRINT_VAR = toPrint;
    toPrint = (toPrint>>4)&0x0F;
    ConsolePut(CharacterArray[toPrint]);
    toPrint = (PRINT_VAR)&0x0F;
    ConsolePut(CharacterArray[toPrint]);
    return;
}
//</test code>
#endif

#endif
