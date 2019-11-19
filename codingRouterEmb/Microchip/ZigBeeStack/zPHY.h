/*********************************************************************
 *
 *                  PHY Generic Header File
 *
 *********************************************************************
 * FileName:        zPHY.h
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
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#ifndef _zPHY
#define _zPHY
#include "generic.h"

typedef struct _SAVED_BITS
{
    unsigned char bGIEH :1;
} SAVED_BITS;

typedef struct _PHY_ERROR
{
    BYTE failedToMallocRx :1;
} PHY_ERROR;

extern SAVED_BITS savedBits;

#define PHY_CSn_1()\
{\
    PHY_CSn = 1;\
    INTCONbits.GIEH = savedBits.bGIEH;\
}

#define PHY_CSn_0()\
{\
    savedBits.bGIEH = INTCONbits.GIEH;\
    INTCONbits.GIEH = 0;\
    PHY_CSn = 0;\
}

#define iPHY_CSn_1()\
{\
    PHY_CSn = 1;\
}

#define iPHY_CSn_0()\
{\
    PHY_CSn = 0;\
}


ZIGBEE_PRIMITIVE PHYTasks(ZIGBEE_PRIMITIVE inputPrimitive);
void PHYInit(void);
void PHYSetOutputPower( BYTE power);

typedef enum _PHY_enums
{
    phyBUSY =                  0x00,
    phyBUSY_RX =               0x01,
    phyBUSY_TX =               0x02,
    phyFORCE_TRX_OFF =         0x03,
    phyIDLE =                  0x04,
    phyINVALID_PARAMETER =     0x05,
    phyRX_ON =                 0x06,
    phySUCCESS =               0x07,
    phyTRX_OFF =               0x08,
    phyTX_ON =                 0x09,
    phyUNSUPPORTED_ATTRIBUTE = 0x0A
} PHY_enums;

typedef struct _PHY_PIB
{
    BYTE phyCurrentChannel;
    BYTE phyCCAMode;
    WORD phyChannelsSupported;
    BYTE phyBackupChannel;
    union _phyTransmitPower
    {
        BYTE val;
        struct _phyTransmitPower_bits
        {
            unsigned char nominalTransmitPower :6;
            unsigned char tolerance :2;
        } bits;
    } phyTransmitPower;
} PHY_PIB;

typedef union _PHYPendingTasks
{
    struct
    {
        unsigned char PLME_SET_TRX_STATE_request_task :1;
        unsigned char PHY_RX :1;
        unsigned char PHY_TX :1;
        unsigned char PHY_SECURITY :1;
        unsigned char PHY_DATA_REQUEST : 1;
        unsigned char PHY_AUTHORIZE : 1;
    } bits;
    BYTE Val;
} PHY_PENDING_TASKS;
extern PHY_PIB phyPIB;

BYTE PHYHasBackgroundTasks(void);

#if RF_CHIP == CC2420
    #include "zPHY_CC2420.h"
#elif (RF_CHIP == MRF24J40) || (RF_CHIP == UZ2400)
    #include "zPHY_MRF24J40.h"

#endif


#endif
