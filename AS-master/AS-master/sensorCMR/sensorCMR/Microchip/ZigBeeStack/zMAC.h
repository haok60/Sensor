/*********************************************************************
 *
 *                  MAC Generic Header File
 *
 *********************************************************************
 * FileName:        zMAC.h
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v3.20 or higher
 *                  MCC30 v3.10 or higher
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
 ********************************************************************/

#ifndef _zMAC
#define _zMAC

#include "SymbolTime.h"
#include "zigbee.h"

//60*32*16
#define aResponseWaitTime 30720

BYTE MACHasBackgroundTasks(void);

ZIGBEE_PRIMITIVE MACTasks(ZIGBEE_PRIMITIVE inputPrimitive);
void MACInit(void);
void MACDiscardRx(void);

#define aUnitBackoffPeriod 20
#define aMaxFrameResponseTime 1220
#define aMaxBE 5

#define SCAN_DURATION_0 SYMBOLS_TO_TICKS(120)
#define SCAN_DURATION_1 SYMBOLS_TO_TICKS(180)
#define SCAN_DURATION_2 SYMBOLS_TO_TICKS(300)
#define SCAN_DURATION_3 SYMBOLS_TO_TICKS(540)
#define SCAN_DURATION_4 SYMBOLS_TO_TICKS(1020)
#define SCAN_DURATION_5 SYMBOLS_TO_TICKS(1980)
#define SCAN_DURATION_6 SYMBOLS_TO_TICKS(3900)
#define SCAN_DURATION_7 SYMBOLS_TO_TICKS(7740)
#define SCAN_DURATION_8 SYMBOLS_TO_TICKS(15420)
#define SCAN_DURATION_9 SYMBOLS_TO_TICKS(30780)
#define SCAN_DURATION_10 SYMBOLS_TO_TICKS(61500)
#define SCAN_DURATION_11 SYMBOLS_TO_TICKS(122940)
#define SCAN_DURATION_12 SYMBOLS_TO_TICKS(245820)
#define SCAN_DURATION_13 SYMBOLS_TO_TICKS(491580)
#define SCAN_DURATION_14 SYMBOLS_TO_TICKS(983100)

/* enumerations for MLME-SYNC-LOSS.indication */
#define PAN_ID_CONFLICT 0xee
#define REALIGNMENT 0xef
#define BEACON_LOST 0xe0

#define MLME_GET_macDSN() macPIB.macDSN

#define MAC_FRAME_TYPE_BEACON 0b000
#define MAC_FRAME_TYPE_DATA 0b001
#define MAC_FRAME_TYPE_ACK 0b010
#define MAC_FRAME_TYPE_CMD 0b011

typedef union _MAC_FRAME_CONTROL
{
    WORD_VAL word;
    struct _MAC_FRAME_CONTROL_bits
    {
        unsigned char FrameType :3;
        unsigned char SecurityEnabled :1;
        unsigned char FramePending :1;
        unsigned char ACKRequest :1;
        unsigned char IntraPAN :1;
        unsigned char :1;
        unsigned char :1;
        unsigned char :1;
        unsigned char DstAddrMode :2;
        unsigned char :1;
        unsigned char :1;
        unsigned char SrcAddrMode :2;
    } bits;
} MAC_FRAME_CONTROL;

typedef struct _TX_STAT
{
	unsigned char filler :3;
	unsigned char no_ack :1;
	unsigned char cipher :1;
    unsigned char ack :1;
    unsigned char finished :1;
    unsigned char success :1;
} TX_STAT;

typedef struct _INT_SAVE
{
    unsigned char filler: 7;
    unsigned char CCP2IntF : 1;
} INT_SAVE;

#define ASSOCIATION_REQUEST 0x01
#define ASSOCIATION_RESPONSE 0x02
#define DISASSOCIATION_NOTIFICATION 0x03
#define DATA_REQUEST 0x04
#define PAN_ID_CONFLICT_NOTIFICATION 0x05
#define ORPHAN_NOTIFICATION 0x06
#define BEACON_REQUEST 0x07
#define COORDINATOR_REALIGNMENT 0x08
#define GTS_REQUEST 0x09

/* MCPS_DATA_confirm.status/MLME_COMM_STATUS.indication values */
#define SUCCESS 0x00
#define TRANSACTION_OVERFLOW 0xf1
#define TRANSACTION_EXPIRED 0xf0
#define CHANNEL_ACCESS_FAILURE 0xe1
#define DENIED 0xe2
#define DISABLE_TRX_FAILURE 0xe3
#define INVALID_GTS 0xe6
#define NO_ACK 0xe9
#define UNAVAILABLE_KEY 0xf3
#define FRAME_TOO_LONG 0xe5
#define FAILED_SECURITY_CHECK 0xe4
#define INVALID_PARAMETER 0xe8
#define MAC_INVALID_HANDLE 0xe7
#define NO_DATA 0xeb
#define NO_SHORT_ADDRESS 0x3c
#define MAC_NO_BEACON 0xea

#define aMaxFrameRetries 3

// MLME_SCAN_request.ScanType values
#define MAC_SCAN_ENERGY_DETECT      0x00
#define MAC_SCAN_ACTIVE_SCAN        0x01
#define MAC_SCAN_PASSIVE_SCAN       0x02
#define MAC_SCAN_ORPHAN_SCAN        0x03


#define MAC_PIB_macBeaconPayloadLength 3


typedef struct _MAC_PIB
{
//    BYTE macAckWaitDuration;  //made a constant
    unsigned char macAssociationPermit :1;
    unsigned char macAutoRequest :1;
    unsigned char macBattLifeExtPeriods :1;
    unsigned char macPromiscuousMode :1;
    unsigned char macRxOnWhenIdle :1;
    BYTE macBeaconPayload[MAC_PIB_macBeaconPayloadLength];   /* always 3 for non-beacon, 5 for beacon */
//    BYTE macBeaconPayloadLength;  /* made a constant */
//    BYTE macBeaconOrder;   //made a constant
    TICK macBeaconTxTime;
    BYTE macBSN;
    LONG_ADDR macCoordExtendedAddress;
    SHORT_ADDR macCoordShortAddress;
    BYTE macDSN;
    BYTE macMaxCSMABackoffs;
    BYTE macMinBE;
    PAN_ADDR macPANId;
    SHORT_ADDR macShortAddress;
//    BYTE macSuperframeOrder;  //made a constant
//    WORD macTransactionPersistenceTime; //made a constant
} MAC_PIB;

typedef union _mac_status
{
    BYTE Val;
    struct _mac_status_bits
    {
        unsigned char allowBeacon :1;
        unsigned char isAssociated :1;
    } bits;
} MAC_STATUS;

#ifdef I_AM_NWK_MANAGER
    typedef struct _ENERGY_DETECT_RECORD
    {
        SHORT_ADDR  ScanDeviceAddr;
        BYTE        EnergyReading[16];
        struct _ENERGY_DETECT_RECORD  *next;
    } ENERGY_DETECT_RECORD;
#endif

extern MAC_PIB macPIB;

void MLME_SET_macPANId_hw( void );
void MLME_SET_macShortAddress_hw( void );

#if RF_CHIP == CC2420
    #include "zMAC_CC2420.h"
#elif (RF_CHIP == MRF24J40) || (RF_CHIP == UZ2400)
    #include "zMAC_MRF24J40.h"
#endif

#endif
