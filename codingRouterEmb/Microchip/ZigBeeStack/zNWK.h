/*********************************************************************
 *
 *                  ZigBee NWK Header File
 *
 *********************************************************************
 * FileName:        zNWK.h
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
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#ifndef _zNWK_H_
#define _zNWK_H_

#include "ZigbeeTasks.h"

// ******************************************************************************
// NWK Layer Spec Constants

// A Boolean flag indicating whether the device is capable of becoming the
// ZigBee coordinator.
#ifdef I_HAVE_COORDINATOR_CAPABILITY
    #define nwkcCoordinatorCapable  0x01
#else
    #define nwkcCoordinatorCapable  0x00
#endif

// The default security level to be used.
#define nwkcDefaultSecurityLevel  ENC-MIC-64

// The maximum number of times a route discovery will be retried.
#define nwkcDiscoveryRetryLimit     0x03

// The maximum depth (minimum number of logical hops from the ZigBee
// coordinator) a device can have.
#define nwkcMaxDepth                0x0f

// The maximum number of octets added by the NWK layer to its payload without
// security. If security is required on a frame, its secure processing may inflate
// the frame length so that it is greater than this value.
#define nwkcMaxFrameOverhead        0x0d

// The maximum number of octets that can be transmitted in the NWK frame payload field.
#define nwkcMaxPayloadSize          (aMaxMACFrameSize – nwkcMaxFrameOverhead)

// The version of the ZigBee NWK protocol in the device.
#define nwkcProtocolVersion         0x02

// Maximum number of allowed communication errors after which the route repair mechanism is initiated.
#define nwkcRepairThreshold         0x03

// Time duration in milliseconds until a route discovery expires.
#define nwkcRouteDiscoveryTime      0x2710

// The maximum broadcast jitter time measured in milliseconds.
#define nwkcMaxBroadcastJitter      0x40

// The number of times the first broadcast transmission of a route request
// command frame is retried.	 
#if defined(AODV_ORIGINAL)
#define nwkcInitialRREQRetries      0x03
#endif
#if defined(AODV_POWER_CONTROL)
#define nwkcInitialRREQRetries      0x00//
#endif

// The number of times the broadcast transmission of a route request command frame is retried on
// relay by an intermediate ZigBee router or ZigBee coordinator.
#define nwkcRREQRetries             0x02

// The number of milliseconds between retries of a broadcast route request command frame.
#define nwkcRREQRetryInterval       0xfe

// The minimum jitter, in 2 millisecond slots, for broadcast retransmission of a route
// request command frame.
#define nwkcMinRREQJitter           0x01

// The maximum jitter, in 2 millisecond slots, for broadcast retransmission of a
// route request command frame.
#define nwkcMaxRREQJitter           0x40

#define nwkMAGICResSeq		0x5A

// ******************************************************************************
// Constants and Enumerations

#define DEFAULT_RADIUS                  (2*PROFILE_nwkMaxDepth)
#define INVALID_NWK_HANDLE              0x00
#define INVALID_NEIGHBOR_KEY            (NEIGHBOR_KEY)(MAX_NEIGHBORS)
#define ROUTE_DISCOVERY_ENABLE          0x01
#define ROUTE_DISCOVERY_FORCE           0x02
#define ROUTE_DISCOVERY_SUPPRESS        0x00


typedef enum _LEAVE_REASONS
{
    COORDINATOR_FORCED_LEAVE        = 0x01,
    SELF_INITIATED_LEAVE            = 0x02
} LEAVE_REASONS;


typedef enum _NWK_STATUS_VALUES
{
    NWK_SUCCESS                     = 0x00,
    NWK_INVALID_PARAMETER           = 0xC1,
    NWK_INVALID_REQUEST             = 0xC2,
    NWK_NOT_PERMITTED               = 0xC3,
    NWK_STARTUP_FAILURE             = 0xC4,
    NWK_ALREADY_PRESENT             = 0xC5,
    NWK_SYNC_FAILURE                = 0xC6,
    NWK_TABLE_FULL                  = 0xC7,
    NWK_UNKNOWN_DEVICE              = 0xC8,
    NWK_UNSUPPORTED_ATTRIBUTE       = 0xC9,
    NWK_NO_NETWORKS                 = 0xCA,
    NWK_LEAVE_UNCONFIRMED           = 0xCB,
    NWK_MAX_FRM_CNTR                = 0xCC, // Security failed - frame counter reached maximum
    NWK_NO_KEY                      = 0xCD, // Security failed - no key
    NWK_BAD_CCM_OUTPUT              = 0xCE, // Security failed - bad output
    NWK_CANNOT_ROUTE                = 0xCF,
    NWK_ROUTE_ERROR                 = 0xD1
} NWK_STATUS_VALUES;


typedef enum _RELATIONSHIP_TYPE
{
    NEIGHBOR_IS_PARENT  = 0,
    NEIGHBOR_IS_CHILD,
    NEIGHBOR_IS_SIBLING,
    NEIGHBOR_IS_NONE
} RELATIONSHIP_TYPE;


typedef enum _ROUTE_STATUS
{
    ROUTE_ACTIVE                    = 0x00,
    ROUTE_DISCOVERY_UNDERWAY        = 0x01,
    ROUTE_DISCOVERY_FAILED          = 0x02,
    ROUTE_INACTIVE                  = 0x03
} ROUTE_STATUS;


// ******************************************************************************
// Structures

typedef struct __Config_NWK_Mode_and_Params
{
     BYTE   ProtocolVersion;
     BYTE   StackProfile;
     BYTE   BeaconOrder;
     BYTE   SuperframeOrder;
     BYTE   BatteryLifeExtension;
     BYTE   SecuritySetting;
     DWORD  Channels;
} _Config_NWK_Mode_and_Params;


typedef BYTE NEIGHBOR_KEY;


typedef union _NEIGHBOR_RECORD_DEVICE_INFO
{
    struct
    {
        BYTE LQI                : 8;
        BYTE Depth              : 4;
        BYTE StackProfile       : 4;    // Needed for network discovery
        BYTE ZigBeeVersion      : 4;    // Needed for network discovery
        BYTE deviceType         : 2;
        BYTE Relationship       : 2;
        BYTE RxOnWhenIdle       : 1;
        BYTE bInUse             : 1;
        BYTE PermitJoining      : 1;
        BYTE PotentialParent    : 1;
    } bits;
    DWORD Val;
} NEIGHBOR_RECORD_DEVICE_INFO;


typedef struct _NEIGHBOR_RECORD
{
    LONG_ADDR                   longAddr;
    SHORT_ADDR                  shortAddr;
    PAN_ADDR                    panID;
    NEIGHBOR_RECORD_DEVICE_INFO deviceInfo;
    BYTE                        LogicalChannel; // Needed to add for NLME_JOIN_request and other things.
    #ifdef I_SUPPORT_SECURITY
    BOOL						bSecured;
    #endif
} NEIGHBOR_RECORD;  // 15 bytes long


typedef struct _NEIGHBOR_TABLE_INFO
{
    WORD        validityKey;
    BYTE        neighborTableSize;

#ifndef I_AM_COORDINATOR
    BYTE        parentNeighborTableIndex;
#endif

#ifndef I_AM_END_DEVICE
    BYTE        depth;              // Our depth in the network
    SHORT_ADDR  cSkip;              // Address block size
    SHORT_ADDR  nextEndDeviceAddr;  // Next address available to give to an end device
    SHORT_ADDR  nextRouterAddr;     // Next address available to give to a router
    BYTE        numChildren;        // How many children we have
    BYTE        numChildRouters;    // How many of our children are routers
    union _flags
    {
        BYTE    Val;
        struct _bits
        {
            BYTE    bChildAddressInfoValid : 1;  // Child addressing information is valid
        } bits;
    }flags;
#endif
} NEIGHBOR_TABLE_INFO;


typedef struct _ROUTING_ENTRY
{
    SHORT_ADDR      destAddress;
    ROUTE_STATUS    status;     // 3 bits valid
    SHORT_ADDR      nextHop;	 
	#if defined(AODV_POWER_CONTROL)
	BYTE            realPt;    //@cuong cho them dieu khien cong suat
	#endif
} ROUTING_ENTRY;

typedef struct _ROUTE_DST_INFO
{
    SHORT_ADDR dstAddr;
    BYTE counter;
} ROUTE_DST_INFO;
#define ROUTE_DST_INFO_SIZE 5

#ifdef I_SUPPORT_SECURITY
typedef struct _NETWORK_KEY_INFO
{
	KEY_VAL		NetKey;
	WORD_VAL	SeqNumber;
} NETWORK_KEY_INFO;
#endif

typedef struct _CHANNEL_INFO
{
    BYTE    channel;
    BYTE    energy;
    BYTE    networks;
} CHANNEL_INFO;

typedef struct _DISCOVERY_INFO
{
    BYTE            currentIndex;
    BYTE            numChannels;
    CHANNEL_INFO    *channelList;
} DISCOVERY_INFO;


typedef union _NWK_FRAME_CONTROL_LSB
{
    BYTE    Val;
    struct _NWK_FRAME_CONTROL_LSB_bits
    {
        BYTE    frameType       : 2;
        BYTE    protocolVersion : 4;
        BYTE    routeDiscovery  : 2;
    } bits;
} NWK_FRAME_CONTROL_LSB;


typedef union _NWK_FRAME_CONTROL_MSB
{
    BYTE    Val;
    struct _NWK_FRAME_CONTROL_MSB_bits
    {
        BYTE    multicastFlag   : 1;
        BYTE    security        : 1;
        BYTE    sourceRoute     : 1;
        BYTE    dstIEEEAddr     : 1;
        BYTE    srcIEEEAddr     : 1;
        BYTE    reserved        : 3;
    } bits;
} NWK_FRAME_CONTROL_MSB;


typedef struct _BTR     // Broadcast Transaction Record
{
    BYTE                    *dataPtr;
    BYTE                    dataLength;
    BYTE                    nwkSequenceNumber;
    BYTE                    nwkRadius;
    SHORT_ADDR              nwkSourceAddress;
    SHORT_ADDR              nwkDestinationAddress;
    NWK_FRAME_CONTROL_MSB   nwkFrameControlMSB;
    NWK_FRAME_CONTROL_LSB   nwkFrameControlLSB;
    BYTE                    currentNeighbor;
    TICK                    broadcastJitterTimer;
    TICK                    broadcastTime;
    struct _BTR_flags1
    {
        BYTE    nRetries                : 4;
        BYTE    bMessageFromUpperLayers : 1;
        BYTE    bConfirmSent            : 1;
    #ifdef I_SUPPORT_SECURITY
        BYTE    bAlreadySecured         : 1;
    #endif
    } btrInfo;

    union _BTR_flags2
    {
        // TODO Since we are down to one flag per neighbor, would it be worth
        // bitmapping these?  Maybe not - a lot of overhead to calculate mask and index,
        // and this is only alloc'd.
        BYTE    byte;
        struct BTR__bits
        {
            BYTE    bMessageNotRelayed      : 1;
        } bits;
    } flags[MAX_NEIGHBORS];
} BTR;
#define BTR_FLAGS_INIT  0x03


typedef struct _BUFFERED_MESSAGE_INFO
{
    BYTE        *dataPtr;
    BYTE        dataLength;
    SHORT_ADDR  sourceAddress;
    SHORT_ADDR  destinationAddress;
    TICK        timeStamp;
} BUFFERED_MESSAGE_INFO;

#ifdef I_SUPPORT_ROUTING
    typedef struct _ROUTE_HANDLE_RECORD
    {
        BYTE        nwkSequence;
        BYTE        macSequence;
        SHORT_ADDR  SourceAddress;
    } ROUTE_HANDLE_RECORD;
#endif

typedef struct _NWK_STATUS
{
    union _NWK_STATUS_flags
    {
        DWORD    Val;
        struct _NWK_STATUS_bits
        {
            // Background Task Flags
            BYTE    bSendingBroadcastMessage    : 1;
            BYTE    bAwaitingRouteDiscovery     : 1;
            BYTE    bTimingJoinPermitDuration   : 1;
            BYTE    bLeavingTheNetwork          : 1;
            BYTE    bLeaveDisassociate          : 1;
            BYTE    bLeaveWaitForConfirm        : 1;
            BYTE    bLeaveReset                 : 1;
            BYTE    bRejoinIndication           : 1;

            // Status Flags
            BYTE    bCanRoute                   : 1;
            BYTE    bRemovingChildren           : 1;    // Not used by end device
            BYTE    bAllChildrenLeft            : 1;    // Not used by end device
            BYTE    bAllEndDeviceAddressesUsed  : 1;
            BYTE    bAllRouterAddressesUsed     : 1;
            BYTE    bNRejoin                    : 1;    // added for 2006
            BYTE    bRejoinScan                 : 1;    // added for 2006
            BYTE    bRejoinInProgress           : 1;    // added for 2006
            
            #ifdef I_SUPPORT_FREQUENCY_AGILITY
                BYTE    bScanRequestFromZDO         : 1;    // added for PRO
            #endif
        } bits;
    } flags;



    #ifndef I_AM_RFD
        BTR                     *BTT[NUM_BUFFERED_BROADCAST_MESSAGES];  // Broadcast Transaction Table
    #endif
    #if defined( I_AM_COORDINATOR ) || defined( I_AM_ROUTER )
        TICK                    joinDurationStart;
        BYTE                    joinPermitDuration;
    #endif
    #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
        BYTE                    routeRequestID;
        BUFFERED_MESSAGE_INFO   *routingMessages[NUM_BUFFERED_ROUTING_MESSAGES];
        ROUTE_HANDLE_RECORD     routeHandleRecords[NUM_BUFFERED_ROUTING_MESSAGES];
    #endif

    DISCOVERY_INFO              discoveryInfo;
    ASSOCIATE_CAPABILITY_INFO   lastCapabilityInformation;
    DWORD_VAL                   lastScanChannels;
    BYTE                        lastScanDuration;
    #ifndef I_AM_END_DEVICE
        BYTE                    leaveCurrentNode;
        TICK                    leaveStartTime;
    #endif
    BYTE                        leaveReason;
    #ifdef I_AM_COORDINATOR
        PAN_ADDR                requestedPANId;
    #else
        LONG_ADDR               rejoinExtendedPANId;
        TICK                    rejoinStartTick;
    #endif
    #ifdef I_AM_RFD
        BYTE                    rejoinCommandSent;
    #endif
} NWK_STATUS;
#define NWK_BACKGROUND_TASKS    0x007F



// ******************************************************************************
// Macro Definitions

#define NWKDiscardRx() MACDiscardRx()  
#if defined(AODV_POWER_CONTROL)
extern BYTE energy_level;	  
#endif
// ******************************************************************************
// Function Prototypes

void                NWKClearNeighborTable( void );
void                NWKClearRoutingTable( void );
NEIGHBOR_KEY        NWKLookupNodeByLongAddr( LONG_ADDR *longAddr );
BOOL                NWKHasBackgroundTasks( void );
void                NWKTidyNeighborTable( void );
void                NWKInit( void );
ZIGBEE_PRIMITIVE    NWKTasks( ZIGBEE_PRIMITIVE inputPrimitive );
BOOL                NWKThisIsMyLongAddress( LONG_ADDR *address );
BYTE                NLME_GET_nwkBCSN( void );
BOOL 				APSFromShortToLong(INPUT SHORT_ADDR *ShortAddr);
BOOL				APSFromLongToShort(INPUT LONG_ADDR *LongAddr);
#endif
