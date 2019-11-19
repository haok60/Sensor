/*********************************************************************
 *
 *                  ZigBee APS Layer
 *
 *********************************************************************
 * FileName:        zAPS.c
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
 * DPL                  08/01/08 Microchip ZigBee-2006 Stack v2.0-2.6
 ********************************************************************/

#include "ZigBeeTasks.h"
#include "sralloc.h"
#include "zAPS.h"
#include "zAPL.h"
#include "zNWK.h"
#include "zMAC.h"
#include "zNVM.h"
#include "generic.h"
//#ifdef ZCP_DEBUG
    #include "console.h"
//#else
//    #define ConsolePutROMString(x)
//    #undef printf
//    #define printf(x)
//    #define PrintChar(x)
//#endif

#ifdef I_SUPPORT_SECURITY
	#include "zSecurity.h"
	#include "zPHY_MRF24J40.h"
#endif

// ******************************************************************************
// Configuration Definitions and Error Checks

#ifdef I_AM_RFD
    #if !defined(RFD_POLL_RATE)
        #error Please use ZENA(TM) to define the internal stack message poll rate.
    #endif
#endif


// ******************************************************************************
// Constant Definitions

//-----------------------------------------------------------------------------
// Frame Control Bits
#define APS_FRAME_DATA              0x00
#define APS_FRAME_COMMAND           0x01
#define APS_FRAME_ACKNOWLEDGE       0x02

#define APS_DELIVERY_DIRECT         0x00
#define APS_DELIVERY_INDIRECT       0x01
#define APS_DELIVERY_BROADCAST      0x02
#define APS_DELIVERY_GROUP          0x03

#define APS_INDIRECT_ADDRESS_MODE_TO_COORD      0x01
#define APS_INDIRECT_ADDRESS_MODE_FROM_COORD    0x00

#define APS_SECURITY_ON             0x01
#define APS_SECURITY_OFF            0x00

#define APS_ACK_REQUESTED           0x01
#define APS_ACK_NOT_REQUESTED       0x00
//-----------------------------------------------------------------------------


// This value marks whether or not the binding table contains valid
// values left over from before a loss of power.
#define BINDING_TABLE_VALID        0xC35A

#define END_BINDING_RECORDS         0xff

// ******************************************************************************
// Data Structures

#if NUM_BUFFERED_INDIRECT_MESSAGES > 14
    #error Maximum buffered indirect messages is 14.
#endif
#define INVALID_INDIRECT_RELAY_INDEX      15

#if MAX_APL_FRAMES > 14
    #error Maximum APL messages is 14.
#endif
#define INVALID_APL_INDEX      15

#if apscMaxFrameRetries > 7
    #error apscMaxFrameRetries too large.
#endif

typedef struct _INDIRECT_MESSAGE_INFO
{
    WORD_VAL            profileID;
    BYTE                *message;
    APS_FRAME_CONTROL   apsFrameControl;
    BYTE                messageLength;
    BYTE                currentBinding;
    BYTE                sourceEndpoint;     // Used only for messages from upper layers
    union
    {
        BYTE    Val;
        struct
        {
            BYTE    nTransmitAttempts   : 3;
            BYTE    bSendMessage        : 1;
            BYTE    bFromMe             : 1;
            BYTE    bRouteRepair        : 1;
        } bits;
    } flags;
} INDIRECT_MESSAGE_INFO;


typedef struct _APS_STATUS
{
    union _APS_STATUS_flags
    {
        BYTE    Val;
        struct _APS_STATUS_bits
        {
            // Background Task Flags
            BYTE    bSendingIndirectMessage     : 1;
            BYTE    bFramesPendingConfirm       : 1;
            BYTE    bDataIndicationPending      : 1;
            BYTE    bFramesAwaitingTransmission : 1;
            BYTE    bGroupAddressing            : 1;
            BYTE    bDuplicateTable             : 1;
            BYTE    bBroadcastingNetworkKey     : 1;        /* Zigbee 2006 */
            BYTE    bBroadcastingSwitchKey      : 1;
            
            // Status Flags
        } bits;
    } flags;

    #if defined(__C30__)
        BYTE filler;
    #endif
    
    // If we receive an indirect message and must relay it, or an indirect transmission
    // is sent down from the upper layers, we must buffer it for multiple transmissions
    // from the background.
    #if defined (I_SUPPORT_BINDINGS)
//        INDIRECT_MESSAGE_INFO   *indirectMessages[NUM_BUFFERED_INDIRECT_MESSAGES];
    #endif

    #if defined(I_SUPPORT_GROUP_ADDRESSING)
        APS_GROUP_RECORD    *apsGroupRecord[MAX_GROUP_RECORD_BUFFER];
    #endif

    // All messages that come in from the APL level are actually sent in the background,
    // because they must be buffered to allow for multiple retries.
    APL_FRAME_INFO  *aplMessages[MAX_APL_FRAMES];

    // If we get a message that requires an APS ACK, we must buffer the message, send the ACK,
    // and then return the APSDE_DATA_indication in the background, using the buffered message info.
    struct _ACK_MESSAGE
    {
        BYTE        asduLength;
        BYTE        SecurityStatus;
        BYTE *      asdu;
        WORD_VAL    ProfileId;
        BYTE        SrcAddrMode;
        BYTE        WasBroadcast;
        SHORT_ADDR  SrcAddress;         // NOTE: This is ADDR in params, but we cannot get a long address from the NWK layer
        BYTE        SrcEndpoint;
        BYTE        DstEndpoint;
        WORD_VAL    ClusterId;
        SHORT_ADDR  GroupId;
    } ackMessage;

} APS_STATUS;
#define APS_BACKGROUND_TASKS 0x1F


typedef struct _APS_DUPLICATE_TABLE
{
    TICK        StartTick;
    SHORT_ADDR  SrcAddress;
    BYTE        APSCounter;
    BYTE        filler;
}APS_DUPLICATE_TABLE;


// ******************************************************************************
// Variable Definitions

BYTE            APSCounter;


DWORD_VAL   apsChannelMask;


APS_STATUS      apsStatus;
APS_FRAMES      *apsConfirmationHandles[MAX_APS_FRAMES];
#if !defined(__C30__)
    #pragma udata DUP_TABLE=0xc00
#endif    
        APS_DUPLICATE_TABLE apsDuplicateTable[MAX_DUPLICATE_TABLE];
#if !defined(__C30__)        
    #pragma udata
#endif

#ifdef I_SUPPORT_SECURITY
    KEY_VAL     KeyVal;
	extern SECURITY_STATUS	securityStatus;
    TICK        AuthorizationTimeout;
    #ifdef I_AM_RFD
        extern volatile PHY_PENDING_TASKS  PHYTasksPending;
        TICK    lastPollTime;
    #endif
#endif
extern volatile TX_STAT TxStat;

// ******************************************************************************
// Function Prototypes

#if MAX_APS_ADDRESSES > 0
    BOOL LookupAPSAddress( LONG_ADDR *longAddr );
    BOOL LookupAPSLongAddress(INPUT SHORT_ADDR *shortAddr);
    BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap);
#endif
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
    BYTE LookupSourceBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID );
    void RemoveAllBindings( SHORT_ADDR shortAddr );
#endif
#if defined(I_SUPPORT_GROUP_ADDRESSING)
    GROUP_ADDRESS_RECORD currentGroupAddressRecord;
    BYTE    GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr);
    BYTE    GetEmptyGroup(void);
    BYTE    AddGroup(INPUT SHORT_ADDR GroupAddress, INPUT BYTE EndPoint);
    BYTE    RemoveGroup(INPUT WORD GroupAddress, INPUT BYTE EndPoint);
#endif
#ifdef I_SUPPORT_SECURITY
    BOOL    APSFillSecurityRequest(INPUT LONG_ADDR *DestAddr, INPUT BOOL bSecureFrame);
#endif
BOOL    DuplicatePacket(INPUT SHORT_ADDR SrcAddress, INPUT BYTE currentAPSCounter);
extern BOOL NWKThisIsMyLongAddress(LONG_ADDR *);
extern NEIGHBOR_KEY NWKLookupNodeByShortAddrVal( WORD shortAddrVal );

extern ZDO_STATUS zdoStatus;
BYTE    SendingEDBRequest  = 0;
BYTE    SentBindRequest = 0;

#ifdef I_SUPPORT_SECURITY
    BOOL    firstKeyHasBeenSent = FALSE;
    BOOL    previousKeyNotSeq0;
    
    #if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER)
        APS_ADDRESS_MAP updateDevAPSAddr;
    #endif 
    #ifdef I_AM_ROUTER
        BOOL        waitingForKey = FALSE;
    #endif    
    #if defined(USE_EXTERNAL_NVM)
	    extern NETWORK_KEY_INFO plainSecurityKey[2];
	    extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
	    extern BOOL InitSecurityKey(void);
	#endif
#endif

#ifdef I_AM_COORDINATOR
    APS_ADDRESS_MAP currentAPSAddress1;
#endif

/*********************************************************************
 * Function:        BOOL APSHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - APS layer has background tasks to run
 *                  FALSE - APS layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the APS layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BOOL APSHasBackgroundTasks( void )
{
    return ((apsStatus.flags.Val & APS_BACKGROUND_TASKS) != 0);
}


/*********************************************************************
 * Function:        void APSInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    APS layer data structures are initialized.
 *
 * Overview:        This routine initializes all APS layer data
 *                  structures.
 *
 * Note:            This routine is intended to be called as part of
 *                  a network or power-up initialization.  If called
 *                  after the network has been running, heap space
 *                  may be lost unless the heap is also reinitialized.
 ********************************************************************/

void APSInit( void )
{
    BYTE        i;
    #if defined(I_SUPPORT_BINDINGS)
        WORD    key;
    #endif

    apsStatus.flags.Val = 0;

    // Initialize APS frame handles awaiting confirmation.
    for (i=0; i<MAX_APS_FRAMES; i++)
    {
        apsConfirmationHandles[i] = NULL;
    }

    // Initialize the buffered APL message pointers.
    for (i=0; i<MAX_APL_FRAMES; i++)
    {
        apsStatus.aplMessages[i] = NULL;
    }
    
    // Initialize the buffered Group pointers
    #ifdef I_SUPPORT_GROUP_ADDRESSING
        for( i = 0; i < MAX_GROUP_RECORD_BUFFER; i++)
        {
            apsStatus.apsGroupRecord[i] = NULL;
        }
    #endif
    
    // Initialize the duplicate table
    for(i = 0; i < MAX_DUPLICATE_TABLE; i++)
    {
        apsDuplicateTable[i].SrcAddress.Val = 0xFFFF;
    }
    
    #if defined(I_SUPPORT_BINDINGS)

        GetBindingValidityKey( &key );
        if (key != BINDING_TABLE_VALID)
        {
            key = BINDING_TABLE_VALID;
            PutBindingValidityKey( &key );
            ClearBindingTable();
        }
    #endif
    #if MAX_APS_ADDRESSES > 0
    {
        WORD ValidKey;
        GetAPSAddressValidityKey(&ValidKey);
        /* Otherwise this causes the chain of device Update key to fail if depth > 1 */
        {
            ValidKey = apsMAGICValid;
            PutAPSAddressValidityKey(&ValidKey);
            currentAPSAddress.shortAddr.v[0] = 0xFF;
            currentAPSAddress.shortAddr.v[1] = 0xFF;
            for(i = 0; i < MAX_APS_ADDRESSES; i++)
            {
                #ifdef USE_EXTERNAL_NVM
                    PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress );
                #else
                    PutAPSAddress( &apsAddressMap[i], &currentAPSAddress );
                #endif
            }
        }
    }
    #endif
    APSCounter = 0;
	#ifdef I_SUPPORT_SECURITY
    	#if !defined(I_AM_COORDINATOR)
        	securityStatus.flags.bits.bAuthorization = FALSE;
    	#endif
	#endif
	
	apsChannelMask.Val = ALLOWED_CHANNELS;
	
	
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE APSTasks(ZIGBEE_PRIMITIVE inputPrimitive)
 *
 * PreCondition:    None
 *
 * Input:           inputPrimitive - the next primitive to run
 *
 * Output:          The next primitive to run.
 *
 * Side Effects:    Numerous
 *
 * Overview:        This routine executes the indicated ZigBee primitive.
 *                  If no primitive is specified, then background
 *                  tasks are executed.
 *
 * Note:            If this routine is called with NO_PRIMITIVE, it is
 *                  assumed that the TX and RX paths are not blocked,
 *                  and the background tasks may initiate a transmission.
 *                  It is the responsibility of this task to ensure that
 *                  only one output primitive is generated by any path.
 *                  If multiple output primitives are generated, they
 *                  must be generated one at a time by background processing.
 ********************************************************************/

ZIGBEE_PRIMITIVE APSTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    BYTE    i;
    BYTE    j;
    BYTE    *ptr;
    
    #ifdef I_SUPPORT_SECURITY
        LONG_ADDR       transportKeySrcAddr;
    #endif
    
    if (inputPrimitive == NO_PRIMITIVE)
    {
        // Perform background tasks
        TICK currentTime = TickGet();
        
     	// Handle responder authorization
		#if defined(I_SUPPORT_SECURITY) && !defined(I_AM_COORDINATOR)
        	if (securityStatus.flags.bits.bAuthorization)
        	{
            	//if( TickGetDiff( currentTime, AuthorizationTimeout) > AUTHORIZATION_TIMEOUT)
            	if( TickGetDiff( currentTime, AuthorizationTimeout) > AUTHORIZATION_TIMEOUT * 5)
            	{
                	ConsolePutROMString((ROM char *)"Authorization timeout\r\n");
                	/* Added at NTS - reset this bAuthorization flag in the NLME_LEAVE primitive */
                	//securityStatus.flags.bits.bAuthorization = 0;
        			#ifdef I_AM_RFD
                		PHYTasksPending.bits.PHY_AUTHORIZE = 0;
        			#endif
                	//GetMACAddress(&(params.NLME_LEAVE_request.DeviceAddress));
                	for(i=0; i < 8; i++)
                	{
                    	/* force a self initiated leave */
                        params.NLME_LEAVE_request.DeviceAddress.v[i] = 0x00;
                    }	
                	params.NLME_LEAVE_request.RemoveChildren = TRUE;
                	params.NLME_LEAVE_request.Rejoin         = FALSE;
                	params.NLME_LEAVE_request.Silent         = TRUE;
                	params.NLME_LEAVE_request.ReuseAddress   = FALSE;
                	return NLME_LEAVE_request;
            	}
        		#ifdef I_AM_RFD
            		if (TickGetDiff( currentTime, lastPollTime ) > (DWORD)RFD_POLL_RATE)
            		{
                		// Send a data request message so we can try to receive our Key
                		lastPollTime = currentTime;
                		params.NLME_SYNC_request.Track = FALSE;
                		return NLME_SYNC_request;
            		}
        		#endif
        	}
		#endif
        // ---------------------------------------------------------------------
        // Handle pending data indication from an ACK request
        if (apsStatus.flags.bits.bDataIndicationPending)
        {
            // If we have't finished the last transmission, don't start processing the message.
            // Otherwise, we'll never see the MAC ACK for the APS ACK
            if (ZigBeeReady())
            {
                apsStatus.flags.bits.bDataIndicationPending = 0;
                #ifdef I_AM_RFD
                    ZigBeeStatus.flags.bits.bRequestingData = 0;
                #endif

                params.APSDE_DATA_indication.asduLength             = apsStatus.ackMessage.asduLength;
                params.APSDE_DATA_indication.asdu                   = apsStatus.ackMessage.asdu;
                params.APSDE_DATA_indication.ProfileId              = apsStatus.ackMessage.ProfileId;
                params.APSDE_DATA_indication.SrcAddrMode            = apsStatus.ackMessage.SrcAddrMode;
                params.APSDE_DATA_indication.WasBroadcast           = apsStatus.ackMessage.WasBroadcast;
                params.APSDE_DATA_indication.SrcAddress.ShortAddr   = apsStatus.ackMessage.SrcAddress;
                params.APSDE_DATA_indication.SrcEndpoint            = apsStatus.ackMessage.SrcEndpoint;
                params.APSDE_DATA_indication.DstEndpoint            = apsStatus.ackMessage.DstEndpoint;
                params.APSDE_DATA_indication.ClusterId.Val          = apsStatus.ackMessage.ClusterId.Val;
                if (params.APSDE_DATA_indication.DstEndpoint == 0)
                {
                    return ZDO_DATA_indication;
                }
                else
                {
                    return APSDE_DATA_indication;
                }
            }
        }

        // --------------------------------------------------------------------
        // Handle Group addressing
        #ifdef I_SUPPORT_GROUP_ADDRESSING
            if( apsStatus.flags.bits.bGroupAddressing)
            {
                for(i = 0; i < MAX_GROUP_RECORD_BUFFER; i++)
                {
                    if( apsStatus.apsGroupRecord[i] != NULL )
                    {
                        if( apsStatus.apsGroupRecord[i]->EndPoints[apsStatus.apsGroupRecord[i]->EndPointIndex] == 0xFF ) 
                        {
                            // this is the end of the group address map
                            nfree(apsStatus.apsGroupRecord[i]->CurrentRxPacket);
                            nfree(apsStatus.apsGroupRecord[i]);
                        }
                        else 
                        {
                            params.APSDE_DATA_indication.DstEndpoint        = apsStatus.apsGroupRecord[i]->EndPoints[apsStatus.apsGroupRecord[i]->EndPointIndex++];
                            params.APSDE_DATA_indication.SrcAddrMode        = apsStatus.apsGroupRecord[i]->SrcAddrMode;
                            params.APSDE_DATA_indication.SrcAddress         = apsStatus.apsGroupRecord[i]->SrcAddress;
                            params.APSDE_DATA_indication.SrcEndpoint        = apsStatus.apsGroupRecord[i]->SrcEndpoint;
                            params.APSDE_DATA_indication.ProfileId.Val      = apsStatus.apsGroupRecord[i]->ProfileId.Val;
                            params.APSDE_DATA_indication.ClusterId.Val      = apsStatus.apsGroupRecord[i]->ClusterId.Val;
                            params.APSDE_DATA_indication.asduLength         = apsStatus.apsGroupRecord[i]->asduLength;
                            params.APSDE_DATA_indication.asdu               = apsStatus.apsGroupRecord[i]->asdu;
                            params.APSDE_DATA_indication.WasBroadcast       = TRUE;
                            params.APSDE_DATA_indication.SecurityStatus     = apsStatus.apsGroupRecord[i]->SecurityStatus;
                            params.APSDE_DATA_indication.DstAddrMode        = APS_ADDRESS_GROUP;
                            CurrentRxPacket                                 = NULL;
                            return APSDE_DATA_indication;
                        }
                    }
                }
            
                apsStatus.flags.bits.bGroupAddressing = 0;
            }
        #endif

        // Handle duplicate table expiration
        if (apsStatus.flags.bits.bDuplicateTable)
        {
            BOOL isValid = FALSE;

            for(i = 0; i < MAX_DUPLICATE_TABLE; i++)   
            {
                if( apsDuplicateTable[i].SrcAddress.Val != 0xFFFF )
                {
                    if( TickGetDiff(currentTime, apsDuplicateTable[i].StartTick) > ((ONE_SECOND) * ((DWORD)DUPLICATE_TABLE_EXPIRATION)))
                    {
                        apsDuplicateTable[i].SrcAddress.Val = 0xFFFF;
                    }
                    else
                    {
                        isValid = TRUE;
                    }
                }   
            }
            if( isValid == FALSE )
            {
                apsStatus.flags.bits.bDuplicateTable = 0;
            }
        }

        // ---------------------------------------------------------------------
        // Handle frames awaiting sending (or resending) from upper layers
        if (apsStatus.flags.bits.bFramesAwaitingTransmission)
        {
            #ifdef I_AM_RFD
            if (ZigBeeReady() && ZigBeeStatus.flags.bits.bDataRequestComplete)
            #else
            if (ZigBeeReady())
            #endif
            {
                for (i=0; i<MAX_APL_FRAMES; i++)
                {
                    if (apsStatus.aplMessages[i] != NULL)
                    {
                        if (apsStatus.aplMessages[i]->flags.bits.bSendMessage)
                        {
                            BYTE    cIndex;

                            cIndex = apsStatus.aplMessages[i]->confirmationIndex;

                            // If we've run out of retries, destroy everything and send up a confirm.
                            if (apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts == 0)
                            {
                                // We have run out of transmit attempts.  Prepare the confirmation primitive.
                                params.APSDE_DATA_confirm.Status        = apsStatus.aplMessages[i]->status; //APS_NO_ACK;
                                params.APSDE_DATA_confirm.DstAddrMode   = apsConfirmationHandles[cIndex]->DstAddrMode;
                                params.APSDE_DATA_confirm.DstAddress    = apsConfirmationHandles[cIndex]->DstAddress;
                                params.APSDE_DATA_confirm.SrcEndpoint   = apsConfirmationHandles[cIndex]->SrcEndpoint;
                                params.APSDE_DATA_confirm.DstEndpoint   = apsConfirmationHandles[cIndex]->DstEndpoint;

                                // Clean up everything.
                                if (apsStatus.aplMessages[i]->message != NULL)
                                {
                                    nfree( apsStatus.aplMessages[i]->message );
                                }
                                nfree( apsStatus.aplMessages[i] );
                                nfree( apsConfirmationHandles[cIndex] );
                                return APSDE_DATA_confirm;
                            }

                            // We still have retries left.
                            // Load the primitive parameters.
                            params.NLDE_DATA_request.BroadcastRadius = apsStatus.aplMessages[i]->radiusCounter;
                            if( apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts == 1 &&
                                apsStatus.aplMessages[i]->flags.bits.bRouteRepair )
                            {
                                params.NLDE_DATA_request.DiscoverRoute   = ROUTE_DISCOVERY_FORCE;
                            }
                            else
                            {
                                params.NLDE_DATA_request.DiscoverRoute   = apsStatus.aplMessages[i]->flags.bits.nDiscoverRoute;
                            }
                            
                            params.NLDE_DATA_request.NsduLength      = apsStatus.aplMessages[i]->messageLength;
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                if( apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP )
                                {
                                    params.NLDE_DATA_request.DstAddr.Val = 0xFFFF;
                                    apsStatus.aplMessages[i]->apsFrameControl.bits.acknowledgeRequest = 0;
                                }
                                else
                            #endif
                            params.NLDE_DATA_request.DstAddr         = apsStatus.aplMessages[i]->shortDstAddress;
                            params.NLDE_DATA_request.NsduHandle      = NLME_GET_nwkBCSN();

                            // Update the confirmation queue entry.  The entry already exists from when we received
                            // the original request.  We just need to fill in the nsduHandle and timeStamp.
                            apsConfirmationHandles[cIndex]->nsduHandle = params.NLDE_DATA_request.NsduHandle;
                            apsConfirmationHandles[cIndex]->timeStamp  = TickGet();

							#ifdef I_SUPPORT_SECURITY

  								#if PROFILE_nwkSecureAllFrames
                            		TxBuffer[TxData++] = apsStatus.aplMessages[i]->apsFrameControl.Val & 0b11011111;
                            		if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_INDIRECT &&
                            		    apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                            		{
                                		TxBuffer[TxData++] = apsConfirmationHandles[cIndex]->DstEndpoint;
                            		}
                            		#ifdef I_SUPPORT_GROUP_ADDRESSING
                            		    if( apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                            		    {
                                		    TxBuffer[TxData++] = apsStatus.aplMessages[i]->shortDstAddress.byte.LSB;
                                		    TxBuffer[TxData++] = apsStatus.aplMessages[i]->shortDstAddress.byte.MSB;
                                		}
                            		#endif
                            		TxBuffer[TxData++] = apsStatus.aplMessages[i]->clusterID.byte.LSB;
                            		TxBuffer[TxData++] = apsStatus.aplMessages[i]->clusterID.byte.MSB;
                            		TxBuffer[TxData++] = apsStatus.aplMessages[i]->profileID.byte.LSB;
                            		TxBuffer[TxData++] = apsStatus.aplMessages[i]->profileID.byte.MSB;
                            		TxBuffer[TxData++] = apsConfirmationHandles[cIndex]->SrcEndpoint;
                            		TxBuffer[TxData++] = apsStatus.aplMessages[i]->APSCounter;

                            		for (j=0; j<params.NLDE_DATA_request.NsduLength; j++)
                            		{
                                		TxBuffer[TxData++] = apsStatus.aplMessages[i]->message[j];
                            		}
                            		params.NLDE_DATA_request.SecurityEnable = apsStatus.aplMessages[i]->apsFrameControl.bits.security;
  								#else
                            		// Load the APS Payload.
                            		for (j=0; j<params.NLDE_DATA_request.NsduLength; j++)
                            		{
                                		TxBuffer[TxData++] = apsStatus.aplMessages[i]->message[j];
                            		}

                            		// Load the APS Header (backwards).
                            		TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->APSCounter;
                            		TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->SrcEndpoint;
                            		TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.MSB;
                            		TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.LSB;
                            		TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.MSB;
                            		TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.LSB;
                            		#ifdef I_SUPPORT_GROUP_ADDRESSING
                            		    if( apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                            		    {
                                		    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.MSB;
                                		    TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.LSB;    
                                		}
                            		#endif
                            		if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_INDIRECT &&
                            		    apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                            		{
                                		TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->DstEndpoint;
                            		}
                            		TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->apsFrameControl.Val;
                            		if( apsStatus.aplMessages[i]->apsFrameControl.bits.security )
                            		{
                                		if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1), (TX_HEADER_START-TxHeader), ID_NetworkKey, FALSE) )
                                		{
                                    		ZigBeeUnblockTx();
                                    		return NO_PRIMITIVE;
                                		}
                            		}
  								#endif
  								
							#else

							    params.NLDE_DATA_request.SecurityEnable = FALSE;
							    
                            	// Load the APS Payload.
                            	for (j=0; j<params.NLDE_DATA_request.NsduLength; j++)
                            	{
                                	TxBuffer[TxData++] = apsStatus.aplMessages[i]->message[j];
                            	}

                            	// Load the APS Header (backwards).
                            	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->APSCounter;
                            	TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->SrcEndpoint;
                            	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.MSB;
                            	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->profileID.byte.LSB;
                            	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.MSB;
                            	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->clusterID.byte.LSB;
                            	#ifdef I_SUPPORT_GROUP_ADDRESSING
                                	if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                                	{
                                    	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.MSB;
                                    	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->shortDstAddress.byte.LSB;
                                    }
                                #endif
                            	if (apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_INDIRECT &&
                            	    apsStatus.aplMessages[i]->apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP)
                            	{
                                	TxBuffer[TxHeader--] = apsConfirmationHandles[cIndex]->DstEndpoint;
                            	}

                            	TxBuffer[TxHeader--] = apsStatus.aplMessages[i]->apsFrameControl.Val;
							#endif

                            // Update the message info.
                            apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts--;
                            apsStatus.aplMessages[i]->flags.bits.bSendMessage   = 0;
                            #ifdef I_AM_RFD
                                apsStatus.aplMessages[i]->dataRequestTimer      = TickGet();
                            #endif

                            apsStatus.flags.bits.bFramesPendingConfirm = 1;
                            
                            /* For ZigBee 2006: Free space now so that we don't run out on 
                             * when creating BTT at nwk layer on PIC18 
                            */
                            if ( (apsStatus.aplMessages[i]->flags.bits.nTransmitAttempts == 0) )
                            {
                                // Delete this entry to save space.
                                if (apsStatus.aplMessages[i]->message != NULL)
                                {
                                    nfree( apsStatus.aplMessages[i]->message );
                                }
                            }

                            ZigBeeBlockTx();
                            return NLDE_DATA_request;
                        }
#ifdef I_AM_RFD
                        else if (apsStatus.aplMessages[i]->apsFrameControl.bits.acknowledgeRequest)
                        {
                            if (TickGetDiff( currentTime, apsStatus.aplMessages[i]->dataRequestTimer ) > (DWORD)RFD_POLL_RATE)
                            {
                                // Send a data request message so we can try to receive our ACK
                                apsStatus.aplMessages[i]->dataRequestTimer = currentTime;
                                params.NLME_SYNC_request.Track = FALSE;
                                return NLME_SYNC_request;
                            }
                        }
#endif
                    }
                }
            }

            for (i=0; (i<MAX_APL_FRAMES) && (apsStatus.aplMessages[i] == NULL); i++) {}
            if (i == MAX_APL_FRAMES)
            {
                //ConsolePutROMString( (ROM char *)"APS: No APL frames awaiting transmission\r\n" );
                apsStatus.flags.bits.bFramesAwaitingTransmission = 0;
            }
        }

        // ---------------------------------------------------------------------
        // Handle frames awaiting confirmation
        if (apsStatus.flags.bits.bFramesPendingConfirm)
        {
            // NOTE: Compiler SSR27744, TickGet() output must be assigned to a variable.
            
            for (i=0; i<MAX_APS_FRAMES; i++)
            {
                if (apsConfirmationHandles[i] != NULL)
                {
                    if ((apsConfirmationHandles[i]->nsduHandle != INVALID_NWK_HANDLE) &&
                        (TickGetDiff( currentTime, apsConfirmationHandles[i]->timeStamp ) > apscAckWaitDuration))
                    {
                        // The frame has timed out while waiting for an ACK.  See if we can try again.

                        // Get the index to either the indirect message buffer of APL message buffer
                        // (nIndirectRelayIndex is the same as nAPLFrameIndex).
                        j = apsConfirmationHandles[i]->flags.bits.nAPLFrameIndex;

                        #ifdef I_SUPPORT_BINDINGS
                        if (apsConfirmationHandles[i]->flags.bits.bWeAreOriginator)
                        #endif
                        {
                            // We are the originator of the frame, so look in aplMessages.

                            // Try to send the message again.
                            apsStatus.aplMessages[j]->flags.bits.bSendMessage = 1;
                        }

                        #ifdef I_SUPPORT_BINDINGS
                            if (!apsConfirmationHandles[i]->flags.bits.bWeAreOriginator)
                            {
                                nfree( apsConfirmationHandles[i] );
                            }
                        #endif
                    }
                }
            }

            for (i=0; (i<MAX_APS_FRAMES) && (apsConfirmationHandles[i]==NULL); i++) {}
            if (i == MAX_APS_FRAMES)
            {
                //ConsolePutROMString( (ROM char *)"APS: No APS frames awaiting confirmation\r\n" );
                apsStatus.flags.bits.bFramesPendingConfirm = 0;
                params.APSDE_DATA_confirm.Status = SUCCESS;
            }
        }
    }
    else
    {
        switch (inputPrimitive)
        {
            case NLDE_DATA_confirm:
                for (i=0; i<MAX_APS_FRAMES; i++)
                {
                    if ((apsConfirmationHandles[i] != NULL) &&
                        (apsConfirmationHandles[i]->nsduHandle == params.NLDE_DATA_confirm.NsduHandle))
                    {
                        if (!apsConfirmationHandles[i]->flags.bits.bWaitingForAck ||
                            (params.NLDE_DATA_confirm.Status != NWK_SUCCESS))
                        {
FinishConfirmation:
                            {
                                // Get the index into the message info buffer
                                j = apsConfirmationHandles[i]->flags.bits.nIndirectRelayIndex;

                                #ifdef I_SUPPORT_BINDINGS
                                if (apsConfirmationHandles[i]->flags.bits.bWeAreOriginator)
                                #endif
                                {
                                    if (params.NLDE_DATA_confirm.Status != NWK_SUCCESS)
                                    {
                                        // The transmission failed, so try again.
                                        apsStatus.aplMessages[j]->flags.bits.bSendMessage = 1;
                                        apsStatus.aplMessages[j]->status = params.NLDE_DATA_confirm.Status;
                                    }
                                    else
                                    {
                                        // We have a successful confirmation for a frame we transmitted for the upper layers.
                                        params.APSDE_DATA_confirm.DstAddrMode   = apsConfirmationHandles[i]->DstAddrMode;
                                        params.APSDE_DATA_confirm.DstAddress    = apsConfirmationHandles[i]->DstAddress;
                                        params.APSDE_DATA_confirm.SrcEndpoint   = apsConfirmationHandles[i]->SrcEndpoint;
                                        params.APSDE_DATA_confirm.DstEndpoint   = apsConfirmationHandles[i]->DstEndpoint;

                                        // Clean up everything.
                                        if (apsStatus.aplMessages[j]->message != NULL)
                                        {
                                            nfree( apsStatus.aplMessages[j]->message );
                                        }
                                        nfree( apsStatus.aplMessages[j] );
                                        nfree( apsConfirmationHandles[i] );
                                        return APSDE_DATA_confirm;
                                    }
                                }
                            }
                        }
                    }
                }
                break;

            case NLDE_DATA_indication:
                {
                    APS_FRAME_CONTROL   apsFrameControl;
                    BYTE                currentAPSCounter = 0;
                    #define destinationEPL      apsStatus.ackMessage.DstEndpoint
                    #define clusterIDL          apsStatus.ackMessage.ClusterId
                    #define profileIDL          apsStatus.ackMessage.ProfileId
                    #define sourceEPL           apsStatus.ackMessage.SrcEndpoint
                    #define addressModeL        apsStatus.ackMessage.SrcAddrMode
                    #define ackSourceAddressL   apsStatus.ackMessage.SrcAddress
                    #define groupIDL            apsStatus.ackMessage.GroupId

                    // Start extracting the APS header
                    apsFrameControl.Val = APSGet();

                    if ((apsFrameControl.bits.frameType == APS_FRAME_DATA) ||
                        (apsFrameControl.bits.frameType == APS_FRAME_ACKNOWLEDGE))
                    {
                        // Finish reading the APS header
                        if (!((apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT) &&
                              (apsFrameControl.bits.indirectAddressMode == APS_INDIRECT_ADDRESS_MODE_TO_COORD))&&
                              (apsFrameControl.bits.deliveryMode != APS_DELIVERY_GROUP))
                        {
                            destinationEPL   = APSGet();
                        }
                        if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP)
                        {
                            #ifndef I_SUPPORT_GROUP_ADDRESSING
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            #endif
                            groupIDL.v[0]   = APSGet();
                            groupIDL.v[1]   = APSGet();
                        }
                        clusterIDL.v[0]      = APSGet();
                        clusterIDL.v[1]      = APSGet();
                        profileIDL.byte.LSB  = APSGet();
                        profileIDL.byte.MSB  = APSGet();

                        if (!((apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT) &&
                              (apsFrameControl.bits.indirectAddressMode == APS_INDIRECT_ADDRESS_MODE_FROM_COORD)))
                        {
                            sourceEPL        = APSGet();
                        }
                        currentAPSCounter = APSGet(); // APS counter
                        
                        if( (apsFrameControl.bits.frameType == APS_FRAME_DATA) &&
                            DuplicatePacket(params.NLDE_DATA_indication.SrcAddress, currentAPSCounter) )
                        {
                            APSDiscardRx();
                            return NO_PRIMITIVE;
                        }
                    }


                    #ifdef I_SUPPORT_SECURITY
                        #if PROFILE_nwkSecureAllFrames == 0
                            if( apsFrameControl.bits.security )
                            {
                                if( !APSFromShortToLong(&(params.NLDE_DATA_indication.SrcAddress) ) )
                                {
                                    APSDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                                if( !DataDecrypt(params.NLDE_DATA_indication.Nsdu, &(params.NLDE_DATA_indication.NsduLength), apsHeader, apsHeaderSize, ID_NetworkKey, &currentAPSAddress.longAddr) )
                                {
                                    APSDiscardRx();
                                    return NO_PRIMITIVE;
                                }
                                params.APSDE_DATA_indication.SecurityStatus = 0x01; // SECURED_NWK_KEY
                            } 
                            else
                            {
                                params.APSDE_DATA_indication.SecurityStatus = 0x00; // UNSECURED
                            }
                        #else
                            params.APSDE_DATA_indication.SecurityStatus = 0x00; // UNSECURED
                        #endif
                    #else
                        params.APSDE_DATA_indication.SecurityStatus = 0x00; // UNSECURED
                    #endif

                    if (apsFrameControl.bits.frameType == APS_FRAME_DATA)
                    {
#ifdef I_SUPPORT_GROUP_ADDRESSING                        
                        if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP )
                        {
                            /* ZigBee 2006: Don't send group broadcast request to self */
                            if( 
                            (GetEndPointsFromGroup(groupIDL) == MAX_GROUP ) )
                            {
                                APSDiscardRx();
                                return NO_PRIMITIVE;
                            }
                        }
#endif

#ifdef I_SUPPORT_BINDINGS
                        if( (apsFrameControl.bits.deliveryMode == APS_DELIVERY_DIRECT) ||
                            (apsFrameControl.bits.deliveryMode == APS_DELIVERY_BROADCAST) ||
                            (apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP) ||
                            ((apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT) &&
                             (apsFrameControl.bits.indirectAddressMode == APS_INDIRECT_ADDRESS_MODE_FROM_COORD)) )
#endif
                        {
                            // The packet is for me, either directly or indirectly
                            // Determine the address mode.
                            if (apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT)
                            {
                                addressModeL = APS_ADDRESS_NOT_PRESENT;
                            }
                            else
                            {
                                addressModeL = APS_ADDRESS_16_BIT;
                            }

                            // Check for and send APS ACK here.
                            if (apsFrameControl.bits.acknowledgeRequest == APS_ACK_REQUESTED)
                            {
                                // Since we also need to process this message, buffer the info so we can send up an
                                // APSDE_DATA_indication as soon as possible. Set the flag to send the data indication
                                // in the background
                                apsStatus.flags.bits.bDataIndicationPending = 1;

                                // Buffer the old message info
                                apsStatus.ackMessage.asduLength     = params.NLDE_DATA_indication.NsduLength;
                                apsStatus.ackMessage.asdu           = params.NLDE_DATA_indication.Nsdu;
                                apsStatus.ackMessage.WasBroadcast   = (apsFrameControl.bits.deliveryMode == APS_DELIVERY_BROADCAST);
                                apsStatus.ackMessage.SrcAddress     = params.NLDE_DATA_indication.SrcAddress;
                                
                                /* ForZigBee 2006: The NLDE_DATA_request.DstAddr 
                                 * for an acknowledge cannot be a long address like the source
                                 * address of the message to which we are responding - it MUST
                                 * be a short address, this is why its changed back to a short address
                                 * The net effect is that the ZDO_data_indication goes up as a long address
                                 * but the acknowledge is returned to the sender using its short address.
                                */
                                params.NLDE_DATA_request.DstAddr = params.NLDE_DATA_indication.SrcAddress;
                                params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
                                params.NLDE_DATA_request.DiscoverRoute      = ROUTE_DISCOVERY_ENABLE;

                                params.NLDE_DATA_request.NsduLength         = 0;
                                params.NLDE_DATA_request.NsduHandle         = NLME_GET_nwkBCSN();

                                apsFrameControl.bits.acknowledgeRequest = APS_ACK_NOT_REQUESTED;
                                apsFrameControl.bits.frameType          = APS_FRAME_ACKNOWLEDGE;
                                apsFrameControl.bits.security           = 0; // don't need to secure the ack frame in APL, no payload


                                #if defined(I_SUPPORT_SECURITY) && PROFILE_nwkSecureAllFrames
                                    params.NLDE_DATA_request.SecurityEnable = TRUE;
                                    if (apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT)
                                    {
                                        apsFrameControl.bits.indirectAddressMode = APS_INDIRECT_ADDRESS_MODE_TO_COORD;
                                        TxBuffer[TxData++] = apsFrameControl.Val;
                                    }
                                    else
                                    {
                                        TxBuffer[TxData++] = apsFrameControl.Val & 0b11110011; // set delivery mode to the direct
                                        TxBuffer[TxData++] = sourceEPL;
                                    }
                                    TxBuffer[TxData++] = clusterIDL.byte.LSB;
                                    TxBuffer[TxData++] = clusterIDL.byte.MSB;
                                    TxBuffer[TxData++] = profileIDL.byte.LSB;
                                    TxBuffer[TxData++] = profileIDL.byte.MSB;
                                    TxBuffer[TxData++] = destinationEPL;
                                    TxBuffer[TxData++] = currentAPSCounter;
                                #else
                                    params.NLDE_DATA_request.SecurityEnable = FALSE;
                                    // Load up the APS Header (backwards).  Note that the source and destination EP's get flipped.
                                    TxBuffer[TxHeader--] = currentAPSCounter;
                                    TxBuffer[TxHeader--] = destinationEPL;
                                    TxBuffer[TxHeader--] = profileIDL.byte.MSB;
                                    TxBuffer[TxHeader--] = profileIDL.byte.LSB;
                                    TxBuffer[TxHeader--] = clusterIDL.byte.MSB;
                                    TxBuffer[TxHeader--] = clusterIDL.byte.LSB;

                                    if (apsFrameControl.bits.deliveryMode == APS_DELIVERY_INDIRECT)
                                    {
                                        apsFrameControl.bits.indirectAddressMode = APS_INDIRECT_ADDRESS_MODE_TO_COORD;
                                    }
                                    else
                                    {
                                        apsFrameControl.bits.deliveryMode       = APS_DELIVERY_DIRECT;
                                        TxBuffer[TxHeader--] = sourceEPL;
                                    }

                                    apsFrameControl.bits.acknowledgeRequest = APS_ACK_NOT_REQUESTED;
                                    apsFrameControl.bits.frameType          = APS_FRAME_ACKNOWLEDGE;
                                    apsFrameControl.bits.security           = 0; // don't need to secure the ack frame in APL, no payload

                                    TxBuffer[TxHeader--] = apsFrameControl.Val;
                                #endif

                                ZigBeeBlockTx();
                                return NLDE_DATA_request;
                            }

                            if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP ||
                                destinationEPL == 0xFF )
                            {
                                #ifdef I_SUPPORT_GROUP_ADDRESSING
                                    for(i = 0; i < MAX_GROUP_RECORD_BUFFER; i++)
                                    {
                                        if( apsStatus.apsGroupRecord[i] == NULL )
                                        {
                                            apsStatus.apsGroupRecord[i] = (APS_GROUP_RECORD *)SRAMalloc(sizeof(APS_GROUP_RECORD));
                                            if( apsStatus.apsGroupRecord[i] == NULL )
                                            {
                                                return NO_PRIMITIVE;
                                            }
                                            if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP )
                                            {
                                                for(j = 0; j < MAX_GROUP_END_POINT; j++)
                                                {
                                                    apsStatus.apsGroupRecord[i]->EndPoints[j] = currentGroupAddressRecord.EndPoint[j];
                                                }
                                            }
                                            else
                                            {
                                                for(j = 0; j < NUM_USER_ENDPOINTS; j++)
                                                {
                                                    apsStatus.apsGroupRecord[i]->EndPoints[j] = Config_Simple_Descriptors[j+1].Endpoint;
                                                }
                                                apsStatus.apsGroupRecord[i]->EndPoints[j] = 0xFF;
                                                    
                                            }
                                            apsStatus.apsGroupRecord[i]->EndPointIndex   = 0;
                                            apsStatus.apsGroupRecord[i]->SrcAddrMode     = addressModeL;
                                            apsStatus.apsGroupRecord[i]->SrcAddress      = params.APSDE_DATA_indication.SrcAddress;
                                            apsStatus.apsGroupRecord[i]->SrcEndpoint     = sourceEPL;
                                            apsStatus.apsGroupRecord[i]->ProfileId       = profileIDL;
                                            apsStatus.apsGroupRecord[i]->ClusterId       = clusterIDL;
                                            apsStatus.apsGroupRecord[i]->asduLength      = params.APSDE_DATA_indication.asduLength;
                                            apsStatus.apsGroupRecord[i]->asdu            = params.APSDE_DATA_indication.asdu;
                                            apsStatus.apsGroupRecord[i]->SecurityStatus  = apsFrameControl.bits.security;
                                            apsStatus.apsGroupRecord[i]->CurrentRxPacket = CurrentRxPacket;
                                            CurrentRxPacket                              = NULL;
                                            apsStatus.flags.bits.bGroupAddressing = 1;
                                            break;
                                        }    
                                    }
                                    
                                    if( i == MAX_GROUP_RECORD_BUFFER )
                                    {
                                        //report error here
                                    }
                                #else
                                    APSDiscardRx();
                                #endif
                                return NO_PRIMITIVE;    
                            }


                            // asduLength already in place
                            // *asdu already in place
                            // SrcAddress already in place
                            params.APSDE_DATA_indication.SrcAddrMode    = addressModeL;
                            params.APSDE_DATA_indication.WasBroadcast   = (apsFrameControl.bits.deliveryMode == APS_DELIVERY_BROADCAST);
                            params.APSDE_DATA_indication.DstEndpoint    = destinationEPL;
                            params.APSDE_DATA_indication.ClusterId      = clusterIDL;
                            params.APSDE_DATA_indication.ProfileId      = profileIDL;
                            params.APSDE_DATA_indication.SrcEndpoint    = sourceEPL;   // May be invalid.  SrcAddrMode will indicate.

                            if (params.APSDE_DATA_indication.DstEndpoint == 0)
                            {
                                /* this is used to reset the polling rate to back to normal at the appl level */
                                #ifdef I_AM_END_DEVICE
                                    if( params.APSDE_DATA_indication.ClusterId.Val == END_DEVICE_BIND_rsp)
                                    {
                                        SendingEDBRequest = 0;
                                    }
                                #endif
                              
                                return ZDO_DATA_indication;
                            }
                            else
                            {
                                return APSDE_DATA_indication;
                            }
                        }

                    }
                    else if (apsFrameControl.bits.frameType == APS_FRAME_ACKNOWLEDGE)
                    {
                        for (i=0; i<MAX_APS_FRAMES; i++)
                        {
                            if (apsConfirmationHandles[i] != NULL)
                            {
                                if (apsConfirmationHandles[i]->DstAddrMode == APS_ADDRESS_64_BIT)
                                {
                                    #if MAX_APS_ADDRESSES > 0
                                        if (!LookupAPSAddress( &apsConfirmationHandles[i]->DstAddress.LongAddr ))
                                        {
                                            ackSourceAddressL = currentAPSAddress.shortAddr;
                                        }
                                        else
                                        {
                                            continue;
                                        }
                                    #else
                                        continue;
                                    #endif
                                }
                                else
                                {
                                    ackSourceAddressL = apsConfirmationHandles[i]->DstAddress.ShortAddr;
                                }

                                if (ackSourceAddressL.Val == params.NLDE_DATA_indication.SrcAddress.Val)
                                {
                                    if( currentAPSCounter != apsConfirmationHandles[i]->APSCounter )
                                    {
                                        continue;
                                    }

                                    // If all the above tests pass, the frame has been ACK'd
                                    APSDiscardRx();

                                    params.APSDE_DATA_confirm.Status        = SUCCESS;
                              
                                    // This is the same as when we get an NLDE_DATA_confirm
                                    goto FinishConfirmation;
                                }
                            }
                        }
                    }
                    // There are currently no APS-level commands

				#ifdef I_SUPPORT_SECURITY
                    else if( apsFrameControl.bits.frameType == APS_FRAME_COMMAND ) 
                    {
                        BYTE CommandIdentifier;
                        
                        if( apsFrameControl.bits.deliveryMode == APS_DELIVERY_GROUP )
                        {
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                APSGet();
                                APSGet();
                            #else
                                APSDiscardRx();
                                return NO_PRIMITIVE;    
                            #endif
                        }
                        
                        currentAPSCounter = APSGet();
                        if( DuplicatePacket(params.NLDE_DATA_indication.SrcAddress, currentAPSCounter) )
                        {
                            APSDiscardRx();
                            return NO_PRIMITIVE;
                        }
                        
                        CommandIdentifier = APSGet();

                        switch( CommandIdentifier )
                        {
                            case APS_CMD_TRANSPORT_KEY:
                            {
                                BYTE KeyType = APSGet();
                                for(i = 0; i < 16; i++)
                                {
                                    KeyVal.v[i] = APSGet();
                                }

                                switch( KeyType )
                                {
                                    case 0x01:  // network key
                                    {
                                        LONG_ADDR DstAddr;
                                        LONG_ADDR SrcAddr;
                                        BYTE SeqNum;

                                        SeqNum = APSGet();

                                        for(i = 0; i < 8; i++)
                                        {
                                            DstAddr.v[i] = APSGet();
                                        }
                                        for(i = 0; i < 8; i++) {
                                            SrcAddr.v[i] = APSGet();
                                            
                                            /* added at NTS */
                                            transportKeySrcAddr.v[i] = SrcAddr.v[i];
                                        }


                                        /* For Zigbee 2006: if this was broadcast or I am the destination
                                         * then just handle the return indication here 
                                        */
                                        if( NWKThisIsMyLongAddress(&DstAddr)   ||  
                                           ( ( DstAddr.v[0] == 0x00) && ( DstAddr.v[1] == 0x00) &&
                                             ( DstAddr.v[2] == 0x00) && ( DstAddr.v[3] == 0x00) &&
                                             ( DstAddr.v[4] == 0x00) && ( DstAddr.v[5] == 0x00) &&
                                             ( DstAddr.v[6] == 0x00) && ( DstAddr.v[7] == 0x00) ) )
                                        {
                                            params.APSME_TRANSPORT_KEY_indication.KeyType = KeyType;
                                            params.APSME_TRANSPORT_KEY_indication.Key = &KeyVal;
                                            params.APSME_TRANSPORT_KEY_indication.SrcAddr = SrcAddr;
                                            params.APSME_TRANSPORT_KEY_indication.TransportKeyData.NetworkKey.KeySeqNumber = SeqNum;
                                            APSDiscardRx();
                                            return APSME_TRANSPORT_KEY_indication;
                                        }

                                        params.APSME_TRANSPORT_KEY_request.KeyType = 0x01;
                                        params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                                        params.APSME_TRANSPORT_KEY_request.DestinationAddress = DstAddr;
                                        switch(KeyType)
                                        {
                                            case 0x01:
                                                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = 0;
                                                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = SeqNum;
                                                break;
                                            default:
                                                break;

                                        }
                                        
                                        /* ZigBee 2006 Requirement: Added at NTS - if this is my child's key 
                                         * then turn off waiting for key flag, and we are free to send
                                         * APS_REMOVE_DEVICE if required, else must be silent
                                        */
                                        #ifdef I_AM_ROUTER
                                            if (!memcmp( (void *)&updateDevAPSAddr.longAddr, (void *)&DstAddr, 8 ) )
                                            {   
                                                waitingForKey = FALSE;    
                                            }
                                        #endif
                                        APSDiscardRx();
                                        /* ZigBee 2006: if device has key already always send new key or dummy key secure */
                                        #ifdef PRECONFIGURE_KEY
                                            params.APSME_TRANSPORT_KEY_request._UseSecurity = TRUE;
                                        #else
                                            params.APSME_TRANSPORT_KEY_request._UseSecurity = FALSE;
                                        #endif
                                        return APSME_TRANSPORT_KEY_request;
                                    }
                                }
                                APSDiscardRx();
                                return NO_PRIMITIVE;

                            }

#if defined(I_AM_TRUST_CENTER)
                            case APS_CMD_UPDATE_DEVICE:
                            {
                                if( !APSFromShortToLong( &params.NLDE_DATA_indication.SrcAddress ) )
                                {
                                    APSDiscardRx();
                                    break;
                                }
                                params.APSME_UPDATE_DEVICE_indication.SrcAddress = currentAPSAddress.longAddr;

                                for(i = 0; i < 8; i++)
                                {
                                    params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i] = APSGet();
                                    
                                    /* ZigBee 2006: prepare to store in APS table in case device joined in out of range */
                                    updateDevAPSAddr.longAddr.v[i] = params.APSME_UPDATE_DEVICE_indication.DeviceAddress.v[i];
                                }
                                params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[0] = APSGet();
                                params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[1] = APSGet();
                                /* ZigBee 2006: Just in case device is out of range and we didn't have its
                                * address mappings, then store it now while we have a chance
                                */
                                updateDevAPSAddr.shortAddr.v[0] = params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[0];
                                updateDevAPSAddr.shortAddr.v[1] = params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress.v[1];
                                if( LookupAPSAddress(&updateDevAPSAddr.longAddr) == FALSE )
                                {
                                    APSSaveAPSAddress( &updateDevAPSAddr);   
                                }

                                params.APSME_UPDATE_DEVICE_indication.Status = APSGet();

                                APSDiscardRx();
                                return APSME_UPDATE_DEVICE_indication;
                            }
#endif

#if defined(I_AM_COORDINATOR)|| defined(I_AM_ROUTER)
                            case APS_CMD_REMOVE_DEVICE:
                            {
                                if( !APSFromShortToLong( &params.NLDE_DATA_indication.SrcAddress ) )
                                {
                                    /* For ZCP 2006 assume the Coodinator/TC is orgin of APSME-REMOVE_DEVICE_request */
                                    currentAPSAddress.longAddr = macPIB.macCoordExtendedAddress;

                                }
                                params.APSME_REMOVE_DEVICE_indication.SrcAddress = currentAPSAddress.longAddr;

                                for(i = 0; i < 8; i++)
                                {
                                    params.APSME_REMOVE_DEVICE_indication.ChildAddress.v[i] = APSGet();
                                }
                                APSDiscardRx();
                                return APSME_REMOVE_DEVICE_indication;
                            }
#endif

#ifdef I_AM_TRUST_CENTER
                            case APS_CMD_REQUEST_KEY:
                            {
                                if( !APSFromShortToLong( &params.NLDE_DATA_indication.SrcAddress ) )
                                {
                                    break;
                                }
                                params.APSME_REQUEST_KEY_indication.SrcAddress = currentAPSAddress.longAddr;

                                params.APSME_REQUEST_KEY_indication.KeyType = APSGet();
                                if( ID_NetworkKey != params.APSME_REQUEST_KEY_indication.KeyType )
                                {
                                    for(i = 0; i < 8; i++)
                                    {
                                        params.APSME_REQUEST_KEY_indication.PartnerAddress.v[i] = APSGet();
                                    }
                                }
                                APSDiscardRx();
                                return APSME_REQUEST_KEY_indication;

                            }
#endif
                            case APS_CMD_SWITCH_KEY:
                            {
                                if( !APSFromShortToLong( &params.NLDE_DATA_indication.SrcAddress ) )
                                {
                                    /* For ZCP we can use the trust center/Coordinator
                                     *  address  as the source of the switch command.
                                    */
                                    for(i = 0; i < 8; i++)
                                        currentAPSAddress.longAddr.v[i] = 0xaa;
                                        
                                }
                                params.APSME_SWITCH_KEY_indication.SrcAddress = currentAPSAddress.longAddr;

                                params.APSME_SWITCH_KEY_indication.KeySeqNumber = APSGet();
                                APSDiscardRx();
                                return APSME_SWITCH_KEY_indication;
                            }

                            default:
                                break;
                        }
                    }
				#endif  // if I_SUPPORT SECURITY
                    APSDiscardRx();
                    return NO_PRIMITIVE;
                }
                break;
                #undef destinationEPL
                #undef clusterIDL
                #undef profileIDL
                #undef sourceEPL
                #undef addressModeL
                #undef ackSourceAddressL

            case APSDE_DATA_request:
                {
                    APS_FRAME_CONTROL   apsFrameControl;
                    #ifdef I_SUPPORT_BINDINGS
                        BYTE                originalBinding;
                        BYTE                currentBinding = 0;
                    #endif
                    
                    /* Don't attempt to send any data if we are not to on the network */
                    #ifdef I_AM_COORDINATOR
                    if (!ZigBeeStatus.flags.bits.bNetworkFormed)
                    #else
                    if (!ZigBeeStatus.flags.bits.bNetworkJoined)
                    #endif
                    {
                        params.APSDE_DATA_confirm.DstAddrMode           = params.APSDE_DATA_request.DstAddrMode;
                        if(params.APSDE_DATA_request.DstAddrMode != APS_ADDRESS_NOT_PRESENT)
                        {
                            params.APSDE_DATA_confirm.DstAddress        = params.APSDE_DATA_request.DstAddress;  //macPIB.macShortAddress.byte.MSB;
                            params.APSDE_DATA_confirm.DstEndpoint       = params.APSDE_DATA_request.DstEndpoint;
                        }
                        params.APSDE_DATA_confirm.SrcEndpoint       = params.APSDE_DATA_request.SrcEndpoint;
                        params.APSDE_DATA_confirm.Status            = NWK_INVALID_REQUEST; // APS_INVALID_REQUEST;
                        ZigBeeUnblockTx();
                        return APSDE_DATA_confirm;
                    }
                    
                    /*ZigBee 2006:  If I am the coordinator and doing a self end device bind request, capture the 
                     * request here, and make sure its tagged as fromself
                    */
                    #ifdef I_AM_COORDINATOR
                        if( (params.APSDE_DATA_request.ClusterId.Val == END_DEVICE_BIND_req)  )
                        {
                            return ZDO_END_DEVICE_BIND_req;      
                        }
                    #endif
                    
                    
                    #ifdef I_SUPPORT_GROUP_ADDRESSING
                        // addressing myself, because I will not be able to receive my own broadcast message
                        if( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP )
                        {
                            
                            if( GetEndPointsFromGroup(params.APSDE_DATA_request.DstAddress.ShortAddr) != MAX_GROUP )
                            {  
                                for(j = 0; j < MAX_GROUP_RECORD_BUFFER; j++)
                                {
                                    if( apsStatus.apsGroupRecord[j] == NULL )
                                    {
                                        BYTE k;
                                        
                                        apsStatus.apsGroupRecord[j] = (APS_GROUP_RECORD *)SRAMalloc(sizeof(APS_GROUP_RECORD));
                                        if( apsStatus.apsGroupRecord[j] == NULL )
                                        {
                                            //  report error here
                                            break;
                                        }
                                        for(k = 0; k < MAX_GROUP_END_POINT; k++)
                                        {
                                            apsStatus.apsGroupRecord[j]->EndPoints[k] = currentGroupAddressRecord.EndPoint[k];
                                        }
                                        apsStatus.apsGroupRecord[j]->EndPointIndex          = 0;
                                        apsStatus.apsGroupRecord[j]->SrcAddrMode            = APS_ADDRESS_16_BIT;
                                        apsStatus.apsGroupRecord[j]->SrcAddress.ShortAddr   = macPIB.macShortAddress;
                                        apsStatus.apsGroupRecord[j]->ProfileId.Val          = params.APSDE_DATA_request.ProfileId.Val;
                                        apsStatus.apsGroupRecord[j]->ClusterId.Val          = params.APSDE_DATA_request.ClusterId.Val;
                                        apsStatus.apsGroupRecord[j]->asduLength             = TxData;
                                        apsStatus.apsGroupRecord[j]->asdu                   = (BYTE *)SRAMalloc(TxData); 
                                        if( apsStatus.apsGroupRecord[j]->asdu == NULL )
                                        {
                                                   /* Add Error recovery as needed */
                                        }
                                        apsStatus.apsGroupRecord[j]->SecurityStatus         = params.APSDE_DATA_request.TxOptions.bits.securityEnabled;
                                        apsStatus.apsGroupRecord[j]->CurrentRxPacket        = apsStatus.apsGroupRecord[j]->asdu;
                                        CurrentRxPacket                                     = NULL;
                                        apsStatus.flags.bits.bGroupAddressing = 1;
                                                   
                                        // Copy the message.
                                        for (k=0; k<TxData; k++)
                                        {
                                            apsStatus.apsGroupRecord[j]->asdu[k] = TxBuffer[k];
                                        }
                                        break;
                                    }
                                }  
                            }
                            #ifdef I_AM_ROUTER 
                            /* if group addressing but no binding record exist, then return */
                                
                            
                            #endif  
                        }

                    #endif


                    #ifndef I_SUPPORT_BINDINGS
                        // Bindings are not supported, so all messages are buffered in the aplMessages buffer
                        // and all messages have an apsConfirmationHandles entry.
                        
                        // Validate what we can before allocating space.
                        if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                        {
                            if (!APSFromLongToShort( &params.APSDE_DATA_request.DstAddress.LongAddr ))
                            {
                                // We do not have a short address for this long address, so return an error.
                                params.APSDE_DATA_confirm.Status = APS_INVALID_REQUEST;
                                ZigBeeUnblockTx();
                                return APSDE_DATA_confirm;
                            }
                        }

                        // Prepare a confirmation handle entry.
                        for (i=0; (i<MAX_APS_FRAMES) && (apsConfirmationHandles[i]!=NULL); i++) {}
                        if ((i == MAX_APS_FRAMES) ||
                            ((apsConfirmationHandles[i] = (APS_FRAMES *)SRAMalloc( sizeof(APS_FRAMES) )) == NULL))
                        {
                            // There was no room for another frame, so return an error.
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Prepare an APL message buffer
                        for (j=0; (j<MAX_APL_FRAMES) && (apsStatus.aplMessages[j]!=NULL); j++) {}
                        if ((j == MAX_APL_FRAMES) ||
                            ((apsStatus.aplMessages[j] = (APL_FRAME_INFO *)SRAMalloc( sizeof(APL_FRAME_INFO) )) == NULL))
                        {
                            // There was no room for another frame, so return an error.
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Load the confirmation handle entry.  Set nsduHandle to INVALID, since we do the actual
                        // transmission from the background.
                        apsConfirmationHandles[i]->nsduHandle                   = INVALID_NWK_HANDLE;
                        apsConfirmationHandles[i]->DstAddrMode                  = params.APSDE_DATA_request.DstAddrMode;
                        apsConfirmationHandles[i]->SrcEndpoint                  = params.APSDE_DATA_request.SrcEndpoint;
                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            if( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP )
                                apsConfirmationHandles[i]->DstEndpoint          = 0xFE;
                            else
                        #endif
                        apsConfirmationHandles[i]->DstEndpoint                  = params.APSDE_DATA_request.DstEndpoint;
                        apsConfirmationHandles[i]->DstAddress                   = params.APSDE_DATA_request.DstAddress; // May change later...
                        apsConfirmationHandles[i]->flags.bits.nAPLFrameIndex    = j;
                        apsConfirmationHandles[i]->flags.bits.bWaitingForAck    = FALSE;    // May change later...
                        apsConfirmationHandles[i]->APSCounter                   = APSCounter;
                        apsConfirmationHandles[i]->timeStamp                    = TickGet();

                        // Start loading the APL message info.
                        if ((apsStatus.aplMessages[j]->message = SRAMalloc( TxData )) == NULL)
                        {
                            nfree( apsStatus.aplMessages[j] );
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }
                        apsStatus.aplMessages[j]->flags.Val                     = 0;
                        apsStatus.aplMessages[j]->profileID                     = params.APSDE_DATA_request.ProfileId;
                        apsStatus.aplMessages[j]->radiusCounter                 = params.APSDE_DATA_request.RadiusCounter;
                        apsStatus.aplMessages[j]->clusterID.Val                 = params.APSDE_DATA_request.ClusterId.Val;
                        apsStatus.aplMessages[j]->confirmationIndex             = i;
                        apsStatus.aplMessages[j]->flags.bits.nDiscoverRoute     = params.APSDE_DATA_request.DiscoverRoute;
                        apsStatus.aplMessages[j]->shortDstAddress               = params.APSDE_DATA_request.DstAddress.ShortAddr; // May not be correct - fixed later.
                        apsStatus.aplMessages[j]->flags.bits.bSendMessage       = 1;
                        apsStatus.aplMessages[j]->messageLength                 = TxData;
                        apsStatus.aplMessages[j]->APSCounter                    = APSCounter++;

                        // Start building the frame control.
                        apsStatus.aplMessages[j]->apsFrameControl.Val = APS_FRAME_DATA;   // APS_DELIVERY_DIRECT

                        apsStatus.aplMessages[j]->apsFrameControl.bits.security = params.APSDE_DATA_request.TxOptions.bits.securityEnabled;
                        

                        if (
                            #ifdef I_SUPPORT_GROUP_ADDRESSING
                                (params.APSDE_DATA_request.DstAddrMode != APS_ADDRESS_GROUP) &&
                            #endif
                            params.APSDE_DATA_request.TxOptions.bits.acknowledged)
                        {
                            apsConfirmationHandles[i]->flags.bits.bWaitingForAck = TRUE;
                            apsStatus.aplMessages[j]->apsFrameControl.bits.acknowledgeRequest = APS_ACK_REQUESTED;
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts  = apscMaxFrameRetries + 1;
                            apsStatus.aplMessages[j]->flags.bits.bRouteRepair = 1;
                        }
                        else
                        {
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts  = 1;
                        }

                        #if MAX_APS_ADDRESSES > 0
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                            {
                                // If we get to here, then currentAPSAddress is waiting for us.
                                apsStatus.aplMessages[j]->shortDstAddress = currentAPSAddress.shortAddr;
                                // apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT, which is 0x00
                            }
                            else
                        #endif

                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                            {
                                apsStatus.aplMessages[j]->apsFrameControl.bits.deliveryMode = APS_DELIVERY_GROUP;
                            }
                            else
                        #endif
                        if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT)
                        {
                            // NOTE According to the spec, broadcast at this level means the frame
                            // goes to all devices AND all endpoints.  That makes no sense - EP0 Cluster 0
                            // means something very different from Cluster 0 on any other endpoint.  We'll
                            // set it for all devices...
                            if (params.APSDE_DATA_request.DstAddress.ShortAddr.Val == 0xFFFF)
                            {
                                apsStatus.aplMessages[j]->apsFrameControl.bits.deliveryMode = APS_DELIVERY_BROADCAST;
                            }
                            // else apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT, which is 0x00
                        }
                        else
                        {
                            // Since we do not support binding, then we must be sending to the coordinator
                            apsStatus.aplMessages[j]->shortDstAddress.Val                       = 0x0000;
                            apsStatus.aplMessages[j]->apsFrameControl.bits.deliveryMode         = APS_DELIVERY_INDIRECT;
                            apsStatus.aplMessages[j]->apsFrameControl.bits.indirectAddressMode  = APS_INDIRECT_ADDRESS_MODE_TO_COORD;
                            apsConfirmationHandles[i]->DstAddress.ShortAddr.Val                 = 0x0000;
                        }
                        //apsStatus.aplMessages[j]->apsFrameControl = apsFrameControl;

                        // Buffer the message payload.
                        ptr = apsStatus.aplMessages[i]->message;
                        i = 0;
                        while (TxData--)
                        {
                            *ptr++ = TxBuffer[i++];
                        }

                        apsStatus.flags.bits.bFramesAwaitingTransmission = 1;
                        ZigBeeUnblockTx();
                        return NO_PRIMITIVE;

                    #else

                        // Bindings are supported, so we are either a coordinator or a router.  If we are sending
                        // a direct message or an indirect message to the coordinator, we need to create an
                        // aplMessages and an apsConfirmationHandles entry.  If we are trying to send indirect
                        // messages from the coordinator, we need to create an indirectMessages queue entry.

                        // Start building the frame control.
                        /* this will be used to speed up the poll rate during end_device bind */
                        #ifdef I_AM_END_DEVICE
                            if(params.APSDE_DATA_request.ClusterId.Val == END_DEVICE_BIND_req)
                            {
                                SendingEDBRequest = 1;   
                            }
                        #endif
                        
                        #ifdef I_SUPPORT_BINDINGS
                            if( (params.APSDE_DATA_request.ClusterId.Val == BIND_req)  ||
                                (params.APSDE_DATA_request.ClusterId.Val == UNBIND_req)
                               )
                            {
                                SentBindRequest = 1;   
                            }
                        #endif 
                        
                        apsFrameControl.Val = APS_FRAME_DATA;   // and APS_DELIVERY_DIRECT
                        if (params.APSDE_DATA_request.TxOptions.bits.securityEnabled)
                        {
                            apsFrameControl.bits.security = APS_SECURITY_ON;
                        }

                        if (params.APSDE_DATA_request.TxOptions.bits.acknowledged)
                        {
                            apsFrameControl.bits.acknowledgeRequest = APS_ACK_REQUESTED;
                        }

                        // Validate what we can before allocating space, and determine the addressing mode.
                        if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                        {
                            if (!APSFromLongToShort( &params.APSDE_DATA_request.DstAddress.LongAddr ))
                            {
                                // We do not have a short address for this long address, so return an error.
                                params.APSDE_DATA_confirm.Status = APS_INVALID_REQUEST;
                                ZigBeeUnblockTx();
                                #ifdef I_AM_END_DEVICE
                                    SendingEDBRequest = 0;
                                #endif
                                return APSDE_DATA_confirm;
                            }
                            // else apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT, which is 0x00
                        }
                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            else if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                            {
                                apsFrameControl.bits.deliveryMode = APS_DELIVERY_GROUP;
                            }
                        #endif                        
                        else if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_16_BIT)
                        {
                            // NOTE According to the spec, broadcast at this level means the frame
                            // goes to all devices AND all endpoints.  That makes no sense - EP0 Cluster 0
                            // means something very different from Cluster 0 on any other endpoint.  We'll
                            // set it for all devices...
                            if (params.APSDE_DATA_request.DstAddress.ShortAddr.Val == 0xFFFF)
                            {
                                apsFrameControl.bits.deliveryMode = APS_DELIVERY_BROADCAST;
                            }
                            // else apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT, which is 0x00
                        }
                        else
                        {
                            
                            if(LookupSourceBindingInfo(macPIB.macShortAddress, params.APSDE_DATA_request.SrcEndpoint, params.APSDE_DATA_request.ClusterId ) == END_BINDING_RECORDS)
                            {
                                // I don't have a binding record for it, so issue APSDE_DATA_confirm primitives with 
                                // status of NO_BOND_DEVICE
                                params.APSDE_DATA_confirm.Status = APS_NO_BOUND_DEVICE;
                                /* ZigBee 2006: When toggling end_device_bind-request clusters, this is encountered
                                 * a lot, so proper cleanup is neccessary here
                                */
                                ZigBeeUnblockTx();
                                #ifdef I_AM_END_DEVICE
                                    SendingEDBRequest = 0;
                                #endif
                                return APSDE_DATA_confirm;
                            }
                            currentBinding = currentBindingRecord.nextBindingRecord;
                            originalBinding = currentBinding;
                        }

                        // We are sending either an indirect message to the coordinator or a direct message to someone.
                        // Create both an aplMessages entry and an apsConfirmationHandles entry.
BufferAPLMessage:
                        // Prepare a confirmation handle entry.
                        for (i=0; (i<MAX_APS_FRAMES) && (apsConfirmationHandles[i]!=NULL); i++) {}
                        if ((i == MAX_APS_FRAMES) ||
                            ((apsConfirmationHandles[i] = (APS_FRAMES *)SRAMalloc( sizeof(APS_FRAMES) )) == NULL))
                        {
                            // There was no room for another frame, so return an error.
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Prepare an APL message buffer
                        for (j=0; (i<MAX_APL_FRAMES) && (apsStatus.aplMessages[j]!=NULL); j++) {}
                        if ((j == MAX_APL_FRAMES) ||
                            ((apsStatus.aplMessages[j] = (APL_FRAME_INFO *)SRAMalloc( sizeof(APL_FRAME_INFO) )) == NULL))
                        {
                            // There was no room for another frame, so return an error.
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }
                        
                        // Start loading the APL message info.
                        if ((apsStatus.aplMessages[j]->message = SRAMalloc( TxData )) == NULL)
                        {
                            nfree( apsStatus.aplMessages[j] );
                            nfree( apsConfirmationHandles[i] );
                            params.APSDE_DATA_confirm.Status = TRANSACTION_OVERFLOW;
                            ZigBeeUnblockTx();
                            return APSDE_DATA_confirm;
                        }

                        // Load the confirmation handle entry.  Set nsduHandle to INVALID, and do not load
                        // timeStamp, since we do the actual transmission from the background.
                        apsConfirmationHandles[i]->nsduHandle                   = INVALID_NWK_HANDLE;
                        apsConfirmationHandles[i]->SrcEndpoint                  = params.APSDE_DATA_request.SrcEndpoint;
                        apsConfirmationHandles[i]->APSCounter                   = APSCounter;
                        apsConfirmationHandles[i]->flags.bits.nAPLFrameIndex    = j;
                        apsConfirmationHandles[i]->flags.bits.bWaitingForAck    = 0;    // May change later...
                        apsConfirmationHandles[i]->flags.bits.bWeAreOriginator  = 1;
                        
                        // For ZigBee 2006
                        apsStatus.aplMessages[j]->flags.Val                      = 0;
                        apsStatus.aplMessages[j]->profileID                      = params.APSDE_DATA_request.ProfileId;
                        apsStatus.aplMessages[j]->radiusCounter                  = params.APSDE_DATA_request.RadiusCounter;
                        apsStatus.aplMessages[j]->clusterID                      = params.APSDE_DATA_request.ClusterId;
                        apsStatus.aplMessages[j]->confirmationIndex              = i;
                        apsStatus.aplMessages[j]->shortDstAddress                = params.APSDE_DATA_request.DstAddress.ShortAddr; // May not be correct - fixed later.
                        apsStatus.aplMessages[j]->messageLength                  = TxData;
                        apsStatus.aplMessages[j]->flags.bits.nDiscoverRoute      = params.APSDE_DATA_request.DiscoverRoute;
                        apsStatus.aplMessages[j]->flags.bits.bSendMessage        = 1;
                        apsStatus.aplMessages[j]->APSCounter                     = APSCounter++;

                        if (params.APSDE_DATA_request.TxOptions.bits.acknowledged)
                        {
                            apsConfirmationHandles[i]->flags.bits.bWaitingForAck   = 1;
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts = apscMaxFrameRetries + 1;
                            apsStatus.aplMessages[j]->flags.bits.bRouteRepair = 1;
                        }
                        else
                        {
                            apsStatus.aplMessages[j]->flags.bits.nTransmitAttempts = 1;
                        }

                        #if MAX_APS_ADDRESSES > 0
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_64_BIT)
                            {
                                // If we get to here, then currentAPSAddress is waiting for us.
                                apsStatus.aplMessages[j]->shortDstAddress = currentAPSAddress.shortAddr;
                            }
                            else
                        #endif

                        #ifdef I_SUPPORT_GROUP_ADDRESSING
                            if (params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_GROUP)
                            {
                                apsFrameControl.bits.deliveryMode       = APS_DELIVERY_GROUP;    
                            }
                        #endif

                        if( params.APSDE_DATA_request.DstAddrMode == APS_ADDRESS_NOT_PRESENT )
                        {
                            if( currentBinding != END_BINDING_RECORDS )
                            {
                                BINDING_RECORD thisBindingRecord;
                                
                                #ifdef USE_EXTERNAL_NVM
                                    pCurrentBindingRecord = apsBindingTable + (WORD)(currentBinding) * (WORD)sizeof(BINDING_RECORD);
                                #else
                                    pCurrentBindingRecord = &apsBindingTable[currentBinding];
                                #endif
                                GetBindingRecord(&thisBindingRecord, pCurrentBindingRecord);
                                
                                if( thisBindingRecord.endPoint == 0xFE )
                                {
                                    apsConfirmationHandles[i]->DstAddrMode              = APS_ADDRESS_GROUP;
                                    apsFrameControl.bits.deliveryMode                   = APS_DELIVERY_GROUP;   
                                }
                                else
                                {
                                    apsConfirmationHandles[i]->DstAddrMode              = APS_ADDRESS_16_BIT;
                                    apsFrameControl.bits.deliveryMode                   = APS_DELIVERY_DIRECT;
                                }
                                apsConfirmationHandles[i]->DstEndpoint              = thisBindingRecord.endPoint;
                                apsConfirmationHandles[i]->DstAddress.ShortAddr     = thisBindingRecord.shortAddr;
                                /* For ZigBee 2006: The proper destination address is dest address
                                 * that is in the destination bind record 
                                */
                                apsStatus.aplMessages[j]->shortDstAddress.Val       = thisBindingRecord.shortAddr.Val;
                               
                                apsStatus.aplMessages[j]->apsFrameControl = apsFrameControl;
                                apsStatus.aplMessages[j]->apsFrameControl.bits.security = params.APSDE_DATA_request.TxOptions.bits.securityEnabled;

                                // Buffer the message payload.
                                ptr = apsStatus.aplMessages[i]->message;
                                i = 0;
                                while (TxData > i)
                                {
                                    *ptr++ = TxBuffer[i++];
                                }
          
                                currentBinding = thisBindingRecord.nextBindingRecord;
                                /* For ZigBee 2006: This check is necessary in order to terminate 
                                 * the sending of requests at the end of the bind list
                                 */
                                if(currentBinding == END_BINDING_RECORDS)
                                {
                                    goto endofbindlist;
                                }
                                goto BufferAPLMessage;
                            }
                        }
                        else
                        {
                            apsConfirmationHandles[i]->DstAddrMode                  = params.APSDE_DATA_request.DstAddrMode;
                            apsConfirmationHandles[i]->DstEndpoint                  = params.APSDE_DATA_request.DstEndpoint;
                            apsConfirmationHandles[i]->DstAddress                   = params.APSDE_DATA_request.DstAddress;
                        
                            apsStatus.aplMessages[j]->apsFrameControl = apsFrameControl;
                            apsStatus.aplMessages[j]->apsFrameControl.bits.security = params.APSDE_DATA_request.TxOptions.bits.securityEnabled;

                            // Buffer the message payload.
                            ptr = apsStatus.aplMessages[i]->message;
                            i = 0;
                            while (TxData > i)
                            {
                                *ptr++ = TxBuffer[i++];
                            }
                        }

endofbindlist:          apsStatus.flags.bits.bFramesAwaitingTransmission = 1;
                        ZigBeeUnblockTx();
                        return NO_PRIMITIVE;

                    #endif
                }
                break;
                
            case APSME_ADD_GROUP_request:
                {
                    #ifndef I_SUPPORT_GROUP_ADDRESSING
                        params.APSME_ADD_GROUP_confirm.Status = GROUP_INVALID_PARAMETER;
                        params.APSME_ADD_GROUP_confirm.GroupAddress = params.APSME_ADD_GROUP_request.GroupAddress;
                        params.APSME_ADD_GROUP_confirm.Endpoint     = params.APSME_ADD_GROUP_request.Endpoint;
                        return APSME_ADD_GROUP_confirm;
                    #else
                        params.APSME_ADD_GROUP_confirm.Status       = AddGroup(params.APSME_ADD_GROUP_request.GroupAddress, params.APSME_ADD_GROUP_request.Endpoint);
                        params.APSME_ADD_GROUP_confirm.GroupAddress = params.APSME_ADD_GROUP_request.GroupAddress;
                        params.APSME_ADD_GROUP_confirm.Endpoint     = params.APSME_ADD_GROUP_request.Endpoint;
                        return APSME_ADD_GROUP_confirm;
                    #endif
                }
                break;
                
            case APSME_REMOVE_GROUP_request:
                {
                    #ifndef I_SUPPORT_GROUP_ADDRESSING
                        params.APSME_REMOVE_GROUP_confirm.Status            = GROUP_INVALID_PARAMETER;
                        params.APSME_REMOVE_GROUP_confirm.GroupAddress.Val  =  params.APSME_REMOVE_GROUP_request.GroupAddress.Val;
                        params.APSME_REMOVE_GROUP_confirm.Endpoint  =  params.APSME_REMOVE_GROUP_request.Endpoint;
                        return APSME_REMOVE_GROUP_confirm;
                    #else
                        params.APSME_REMOVE_GROUP_confirm.Status            = RemoveGroup(params.APSME_REMOVE_GROUP_request.GroupAddress.Val, params.APSME_REMOVE_GROUP_request.Endpoint);
                        params.APSME_REMOVE_GROUP_confirm.GroupAddress.Val  =  params.APSME_REMOVE_GROUP_request.GroupAddress.Val;
                        params.APSME_REMOVE_GROUP_confirm.Endpoint          =  params.APSME_REMOVE_GROUP_request.Endpoint;
                        return APSME_REMOVE_GROUP_confirm;
                    #endif
                }
                break;
                
            case APSME_REMOVE_ALL_GROUPS_request:
                {
                    #ifndef I_SUPPORT_GROUP_ADDRESSING
                        params.APSME_REMOVE_ALL_GROUPS_confirm.Status = GROUP_INVALID_PARAMETER;
                        params.APSME_REMOVE_ALL_GROUPS_confirm.Endpoint = params.APSME_REMOVE_ALL_GROUPS_request.Endpoint;
                        return APSME_REMOVE_ALL_GROUPS_confirm;
                    #else
                        params.APSME_REMOVE_ALL_GROUPS_confirm.Status   = RemoveGroup(0xFFFF, params.APSME_REMOVE_ALL_GROUPS_request.Endpoint);
                        params.APSME_REMOVE_ALL_GROUPS_confirm.Endpoint = params.APSME_REMOVE_ALL_GROUPS_request.Endpoint;
                        return APSME_REMOVE_ALL_GROUPS_confirm;   
                    #endif
                }
                break;

            case APSME_BIND_request:
                #ifndef I_SUPPORT_BINDINGS
                    params.APSME_BIND_confirm.Status = BIND_NOT_SUPPORTED;
                    return APSME_BIND_confirm;
                #else
                    // NOTE - The spec allows bindings to be created even if we are not
                    // associated.  However, it doesn't allow us to unbind...
                    {
                        SHORT_ADDR  srcShortAddress;
                        SHORT_ADDR  dstShortAddress;

                        if( APSFromLongToShort( &params.APSME_BIND_request.SrcAddr ) == FALSE )
                        {
                            params.APSME_BIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                            return APSME_BIND_confirm;
                        }
                        else
                        {
                            srcShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                        }

                        if( params.APSME_BIND_request.DstAddrMode != APS_ADDRESS_GROUP )
                        {
                            if( APSFromLongToShort( &params.APSME_BIND_request.DstAddr.LongAddr ) == FALSE )
                            {
                                params.APSME_BIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                                return APSME_BIND_confirm;
                            }
                            else
                            {
                                dstShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                            }
                        } 
                        else
                        {
                            dstShortAddress = params.APSME_BIND_request.DstAddr.ShortAddr;
                            params.APSME_BIND_request.DstEndpoint = 0xFE; // indicate it is group binding
                        }

                        params.APSME_BIND_confirm.Status = APSAddBindingInfo( srcShortAddress,
                            params.APSME_BIND_request.SrcEndpoint, params.APSME_BIND_request.ClusterId,
                            dstShortAddress, params.APSME_BIND_request.DstEndpoint );
                        return APSME_BIND_confirm;
                    }
                #endif
                break;

            case APSME_UNBIND_request:
                #ifndef I_SUPPORT_BINDINGS
                    // NOTE - This is a deviation from the spec.  The spec does not specify
                    // what to do with this primitive on an end device, only the Bind Request.
                    params.APSME_UNBIND_confirm.Status = BIND_NOT_SUPPORTED;
                    return APSME_UNBIND_confirm;
                #else
                    // NOTE - The spec allows bindings to be created even if we are not
                    // associated.  However, it doesn't allow us to unbind...
                    #ifdef I_AM_COORDINATOR
                    if (!ZigBeeStatus.flags.bits.bNetworkFormed)
                    #else
                    if (!ZigBeeStatus.flags.bits.bNetworkJoined)
                    #endif
                    params.APSME_UNBIND_confirm.Status = BIND_ILLEGAL_REQUEST;
                    return APSME_UNBIND_confirm;

                    {
                        SHORT_ADDR  srcShortAddress;
                        SHORT_ADDR  dstShortAddress;

                        if( APSFromLongToShort( &params.APSME_UNBIND_request.SrcAddr ) == FALSE )
                        {
                            params.APSME_UNBIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                            return APSME_UNBIND_confirm;
                        }
                        else
                        {
                            srcShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                        }

                        if( params.APSME_UNBIND_request.DstAddrMode != APS_ADDRESS_GROUP )
                        {
                            if( APSFromLongToShort( &params.APSME_UNBIND_request.DstAddr.LongAddr ) == FALSE )
                            {
                                params.APSME_UNBIND_confirm.Status = BIND_ILLEGAL_DEVICE;
                                return APSME_UNBIND_confirm;
                            }
                            else
                            {
                                dstShortAddress = currentAPSAddress.shortAddr; //currentNeighborRecord.shortAddr;
                            }
                        }
                        else
                        {
                            dstShortAddress = params.APSME_UNBIND_request.DstAddr.ShortAddr;
                            params.APSME_UNBIND_request.DstEndpoint = 0xFE; // indicate group binding
                        }

                        params.APSME_UNBIND_confirm.Status = APSRemoveBindingInfo( srcShortAddress,
                            params.APSME_UNBIND_request.SrcEndpoint, params.APSME_UNBIND_request.ClusterId,
                            dstShortAddress, params.APSME_UNBIND_request.DstEndpoint );
                        return APSME_UNBIND_confirm;
                    }
                #endif
                break;

		#ifdef I_SUPPORT_SECURITY
		
	        #ifndef I_AM_END_DEVICE  
        	case APSME_TRANSPORT_KEY_request:
            {
                BOOL nwkKeySecure = TRUE;
                LONG_ADDR *DstAddress;
                BYTE ActiveKeyIndex;
                
                BYTE useSecurity = params.APSME_TRANSPORT_KEY_request._UseSecurity;
                #ifdef I_SUPPORT_SECURITY_SPEC
                    if( INVALID_NEIGHBOR_KEY != (NTIndex = NWKLookupNodeByLongAddr(&(params.APSME_TRANSPORT_KEY_request.DestinationAddress))) )
                    {
                        if( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD &&
                            !currentNeighborRecord.bSecured )
                        {
                            nwkKeySecure = FALSE;
                            currentNeighborRecord.bSecured = TRUE;
                            #ifdef USE_EXTERNAL_NVM
                                PutNeighborRecord( neighborTable + (WORD)NTIndex * (WORD)sizeof(NEIGHBOR_RECORD), &currentNeighborRecord );
                            #else
                                PutNeighborRecord( &(neighborTable[NTIndex]), &currentNeighborRecord );
                            #endif
                        }
                    }
                #endif

                #if PROFILE_nwkSecureAllFrames
                    TxData++;   // reserve space for frame control
                #endif
                TxBuffer[TxData++] = APSCounter++;
                
                TxBuffer[TxData++] = APS_CMD_TRANSPORT_KEY;
                TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.KeyType;
                for(i = 0; i < 16; i++)
                {
                    TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.Key->v[i];
                    #ifdef I_AM_TRUST_CENTER 
                        currentNetworkKeyInfo.NetKey.v[i] = params.APSME_TRANSPORT_KEY_request.Key->v[i];
                    #endif
                }
                
                /* For ZigBee 2006:  Update the Key Sequence Number to match key bytes 
                 * since this is the way ZCP tells explicitily what key we are using the
                 * correct keys when we have not only key0 and Key1 but Key2 Key3 etc...
                 * the key number must now kept where as before it was on 0 or 1 
                */
              #ifdef I_AM_TRUST_CENTER
                currentNetworkKeyInfo.SeqNumber.v[0] = params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber;
                currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;
                GetNwkActiveKeyNumber(&ActiveKeyIndex);
                if ( ((firstKeyHasBeenSent && currentNetworkKeyInfo.SeqNumber.v[0] != 0x00)) ||
                       previousKeyNotSeq0 )   /* Skip first */
                {
                    previousKeyNotSeq0 = TRUE;
                    if(ActiveKeyIndex == 0x01)
                    {
                        /* put it in the not currently active slot */
                        #ifdef USE_EXTERNAL_NVM
        			        SetSecurityKey(1, currentNetworkKeyInfo);
            	        #else
            		        PutNwkKeyInfo( &networkKeyInfo[1] , &currentNetworkKeyInfo );
            	        #endif      
                    }
                    else
                    {
                        /* put it in the not currrently active slot */
                        #ifdef USE_EXTERNAL_NVM
        			        SetSecurityKey(0, currentNetworkKeyInfo);
            	        #else
                            PutNwkKeyInfo( &networkKeyInfo[0] , &currentNetworkKeyInfo );  
                        #endif   
                    }
               }
             #endif
                /* ZigBee 2006: With Mindteck's Application we need to free this memory here because
                 * they don't do it in their application 
                */
                
                switch( params.APSME_TRANSPORT_KEY_request.KeyType )
                {
                    case 0x01:  // Network key
                    {
                        LONG_ADDR   SrcAddr;
                        TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber;
                        
                        /* For Zigbee 2006:  If not broadcast then use proper longAddress */
                        if(params.APSME_TRANSPORT_KEY_request._DstAddrMode != 0xff  &&
                           params.APSME_TRANSPORT_KEY_request._DstAddrMode != 0xfd )
                        {
                            for(i = 0; i < 8; i++)
                            {
                                TxBuffer[TxData++] = params.APSME_TRANSPORT_KEY_request.DestinationAddress.v[i];
                            }
                        }
                        else        /* if broadcasting then destAddr must be 0x000... */
                        {
                            apsStatus.flags.bits.bBroadcastingNetworkKey = 1;
                            for(i = 0; i < 8; i++)
                            {
                                TxBuffer[TxData++] = 0x00;
                            }
                        }
                         
                        /* Added at NTS - preserve the trust center's address as source */   
                        GetMACAddress(&SrcAddr);  
                        for(i = 0; i < 8; i++)
                        {
                            #ifdef I_AM_TRUST_CENTER 
                                TxBuffer[TxData++] = SrcAddr.v[i];
                            #else
                                TxBuffer[TxData++] = transportKeySrcAddr.v[i];
                            #endif
                        }
                        break;
                    }
                }

                if( params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent )
                {
                    DstAddress = &(params.APSME_TRANSPORT_KEY_request.ParentAddress);
                }
                else
                {
                    DstAddress = &(params.APSME_TRANSPORT_KEY_request.DestinationAddress);
                }

                if( !APSFillSecurityRequest(DstAddress, (!securityStatus.flags.bits.nwkSecureAllFrames && nwkKeySecure)) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }

                #ifdef I_SUPPORT_RES_SECURITY
                    #ifdef PROFILE_nwkSecureAllFrames

                    #else
                        if( nwkKeySecure )
                        {
                            if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]), (TX_HEADER_START-TxHeader), ID_NetworkKey, FALSE) )
                            {
                                ZigBeeUnblockTx();
                                return NO_PRIMITIVE;
                            }
                        }
                    #endif
                #endif

                #ifdef I_SUPPORT_SECURITY_SPEC
                    params.NLDE_DATA_request.SecurityEnable = (securityStatus.flags.bits.nwkSecureAllFrames && nwkKeySecure);
                #else
                    if( useSecurity )
                    {
                        params.NLDE_DATA_request.SecurityEnable = TRUE;
                    }
                    else
                    {
                        params.NLDE_DATA_request.SecurityEnable = FALSE;
                    }
                #endif
                
                /* from now on update keySeguence number in ROM to match with switching
                 * keys that are sent 
                */
                firstKeyHasBeenSent = TRUE;
                
                return NLDE_DATA_request;

            }
			#endif  // I_AM_END_DEVICE

		    #if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER)
        	case APSME_UPDATE_DEVICE_request:
            {
                BOOL APSSecure = TRUE;
                
                if( securityStatus.flags.bits.nwkSecureAllFrames )
                {
                    APSSecure = FALSE;
                }

                #if PROFILE_nwkSecureAllFrames
                    TxData++;
                #endif
                /* For Zigbee 2006: Need the ApsCounter here */
                TxBuffer[TxData++] = APSCounter++;
                
                TxBuffer[TxData++] = APS_CMD_UPDATE_DEVICE;
                for(i = 0; i < 8; i++)
                {
                    TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.DeviceAddress.v[i];
                    /* For ZigBee 2006 added at NTS - keep track of my child's key progress */
                    #ifdef I_AM_ROUTER
                        updateDevAPSAddr.longAddr.v[i] = params.APSME_UPDATE_DEVICE_request.DeviceAddress.v[i];
                    #endif
                }
                TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.DeviceShortAddress.byte.LSB;
                TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.DeviceShortAddress.byte.MSB;
                
                /* Added at NTS - keep track of child that is been authenticated */
                #ifdef I_AM_ROUTER
                    updateDevAPSAddr.shortAddr  = params.APSME_UPDATE_DEVICE_request.DeviceShortAddress;
                #endif
                
                TxBuffer[TxData++] = params.APSME_UPDATE_DEVICE_request.Status;

                if( !APSFillSecurityRequest(&(params.APSME_UPDATE_DEVICE_request.DestAddress), APSSecure) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]), (TX_HEADER_START-TxHeader), ID_NetworkKey, FALSE) )
                        {
                            ZigBeeUnblockTx();
                            return NO_PRIMITIVE;
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                /* for ZigBee 2006: Added at NTS - Wait for child's key, before we can send an APS_Reove_device_request */
                #ifdef I_AM_ROUTER
                    waitingForKey = TRUE;
                #endif
                
                params.NLDE_DATA_request.DstAddrMode = 0x02;
                return NLDE_DATA_request;
            }
            break;
            
        	case APSME_REMOVE_DEVICE_request:
            {
                /* for ZigBee 2006: Added at NTS - if we are in authentication period do not send this command - silent */
                #if defined(I_SUPPORT_SECURITY) && defined(I_AM_ROUTER) 
                    if(waitingForKey  && 
                            !memcmp( (void *)&updateDevAPSAddr.longAddr, (void *)&params.APSME_REMOVE_DEVICE_request.ChildAddress, 8  ) ) 
                    {
                        return NO_PRIMITIVE;
                    }
                #endif 
                  
                #if PROFILE_nwkSecureAllFrames
                    TxData++;
                #endif
                /* For Zigbee 2006: Need the ApsCounter here */
                TxBuffer[TxData++] = APSCounter++;
                
                TxBuffer[TxData++] = APS_CMD_REMOVE_DEVICE;
                for(i = 0; i < 8; i++)
                {
                    TxBuffer[TxData++] = params.APSME_REMOVE_DEVICE_request.ChildAddress.v[i];
                }
                if( !APSFillSecurityRequest(&(params.APSME_REMOVE_DEVICE_request.ParentAddress), !securityStatus.flags.bits.nwkSecureAllFrames) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]), (TX_HEADER_START-TxHeader), ID_NetworkKey, FALSE) )
                        {
                            ZigBeeUnblockTx();
                            return NO_PRIMITIVE;
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                return NLDE_DATA_request;
            }
			#endif // I_AM_COORDINATOR || I_AM_ROUTER

            #if !defined(I_AM_TRUST_CENTER)
        	case APSME_REQUEST_KEY_request:
            {
                #if PROFILE_nwkSecureAllFrames
                    TxData++;
                #endif
                /* Zigbee 2006 needs this APSCounter */
                TxBuffer[TxData++] = APSCounter++;
                
                TxBuffer[TxData++] = APS_CMD_REQUEST_KEY;
                TxBuffer[TxData++] = params.APSME_REQUEST_KEY_request.KeyType;

                if( 0x01 != params.APSME_REQUEST_KEY_request.KeyType )
                {
                    for(i = 0; i < 8; i++)
                    {
                        TxBuffer[TxData++] = params.APSME_REQUEST_KEY_request.PartnerAddress.v[i];
                    }
                }
                if( !APSFillSecurityRequest(&(params.APSME_REQUEST_KEY_request.DestAddress), !securityStatus.flags.bits.nwkSecureAllFrames) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]), (TX_HEADER_START-TxHeader), ID_NetworkKey, FALSE) )
                        {
                            ZigBeeUnblockTx();
                            return NO_PRIMITIVE;
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                return NLDE_DATA_request;
            }
            #endif

			#if defined(I_AM_TRUST_CENTER)
        	case APSME_SWITCH_KEY_request:
            {
                zdoStatus.KeySeq = params.APSME_SWITCH_KEY_request.KeySeqNumber;
                zdoStatus.SwitchKeyTick = TickGet();
                zdoStatus.flags.bits.bSwitchKey = 1;
                
                #if PROFILE_nwkSecureAllFrames
                    TxData++;
                #endif
                /* Zigbee 2006 needs this APSCounter */
                TxBuffer[TxData++] = APSCounter++;
                
                TxBuffer[TxData++] = APS_CMD_SWITCH_KEY;
                TxBuffer[TxData++] = params.APSME_SWITCH_KEY_request.KeySeqNumber;
                    
                apsStatus.flags.bits.bBroadcastingSwitchKey = 1;

                if( !APSFillSecurityRequest(&(params.APSME_SWITCH_KEY_request.DestAddress), !securityStatus.flags.bits.nwkSecureAllFrames) )
                {
                    ZigBeeUnblockTx();
                    return NO_PRIMITIVE;
                }
                #ifdef I_SUPPORT_RES_SECURITY
                    #if PROFILE_nwkSecureAllFrames

                    #else
                        if( !DataEncrypt(TxBuffer, &TxData, &(TxBuffer[TxHeader+1]), (TX_HEADER_START-TxHeader), ID_NetworkKey, FALSE) )
                        {
                            ZigBeeUnblockTx();
                            return NO_PRIMITIVE;
                        }
                    #endif
                #endif
                params.NLDE_DATA_request.SecurityEnable = securityStatus.flags.bits.nwkSecureAllFrames;

                return NLDE_DATA_request;
            }

			#endif //I_AM_TRUST_CENTER

		#endif   // I_SUPPORT_SECURITY
		
		    default:
		        break;
        }
    }

    return NO_PRIMITIVE;
}

/*********************************************************************
 * Function:        BOOL    APSFillSecurityRequest(INPUT LONG_ADDR *DestAddr, INPUT BOOL bSecuredFrame)
 *
 * PreCondition:    None
 *
 * Input:           DestAddr - pointer to destination long address
 *                  bSecuredFrame - boolean to specify if secure the frame
 *
 * Output:          TRUE - information successfully filled
 *                  FALSE - failed to fill the information to send data
 *
 * Side Effects:    None
 *
 * Overview:        Fill the parameters for NLDE_DATA_request for Security APS commands
 *
 * Note:            None
 ********************************************************************/
#ifdef I_SUPPORT_SECURITY
BOOL    APSFillSecurityRequest(INPUT LONG_ADDR *DestAddr, INPUT BOOL bSecuredFrame)
{
    APS_FRAME_CONTROL   apsFrameControl;
    BYTE i;
    
    if( !ZigBeeReady() )
    {
        return FALSE;
    }
 
    ZigBeeBlockTx();
    
    /* Zigbee 2006: Check if  long address is 0x0000... See what we are broadcasting 
     *  it will be either transport_key_requests or switch_key_requests.
     */
    for(i = 0; i < 8; i++)
    {
            currentAPSAddress.longAddr.v[i] = 0x00;
    }
           
    if( (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&(DestAddr->v[0]), 8 )) &&
    apsStatus.flags.bits.bBroadcastingNetworkKey == 1 )
    {
        if(params.APSME_TRANSPORT_KEY_request._DstAddrMode == 0xfd)
            currentAPSAddress.shortAddr.Val = 0xfffd;
        else 
            currentAPSAddress.shortAddr.Val = 0xffff;
            
        apsStatus.flags.bits.bBroadcastingNetworkKey  = 0;
        goto finish_req;
    }
    else if( (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&(DestAddr->v[0]), 8 )) &&
    apsStatus.flags.bits.bBroadcastingSwitchKey == 1 )
    {
         currentAPSAddress.shortAddr.Val = 0xfffd;
         apsStatus.flags.bits.bBroadcastingSwitchKey  = 0;
         goto finish_req;
    }
    
    if( !APSFromLongToShort(DestAddr) )
    {
        /* This will send an incorrect msg to Coordinator to at least alert operator 
         */
        currentAPSAddress.shortAddr.Val = 0;
    }
    
    /* Zigbee 2006: if this was to a broadcast long address then make the short address 0xffff */
finish_req:
    params.NLDE_DATA_request.DstAddr.Val = currentAPSAddress.shortAddr.Val;
    params.NLDE_DATA_request.NsduHandle = NLME_GET_nwkBCSN();
    params.NLDE_DATA_request.BroadcastRadius    = DEFAULT_RADIUS;
    params.NLDE_DATA_request.DiscoverRoute      = 0; 

    apsFrameControl.Val = APS_FRAME_COMMAND;
    apsFrameControl.bits.deliveryMode = APS_DELIVERY_DIRECT;
    apsFrameControl.bits.acknowledgeRequest = 0;
    if( bSecuredFrame )
    {
        apsFrameControl.bits.security = 1;
    }
    else
    {
        apsFrameControl.bits.security = 0;
    }

    #if PROFILE_nwkSecureAllFrames
        TxBuffer[TX_DATA_START] = apsFrameControl.Val;
    #else
        TxBuffer[TxHeader--] = apsFrameControl.Val;
    #endif

    return TRUE;
}
#endif



/*********************************************************************
 * Function:        BYTE APSAddBindingInfo(SHORT_ADDR srcAddr,
 *                                      BYTE srcEP,
 *                                      WORD_VAL clusterID,
 *                                      SHORT_ADDR destAddr,
 *                                      BYTE destEP)
 *
 *
 * PreCondition:    srcAddr and destAddr must be valid addresses of
 *                  devices on the network
 *
 * Input:           srcAddr     - source short address
 *                  srcEP       - source end point
 *                  clusterID   - cluster id
 *                  destAddr    - destination short address
 *                  destEP      - destination end point
 *
 * Output:          SUCCESS if an entry was created or already exists
 *                  TABLE_FULL if the binding table is full
 *                  FALSE if illegal binding attempted
 *
 * Side Effects:    None
 *
 * Overview:        Creates/updates a binding entry for given
 *                  set of data.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
BYTE APSAddBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID,
                     SHORT_ADDR destAddr, BYTE destEP )
{
    BYTE            bindingDestIndex    = 0;
    BYTE            bindingSrcIndex     = 0;
    BYTE            bindingMapSourceByte;
    BYTE            bindingMapUsageByte;
    BOOL            Found           = FALSE;
    BYTE            oldBindingLink;
    BINDING_RECORD  tempBindingRecord;

    // See if a list for the source data already exists.
    while ((bindingSrcIndex < MAX_BINDINGS) && !Found)
    {
        GetBindingSourceMap( &bindingMapSourceByte, bindingSrcIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingSrcIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingSrcIndex ))
        {
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingSrcIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingSrcIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );

            if ((currentBindingRecord.shortAddr.Val == srcAddr.Val) &&
                (currentBindingRecord.endPoint == srcEP) &&
                (currentBindingRecord.clusterID.Val == clusterID.Val))
            {
                Found = TRUE;
                break;
            }
        }
        bindingSrcIndex ++;
    }

    // If there was no source data list, create one.
    if (!Found)
    {
        bindingSrcIndex = 0;
        GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
        while ((bindingSrcIndex < MAX_BINDINGS) &&
               BindingIsUsed( bindingMapUsageByte,  bindingSrcIndex ))
        {
            bindingSrcIndex ++;
            GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
        }
        if (bindingSrcIndex == MAX_BINDINGS)
        {
            return BIND_TABLE_FULL;
        }

        #ifdef USE_EXTERNAL_NVM
            pCurrentBindingRecord = apsBindingTable + (WORD)(bindingSrcIndex) * (WORD)sizeof(BINDING_RECORD);
        #else
            pCurrentBindingRecord = &apsBindingTable[bindingSrcIndex];
        #endif
        currentBindingRecord.shortAddr = srcAddr;
        currentBindingRecord.endPoint = srcEP;
        currentBindingRecord.clusterID.Val = clusterID.Val;
        currentBindingRecord.nextBindingRecord = END_BINDING_RECORDS;
    }
    // If we found the source data, make sure the destination link
    // doesn't already exist.  Leave currentBindingRecord as the source node.
    else
    {
        tempBindingRecord.nextBindingRecord = currentBindingRecord.nextBindingRecord;
        while (tempBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
        {
            #ifdef USE_EXTERNAL_NVM
                GetBindingRecord(&tempBindingRecord, apsBindingTable + (WORD)tempBindingRecord.nextBindingRecord * (WORD)sizeof(BINDING_RECORD) );
            #else
                GetBindingRecord(&tempBindingRecord, &apsBindingTable[tempBindingRecord.nextBindingRecord] );
            #endif
            if ((tempBindingRecord.shortAddr.Val == destAddr.Val) &&
                 (tempBindingRecord.endPoint == destEP)) 
            {
                return SUCCESS;  // already exists
            }
        }
    }

    // Make sure there is room for a new destination node. Make sure we avoid the
    // node that we're trying to use for the source in case it's a new one!
    bindingDestIndex = 0;
    GetBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    while (((bindingDestIndex < MAX_BINDINGS) &&
            BindingIsUsed( bindingMapUsageByte,  bindingDestIndex )) ||
           (bindingDestIndex == bindingSrcIndex))
    {
        bindingDestIndex ++;
        GetBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    }
    if (bindingDestIndex == MAX_BINDINGS)
    {
        return BIND_TABLE_FULL;
    }

    // Update the source node to point to the new destination node.
    oldBindingLink = currentBindingRecord.nextBindingRecord;
    currentBindingRecord.nextBindingRecord = bindingDestIndex;
    PutBindingRecord( pCurrentBindingRecord, &currentBindingRecord );

    // Create the new binding record, inserting it at the head of the destinations.
    currentBindingRecord.shortAddr = destAddr;
    currentBindingRecord.endPoint = destEP;
    
    currentBindingRecord.nextBindingRecord = oldBindingLink;
    #ifdef USE_EXTERNAL_NVM
        pCurrentBindingRecord = apsBindingTable + (WORD)(bindingDestIndex) * (WORD)sizeof(BINDING_RECORD);
    #else
        pCurrentBindingRecord = &apsBindingTable[bindingDestIndex];
    #endif
    PutBindingRecord( pCurrentBindingRecord, &currentBindingRecord );

    // Mark the source node as used.  Is redundant if it already existed, but if there
    // was room for the source but not the destination, we don't want to take the
    // source node.
    GetBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
    MarkBindingUsed( bindingMapUsageByte, bindingSrcIndex );
    PutBindingUsageMap( &bindingMapUsageByte, bindingSrcIndex );
    GetBindingSourceMap( &bindingMapSourceByte, bindingSrcIndex );
    MarkBindingUsed( bindingMapSourceByte, bindingSrcIndex );
    PutBindingSourceMap( &bindingMapSourceByte, bindingSrcIndex );

    // Mark the destination node as used, but not a source node
    GetBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    MarkBindingUsed( bindingMapUsageByte, bindingDestIndex );
    PutBindingUsageMap( &bindingMapUsageByte, bindingDestIndex );
    GetBindingSourceMap( &bindingMapSourceByte, bindingDestIndex );
    MarkBindingUnused( bindingMapSourceByte, bindingDestIndex );
    PutBindingSourceMap( &bindingMapSourceByte, bindingDestIndex );

    return SUCCESS;
}
#endif


/*********************************************************************
 * Function:        void APSClearAPSAddressTable( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    currentAPSAddress is destroyed
 *
 * Overview:        This function sets the entire APS address map table
 *                  to all 0xFF.
 *
 * Note:            None
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0
void APSClearAPSAddressTable( void )
{
    BYTE    i;

    //memset( &currentAPSAddress, 0xFF, sizeof(APS_ADDRESS_MAP) );
    for(i = 0; i < 8; i++)
    {
        currentAPSAddress.longAddr.v[i] = 0xFF;
    }
    currentAPSAddress.shortAddr.v[0] = 0xFF;
    currentAPSAddress.shortAddr.v[1] = 0xFF;
    
    for (i=0; i<MAX_APS_ADDRESSES; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress );
        #else
            PutAPSAddress( &apsAddressMap[i], &currentAPSAddress );
        #endif
    }
}
#endif

/*********************************************************************
 * Function:        BYTE APSGet( void )
 *
 * PreCondition:    Must be called from the NLDE_DATA_indication
 *                  primitive.
 *
 * Input:           none
 *
 * Output:          One byte from the msdu if the length is greater
 *                  than 0; otherwise, 0.
 *
 * Side Effects:    The msdu pointer is incremented to point to the
 *                  next byte, and msduLength is decremented.
 *
 * Overview:        This function returns the next byte from the
 *                  msdu.
 *
 * Note:            None
 ********************************************************************/

BYTE APSGet( void )
{
    if (params.NLDE_DATA_indication.NsduLength == 0)
    {
        return 0;
    }
    else
    {
        params.NLDE_DATA_indication.NsduLength--;
        return *params.NLDE_DATA_indication.Nsdu++;
    }
}

/*********************************************************************
 * Function:        BYTE APSRemoveBindingInfo(SHORT_ADDR srcAddr,
 *                                      BYTE srcEP,
 *                                      WORD_VAL clusterID,
 *                                      SHORT_ADDR destAddr,
 *                                      BYTE destEP)
 *
 *
 * PreCondition:    srcAddr and destAddr must be valid addresses of
 *                  devices on the network
 *
 * Input:           srcAddr     - source short address
 *                  srcEP       - source end point
 *                  clusterID   - cluster id
 *                  destAddr    - destination short address
 *                  destEP      - destination EP
 *
 * Output:          SUCCESS if the entry was removed
 *                  INVALID_BINDING if the binding did not exist
 *
 * Side Effects:    None
 *
 * Overview:        Removes a binding entry for given set of data.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
BYTE APSRemoveBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID,
                    SHORT_ADDR destAddr, BYTE destEP )
{
    BYTE    bindingMapUsageByte;
    BYTE    currentKey;
    BYTE    nextKey;
    BYTE    previousKey;
    BYTE    sourceKey;

    if ((sourceKey = LookupSourceBindingInfo( srcAddr, srcEP, clusterID)) ==
            END_BINDING_RECORDS)
    {
        return BIND_INVALID_BINDING;
    }

    previousKey = sourceKey;
    #ifdef USE_EXTERNAL_NVM
        pCurrentBindingRecord = apsBindingTable + (WORD)(previousKey) * (WORD)sizeof(BINDING_RECORD);
    #else
        pCurrentBindingRecord = &apsBindingTable[previousKey];
    #endif
    GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
    while (currentBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
    {
        currentKey = currentBindingRecord.nextBindingRecord;
        #ifdef USE_EXTERNAL_NVM
            pCurrentBindingRecord = apsBindingTable + (WORD)(currentKey) * (WORD)sizeof(BINDING_RECORD);
        #else
            pCurrentBindingRecord = &apsBindingTable[currentKey];
        #endif
        GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
        if ((currentBindingRecord.shortAddr.Val == destAddr.Val) &&
                (currentBindingRecord.endPoint == destEP))
        {
            nextKey = currentBindingRecord.nextBindingRecord;

            // Go back and get the previous record, and point it to the record
            // that the deleted record was pointing to.
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(previousKey) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[previousKey];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
            currentBindingRecord.nextBindingRecord = nextKey;
            PutBindingRecord( pCurrentBindingRecord, &currentBindingRecord );

            // Update the usage map to delete the current node.
            GetBindingUsageMap( &bindingMapUsageByte, currentKey );
            MarkBindingUnused( bindingMapUsageByte, currentKey );
            PutBindingUsageMap( &bindingMapUsageByte, currentKey );

            // If we just deleted the only destination, delete the source as well.
            if ((sourceKey == previousKey) && (nextKey == END_BINDING_RECORDS))
            {
                GetBindingUsageMap( &bindingMapUsageByte, sourceKey );
                MarkBindingUnused( bindingMapUsageByte, sourceKey );
                PutBindingUsageMap( &bindingMapUsageByte, sourceKey );
            }

            return SUCCESS;
         }
         else
         {
             previousKey = currentKey;
         }
     }
     return BIND_INVALID_BINDING;
}
#endif

/*********************************************************************
 * Function:        BYTE LookupAPSAddress( LONG_ADDR *longAddr )
 *
 * PreCondition:    None
 *
 * Input:           longAddr - pointer to long address to match
 *
 * Output:          TRUE - a corresponding short address was found and
 *                      is held in currentAPSAddress.shortAddr
 *                  FALSE - a corresponding short address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map for the short address
 *                  of a given long address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0

BOOL LookupAPSAddress( LONG_ADDR *longAddr )
{
    BYTE    i;
  
    for (i=0; i<apscMaxAddrMapEntries; i++)
    {
        
        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
        #endif

        if (currentAPSAddress.shortAddr.Val != 0xFFFF)
        {
            if ( !memcmp((void*)longAddr, (void*)&currentAPSAddress.longAddr, (BYTE)(sizeof(LONG_ADDR))) )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
#endif

/*********************************************************************
 * Function:        BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap)
 *
 * PreCondition:    None
 *
 * Input:           AddressMap - pointer to the APS_ADDRESS_MAP to be saved
 *
 * Output:          TRUE - operation successful
 *                  FALSE - no more APS_ADDRESS_MAP slot
 *
 * Side Effects:    address map modified to include the new item
 *
 * Overview:        Searches the APS address map, find empty slot to save
 *                  the new match between short address and long address
 *
 * Note:            None
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0

BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap)
{
    BYTE i;
    APS_ADDRESS_MAP tmpMap = *AddressMap;

    if( LookupAPSLongAddress(&(AddressMap->shortAddr)) )
    {
        return TRUE;
    }

    for( i = 0; i < apscMaxAddrMapEntries; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
        #endif
        if (currentAPSAddress.shortAddr.Val == 0xFFFF)
        {
            #ifdef USE_EXTERNAL_NVM
                PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &tmpMap );
            #else
                PutAPSAddress( &apsAddressMap[i], &tmpMap );
            #endif
            return TRUE;
        }
    }
    
    /* APS Table is full so return an error indication */
    return FALSE;
}
#endif

/*********************************************************************
 * Function:        BYTE LookupAPSLongAddress( LONG_ADDR *longAddr )
 *
 * PreCondition:    None
 *
 * Input:           shortAddr - pointer to short address to match
 *
 * Output:          TRUE - a corresponding long address was found and
 *                      is held in currentAPSAddress.shortAddr
 *                  FALSE - a corresponding long address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map for the long address
 *                  of a given short address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/
#if MAX_APS_ADDRESSES > 0

BOOL LookupAPSLongAddress(INPUT SHORT_ADDR *shortAddr)
{
    BYTE i;

    for(i = 0; i < apscMaxAddrMapEntries; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
        #else
            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
        #endif
        if( currentAPSAddress.shortAddr.Val != 0xFFFF )
        {
            
            if( currentAPSAddress.shortAddr.Val == shortAddr->Val)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}
#endif


/*********************************************************************
 * Function:        BYTE LookupSourceBindingInfo( SHORT_ADDR srcAddr,
 *                                          BYTE srcEP,
 *                                          WORD_VAL clusterID)
 *
 * PreCondition:    None
 *
 * Input:           srcAddr     - short address of source node
 *                  srcEP       - source end point
 *                  clusterID   - cluster id
 *
 * Output:          key to the source binding record if matching record found
 *                  END_BINDING_RECORDS otherwise
 *
 * Side Effects:    pCurrentBindingRecord and currentBindingRecord
 *                  are set to the source binding record
 *
 * Overview:        Searches binding table for matching source binding.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
BYTE LookupSourceBindingInfo( SHORT_ADDR srcAddr, BYTE srcEP, WORD_VAL clusterID )
{
    BYTE            bindingIndex    = 0;
    BYTE            bindingMapSourceByte;
    BYTE            bindingMapUsageByte;

    while (bindingIndex < MAX_BINDINGS)
    {
        GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingIndex ))
        {
           
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
         
            // If we find the matching source, we're done
            if ((currentBindingRecord.shortAddr.Val == srcAddr.Val) &&
                (currentBindingRecord.endPoint == srcEP) &&
                (currentBindingRecord.clusterID.Val == clusterID.Val))
            {
                return bindingIndex;
            }
        }
        bindingIndex ++;
    }

    // We didn't find a match, so return an error condition
    return END_BINDING_RECORDS;
}
#endif

/*********************************************************************
 * Function:        void RemoveAllBindings(SHORT_ADDR shortAddr)
 *
 * PreCondition:    none
 *
 * Input:           shortAddr - Address of a node on the network
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Searches binding table and removes all entries
 *                  associated with the address shortAddr.  Nodes are
 *                  removed by clearing the usage flag in the usage
 *                  map and fixing up any destination links.  If the
 *                  node is a source node, then all associated
 *                  destination nodes are also removed.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)
void RemoveAllBindings(SHORT_ADDR shortAddr)
{
    BYTE                bindingIndex    = 0;
    BYTE                bindingMapSourceByte;
    BYTE                bindingMapUsageByte;
    BINDING_RECORD      nextRecord;
    #ifdef USE_EXTERNAL_NVM
        WORD                pPreviousRecord;
    #else
        ROM BINDING_RECORD  *pPreviousRecord;
    #endif

    while (bindingIndex < MAX_BINDINGS)
    {
        // Get the maps for each check, in case we overwrote them removing other entries
        GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
        GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );

        if (BindingIsUsed( bindingMapUsageByte,  bindingIndex ) &&
            BindingIsUsed( bindingMapSourceByte, bindingIndex ))
        {
            // Read the source node record into RAM.
            #ifdef USE_EXTERNAL_NVM
                pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
            #else
                pCurrentBindingRecord = &apsBindingTable[bindingIndex];
            #endif
            GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );

            // See if the source node address matches the address we are trying to find
            if (currentBindingRecord.shortAddr.Val == shortAddr.Val)
            {
                // Remove this source node and all destination nodes
                MarkBindingUnused( bindingMapUsageByte, bindingIndex );
                PutBindingUsageMap( &bindingMapUsageByte, bindingIndex );
                MarkBindingUnused( bindingMapSourceByte, bindingIndex );
                PutBindingSourceMap( &bindingMapSourceByte, bindingIndex );
                while (currentBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
                {
                    // Read the destination node record into RAM.
                    GetBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                    MarkBindingUnused( bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                    PutBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                    #ifdef USE_EXTERNAL_NVM
                        GetBindingRecord(&currentBindingRecord, apsBindingTable + (WORD)currentBindingRecord.nextBindingRecord * (WORD)sizeof(BINDING_RECORD) );
                    #else
                        GetBindingRecord(&currentBindingRecord, &apsBindingTable[currentBindingRecord.nextBindingRecord] );
                    #endif
                }
            }
            else
            {
                #ifdef USE_EXTERNAL_NVM
                    pPreviousRecord = apsBindingTable + (WORD)bindingIndex * (WORD)sizeof(BINDING_RECORD);
                #else
                    pPreviousRecord = &apsBindingTable[bindingIndex];
                #endif

                // See if any of the destination nodes in this list matches the address
                while (currentBindingRecord.nextBindingRecord != END_BINDING_RECORDS)
                {
                    // Read the destination node record into RAM.
                    #ifdef USE_EXTERNAL_NVM
                        pCurrentBindingRecord = apsBindingTable + (WORD)(currentBindingRecord.nextBindingRecord) * (WORD)sizeof(BINDING_RECORD);
                    #else
                        pCurrentBindingRecord = &apsBindingTable[currentBindingRecord.nextBindingRecord];
                    #endif
                    GetBindingRecord(&nextRecord, pCurrentBindingRecord );
                    if (nextRecord.shortAddr.Val == shortAddr.Val)
                    {
                        // Remove the destination node and patch up the list
                        GetBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                        MarkBindingUnused( bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                        PutBindingUsageMap( &bindingMapUsageByte, currentBindingRecord.nextBindingRecord );
                        currentBindingRecord.nextBindingRecord = nextRecord.nextBindingRecord;
                        PutBindingRecord( pPreviousRecord, &currentBindingRecord );
                    }
                    else
                    {
                        // Read the next destination node record into RAM.
                        pPreviousRecord = pCurrentBindingRecord;
                        #ifdef USE_EXTERNAL_NVM
                            pCurrentBindingRecord = apsBindingTable + (WORD)(currentBindingRecord.nextBindingRecord) * (WORD)sizeof(BINDING_RECORD);
                        #else
                            pCurrentBindingRecord = &apsBindingTable[currentBindingRecord.nextBindingRecord];
                        #endif
                        GetBindingRecord(&currentBindingRecord, pCurrentBindingRecord );
                    }
                }

                // If we just deleted the only destination, delete the source as well.
                // Read the source node record into RAM.
                #ifdef USE_EXTERNAL_NVM
                    pCurrentBindingRecord = apsBindingTable + (WORD)(bindingIndex) * (WORD)sizeof(BINDING_RECORD);
                #else
                    pCurrentBindingRecord = &apsBindingTable[bindingIndex];
                #endif
                GetBindingRecord( &currentBindingRecord, pCurrentBindingRecord );
                if (currentBindingRecord.nextBindingRecord == END_BINDING_RECORDS)
                {
                    GetBindingUsageMap( &bindingMapUsageByte, bindingIndex );
                    MarkBindingUnused( bindingMapUsageByte, bindingIndex );
                    PutBindingUsageMap( &bindingMapUsageByte, bindingIndex );
                    GetBindingSourceMap( &bindingMapSourceByte, bindingIndex );
                    MarkBindingUnused( bindingMapSourceByte, bindingIndex );
                    PutBindingSourceMap( &bindingMapSourceByte, bindingIndex );
                }
            }
        }
        bindingIndex ++;
    }
}
#endif

/*********************************************************************
 * Function:        BOOL APSFromShortToLong(INPUT SHORT_ADDR *ShortAddr)
 *
 * PreCondition:    None
 *
 * Input:           ShortAddr - pointer to short address to match long address
 *
 * Output:          TRUE - a corresponding long address was found and
 *                      is held in currentAPSAddress.longAddr
 *                  FALSE - a corresponding long address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map and neighbor table for the long address
 *                  of a given short address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/


BOOL APSFromShortToLong(INPUT SHORT_ADDR *ShortAddr)
{
    #if MAX_APS_ADDRESSES > 0
        if( LookupAPSLongAddress(ShortAddr) )
        {
            return TRUE;
        }
    #endif
    
    if( INVALID_NEIGHBOR_KEY != NWKLookupNodeByShortAddrVal(ShortAddr->Val) )
    {
        if( currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_CHILD ||
            currentNeighborRecord.deviceInfo.bits.Relationship == NEIGHBOR_IS_PARENT )
        {
            currentAPSAddress.longAddr = currentNeighborRecord.longAddr;
            return TRUE;
        }
    }

    return FALSE;

}


/*********************************************************************
 * Function:        BOOL APSFromLongToShort(INPUT SHORT_ADDR *LongAddr)
 *
 * PreCondition:    None
 *
 * Input:           LongAddr - pointer to long address to match short address
 *
 * Output:          TRUE - a corresponding short address was found and
 *                      is held in currentAPSAddress.shortAddr
 *                  FALSE - a corresponding short address was not found
 *
 * Side Effects:    currentAPSAddress is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the APS address map and neighbor table for the short address
 *                  of a given long address.
 *
 * Note:            The end application is responsible for populating
 *                  this table.
 ********************************************************************/


BOOL APSFromLongToShort(INPUT LONG_ADDR *LongAddr)
{
    #if MAX_APS_ADDRESSES > 0
        if( LookupAPSAddress(LongAddr) )
        {
            return TRUE;
        }
    #endif

    if( INVALID_NEIGHBOR_KEY != NWKLookupNodeByLongAddr(LongAddr) )
    {
        currentAPSAddress.shortAddr = currentNeighborRecord.shortAddr;
        return TRUE;
    }

    return FALSE;
}



BOOL    DuplicatePacket(INPUT SHORT_ADDR SrcAddress, INPUT BYTE currentAPSCounter)
{
    BYTE i;
    BYTE updateIndex = MAX_DUPLICATE_TABLE;
    
    apsStatus.flags.bits.bDuplicateTable = 1;
    for(i = 0; i < MAX_DUPLICATE_TABLE; i++)
    {
        if( SrcAddress.Val == apsDuplicateTable[i].SrcAddress.Val &&
            currentAPSCounter == apsDuplicateTable[i].APSCounter )
        {
            apsDuplicateTable[i].StartTick = TickGet();
            if( updateIndex < MAX_DUPLICATE_TABLE )
            {
                apsDuplicateTable[updateIndex].SrcAddress.Val = 0xFFFF;
            }
            return TRUE;
        }
        
        if( updateIndex == MAX_DUPLICATE_TABLE && apsDuplicateTable[i].SrcAddress.Val == 0xFFFF )
        {
            apsDuplicateTable[i].SrcAddress.Val = params.NLDE_DATA_indication.SrcAddress.Val;
            apsDuplicateTable[i].APSCounter = currentAPSCounter;
            apsDuplicateTable[i].StartTick = TickGet();
            updateIndex = i;
        }            
    }
    
    return FALSE;
}
#if defined(I_SUPPORT_GROUP_ADDRESSING)

/*********************************************************************
 * Function:        BYTE    GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr)
 *
 * PreCondition:    None
 *
 * Input:           GroupAddr - pointer to long address to match short address
 *
 * Output:          BYTE - a corresponding to the index in the group table 
 *                      where the matching GroupAddr entry was found
 *                  MAX_GROUP - a corresponding index if no match was  found
 *
 * Side Effects:    currentGroupAddressRecord is destroyed and set to the
 *                  matching entry if found
 *
 * Overview:        Searches the Group table for the GroupAddr.Val
 *                  given as input.
 *
 * Note:            The end application is responsible for populating
 *                  the Group table.
 ********************************************************************/

BYTE    GetEndPointsFromGroup(INPUT SHORT_ADDR GroupAddr)
{
    BYTE i;
    
    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif        
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        if( currentGroupAddressRecord.GroupAddress.Val == GroupAddr.Val )
        {
            return i;
        }
    }    
    return MAX_GROUP;
}


/*********************************************************************
 * Function:        BYTE    GetEmptyGroup(void)
 *
 * PreCondition:    None
 *
 * Input:           None - 
 *
 * Output:          BYTE - corresponding to the index in the group table 
 *                      where the first empty slot is found i.e. 0xffff
 *                  MAX_GROUP - corresponding index if no match was  found
 *
 * Side Effects:    currentGroupAddressRecord is destroyed 
 *                  
 *
 * Overview:        Searches the Group table for the first empty slot
 *
 * Note:            The end application is responsible for populating
 *                  the Group table.
 ********************************************************************/
BYTE    GetEmptyGroup(void)
{
    BYTE i;
    
    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        if( currentGroupAddressRecord.GroupAddress.Val == 0xFFFF )
        {
            return i;
        }
    }
    return MAX_GROUP;
}

/*********************************************************************
 * Function:        BYTE    AddGroup(INPUT WORD_VAL GroupAddress, INPUT BYTE EndPoint)
 *
 * PreCondition:    None
 *
 * Input:           GroupAddress - WORD_VAL representing the GroupAddress to be added to the 
 *                                  the group table
 *                  Endpoint    - BYTE the destination endpoint to be linked to the GroupAddress
 *
 * Output:          BYTE  GROUP_SUCCESS     - if entry was successfully added to GroupTable 
 *                  BYTE  GROUP_TABLE_FULL  - if GroupTable was full
 *                  BYTE  GROUP_INVALID_PARAMETER   - If invalid GroupAddress used i.e > 0xFFF7 or EndPoint == 0
 *
 * Side Effects:    currentGroupAddressRecord is destroyed, GroupTable modified 
 *
 * Overview:        Searches the Group table for empty slot and add GroupAddress + EndPoint to table
 *
 * Note:            The end application is responsible for creating 
 *                  the Group table.
 ********************************************************************/
BYTE    AddGroup(INPUT SHORT_ADDR GroupAddress, INPUT BYTE EndPoint)
{
    BYTE groupIndex;
    BYTE i;
    BYTE j;
                    
    if( EndPoint == 0x00 ||
        GroupAddress.Val > 0xFFF7 )
    {
        return GROUP_INVALID_PARAMETER;
    }
                    
    groupIndex = GetEndPointsFromGroup(GroupAddress);
    if( groupIndex == MAX_GROUP )
    {
        if( (groupIndex = GetEmptyGroup()) == MAX_GROUP )
        {
            return GROUP_TABLE_FULL;
        }
        currentGroupAddressRecord.GroupAddress.Val = GroupAddress.Val;
        currentGroupAddressRecord.EndPoint[0] = EndPoint;
        for(j = 1; j < MAX_GROUP_END_POINT; j++)
        {
            currentGroupAddressRecord.EndPoint[j] = 0xFF;
        }
    }
    else
    {
        for(i = 0; i < MAX_GROUP_END_POINT; i++)
        {
            if( currentGroupAddressRecord.EndPoint[i] == 0xFF )
            {
                break;
            }
            if( currentGroupAddressRecord.EndPoint[i] == EndPoint )
            {
                return GROUP_SUCCESS;
            }
        }
        if( i == MAX_GROUP_END_POINT )
        {
            return GROUP_TABLE_FULL;
        }
        currentGroupAddressRecord.EndPoint[i] = EndPoint;
    }
    
    #ifdef USE_EXTERNAL_NVM
        pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(groupIndex) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
    #else
        pCurrentGroupAddressRecord = &apsGroupAddressTable[groupIndex];
    #endif
    PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);
                        
    return GROUP_SUCCESS;
}    

/*********************************************************************
 * Function:        BYTE    RemoveGroup(INPUT WORD GroupAddress, INPUT BYTE EndPoint)
 *
 * PreCondition:    None
 *
 * Input:           GroupAddress - WORD_VAL representing the GroupAddress to be removed from the 
 *                                  the group table
 *                  Endpoint    - the destination endpoint to be unlinked from the GroupAddress in table
 *
 * Output:          BYTE  GROUP_SUCCESS     - if entry was successfully removed from GroupTable 
 *                  BYTE  GROUP_INVALID_PARAMETER   - If invalid GroupAddress used i.e > 0xFFF7 or EndPoint == 0
 *
 * Side Effects:    currentGroupAddressRecord is destroyed, GroupTable modified 
 *
 * Overview:        Searches the Group table for  GroupAddress + EndPoint and set them to 0xffff and 0xff in table
 *
 * Note:            The end application is responsible for creating 
 *                  the Group table.
 ********************************************************************/    
BYTE    RemoveGroup(INPUT WORD GroupAddress, INPUT BYTE EndPoint)
{
    BYTE i;
    BYTE j;
    BYTE k;
                    
    if( EndPoint == 0x00 ||
        ( GroupAddress > 0xFFF7 && GroupAddress != 0xFFFF ) )
    {
        return GROUP_INVALID_PARAMETER;
    }
    
    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        if( GroupAddress == 0xFFFF ||
            GroupAddress == currentGroupAddressRecord.GroupAddress.Val )
        {
            for(j = 0; j < MAX_GROUP_END_POINT; j++)
            {
                if( currentGroupAddressRecord.EndPoint[j] == EndPoint )
                {
                    break;    
                }    
            }
            if( j != MAX_GROUP_END_POINT )
            {
                if( j == 0 && (currentGroupAddressRecord.EndPoint[1] == 0xFF))
                {
                    currentGroupAddressRecord.EndPoint[0] = 0xFF;
                    currentGroupAddressRecord.GroupAddress.Val = 0xFFFF;
                    PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);
                }
                else
                {
                    for(k = j; k < MAX_GROUP_END_POINT-1; k++)
                    {
                        currentGroupAddressRecord.EndPoint[k] = currentGroupAddressRecord.EndPoint[k+1];
                    }
                    currentGroupAddressRecord.EndPoint[MAX_GROUP_END_POINT-1] = 0xFF;
                    PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);    
                }
                
                if( GroupAddress != 0xFFFF )
                {
                    break;
                }
            }
        }
    } 
    return GROUP_SUCCESS;  
}

/*********************************************************************
 * Function:        VOID    RemoveAllGroups(void)
 *
 * PreCondition:    None
 *
 * Input:           None - 
 *
 * Output:          None
 *
 * Side Effects:    currentGroupAddressRecord is destroyed 
 *                  
 *
 * Overview:        Searches the Group table and marks all the slots as empty 
 *
 * Note:            The end application is responsible for populating
 *                  the Group table.
 ********************************************************************/
void RemoveAllGroups(void)
{
    BYTE i;
    BYTE j;
    
    for(i = 0; i < MAX_GROUP; i++)
    {
        #ifdef USE_EXTERNAL_NVM
            pCurrentGroupAddressRecord = apsGroupAddressTable + (WORD)(i) * (WORD)sizeof(GROUP_ADDRESS_RECORD);
        #else
            pCurrentGroupAddressRecord = &apsGroupAddressTable[i];
        #endif
        GetGroupAddress(&currentGroupAddressRecord, pCurrentGroupAddressRecord);
        currentGroupAddressRecord.GroupAddress.Val = 0xFFFF;
        for(j = 0; j < MAX_GROUP_END_POINT; j++)
        {
            currentGroupAddressRecord.EndPoint[j] = 0xFF;
        }
        PutGroupAddress(pCurrentGroupAddressRecord, &currentGroupAddressRecord);
    }
}
#endif
