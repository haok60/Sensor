/*********************************************************************

                    ZigBee Tasks Header File

 This file specifies data structures and constants that go between the stack layers.
 In particular, it contains the parameter lists for all of the primitives.  The
 parameter names follow the ZigBee and IEEE 802.15.4 specifications.  The order of
 the parameters have been changed to optimize reuse between successive primitives
 and to optimize alignment for both 8-bit and 16-bit microcontrollers.

 TAKE GREAT CARE WHEN MODIFYING THE PARAMS STRUCTURE OR ANY STRUCTURES/UNIONS
 INCLUDED WITHIN IT!!!

 *********************************************************************
 * FileName:        ZigBeeTasks.h
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
 ********************************************************************/
#ifndef ZIGBEETASKS_H
#define ZIGBEETASKS_H


#include "zigbee.h"
#include "SymbolTime.h"

#include "Console.h"
#include <math.h>
#define TX_BUFFER_SIZE   128
#define TX_DATA_START   0
#define TX_HEADER_START (TX_BUFFER_SIZE-1)

#define FRAME_BEACON        0b00000000
#define FRAME_DATA          0b00000001
#define FRAME_ACK           0b00000010
#define FRAME_MAC_COMMAND   0b00000011

#define FRAME_NWK_COMMAND   0b00000001
#define FRAME_NWK_DATA      0b00000000

typedef union _ASSOCIATE_CAPABILITY_INFO
{
    BYTE Val;
    struct _CapBits
    {
        unsigned char AlternatePANCoordinator :1;
        unsigned char DeviceType :1;
        unsigned char PowerSource :1;
        unsigned char RxOnWhenIdle :1;
        unsigned char :2;
        unsigned char SecurityCapability :1;
        unsigned char AllocateAddress :1;
    } CapBits;
} ASSOCIATE_CAPABILITY_INFO;

typedef struct _FIFO_CONTROL
{
    unsigned char bFIFOInUse :1;
    unsigned char bZDO       :1;
    unsigned char bAPL       :1;
    unsigned char bAPS       :1;
    unsigned char bNWK       :1;
    unsigned char bMAC       :1;
    unsigned char bPHY       :1;
} FIFO_CONTROL;


typedef union _MLME_START_fields
{
    BYTE    Val;
    struct __MLME_START_bits
    {
        BYTE    PANCoordinator          : 1;
        BYTE    BatteryLifeExtension    : 1;
        BYTE    CoordRealignment        : 1;
        BYTE    SecurityEnable          : 1;
    } bits;
} MLME_START_fields;

#define MLME_START_IS_PAN_COORDINATOR     0x01
#define MLME_START_BATTERY_LIFE_EXTENSION 0x02
#define MLME_START_COORD_REALIGNMENT      0x04
#define MLME_START_SECURITY_ENABLE        0x08


typedef struct _NETWORK_DESCRIPTOR
{
    PAN_ADDR    PanID;
    BYTE        LogicalChannel;
    BYTE        StackProfile    : 4;
    BYTE        ZigBeeVersion   : 4;
    BYTE        BeaconOrder     : 4;
    BYTE        SuperframeOrder : 4;
    BYTE        PermitJoining   : 1;
    //BYTE        SecurityLevel   : 3; no longer in beacon payload, so we can't get this
    LONG_ADDR   ExtendedPANID;
    struct _NETWORK_DESCRIPTOR *next;
} NETWORK_DESCRIPTOR;


typedef union _SUPERFRAME_SPEC
{
    WORD Val;
    struct _SUPERFRAME_SPEC_bytes
    {
        BYTE LSB;
        BYTE MSB;
    } byte;
    struct _SUPERFRAME_SPEC_bits
    {
        unsigned char BeaconOrder :4;
        unsigned char SuperframeOrder :4;
        unsigned char FinalCAPSlot :4;
        unsigned char BatteryLifeExtension :1;
        unsigned char :1;
        unsigned char PANCoordinator :1;
        unsigned char AssociationPermit :1;
    } bits;
} SUPERFRAME_SPEC;

typedef struct _PAN_DESCRIPTOR
{
    BYTE                    CoordAddrMode   : 1;    // spec uses 0x02 and 0x03, we'll use 0 and 1 (short/long)
    BYTE                    GTSPermit       : 1;
    BYTE                    SecurityUse     : 1;
    BYTE                    SecurityFailure : 1;
    BYTE                    ACLEntry        : 4;

    BYTE                    LogicalChannel;
    PAN_ADDR                CoordPANId;
    ADDR                    CoordAddress;
    LONG_ADDR               ExtendedPANID;
    SUPERFRAME_SPEC         SuperframeSpec;
    TICK                    TimeStamp;
    BYTE                    LinkQuality;
    struct _PAN_DESCRIPTOR  *next;
} PAN_DESCRIPTOR;

typedef union _BackgroundTasks
{
    struct BackgroundTasks_bits
    {
        unsigned char ZDO :1;
        unsigned char APS :1;
        unsigned char NWK :1;
        unsigned char MAC :1;
        unsigned char PHY :1;
    }bits;
    BYTE Val;
} PENDING_TASKS;

typedef union _PARAMS
{
    /* START OF PHY */

    struct _PD_DATA_request
    {
        BYTE    psduLength;
        BYTE    filler1[1];
        BYTE    *psdu;    /* in TXBuffer already */
    } PD_DATA_request;

    struct _PD_DATA_confirm
    {
        BYTE    status;
    } PD_DATA_confirm;

    struct _PD_DATA_indication
    {
        BYTE    psduLength;
        BYTE    filler1[1];
        BYTE    *psdu;    /* in RXBuffer already */
        BYTE    filler2[23];
        BYTE    ppduLinkQuality;
    } PD_DATA_indication;

//    struct _PLME_CCA_request
//    {
//        No Inputs
//    } PLME_CCA_request;

    struct _PLME_CCA_confirm
    {
        BYTE    status;
    } PLME_CCA_confirm;

//    struct _PLME_ED_request
//    {
//        No Inputs
//    } PLME_ED_request;

    struct _PLME_ED_confirm
    {
        BYTE    status;
        BYTE    filler1[26];
        BYTE    EnergyLevel;
    } PLME_ED_confirm;

    struct _PLME_GET_request
    {
        BYTE    filler1[1];
        BYTE    PIBAttribute;
    } PLME_GET_request;

    struct _PLME_GET_confirm
    {
        BYTE    status;
        BYTE    PIBAttribute;
        void    *PIBAttributeValue;
    } PLME_GET_confirm;

    struct _PLME_SET_TRX_STATE_request
    {
        BYTE    state;
    } PLME_SET_TRX_STATE_request;

    struct _PLME_SET_TRX_STATE_confirm
    {
        BYTE    status;
    } PLME_SET_TRX_STATE_confirm;

    struct _PLME_SET_request
    {
        BYTE    filler1[1];
        BYTE    PIBAttribute;
        void    *PIBAttributeValue;
    } PLME_SET_request;

    /* END OF PHY */

    /* START OF MAC */

    struct _MLME_POLL_request
    {
        BYTE        filler1[1];
        BOOL        SecurityEnabled;
        BYTE        filler2[5];
        BYTE        CoordAddrMode;        //aligns with MCPS_DATA_request dest
        BYTE        filler3[8];
        ADDR        CoordAddress;                //aligns with MCPS_DATA_request dest
        PAN_ADDR    CoordPANId;           //aligns with MCPS_DATA_request dest
    } MLME_POLL_request;

    struct _MLME_POLL_confirm
    {
        BYTE    status;                 // Must overlay NLME_SYNC_confirm Status
    } MLME_POLL_confirm;

    struct _MCPS_DATA_request
    {
        BYTE        msduLength;
        BYTE        filler1[1];
        BYTE        frameType; /* this is an addition to the IEEE spec to reuse the most code, either FRAME_DATA, FRAME_BEACON, FRAME_ACK, or FRAME_MAC_COMMAND */
        union _TxOptions
        {
            BYTE Val;
            struct _bits_for_MCPS_DATA_request
            {
                unsigned char acknowledged_transmission :1;
                unsigned char GTS_transmission :1;
                unsigned char indirect_transmission :1;
                unsigned char security_enabled_transmission :1;
                unsigned char :4;
            } bits;
        } TxOptions;
        PAN_ADDR    SrcPANId;
        BYTE        SrcAddrMode;
        BYTE        DstAddrMode;
        ADDR        SrcAddr;
        ADDR        DstAddr;           //must align with MLME_ASSOCIATE_response DeviceAddress
        PAN_ADDR    DstPANId;
        BYTE        msduHandle;
		BYTE        PA_LEVEL_TX;
        /* msdu is in TXBuffer */
    } MCPS_DATA_request;

    struct _MCPS_DATA_confirm
    {
        BYTE        status;                 // Must overlay NLDE_DATA_confirm.Status
        BYTE        filler1[25];
        BYTE        msduHandle;
    } MCPS_DATA_confirm;

    struct _MCPS_DATA_indication
    {
        BYTE        msduLength;         // Must overlay NLDE_DATA_indication NsduLength and PD_DATA_indication.psduLength
        BOOL        SecurityUse;       //overlay MLME_ASSOCIATE_indication
        BYTE*       msdu;               // Must overlay NLDE_DATA_indication.Nsdu and PD_DATA_indication.psdu
        PAN_ADDR    SrcPANId;
        BYTE        SrcAddrMode;        // Must overlay MLME_ASSOCIATE_indication
        BYTE        DstAddrMode;
        ADDR        SrcAddr;
        ADDR        DstAddr;
        PAN_ADDR    DstPANId;
        BYTE        ACLEntry;       //overlay MLME_ASSOCIATE_indication
        BYTE        mpduLinkQuality;    // Must overlay NLDE_DATA_indication LinkQuality
    } MCPS_DATA_indication;

    struct _MCPS_PURGE_request
    {
        BYTE        filler1[26];
        BYTE        msduHandle;
    }   MCPS_PURGE_request;

    struct _MCPS_PURGE_confirm
    {
        BYTE        status;
        BYTE        filler1[25];
        BYTE        msduHandle;
    }   MCPS_PURGE_confirm;

    struct _MLME_ORPHAN_indication
    {
        BYTE        filler1[1];
        BOOL        SecurityUse;       //overlay MLME_ASSOCIATE_indication
        BYTE        filler2[6];
        LONG_ADDR   OrphanAddress;      // MLME_ORPHAN_response OrphanAddress and MCPS_DATA_indication.SrcAddr
        BYTE        filler3[10];
        BYTE        ACLEntry;       //overlay MLME_ASSOCIATE_indication
    } MLME_ORPHAN_indication;

    struct _MLME_ORPHAN_response
    {
        BOOL        AssociatedMember;
        BOOL        SecurityEnable;
        BYTE        filler1[6];
        LONG_ADDR   OrphanAddress;      // Must overlay MLME_ORPHAN_indication OrphanAddress and MCPS_DATA_indication.SrcAddr
        SHORT_ADDR  ShortAddress;
    } MLME_ORPHAN_response;

    struct _MLME_DISASSOCIATE_request
    {
        BYTE        DisassociateReason;
        BOOL        SecurityUse;       //overlay MLME_ASSOCIATE_indication
        BYTE        filler1[14];
        LONG_ADDR   DeviceAddress;      // Must overlay MCPS_DATA_indication.DestAddr
    } MLME_DISASSOCIATE_request;

    struct _MLME_DISASSOCIATE_indication
    {
        BYTE        DisassociateReason;
        BOOL        SecurityUse;       //overlay MLME_ASSOCIATE_indication
        BYTE        filler1[6];
        LONG_ADDR   DeviceAddress;      // Must overlay MCPS_DATA_indication.SrcAddr
        BYTE        filler2[10];
        BYTE        ACLEntry;       //overlay MLME_ASSOCIATE_indication
    } MLME_DISASSOCIATE_indication;

    struct _MLME_DISASSOCIATE_confirm
    {
        BYTE        status;
    } MLME_DISASSOCIATE_confirm;

    struct _MLME_SYNC_LOSS_indication
    {
        BYTE        LossReason;
    } MLME_SYNC_LOSS_indication;

    struct _MLME_BEACON_NOTIFY_indication
    {
        BYTE            sduLength;     /* aligned with msduLength of MCPS_DATA_indication */
        BOOL            SecurityUse;       /* aligned with SecurityUse of MCPS_DATA_indication */
        BYTE            *sdu;          /* aligned with msdu of MCPS_DATA_indication */
        PAN_ADDR        CoordPANId;    /* aligned with SrcPANId of MCPS_DATA_indication */
        BYTE            CoordAddrMode;     /* aligned with SrcAddrMode of MCPS_DATA_indication */
        BYTE            LogicalChannel;
        ADDR            CoordAddress;      /* aligned with SrcAddr of MCPS_DATA_indication */
        TICK            TimeStamp;
        BYTE            BSN;
        BYTE            filler1[1];
        BOOL            SecurityFailure;
        BOOL            GTSPermit;
        SUPERFRAME_SPEC SuperframeSpec;
        BYTE            ACLEntry;          /* aligned with ACLEntry of MCPS_DATA_indication */
        BYTE            LinkQuality;       /* aligned with mpduLinkQuality of MCPS_DATA_indication */

// we don't use AddrList because we have a non-beacon network only.
//        BYTE* AddrList;
// We don't use PendAddrSpec because we use a non-beacon network
//        union PENDING_ADDR
//        {
//            BYTE Val;
//            struct _PENDING_ADDR_bits
//            {
//                unsigned int numShortAddresses :3;
//                unsigned int :1;
//                unsigned int numExtAddresses :3;
//                unsigned int :1;
//            } bits;
//        } PendAddrSpec;

    } MLME_BEACON_NOTIFY_indication;

    struct _MLME_ASSOCIATE_indication
    {
        ASSOCIATE_CAPABILITY_INFO   CapabilityInformation;
        BOOL                        SecurityUse;
        BYTE                        filler1[6];
        LONG_ADDR                   DeviceAddress;  // Must overlay MCPS_DATA_indication
        BYTE                        filler2[10];
        BYTE                        ACLEntry;
    } MLME_ASSOCIATE_indication;

    struct _MLME_ASSOCIATE_confirm
    {
        BYTE        status;                     // Must overlay NLME_JOIN_confirm Status
        BYTE        filler1[5];
        SHORT_ADDR  AssocShortAddress;          // Must NOT overlay MCPS_DATA_indication.SrcAddr
    } MLME_ASSOCIATE_confirm;

    struct _MLME_ASSOCIATE_request
    {
        ASSOCIATE_CAPABILITY_INFO   CapabilityInformation;  // Must overlay MLME_ASSOCIATE_indication CapabilityInformation
        BOOL                        SecurityEnable;             // Must overlay MLME_ASSOCIATE_response SecurityEnable and NLME_JOIN_request MACSecurity
        BYTE                        filler1[4];
        BYTE                        LogicalChannel;
        BYTE                        CoordAddrMode;
        BYTE                        filler2[8];
        ADDR                        CoordAddress;           // Must overlay NLME_DIRECT_JOIN_indication DeviceAddress and MLME_ASSOCIATE_indication DeviceAddress
        PAN_ADDR                    CoordPANId;                 // Must overlay NLME_JOIN_request PANId
    } MLME_ASSOCIATE_request;

    struct _MLME_ASSOCIATE_response
    {
        BYTE                        status;
        BOOL                        SecurityEnable;             // Must overlay MLME_ASSOCIATE_indication SecurityUse
        BYTE                        filler1[4];
        SHORT_ADDR                  AssocShortAddress;
        BYTE                        filler2[8];
        LONG_ADDR                   DeviceAddress;              // Must overlay MLME_ASSOCIATE_indication DeviceAddress
    } MLME_ASSOCIATE_response;

    struct _MLME_COMM_STATUS_indication
    {
        BYTE        status;
        BYTE        filler1[3];
        PAN_ADDR    PANId;              //not populated because its always macPIB.macPANId for Zigbee 1.0
        BYTE        SrcAddrMode;        //not populated because its always 0x03 for zigbee
        BYTE        DstAddrMode;        //not populated because its always 0x03 for Zigbee
        ADDR        SrcAddr;        //not populated because its always MAC_LONG_ADDR for Zigbee
        ADDR        DstAddr;            // Must overlay NLME_JOIN_indication ExtendedAddress
    } MLME_COMM_STATUS_indication;

    struct _MLME_RESET_request
    {
        BOOL        SetDefaultPIB;
    } MLME_RESET_request;

    struct _MLME_RESET_confirm
    {
        BYTE        status;
    } MLME_RESET_confirm;

    struct _MLME_SCAN_request
    {
        BYTE                        filler1[6];
        BYTE                        ScanType;
        BYTE                        ScanDuration;               // Must overlay NLME_NETWORK_FORMATION_request and NLME_JOIN_request ScanDuration
        DWORD_VAL                   ScanChannels;               // Must overlay NLME_NETWORK_FORMATION_request and NLME_JOIN_request ScanChannels
    } MLME_SCAN_request;

    struct _MLME_SCAN_confirm
    {
        BYTE                        status;                     // Must overlay NLME_NETWORK_DISCOVERY_confirm and NMLE_JOIN_confirm Status
        BYTE                        ResultListSize;
        BYTE                        *EnergyDetectList;
        PAN_DESCRIPTOR              *PANDescriptorList;
        BYTE                        ScanType;
        BYTE                        filler1[1];
        DWORD_VAL                   UnscannedChannels;
    } MLME_SCAN_confirm;


    struct _MLME_START_request
    {
        MLME_START_fields           fields;
        BYTE                        filler1[3];
        PAN_ADDR                    PANId;
        BYTE                        LogicalChannel;
        BYTE                        filler2[5];
        BYTE                        BeaconOrder;
        BYTE                        SuperframeOrder;
    } MLME_START_request;

    struct _MLME_START_confirm
    {
        BYTE                        status;                     // Must overlay NLME_NETWORK_FORMATION_confirm Status
    } MLME_START_confirm;


    /* START OF NWK */

    struct _NLDE_DATA_request
    {
        BYTE        NsduLength;
        BOOL        SecurityEnable;
        BYTE        DstAddrMode;        // yfy added for R13, not part of 2006
        BYTE        NonmemberRadius;    // yfy added for R13, not part of 2006
        BYTE        filler1[4];
        BYTE        BroadcastRadius;
        BYTE        DiscoverRoute;
        BYTE        filler2[6];
        SHORT_ADDR  DstAddr;
        BYTE        filler3[8];
        BYTE        NsduHandle;
        // *Nsdu is TXBuffer
    } NLDE_DATA_request;

    struct _NLME_ROUTE_DISCOVERY_request     //For Zigbee 2006
    {
        BYTE        filler1;
        BOOL        filler2;
        BYTE        DstAddrMode;        
        BYTE        filler3;            
        BYTE        filler4[4];
        BYTE        Radius;
        BYTE        filler5;
        BYTE        filler6[6];
        SHORT_ADDR  DstAddr;
        BYTE        filler7[8];
        BYTE        filler8;
    } NLME_ROUTE_DISCOVERY_request;

    struct _NLDE_DATA_confirm
    {
        BYTE        Status;                 // Must overlay MCPS_DATA_confirm.status
        BYTE        filler1[25];
        BYTE        NsduHandle;
    } NLDE_DATA_confirm;

    struct _NLME_ROUTE_DISCOVERY_confirm     // For ZigBee 2006
    {
        BYTE        Status;                 // Must overlay MCPS_DATA_confirm.status
        BYTE        filler1[25];
        BYTE        filler2;                
    } NLME_ROUTE_DISCOVERY_confirm;
    
    struct _NLDE_DATA_indication
    {
        BYTE        NsduLength;             // Must overlay MCPS_DATA_indication msduLength
        BYTE        filler1[1];
        BYTE        *Nsdu;                  // Must overlay MCPS_DATA_indication msdu
        SHORT_ADDR  DstAddr;                // yfy added for R13, not used in 2006
        BYTE        DstAddrMode;            // yfy added for R13, not used in 2006
        BYTE        filler2[1];
        SHORT_ADDR  SrcAddress;
        BYTE        filler3[17];
        BYTE        LinkQuality;            // Must overlay MCPS_DATA_indication mpduLinkQuality
    } NLDE_DATA_indication;

    struct _NLME_DIRECT_JOIN_request
    {
        ASSOCIATE_CAPABILITY_INFO   CapabilityInformation;  // Must overlay MLME_ASSOCIATE_indication CapabilityInformation
        BYTE                        filler1[7];
        LONG_ADDR                   DeviceAddress;  // Must overlay NLME_DIRECT_JOIN_confirm DeviceAddress and MLME_ASSOCIATE_indication DeviceAddress
    } NLME_DIRECT_JOIN_request;

    struct _NLME_DIRECT_JOIN_confirm
    {
        BYTE                        Status;
        BYTE                        filler1[7];
        LONG_ADDR                   DeviceAddress;  // Must overlay NLME_DIRECT_JOIN_request DeviceAddress and MLME_ASSOCIATE_indication DeviceAddress
    } NLME_DIRECT_JOIN_confirm;

    struct _NLME_JOIN_indication
    {
        ASSOCIATE_CAPABILITY_INFO   CapabilityInformation;
        BOOL                        secureJoin;
        BYTE                        filler1[6];
        SHORT_ADDR                  ShortAddress;
        BYTE                        filler2[6];
        LONG_ADDR                   ExtendedAddress;            // Must overlay MLME_COMM_STATUS_indication DstAddr
    } NLME_JOIN_indication;

    struct _NLME_JOIN_request
    {
        BYTE                        RejoinNetwork;
        BOOL                        MACSecurity;                // Must overlay MLME_ASSOCIATE_request SecurityEnable
        BOOL                        JoinAsRouter;
        BYTE                        PowerSource;
        BYTE                        filler1[2];
        BOOL                        RxOnWhenIdle;
        BYTE                        ScanDuration;               // Must overlay NLME_NETWORK_FORMATION_request and MLME_SCAN_request ScanDuration
        DWORD_VAL                   ScanChannels;               // Must overlay NLME_NETWORK_FORMATION_request and MLME_SCAN_request ScanChannels
        BYTE                        filler2[12];
        PAN_ADDR                    PANId;                      // Must overlay MLME_ASSOCIATE_request CoordPANId
        LONG_ADDR                   ExtendedPANID;
    } NLME_JOIN_request;

    struct _NLME_JOIN_confirm
    {
        BYTE                        Status;                     // Must overlay MLME_ASSOCIATE_confirm and MLME_SCAN_confirm status
        BOOL                        HaveNetworkKey;             // yfy added for 2006
        BYTE                        filler[4];
        SHORT_ADDR                  ShortAddress;               // yfy added for 2006
        BYTE                        filler1[16];
        PAN_ADDR                    PANId;
    } NLME_JOIN_confirm;

    struct _NLME_LEAVE_request
    {
        BOOL                        RemoveChildren;
        BOOL                        Rejoin;
        BOOL                        ReuseAddress;
        BOOL                        Silent;
        BYTE                        filler1[12];
        LONG_ADDR                   DeviceAddress;  // Must overlay MLME_DISASSOCIATE_request and NLME_LEAVE_confirm
    } NLME_LEAVE_request;

    struct _NLME_LEAVE_indication
    {
        BOOL                        Rejoin;         // yfy added for 2006
        BYTE                        filler1;
        BYTE                        filler2[6];
        LONG_ADDR                   DeviceAddress;  // Must overlay MLME_DISASSOCIATE_indication and MCPS_DATA_indication.SrcAddr
    } NLME_LEAVE_indication;

    struct _NLME_LEAVE_confirm
    {
        BYTE                        Status;
        BYTE                        filler1[15];
        LONG_ADDR                   DeviceAddress;  // Must overlay MLME_DISASSOCIATE_request and NLME_LEAVE_request
    } NLME_LEAVE_confirm;

    struct _NLME_NETWORK_FORMATION_request
    {
        BYTE                        filler1[4];
        PAN_ADDR                    PANId;
        BOOL                        BatteryLifeExtension;
        BYTE                        ScanDuration;               // Must overlay NLME_JOIN_request and MLME_SCAN_request ScanDuration
        DWORD_VAL                   ScanChannels;               // Must overlay NLME_JOIN_request and MLME_SCAN_request ScanChannels
        BYTE                        BeaconOrder;
        BYTE                        SuperframeOrder;
    } NLME_NETWORK_FORMATION_request;

    struct _NLME_NETWORK_FORMATION_confirm
    {
        BYTE                        Status;                     // Must overlay MLME_START_confirm status
    } NLME_NETWORK_FORMATION_confirm;

    struct _NLME_NETWORK_DISCOVERY_request
    {
        BYTE                        filler1[7];
        BYTE                        ScanDuration;               // Must overlay NLME_JOIN_request and MLME_SCAN_request ScanDuration
        DWORD_VAL                   ScanChannels;               // Must overlay NLME_JOIN_request and MLME_SCAN_request ScanChannels
    } NLME_NETWORK_DISCOVERY_request;

    struct _NLME_NETWORK_DISCOVERY_confirm
    {
        BYTE                        Status;                     // Must overlay MLME_SCAN_confirm status
        BYTE                        NetworkCount;               // Must overlay MLME_SCAN_confirm ResultListSize
        NETWORK_DESCRIPTOR          *NetworkDescriptor;         // Must NOT overlay MLME_SCAN_confirm.PANDescriptorList so we can work with both at the same time
    } NLME_NETWORK_DISCOVERY_confirm;

    struct _NLME_PERMIT_JOINING_request
    {
        BYTE                        PermitDuration;
        BOOL                        _updatePayload;
    } NLME_PERMIT_JOINING_request;

    struct _NLME_PERMIT_JOINING_confirm
    {
        BYTE                        Status;
    } NLME_PERMIT_JOINING_confirm;

    struct _NLME_START_ROUTER_request
    {
        BYTE                        filler1[6];
        BOOL                        BatteryLifeExtension;
        BYTE                        filler2[5];
        BYTE                        BeaconOrder;
        BYTE                        SuperframeOrder;
    } NLME_START_ROUTER_request;

    struct _NLME_START_ROUTER_confirm
    {
        BYTE                        Status;
    } NLME_START_ROUTER_confirm;

    struct _NLME_SYNC_request
    {
        BOOL                        Track;
    } NLME_SYNC_request;

    struct _NLME_SYNC_confirm
    {
        BYTE                        Status;     // Must overlay MLME_POLL_confirm status
    } NLME_SYNC_confirm;


    // APS Layer Primitive Parameters

    struct _APSDE_DATA_request
    {
        BYTE        asduLength;
        BYTE        filler1[3];
        WORD_VAL    ProfileId;
        BYTE        filler2[1];
        BYTE        DstAddrMode;
        BYTE        RadiusCounter;
        BYTE        DiscoverRoute;
        union _TX_OPTIONS
        {
            BYTE    Val;
            struct _TX_OPTIONS_BITS
            {
                BYTE    securityEnabled : 1;
                BYTE    useNWKKey       : 1;
                BYTE    acknowledged    : 1;
            } bits;
        }           TxOptions;
        BYTE        filler3[5];
        ADDR        DstAddress;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
    } APSDE_DATA_request;

    struct _APSDE_DATA_confirm
    {
        BYTE        Status;
        BYTE        filler1[6];
        BYTE        DstAddrMode;
        BYTE        filler2[8];
        ADDR        DstAddress;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
    } APSDE_DATA_confirm;

    struct _APSDE_DATA_indication
    {
        BYTE        asduLength;
        BYTE        SecurityStatus;
        BYTE *      asdu;
        WORD_VAL    ProfileId;
        BYTE        SrcAddrMode;
        BYTE        WasBroadcast;
        ADDR        SrcAddress;
        ADDR        DstAddress;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        BYTE        LinkQuality;            // Must overlay NLDE_DATA_indication LinkQuality (addition to spec)
        BYTE        DstAddrMode;
    } APSDE_DATA_indication;

    struct _APSME_BIND_request
    {
        BYTE        filler1[8];
        LONG_ADDR   SrcAddr;
        ADDR        DstAddr;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        BYTE        DstAddrMode;
    } APSME_BIND_request;

    struct _APSME_BIND_confirm
    {
        BYTE        Status;
        BYTE        filler1[7];
        LONG_ADDR   SrcAddr;
        ADDR        DstAddr;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        BYTE        DstAddrMode;
     } APSME_BIND_confirm;

     struct _APSME_UNBIND_request
     {
        BYTE        filler1[8];
        LONG_ADDR   SrcAddr;
        ADDR        DstAddr;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        BYTE        DstAddrMode;
    } APSME_UNBIND_request;

    struct _APSME_UNBIND_confirm
    {
        BYTE        Status;
        BYTE        filler1[7];
        LONG_ADDR   SrcAddr;
        ADDR        DstAddr;
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        BYTE        DstAddrMode;
    } APSME_UNBIND_confirm;

    struct _APSME_ESTABLISH_KEY_request
    {
	 BYTE		UseParent;
	 BYTE		KeyEstablishmentMethod;
	 BYTE		filler1[14];
	 LONG_ADDR	ResponderAddress;
	 LONG_ADDR  ResponderParentAddress;
    } APSME_ESTABLISH_KEY_request;

    struct _APSME_ESTABLISH_KEY_confirm
    {
    	BYTE		Status;
	 	BYTE		filler1[7];
	 	LONG_ADDR	Address;
    } APSME_ESTABLISH_KEY_confirm;

    struct _APSME_ESTABLISH_KEY_indication
    {
		BYTE		KeyEstablishmentMethod;
    	BYTE		filler1[7];
		LONG_ADDR	InitiatorAddress;
    } APSME_ESTABLISH_KEY_indication;

    struct _APSME_ESTABLISH_KEY_response
    {
	    BYTE		Accept;
	    BYTE		filler1[7];
	 	LONG_ADDR	InitiatorAddress;
    } APSME_ESTABLISH_KEY_response;

    struct _APSME_TRANSPORT_KEY_request
    {
	    BYTE		KeyType;
	    BYTE		filler0[1];
		KEY_VAL		*Key;
		BYTE		filler[10];
		BYTE        _DstAddrMode;    /* Zigbee 2006 so that we can broadcast keys */
		BYTE        _UseSecurity;
		LONG_ADDR	DestinationAddress;
		LONG_ADDR	ParentAddress;
		union _TRANSPORTKEYDATA
		{
			struct _NETWORKKEY
			{
				BYTE		UseParent;
				BYTE		KeySeqNumber;
			} NetworkKey;
			struct _APPLICATIONMASTER_LINKKEY
			{
				BYTE		Initiator;
			} ApplicationMaster_LinkKey;
		} TransportKeyData;
	
    } APSME_TRANSPORT_KEY_request;

    struct _APSME_TRANSPORT_KEY_indication
    {
    	BYTE	KeyType;
    	BYTE	filler0[1];
		KEY_VAL *Key;
		BYTE	filler1[4];
		LONG_ADDR	SrcAddr;
		BYTE	filler2[8];
		union _TransportKeyData
		{
			struct _NetworkKey
			{
//				BYTE	UseParent;
				BYTE	KeySeqNumber;
			} NetworkKey;
			struct _ApplicationMaster_LinkKey
			{
				LONG_ADDR	PatnerAddress;
				BYTE	Initiator;
			} ApplicationMaster_LinkKey;
		} TransportKeyData;

    } APSME_TRANSPORT_KEY_indication;

    struct _APSME_UPDATE_DEVICE_request
    {
    	BYTE		Status;
		BYTE		filler1[7];
		LONG_ADDR	DeviceAddress;
		LONG_ADDR	DestAddress;
		SHORT_ADDR DeviceShortAddress;
    } APSME_UPDATE_DEVICE_request;


    struct _APSME_UPDATE_DEVICE_indication
    {
		BYTE		Status;
		BYTE		filler1[7];
		LONG_ADDR	SrcAddress;
		LONG_ADDR	DeviceAddress;
		SHORT_ADDR DeviceShortAddress;
    } APSME_UPDATE_DEVICE_indication;

    struct _APSME_REMOVE_DEVICE_request
    {
		BYTE		filler[16];
		LONG_ADDR	ParentAddress;
		LONG_ADDR	ChildAddress;
    } APSME_REMOVE_DEVICE_request;

    struct _APSME_REMOVE_DEVICE_indication
    {
		BYTE		filler[8];
		LONG_ADDR	SrcAddress;
		BYTE		filler2[8];
		LONG_ADDR	ChildAddress;
    } APSME_REMOVE_DEVICE_indication;

    struct _APSME_REQUEST_KEY_request
    {
		BYTE		KeyType;
		BYTE		filler[15];
		LONG_ADDR	DestAddress;
		LONG_ADDR	PartnerAddress;
    } APSME_REQUEST_KEY_request;

    struct _APSME_REQUEST_KEY_indication
    {
		BYTE		KeyType;
		BYTE		filler1[7];
		LONG_ADDR	SrcAddress;
		BYTE		filler2[8];
		LONG_ADDR	PartnerAddress;
    } APSME_REQUEST_KEY_indication;

    struct _APSME_SWITCH_KEY_request
    {	
		BYTE		KeySeqNumber;
		BYTE		filler[15];
		LONG_ADDR	DestAddress;
    } APSME_SWITCH_KEY_request;

    struct _APSME_SWITCH_KEY_indication
    {
		BYTE		KeySeqNumber;
		BYTE		filler[7];
		LONG_ADDR	SrcAddress;
    } APSME_SWITCH_KEY_indication;
    
    struct _APSME_ADD_GROUP_request
    {
        BYTE        filler[2];
        SHORT_ADDR  GroupAddress;
        BYTE        Endpoint;
    } APSME_ADD_GROUP_request;
    
    struct _APSME_ADD_GROUP_confirm
    {
        BYTE        Status;
        BYTE        filler[1];
        SHORT_ADDR  GroupAddress;
        BYTE        Endpoint;
    } APSME_ADD_GROUP_confirm;
    
    struct _APSME_REMOVE_GROUP_request
    {
        BYTE        filler[2];
        SHORT_ADDR  GroupAddress;
        BYTE        Endpoint;
    } APSME_REMOVE_GROUP_request;
    
    struct _APSME_REMOVE_GROUP_confirm
    {
        BYTE        Status;
        BYTE        filler[1];
        SHORT_ADDR  GroupAddress;
        BYTE        Endpoint;
    } APSME_REMOVE_GROUP_confirm;
    
    struct _APSME_REMOVE_ALL_GROUPS_request
    {
        BYTE        filler[2];
        BYTE        Endpoint;
    } APSME_REMOVE_ALL_GROUPS_request;
    
    struct _APSME_REMOVE_ALL_GROUPS_confirm
    {
        BYTE        Status;
        BYTE        filler[1];
        BYTE        Endpoint;
    } APSME_REMOVE_ALL_GROUPS_confirm;    
	
    struct _ZDO_DATA_indication     // Must match APSDE_DATA_indication
    {
        BYTE        asduLength;
        BYTE        SecurityStatus;
        BYTE *      asdu;
        WORD_VAL    ProfileId;
        BYTE        SrcAddrMode;
        BYTE        WasBroadcast;
        ADDR        SrcAddress;
        BYTE        filler1[8];
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        BYTE        LinkQuality;            // Must overlay NLDE_DATA_indication LinkQuality (addition to spec)
    } ZDO_DATA_indication;

    struct _ZDO_END_DEVICE_BIND_req
    {
        BYTE        filler1[4];
        WORD_VAL    ProfileID;
        BYTE        NumInClusters;
        BYTE        NumOutClusters;
        BYTE        filler2[8];
        WORD        *InClusterList;
        WORD        *OutClusterList;
        SHORT_ADDR  LocalCoordinator;
        BYTE        endpoint;
        BYTE        sequenceNumber;
    } ZDO_END_DEVICE_BIND_req;

    struct _ZDO_BIND_req
    {
        // NOTE: If we allow long addresses in ZDO_DATA_indication, this structure must change.
        BYTE        filler1[4];
        BYTE        Status;
        BYTE        SrcEndp;
        BYTE        DstEndp;
        BYTE        filler2[1];
        WORD_VAL    ClusterID;
        ADDR        SrcAddress;
        ADDR        DstAddress;
        BYTE        DstAddrMode;
    } ZDO_BIND_req;

    struct _ZDO_UNBIND_req
    {
        // NOTE: If we allow long addresses in ZDO_DATA_indication, this structure must change.
        BYTE        filler1[4];
        BYTE        Status;
        BYTE        SrcEndp;
        BYTE        DstEndp;
        BYTE        filler2[1];
        WORD_VAL    ClusterID;
        ADDR        SrcAddress;
        ADDR        DstAddress;
        BYTE        DstAddrMode;
    } ZDO_UNBIND_req;


} PARAMS;

typedef enum _ZIGBEE_PRIMITIVE
{
    NO_PRIMITIVE = 0,

    PD_DATA_request = 0x01,
    PD_DATA_confirm = 0x02,
    PD_DATA_indication = 0x03,
    PLME_CCA_request = 0x04,
    PLME_CCA_confirm = 0x05,
    PLME_ED_request = 0x06,
    PLME_ED_confirm = 0x07,
    PLME_GET_request = 0x08,
    PLME_GET_confirm = 0x09,
    PLME_SET_TRX_STATE_request = 0x0A,
    PLME_SET_TRX_STATE_confirm = 0x0B,
    PLME_SET_request = 0x0C,
    PLME_SET_confirm = 0x0D,

    MCPS_DATA_request = 0x10,
    MCPS_DATA_confirm = 0x11,
    MCPS_DATA_indication = 0x12,
    MCPS_PURGE_request = 0x13,
    MCPS_PURGE_confirm = 0x14,
    MLME_ASSOCIATE_request = 0x15,
    MLME_ASSOCIATE_indication = 0x16,
    MLME_ASSOCIATE_response = 0x17,
    MLME_ASSOCIATE_confirm = 0x18,
    MLME_DISASSOCIATE_request = 0x19,
    MLME_DISASSOCIATE_indication = 0x1A,
    MLME_DISASSOCIATE_confirm = 0x1B,
    MLME_BEACON_NOTIFY_indication = 0x1C,
    MLME_GET_request = 0x1D,
    MLME_GET_confirm = 0x1E,
    MLME_GTS_request = 0x1F,
    MLME_GTS_confirm = 0x20,
    MLME_GTS_indication = 0x21,
    MLME_ORPHAN_indication = 0x22,
    MLME_ORPHAN_response = 0x23,
    MLME_RESET_request = 0x24,
    MLME_RESET_confirm = 0x25,
    MLME_RX_ENABLE_request = 0x26,
    MLME_RX_ENABLE_confirm = 0x27,
    MLME_SCAN_request = 0x28,
    MLME_SCAN_confirm = 0x29,
    MLME_COMM_STATUS_indication = 0x2A,
    MLME_SET_request = 0x2B,
    MLME_SET_confirm = 0x2C,
    MLME_START_request = 0x2D,
    MLME_START_confirm = 0x2E,
    MLME_SYNC_request = 0x2F,
    MLME_SYNC_LOSS_indication = 0x30,
    MLME_POLL_request = 0x31,
    MLME_POLL_confirm = 0x32,

    NLDE_DATA_request,                  //0x33
    NLDE_DATA_confirm,                  //0x34
    NLDE_DATA_indication,               //0x35

    NLME_NETWORK_DISCOVERY_confirm,     //0x36
    NLME_NETWORK_FORMATION_confirm,     //0x37
    NLME_PERMIT_JOINING_confirm,        //0x38
    NLME_START_ROUTER_confirm,          //0x39
    NLME_JOIN_confirm,                  //0x3a
    NLME_DIRECT_JOIN_confirm,           //0x3b
    NLME_LEAVE_confirm,                 //0x3c
    NLME_RESET_confirm,                 //0x3d
    NLME_SYNC_confirm,                  //0x3e
    NLME_GET_confirm,                   //0x3f
    NLME_SET_confirm,                   //0x40

    NLME_NETWORK_DISCOVERY_request,     //0x41
    NLME_NETWORK_FORMATION_request,     //0x42
    NLME_PERMIT_JOINING_request,        //0x43
    NLME_START_ROUTER_request,          //0x44
    NLME_JOIN_request,                  //0x45
    NLME_DIRECT_JOIN_request,           //0x46
    NLME_LEAVE_request,                 //0x47
    NLME_RESET_request,                 //0x48
    NLME_SYNC_request,                  //0x49
    NLME_GET_request,                   //0x4a
    NLME_SET_request,                   //0x4b

    NLME_JOIN_indication,               //0x4c
    NLME_LEAVE_indication,              //0x4d
    NLME_SYNC_indication,               //0x4e

    APSDE_DATA_request,                 //0x4f
    APSDE_DATA_confirm,                 //0x50
    APSDE_DATA_indication,              //0x51

    APSME_BIND_request,                 //0x52
    APSME_GET_request,                  //0x53
    APSME_SET_request,                  //0x54
    APSME_UNBIND_request,               //0x55
    APSME_BIND_confirm,                 //0x56
    APSME_GET_confirm,                  //0x57
    APSME_SET_confirm,                  //0x58
    APSME_UNBIND_confirm,               //0x59

    ZDO_DATA_indication,                //0x5a
    ZDO_BIND_req,                       //0x5b
    ZDO_UNBIND_req,                     //0x5c
    ZDO_END_DEVICE_BIND_req,            //0x5d


    APSME_ESTABLISH_KEY_request,
    APSME_ESTABLISH_KEY_confirm,
    APSME_ESTABLISH_KEY_indication,
    APSME_ESTABLISH_KEY_response,
    APSME_TRANSPORT_KEY_request,
    APSME_TRANSPORT_KEY_indication,
    APSME_UPDATE_DEVICE_request,
    APSME_UPDATE_DEVICE_indication,
    APSME_REMOVE_DEVICE_request,
    APSME_REMOVE_DEVICE_indication,
    APSME_REQUEST_KEY_request,
    APSME_REQUEST_KEY_indication,
    APSME_SWITCH_KEY_request,
    APSME_SWITCH_KEY_indication,
    
    APSME_ADD_GROUP_request,
    APSME_ADD_GROUP_confirm,
    APSME_REMOVE_GROUP_request,
    APSME_REMOVE_GROUP_confirm,
    APSME_REMOVE_ALL_GROUPS_request,
    APSME_REMOVE_ALL_GROUPS_confirm,
    
    NLME_ROUTE_DISCOVERY_request,
    NLME_ROUTE_DISCOVERY_confirm

} ZIGBEE_PRIMITIVE;


typedef struct _ZIGBEE_STATUS
{
    ZIGBEE_PRIMITIVE    nextZigBeeState;
    union _ZIGBEE_STATUS_FLAGS
    {
        BYTE    Val;
        struct _ZIGBEE_STATUS_BITS
        {
            BYTE        bTxFIFOInUse            : 1;
            BYTE        bRxBufferOverflow       : 1;
            //BYTE        bRxHardwareOverflow     : 1; not used
            BYTE        bHasBackgroundTasks     : 1;
            #ifdef I_AM_COORDINATOR
                BYTE    bNetworkFormed          : 1;
                BYTE    bTryingToFormNetwork    : 1;
            #else
                BYTE    bNetworkJoined          : 1;
                BYTE    bTryingToJoinNetwork    : 1;
                BYTE    bTryOrphanJoin          : 1;
            #endif
            #ifdef I_AM_RFD
                BYTE    bRequestingData         : 1;
                BYTE    bDataRequestComplete    : 1;
                BYTE    bRadioIsSleeping        : 1;    
            #endif
        } bits;
    } flags;
} ZIGBEE_STATUS;

extern BYTE*            CurrentRxPacket;
//extern SHORT_ADDR       macShortAddr;
extern PARAMS           params;
//extern PENDING_TASKS    pendingTasks;
//extern PAN_ADDR         macPANAddr;
extern BYTE             TxBuffer[];
extern BYTE             TxData;
extern BYTE             TxHeader;
//extern FIFO_CONTROL     TxFIFOControl;
//extern FIFO_CONTROL     RxFIFOControl;
extern volatile ZIGBEE_STATUS    ZigBeeStatus;


//#define ZigBeeBlockTx() TxFIFOControl.bFIFOInUse = 1
#define ZigBeeBlockTx()     ZigBeeStatus.flags.bits.bTxFIFOInUse = 1
#define ZigBeeUnblockTx()   {                                               \
                                ZigBeeStatus.flags.bits.bTxFIFOInUse = 0;   \
                                TxHeader = TX_HEADER_START;                 \
                                TxData = TX_DATA_START;                     \
                            }
#define ZigBeeTxBlocked     ZigBeeStatus.flags.bits.bTxFIFOInUse
#define ZigBeeTxUnblocked   !ZigBeeStatus.flags.bits.bTxFIFOInUse
void    ZigBeeInit( void );
//#define ZigBeeReady()   !TxFIFOControl.bFIFOInUse
#define ZigBeeReady()       !ZigBeeStatus.flags.bits.bTxFIFOInUse
BOOL    ZigBeeTasks( ZIGBEE_PRIMITIVE * );

#if defined(AODV_POWER_CONTROL)			
extern ROM unsigned char CharacterArray[];
float convertIndexPtoValueP(BYTE p);
 BYTE convertValuePtoIndexP(float a);
BYTE PA_LEVEL_CONTROL(BYTE pt, BYTE lqi, BYTE lqi_threshold, BYTE offset);
//float getmetrics(BYTE *pathcost,BYTE remainEnergy,BYTE Ptsent,BYTE Ptdes, BYTE LQIhear, BYTE gama);//Anh Cuong52
float getmetrics(BYTE *pathcost,BYTE Ptsent,BYTE Ptdes, BYTE LQIhear, BYTE gama);//Son53
void Pack(float f,BYTE *p);
float unPack(BYTE* p);
void printfloat(float f);
#endif
#endif
