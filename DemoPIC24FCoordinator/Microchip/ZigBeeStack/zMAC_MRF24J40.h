/*********************************************************************
 *
 *                  MAC Header File for the MRF24J40
 *
 *********************************************************************
 * FileName:        zMAC_MRF24J40.h
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
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#if !defined(_ZMACMRF24J40_H_)
#define _ZMACMRF24J40_H_

typedef struct _currentPacket
{
    BYTE sequenceNumber;
    union _currentPacket_info
    {
        struct _currentPacket_info_bits
        {
            unsigned char retries :3;
            unsigned char association :1;   //this bit indicates if the packet was an association packet (results go to COMM_STATUS) or a normal packet (results go to DATA_confirm)
            unsigned char data_request :1;   //this bit indicates if the packet was an data request packet (results go to MLME_POLL_confirm)
            unsigned char expecting_data :1; //this bit indicates that a POLL_request got an ack back and the framePending bit was set
            unsigned char RX_association :1;
            unsigned char disassociation :1;  //this bit indicates that a MLME_DISASSOCIATION_confirm is required
        } bits;
        BYTE Val;
    } info;
    
    TICK startTime;
    LONG_ADDR DstAddr;  //the destination of current packet if it was an association request or coordinator realignment
    SHORT_ADDR ShortDstAddr;
#ifdef I_SUPPORT_SECURITY
    BOOL SecurityEnable;
#endif
} CURRENT_PACKET;

typedef union _MAC_BACKGROUND_TASKS_PENDING
{
    BYTE Val;
    struct _background_bits
    {
        unsigned char indirectPackets :1;
        unsigned char packetPendingAck :1;
        unsigned char scanInProgress :1;
        unsigned char associationPending :1;
        unsigned char dataInBuffer :1;
        unsigned char bSendUpMACConfirm : 1;
        unsigned char channelScanning :1;
    } bits;
} MAC_TASKS_PENDING;

#define POSSIBLE_CHANNEL_MASK 0x07FFF800

void MACEnable(void);
BOOL MACDisable(void);
extern void MRF24J40Sleep(void);
extern void MRF24J40Wake(void);

#endif

