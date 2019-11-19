/*********************************************************************
 *
 *                  ZigBee Header File
 *
 *********************************************************************
 * FileName:        zigbee.h
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
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#ifndef ZIGBEE_H
#define ZIGBEE_H

#include "generic.h"
#include "zigbee.def"

#define MAINS_POWERED               0x00
#define NOT_MAINS_POWERED           0x01

#if defined(I_SUPPORT_RES_SECURITY) || defined(I_SUPPORT_COM_SECURITY)
	#define I_SUPPORT_SECURITY
#endif

// RF Chip Choices
#define CC2420      1
#define MRF24J40    2
#define UZ2400      3

// Frequency Band Choices
#define FB_2400GHz 2400 //2400–2483.5
#define FB_915MHz 915  //902-928
#define FB_868MHz 868  //868-868.6

typedef struct _LONG_ADDR
{
    BYTE v[8];
} LONG_ADDR;

typedef union _SHORT_ADDR
{
    struct _SHORT_ADDR_bits
    {
        BYTE LSB;
        BYTE MSB;
    } byte;
    WORD Val;
    BYTE v[2];
} SHORT_ADDR;

typedef union _ADDRESS
{
    LONG_ADDR LongAddr;
    SHORT_ADDR ShortAddr;
    BYTE v[8];
} ADDR;

typedef union _GTS_HEADER
{
    BYTE Val;
    struct _GTS_HEADER_bits
    {
        unsigned char GTSDescriptorCount :3;
        unsigned char :4;
        unsigned char GTSPermit :1;
    } bits;
} GTS_HEADER;

typedef struct _KEY_VAL
{
	BYTE v[16];
} KEY_VAL;

typedef SHORT_ADDR PAN_ADDR;

#endif
