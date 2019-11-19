/*********************************************************************
 *
 *                  ZigBee ZDO Layer
 *
 *********************************************************************
 * FileName:        zZDO.c
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
#include "zNVM.h"
#include "generic.h"

#include "zMAC.h"
#include "zNWK.h"
#include "zAPS.h"
#include "zZDO.h"
#include "zAPL.h"
#include "zigbee.def"
#include "ZigBee.h"

#include "console.h"
#include "zSecurity.h"

// ******************************************************************************
// Constant Definitions

#define MAX_APS_PACKET_SIZE                 (127-11-8-6-2)  // Max size - MAC, NWK, APS, and AF headers
#define MAX_LONGADDS_TO_SEND                ((MAX_APS_PACKET_SIZE-13)/8)
#define MAX_SHORTADDS_TO_SEND               ((MAX_APS_PACKET_SIZE-13)/2)
#define MSG_HEADER_SIZE                     1

#define BIND_SOURCE_MASK            0x02
#define BIND_FROM_UPPER_LAYERS      0x02
#define UNBIND_FROM_UPPER_LAYERS    0x02
#define BIND_FROM_EXTERNAL          0x00
#define UNBIND_FROM_EXTERNAL        0x00

#define BIND_DIRECTION_MASK         0x01
#define BIND_NOT_DETERMINED         0x02    // NOTE - does not need mask
#define BIND_NODES                  0x01
#define UNBIND_NODES                0x00

// ******************************************************************************
// Macros

// Since the APSDE_DATA_indication and ZDO_DATA_indication parameters align,
// we can use the APSGet function.
#define ZDOGet()        APSGet()
#define ZDODiscardRx()  APSDiscardRx()

// ******************************************************************************
// Data Structures

#if defined(I_SUPPORT_BINDINGS)
typedef struct _BIND_IN_PROGRESS_INFO
{
        LONG_ADDR   sourceAddressLong;
        LONG_ADDR   destinationAddressLong;
        TICK        timeStamp;
        SHORT_ADDR  sourceAddressShort;
        SHORT_ADDR  destinationAddressShort;
        SHORT_ADDR  requestorAddress;
        BYTE        sourceEP;
        WORD_VAL    cluster;
        BYTE        destinationEP;
        BYTE        sequenceNumber;
        union
        {
            struct
            {
                unsigned char    bSourceRequested        : 1;
                unsigned char    bDestinationRequested   : 1;
                unsigned char    bBindNodes              : 1;
                unsigned char    bFromUpperLayers        : 1;
            } bits;
            BYTE    val;
        } status;
} BIND_IN_PROGRESS_INFO;
#endif


#if defined(SUPPORT_END_DEVICE_BINDING)
typedef struct _END_DEVICE_BIND_INFO
{
    TICK        lastTick;
    SHORT_ADDR  bindingTarget;
    SHORT_ADDR  shortAddr;
    LONG_ADDR   longAddr;
    WORD_VAL    profileID;
    WORD        *inClusterList;
    WORD        *outClusterList;
    BYTE        sequenceNumber;
    BYTE        bindEP;
    BYTE        numInClusters;
    BYTE        numOutClusters;
    BYTE        inIndex;
    BYTE        outIndex;
    BYTE        BindRspStatus;
    BYTE        NumReqSent;
    BYTE        status;
    BYTE        NumClusterMatched;
    BYTE        finalEDBRspSent;
    union
    {
        struct
        {
            unsigned char    fResponse       : 3;
            unsigned char    bAllRequestSent : 1;
            unsigned char    bFromSelf       : 1;
            unsigned char    bSendResponse   : 1;
            unsigned char    bSendUnbind     : 1;
            unsigned char    bWaitUnbindRsp  : 1;
            unsigned char    bEDBinding      : 1;
            unsigned char    bBindDirection  : 1;
            unsigned char    bCheckInCluster : 1;
            unsigned char    bCheckOutCluster: 1;
            unsigned char    bInClusterDone  : 1;
            unsigned char    bOutClusterDone : 1;
            unsigned char    bWaitForBindRsp : 1;
            unsigned char    bwaitForPairRsp : 1;
        } bits;
        WORD    Val;
    } flags;
} END_DEVICE_BIND_INFO;
#endif


// ******************************************************************************
// Variable Definitions

#ifdef I_SUPPORT_BINDINGS
    BIND_IN_PROGRESS_INFO *pBindInProgressInfo;
#endif

#ifdef SUPPORT_END_DEVICE_BINDING
    static END_DEVICE_BIND_INFO *pFirstEndBindRequest;
    static END_DEVICE_BIND_INFO *pSecondEndBindRequest;
#endif
BYTE        sequenceNumber;            // Received sequence number, for sending response back
ZDO_STATUS  zdoStatus;
BYTE        ZDOCounter;
SHORT_ADDR dAddr;
#if defined(I_SUPPORT_SECURITY)
    extern APS_ADDRESS_MAP     currentAPSAddress;   
    extern SECURITY_STATUS		securityStatus;
    #ifdef I_AM_RFD
        extern volatile PHY_PENDING_TASKS  PHYTasksPending;
    #endif
    extern KEY_VAL KeyVal;
    extern BOOL APSSaveAPSAddress( APS_ADDRESS_MAP *AddressMap);
    extern MAC_STATUS macStatus;
    
    #if defined(USE_EXTERNAL_NVM)
	    extern NETWORK_KEY_INFO plainSecurityKey[2];
	    extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
	    extern WORD trustCenterLongAddr;
	#else
		extern ROM LONG_ADDR trustCenterLongAddr;
	#endif
	#define NIB_nwkNetworkBroadcastDeliveryTime  0x02
#endif

extern DWORD_VAL apsChannelMask;

#ifdef I_SUPPORT_FREQUENCY_AGILITY
    extern PHY_PIB phyPIB;
    extern NWK_STATUS nwkStatus;
    extern SHORT_ADDR nwkManagerAddr;
    SHORT_ADDR zdoNwkNotifyAddr;
    
    #ifdef I_AM_NWK_MANAGER
        extern ENERGY_DETECT_RECORD *EdRecords;
        extern ENERGY_DETECT_RECORD * locateEDRecord(SHORT_ADDR ScanDeviceAddr);
    #endif
    
#endif
NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;
// ******************************************************************************
// Function Prototypes

void FinishAddressResponses( WORD clusterID );
BOOL IsThisMyShortAddr( void );
void PrepareMessageResponse( WORD clusterID );
BOOL handleMatchDescReq(void);
BOOL handleFindNodeCacheReq(void);
void handleActiveEPReq(void);

/* A Zigbee 2006 requirement:  FFDs can  participate in source binding */

    BOOL ProcessBindAndUnbind( BYTE bindInfo, LONG_ADDR *sourceAddress, BYTE dstAddrMode, ADDR *destinationAddress );


#ifdef I_SUPPORT_BINDINGS
    void SendBindAddressRequest( BYTE requestSource );
    BOOL SendUpBindResult( BYTE status, BYTE bindNodes );
    extern BYTE SentBindRequest;
#endif

#ifdef SUPPORT_END_DEVICE_BINDING
    ZIGBEE_PRIMITIVE ProcessEndDeviceBind( END_DEVICE_BIND_INFO *pRequestInfo );
    ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp( END_DEVICE_BIND_INFO *pBindRequest, BYTE status );
#endif
extern BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap);

#ifdef SUPPORT_END_DEVICE_BINDING
    WORD    matchCluster1;
    WORD    matchCluster2;
#endif

#ifdef I_SUPPORT_SECURITY
    extern BOOL    firstKeyHasBeenSent;
#endif

extern BOOL LookupAPSAddress(LONG_ADDR *);


/*********************************************************************
 * Function:        BOOL ZDOHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - ZDO layer has background tasks to run
 *                  FALSE - ZDO layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the ZDO layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BOOL ZDOHasBackgroundTasks( void )
{
    return ((zdoStatus.flags.Val & ZDO_BACKGROUND_TASKS) != 0);
}

/*********************************************************************
 * Function:        void ZDOInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    ZDO layer data structures are initialized.
 *
 * Overview:        This routine initializes all ZDO layer data
 *                  structures.
 *
 * Note:            This routine is intended to be called as part of
 *                  a network or power-up initialization.  If called
 *                  after the network has been running, heap space
 *                  may be lost unless the heap is also reinitialized.
 *                  End device binding in progress will be lost.
 ********************************************************************/

void ZDOInit( void )
{
    zdoStatus.flags.Val         = 0;
    ZDOCounter                  = 0;
    #ifdef I_SUPPORT_BINDINGS
        pBindInProgressInfo     = NULL;
    #endif

    #ifdef SUPPORT_END_DEVICE_BINDING
        pFirstEndBindRequest    = NULL;
        pSecondEndBindRequest   = NULL;
    #endif
}

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE ZDOTasks(ZIGBEE_PRIMITIVE inputPrimitive)
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
 * Note:            This routine may be called while the TX path is blocked.
 *                  Therefore, we must check the Tx path before doing any
 *                  processing that may generate a message.
 *                  It is the responsibility of this task to ensure that
 *                  only one output primitive is generated by any path.
 *                  If multiple output primitives are generated, they
 *                  must be generated one at a time by background processing.
 ********************************************************************/

ZIGBEE_PRIMITIVE ZDOTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    ZIGBEE_PRIMITIVE    nextPrimitive;
    BYTE                i;

    nextPrimitive = NO_PRIMITIVE;

    // Manage primitive- and Tx-independent tasks here.  These tasks CANNOT
    // produce a primitive or send a message.

    // Handle other tasks and primitives that may require a message to be sent.
    if (inputPrimitive == NO_PRIMITIVE)
    {
        TICK tempTick = TickGet();
        
        // If Tx is blocked, we cannot generate a message or send back another primitive.
        if (!ZigBeeReady())
            return NO_PRIMITIVE;

        // Manage background tasks here

        #ifdef I_SUPPORT_FREQUENCY_AGILITY
            if(zdoStatus.flags.bits.bNwkUpdateEDScan)
            {
                zdoStatus.flags.bits.bNwkUpdateEDScan = 0;
                
                // restore the channel
                phyPIB.phyCurrentChannel = phyPIB.phyBackupChannel;
                PHYSetLongRAMAddr(0x200, (0x01 | (BYTE)((phyPIB.phyCurrentChannel-11)<<4)));
                PHYSetShortRAMAddr(PWRCTL,0x04);
                PHYSetShortRAMAddr(PWRCTL,0x00);
     
                TxData = TX_DATA_START + MSG_HEADER_SIZE;
                TxBuffer[TxData++] = SUCCESS;
                // scanned channels, get scanned channels from unscanned channels. 
                // first set channels other than 11-26 to be unscanned
                params.MLME_SCAN_confirm.UnscannedChannels.Val |= ((POSSIBLE_CHANNEL_MASK) ^ 0xFFFFFFFF);
                for(i = 0; i < 4; i++)
                {
                    TxBuffer[TxData++] = params.MLME_SCAN_confirm.UnscannedChannels.v[i] ^ 0xFF;
                }
                TxBuffer[TxData++] = 0;     // total transmissions LSB
                TxBuffer[TxData++] = 0;     // total transmissions MSB
                TxBuffer[TxData++] = 0;     // transmission failures LSB
                TxBuffer[TxData++] = 0;     // transmission failures MSB
                TxBuffer[TxData++] = 0xFF;  // scanned channels list count
                for(i=0; i < params.MLME_SCAN_confirm.ResultListSize; i++)
                {
                    TxBuffer[TxData++] = params.MLME_SCAN_confirm.EnergyDetectList[i];
                }
                
                if (params.MLME_SCAN_confirm.EnergyDetectList)
                {
                    nfree( params.MLME_SCAN_confirm.EnergyDetectList );
                }
                
                params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val = zdoNwkNotifyAddr.Val;
                PrepareMessageResponse( MGMT_NWK_UPDATE_notify );
                return APSDE_DATA_request;    
            }
        #endif

        // ---------------------------------------------------------------------
        // Handle pending end device bind request
        #ifdef SUPPORT_END_DEVICE_BINDING
            if (zdoStatus.flags.bits.bEndDeviceBinding)
            {
                if (pFirstEndBindRequest)
                {
                    // NOTE: Compiler SSR27744, TickGet() output must be assigned to a variable.
                    if ( !pFirstEndBindRequest->flags.bits.bSendResponse  &&
                        (TickGetDiff(tempTick, pFirstEndBindRequest->lastTick) >= CONFIG_ENDDEV_BIND_TIMEOUT * 2) )
                    {
                        pFirstEndBindRequest->flags.bits.fResponse =  1;
                        pFirstEndBindRequest->flags.bits.bSendResponse = 1;
                    }
                    
                    /* For ZigBee 2006:  End_Device_Bind_rsp is now sent at the very end, not immediately */
                    if ( (pFirstEndBindRequest->flags.bits.bSendResponse == 1) && 
                        (pFirstEndBindRequest->flags.bits.fResponse == 1)   ) 
                    {
                        if(!pFirstEndBindRequest->finalEDBRspSent)
                            nextPrimitive = Send_END_DEVICE_BIND_rsp( pFirstEndBindRequest, 0x85);
                        else if (pSecondEndBindRequest)
                        {
                            nextPrimitive = Send_END_DEVICE_BIND_rsp( pSecondEndBindRequest, 0x85);
                        }
                        if( pFirstEndBindRequest->inClusterList )
                        {
                            nfree(pFirstEndBindRequest->inClusterList);
                        }
                        if( pFirstEndBindRequest->outClusterList )
                        {
                            nfree(pFirstEndBindRequest->outClusterList);
                        }
                        nfree( pFirstEndBindRequest );
                        zdoStatus.flags.bits.bEndDeviceBinding = 0;
                        /* Need to reset this flag in order to avoid sending endless responses */
                        pFirstEndBindRequest->flags.bits.bSendResponse = 0; 
                        return nextPrimitive;
                    }
          
                    // A ZigBee 2006 requirement:  Support source binding
                    if(pFirstEndBindRequest->flags.bits.bEDBinding &&
                        !pFirstEndBindRequest->flags.bits.bCheckInCluster &&
                        !pSecondEndBindRequest->flags.bits.bCheckOutCluster &&
                        !pFirstEndBindRequest->flags.bits.bOutClusterDone &&
                        !pSecondEndBindRequest->flags.bits.bInClusterDone)
                    {
                        WORD firstCluster;
                        WORD secondCluster;

                        pFirstEndBindRequest->flags.bits.bCheckOutCluster = 1;
                        pSecondEndBindRequest->flags.bits.bCheckInCluster = 1;
                        for(; pFirstEndBindRequest->outIndex < pFirstEndBindRequest->numOutClusters;
                            pFirstEndBindRequest->outIndex++)
                        {
                            firstCluster = pFirstEndBindRequest->outClusterList[pFirstEndBindRequest->outIndex];
                            for(; pSecondEndBindRequest->inIndex < pSecondEndBindRequest->numInClusters;
                                pSecondEndBindRequest->inIndex++)
                            {
                                secondCluster = pSecondEndBindRequest->inClusterList[pSecondEndBindRequest->inIndex];
                                if(!pFirstEndBindRequest->flags.bits.bwaitForPairRsp)
                                {
                                    if( firstCluster == secondCluster)   
                                    {
                                        if( (!pFirstEndBindRequest->flags.bits.bBindDirection) && (secondCluster == matchCluster2) )
                                        {
                                            pSecondEndBindRequest->inIndex++;
                                            /* count the skipped one as a fake sent anyway */
                                            pFirstEndBindRequest->NumReqSent++;
                                            continue;
                                        }

                                        if( pFirstEndBindRequest->flags.bits.bFromSelf )
                                        {
                                            WORD_VAL tmpWV;
                                            tmpWV.Val = firstCluster;
                                        
                                            if( pFirstEndBindRequest->flags.bits.bBindDirection )
                                            {
                                                APSAddBindingInfo(pSecondEndBindRequest->shortAddr, pFirstEndBindRequest->bindEP,
                                                    tmpWV, pFirstEndBindRequest->shortAddr, pSecondEndBindRequest->bindEP);
                                            }
                                            else
                                            {
                                                APSRemoveBindingInfo(pSecondEndBindRequest->shortAddr, pFirstEndBindRequest->bindEP,
                                                    tmpWV, pFirstEndBindRequest->shortAddr, pSecondEndBindRequest->bindEP);
                                            }
                                            pFirstEndBindRequest->flags.bits.bWaitForBindRsp    = 1;
                                            pFirstEndBindRequest->NumReqSent++;
                                            pSecondEndBindRequest->inIndex++;    
                                            continue;                                
                                        }
                                    
                                   
                                        ZigBeeBlockTx();
                                        TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                        for(i = 0; i < 8; i++)
                                        {
                                            TxBuffer[TxData++] = pFirstEndBindRequest->longAddr.v[i];
                                        }
                                        TxBuffer[TxData++] = pFirstEndBindRequest->bindEP;
                                        TxBuffer[TxData++] = firstCluster;
                                        TxBuffer[TxData++] = firstCluster >> 8;
                                    
                                        TxBuffer[TxData++] = 0x03; // dstAddrMode: 64bit
                                        for(i = 0; i < 8; i++)
                                        {
                                            TxBuffer[TxData++] = pSecondEndBindRequest->longAddr.v[i];
                                        }
                                        TxBuffer[TxData++] = pSecondEndBindRequest->bindEP;

                                        /*  A ZigBee 2006 requirement: Each Bind_req should be tagged with its own unique sequence number
                                        * so that each response can be differentiated from all the others in the ClusterList
                                        */
                                        sequenceNumber = ZDOCounter++;
                                        /* A ZigBee 2006 requirement: Keep track of the last seqence number used, when we get this number back
                                        * on a bind_rsp, all the responses are in and we are done, and can safely send
                                        * an end_device_bind_rsp 
                                        */
                                        pFirstEndBindRequest->sequenceNumber = sequenceNumber;
                                        pFirstEndBindRequest->NumReqSent++;
                                        
                                        params.ZDO_DATA_indication.SrcAddress.ShortAddr = pFirstEndBindRequest->shortAddr;
                                        if( pFirstEndBindRequest->flags.bits.bBindDirection )
                                        {
                                            PrepareMessageResponse( BIND_req );
                                            
                                            if(pFirstEndBindRequest->NumReqSent >= pSecondEndBindRequest->NumClusterMatched)
                                            {
                                                pFirstEndBindRequest->flags.bits.bAllRequestSent = 1; 
                                            }
                                        }
                                        else
                                        {
                                            PrepareMessageResponse( UNBIND_req );
                                            
                                            /* Don't Count the first probe/test Unbind that was sent */
                                            if(pFirstEndBindRequest->NumReqSent >= (pSecondEndBindRequest->NumClusterMatched - 1) )
                                            {
                                                pFirstEndBindRequest->flags.bits.bAllRequestSent = 1; 
                                            }
                                        }
                                    
                                        /* A ZigBee 2006 requirement:  Must increment here because we never 
                                        * get to the botton of the loop when we come back from this primitive
                                        * so the forloop does'nt increment as expected.  A very subtle BUG otherwise!!! 
                                        */
                                        pSecondEndBindRequest->inIndex++;
                                        /* pause here briefly so as not to over run the end_device or 
                                        * our NUM_BUFFERED_MSG destined for output to a slow end_device 
                                        */
                                        pFirstEndBindRequest->flags.bits.bwaitForPairRsp = 1;
                                        return APSDE_DATA_request;                                        
                                    }
                                }       /* wait for pair */
                                else 
                                {
                                    goto checknextrequest;
                                }
                            }
                            pSecondEndBindRequest->inIndex = 0;
                        }
                        pFirstEndBindRequest->flags.bits.bOutClusterDone = 1;
                        pSecondEndBindRequest->flags.bits.bInClusterDone = 1;
                        pFirstEndBindRequest->flags.bits.bCheckOutCluster = 0;
                        pSecondEndBindRequest->flags.bits.bCheckInCluster = 0;
                        pFirstEndBindRequest->outIndex = 0;
                        /* This properly terminates loop after list is completely sent */
                        pFirstEndBindRequest->flags.bits.bEDBinding = 0;
                    }
                }

                
                /* Decouple these such that two cluster lists can be handled 
                 * simulataneously, as is the case when you have two long cluster lists
                 * that are been sent - the bind_req/bind_rsp are interwined 
                 */
                // A ZigBee 2006 requirement: 
checknextrequest: 
                if( pSecondEndBindRequest && pSecondEndBindRequest->flags.bits.bEDBinding &&
                    !pFirstEndBindRequest->flags.bits.bCheckOutCluster &&
                    !pSecondEndBindRequest->flags.bits.bCheckInCluster &&
                    !pFirstEndBindRequest->flags.bits.bInClusterDone &&
                    !pSecondEndBindRequest->flags.bits.bOutClusterDone )
                {
                    WORD firstCluster;
                    WORD secondCluster;
                    
                    
                    pFirstEndBindRequest->flags.bits.bCheckInCluster = 1;
                    pSecondEndBindRequest->flags.bits.bCheckOutCluster = 1;
                    // Check first request's input clusters against second request's output clusters
                    for( ; pSecondEndBindRequest->outIndex < pSecondEndBindRequest->numOutClusters;
                        pSecondEndBindRequest->outIndex++)
                    {
                        secondCluster = pSecondEndBindRequest->outClusterList[pSecondEndBindRequest->outIndex];
                        for ( ; pFirstEndBindRequest->inIndex < pFirstEndBindRequest->numInClusters;
                            pFirstEndBindRequest->inIndex++ )
                        {
                            firstCluster = pFirstEndBindRequest->inClusterList[pFirstEndBindRequest->inIndex];
                            if(!pSecondEndBindRequest->flags.bits.bwaitForPairRsp)
                            {
                                if ( firstCluster == secondCluster) 
                                {
                                    /* if unbind then skip the test/probe ClusterId that was first sent out */
                                    if( (!pSecondEndBindRequest->flags.bits.bBindDirection) && (firstCluster == matchCluster1)  )
                                    {
                                        pFirstEndBindRequest->inIndex++ ;
                                        pSecondEndBindRequest->NumReqSent++; /* so that the edb_rsp will be timely sent */
                                        continue;
                                    }
                                            
                                    if( pSecondEndBindRequest->flags.bits.bFromSelf )
                                    {
                                        WORD_VAL tmpWV;
                                        tmpWV.Val = secondCluster;
                                        if( pSecondEndBindRequest->flags.bits.bBindDirection )
                                        {
                                            APSAddBindingInfo(pFirstEndBindRequest->shortAddr, pSecondEndBindRequest->bindEP,
                                                tmpWV, pSecondEndBindRequest->shortAddr, pFirstEndBindRequest->bindEP);
                                        }
                                        else
                                        {
                                            APSRemoveBindingInfo(pFirstEndBindRequest->shortAddr, pSecondEndBindRequest->bindEP,
                                                tmpWV, pSecondEndBindRequest->shortAddr, pFirstEndBindRequest->bindEP);
                                        }
                                        pSecondEndBindRequest->flags.bits.bWaitForBindRsp    = 1;    /* since there will be none */
                                        pSecondEndBindRequest->NumReqSent++;
                                        pFirstEndBindRequest->inIndex++;
                                        continue;                                
                                    }

	                                ZigBeeBlockTx();
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                    for(i = 0; i < 8; i++)
                                    {
                                        TxBuffer[TxData++] = pSecondEndBindRequest->longAddr.v[i];
                                    }
                                    TxBuffer[TxData++] = pSecondEndBindRequest->bindEP;
                                    TxBuffer[TxData++] = secondCluster;
                                    TxBuffer[TxData++] = secondCluster >> 8;
                                    
                                    TxBuffer[TxData++] = 0x03; // dstAddrMode: 64bit
                                    for(i = 0; i < 8; i++)
                                    {
                                        TxBuffer[TxData++] = pFirstEndBindRequest->longAddr.v[i];
                                    }
                                    TxBuffer[TxData++] = pFirstEndBindRequest->bindEP;
                                    //sequenceNumber = pSecondEndBindRequest->sequenceNumber;
                                    /* For ZigBee 2006: Each Bind_req should be tagged with its own unique sequence number
                                    * so that its response can be differentiated for all the others in the ClusterList
                                    */
                                    sequenceNumber = ZDOCounter++;
                                    /* For ZigBee 2006: 12/20/07 Keep track of the last seqence number used, when we get this number back
                                    * on a bind_rsp, all the responses are in and we are done, and can safely send
                                    * and end_device_bind_rsp 
                                    */
                                    pSecondEndBindRequest->sequenceNumber = sequenceNumber; 
                                    pSecondEndBindRequest->NumReqSent++;

                                    params.ZDO_DATA_indication.SrcAddress.ShortAddr = pSecondEndBindRequest->shortAddr;
                                    if( pSecondEndBindRequest->flags.bits.bBindDirection )
                                    {
                                        PrepareMessageResponse( BIND_req );
                                        if(pSecondEndBindRequest->NumReqSent >= (pFirstEndBindRequest->NumClusterMatched))
                                        {
                                            pSecondEndBindRequest->flags.bits.bAllRequestSent = 1;
                                        }
                                    }
                                    else
                                    {
                                        PrepareMessageResponse( UNBIND_req );
                                        
                                        /* Don't count the first probe/test unbind that was sent */
                                        if(pSecondEndBindRequest->NumReqSent >= (pFirstEndBindRequest->NumClusterMatched - 1))
                                        {
                                            pSecondEndBindRequest->flags.bits.bAllRequestSent = 1; 
                                        }
                                    }
                                    /* For ZigBee 2006: need this in order to increment through list */
                                    pFirstEndBindRequest->inIndex++;
                                    pSecondEndBindRequest->flags.bits.bwaitForPairRsp = 1;
                                    return APSDE_DATA_request;
                                }
                            } /* wait for pair */
                            else
                            {
                                return NO_PRIMITIVE;  /* go wait for the response pair */
                            }
                        }
                        pFirstEndBindRequest->inIndex = 0;
                    }
                    pFirstEndBindRequest->flags.bits.bInClusterDone = 1;
                    pSecondEndBindRequest->flags.bits.bOutClusterDone = 1;
                    pFirstEndBindRequest->flags.bits.bCheckInCluster = 0;
                    pSecondEndBindRequest->flags.bits.bCheckOutCluster = 0;
                    pSecondEndBindRequest->outIndex = 0;
                }

 
                
                if( pFirstEndBindRequest->flags.bits.bInClusterDone &&
                    pFirstEndBindRequest->flags.bits.bOutClusterDone &&
                    pSecondEndBindRequest->flags.bits.bInClusterDone &&
                    pSecondEndBindRequest->flags.bits.bOutClusterDone &&
                    pSecondEndBindRequest->flags.bits.bWaitForBindRsp &&
                    pFirstEndBindRequest->flags.bits.bWaitForBindRsp )
                {
                    
                    /* For ZigBee 2006:  Now send the END_DEVICE_BIND_RSP 
                     * after all is done - not immediately 
                     */
                    if (pFirstEndBindRequest->flags.bits.bSendResponse)
                    {
                        /* Send final response only after getting the assocated number of bind_rsp */
                        if(pFirstEndBindRequest->flags.bits.bWaitForBindRsp)
                        {
                            //nextPrimitive = Send_END_DEVICE_BIND_rsp( pFirstEndBindRequest, pFirstEndBindRequest->flags.bits.fResponse );
                            /* Pass back the Bind_rsp status up through the END_DEVICE_BIND_rsp */
                            nextPrimitive   = Send_END_DEVICE_BIND_rsp( pFirstEndBindRequest, pFirstEndBindRequest->BindRspStatus );
                            pFirstEndBindRequest->flags.bits.bSendResponse      = 0;
                            /* Final ZCP Testing Addition */
                            pFirstEndBindRequest->finalEDBRspSent               = 1;
                            return nextPrimitive;
                         }
                    }
                    
                    if (pSecondEndBindRequest->flags.bits.bSendResponse)
                    {
                        /* Send final response only after getting the bind_rsp */
                        if(pSecondEndBindRequest->flags.bits.bWaitForBindRsp)
                        {
                            /* Pass back the Bind_rsp status up through the END_DEVICE_BIND_rsp */
                            nextPrimitive   = Send_END_DEVICE_BIND_rsp( pSecondEndBindRequest, pSecondEndBindRequest->BindRspStatus );
                            pSecondEndBindRequest->flags.bits.bSendResponse     = 0;
                            /* Final ZCP Testing Addition */
                            pSecondEndBindRequest->finalEDBRspSent              = 1;
                            return nextPrimitive;
                        }
                    }
                    
                    pSecondEndBindRequest->flags.bits.bWaitForBindRsp   = 0;
                    pFirstEndBindRequest->flags.bits.bWaitForBindRsp    = 0;
                  
                    if( pFirstEndBindRequest->inClusterList )
                    {
                        nfree(pFirstEndBindRequest->inClusterList);
                    }
                    if( pFirstEndBindRequest->outClusterList )
                    {
                        nfree(pFirstEndBindRequest->outClusterList);
                    }
                    nfree(pFirstEndBindRequest);
                    if( pSecondEndBindRequest->inClusterList )
                    {
                        nfree(pSecondEndBindRequest->inClusterList);
                    }
                    if( pSecondEndBindRequest->outClusterList )
                    {
                        nfree(pSecondEndBindRequest->outClusterList);
                    }
                    nfree(pSecondEndBindRequest);
                    
                    if(pBindInProgressInfo != NULL)
                        nfree(pBindInProgressInfo);
                    zdoStatus.flags.bits.bEndDeviceBinding = 0;
                }
            }
            
            /* For Zigbee 2006, send out the first test unbind here */
            if(zdoStatus.flags.bits.bEDUnbind)
            {   
                WORD_VAL tempCluster;
                if(pFirstEndBindRequest && pFirstEndBindRequest->flags.bits.bSendUnbind )
                {
                    /* If from self then test directly, else send out unbind_req */
                    if( pFirstEndBindRequest->flags.bits.bFromSelf )
                    {
                        WORD_VAL tmpWV;

                        tmpWV.Val = matchCluster2;
                        if (APSRemoveBindingInfo(pSecondEndBindRequest->shortAddr, pFirstEndBindRequest->bindEP,
                                    tmpWV, pFirstEndBindRequest->shortAddr, pSecondEndBindRequest->bindEP) != SUCCESS)
                        {
                            pFirstEndBindRequest->flags.bits.bBindDirection = 1;    
                        }
                        pFirstEndBindRequest->flags.bits.bEDBinding  = 1;
                        pFirstEndBindRequest->flags.bits.bSendUnbind = 0; /* turn off */
                        return NO_PRIMITIVE;
                    }
                    
                    ZigBeeBlockTx();
                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
                    for(i = 0; i < 8; i++)
                    {
                        TxBuffer[TxData++] = pFirstEndBindRequest->longAddr.v[i];
                    }
                    TxBuffer[TxData++] = pFirstEndBindRequest->bindEP;
                    
                    tempCluster.Val     =  matchCluster2;  
                    TxBuffer[TxData++]  =  tempCluster.byte.LSB;
                    TxBuffer[TxData++]  =  tempCluster.byte.MSB;
                    
                    TxBuffer[TxData++] = 0x03; // dstAddrMode: 64bit
                    for(i = 0; i < 8; i++)
                    {
                        TxBuffer[TxData++] = pSecondEndBindRequest->longAddr.v[i];
                    }
                    TxBuffer[TxData++] = pSecondEndBindRequest->bindEP;
                    sequenceNumber = pFirstEndBindRequest->sequenceNumber;
                    params.ZDO_DATA_indication.SrcAddress.ShortAddr = pFirstEndBindRequest->shortAddr;
                    /*  UNbind_req goes before bind_req according to specs/testcases */
                    PrepareMessageResponse( UNBIND_req );
                    pFirstEndBindRequest->flags.bits.bSendUnbind = 0;
                    pFirstEndBindRequest->flags.bits.bWaitUnbindRsp = 1;
                    return APSDE_DATA_request;
                }
                if(pSecondEndBindRequest && pSecondEndBindRequest->flags.bits.bSendUnbind )
                {
                    if( pSecondEndBindRequest->flags.bits.bFromSelf )
                    {
                        WORD_VAL tmpWV;
                        tmpWV.Val = matchCluster1;
                        if (APSRemoveBindingInfo(pFirstEndBindRequest->shortAddr, pSecondEndBindRequest->bindEP,
                                    tmpWV, pSecondEndBindRequest->shortAddr, pFirstEndBindRequest->bindEP) != SUCCESS)
                        {
                            pSecondEndBindRequest->flags.bits.bBindDirection = 1;    
                        }
                        pSecondEndBindRequest->flags.bits.bEDBinding  = 1;
                        pSecondEndBindRequest->flags.bits.bSendUnbind = 0; /* turn off */
                        return NO_PRIMITIVE;
                    }
                    
                    ZigBeeBlockTx();
                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
                    for(i = 0; i < 8; i++)
                    {
                        TxBuffer[TxData++] = pSecondEndBindRequest->longAddr.v[i];
                    }
                    TxBuffer[TxData++] = pSecondEndBindRequest->bindEP;
                    
                    {
                        WORD    *wPtr;
                        WORD    dat;
                        
                        wPtr     =  pSecondEndBindRequest->outClusterList;
                        dat     = matchCluster1;

                        TxBuffer[TxData++]  =  dat;
                        TxBuffer[TxData++]  =  dat >> 8;
                    }
                    
                     
                    TxBuffer[TxData++] = 0x03; // dstAddrMode: 64bit
                    for(i = 0; i < 8; i++)
                    {
                        TxBuffer[TxData++] = pFirstEndBindRequest->longAddr.v[i];
                    }
                    TxBuffer[TxData++] = pFirstEndBindRequest->bindEP;
                    sequenceNumber = pSecondEndBindRequest->sequenceNumber;
                    params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val = pSecondEndBindRequest->shortAddr.Val;
                    
                    /* unbind goes before bind according to specs/testcases */
                    PrepareMessageResponse( UNBIND_req );
                    pSecondEndBindRequest->flags.bits.bSendUnbind = 0;
                    pSecondEndBindRequest->flags.bits.bWaitUnbindRsp = 1;
                    return APSDE_DATA_request;
                }
                zdoStatus.flags.bits.bEDUnbind = 0;
            }
        #endif

        #if defined(I_SUPPORT_BINDINGS)
            if (zdoStatus.flags.bits.bBinding)
            {
                BYTE    returnCode;
                if (TickGetDiff(tempTick, pBindInProgressInfo->timeStamp) >= (CONFIG_ENDDEV_BIND_TIMEOUT) * 2 )
                {
                    // Send a bind/unbind confirmation with failure
                    returnCode = ZDO_TIMEOUT;
                    goto SendBackgroundBindResponse;

                }
                else
                {
                    if (!pBindInProgressInfo->status.bits.bSourceRequested)
                    {
                        // Send a NWK_ADDR_req for the source
                        SendBindAddressRequest( TRUE );
                        pBindInProgressInfo->status.bits.bSourceRequested = 1;

                        return APSDE_DATA_request;
                    }
                    else if (!pBindInProgressInfo->status.bits.bDestinationRequested)
                    {
                        // Send a NWK_ADDR_req for the destination
                        SendBindAddressRequest( FALSE );
                        pBindInProgressInfo->status.bits.bDestinationRequested = 1;

                        return APSDE_DATA_request;
                    }
                }

                if ((pBindInProgressInfo->sourceAddressShort.Val != 0xFFFF) &&
                    (pBindInProgressInfo->destinationAddressShort.Val != 0xFFFF))
                {
                    // We have all the information.  Try to create the binding.
                    // If the binding came from the upper layers, make sure we can send up the response.  Otherwise, wait.
                    if (!pBindInProgressInfo->status.bits.bFromUpperLayers || CurrentRxPacket == NULL)
                    {
                        returnCode = SUCCESS;

                        if (pBindInProgressInfo->status.bits.bBindNodes)
                        {
                            if (APSAddBindingInfo( pBindInProgressInfo->sourceAddressShort, pBindInProgressInfo->sourceEP,
                                pBindInProgressInfo->cluster, pBindInProgressInfo->destinationAddressShort, pBindInProgressInfo->destinationEP))
                            {
                                returnCode = ZDO_TABLE_FULL;
                            }
                        }
                        else
                        {
                            BYTE rcode;
                            
                            if (  (rcode = APSRemoveBindingInfo( pBindInProgressInfo->sourceAddressShort, pBindInProgressInfo->sourceEP,
                                pBindInProgressInfo->cluster, pBindInProgressInfo->destinationAddressShort, pBindInProgressInfo->destinationEP))  )
                            {
                                returnCode = ZDO_NO_ENTRY;
                            }
                        }

SendBackgroundBindResponse:
                        if (pBindInProgressInfo->status.bits.bFromUpperLayers)
                        {
                            if (SendUpBindResult( returnCode, pBindInProgressInfo->status.bits.bBindNodes ))
                            {
                                nextPrimitive = APSDE_DATA_indication;
                            }
                            else
                            {
                                nextPrimitive = NO_PRIMITIVE;
                            }
                        }
                        else
                        {
                            ZigBeeBlockTx();
                            TxData = TX_DATA_START + MSG_HEADER_SIZE;
                            TxBuffer[TxData++] = returnCode;
                            sequenceNumber = pBindInProgressInfo->sequenceNumber;
                            params.ZDO_DATA_indication.SrcAddress.ShortAddr = pBindInProgressInfo->requestorAddress;
                            if (pBindInProgressInfo->status.bits.bBindNodes)
                            {
                                PrepareMessageResponse( BIND_rsp );
                            }
                            else
                            {
                                PrepareMessageResponse( UNBIND_rsp );
                            }
                            nextPrimitive = APSDE_DATA_request;
                        }

                        nfree( pBindInProgressInfo );
                        zdoStatus.flags.bits.bBinding = 0;
                        return nextPrimitive;
                    }
                }
            }
        #endif


        #if defined(I_SUPPORT_SECURITY)
            if ( zdoStatus.flags.bits.bSwitchKey )
            {
                if( TickGetDiff(tempTick, zdoStatus.SwitchKeyTick) > (ONE_SECOND * ((DWORD)NIB_nwkNetworkBroadcastDeliveryTime)) )
                {
                    
                    zdoStatus.flags.bits.bSwitchKey = 0;
                    for(i = 0; i < 2; i++)
                    {
                  		#ifdef USE_EXTERNAL_NVM
                		    currentNetworkKeyInfo = plainSecurityKey[i];
                  		#else
                      		GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[i]);
                  		#endif
                  		if( currentNetworkKeyInfo.SeqNumber.v[0] ==  zdoStatus.KeySeq &&
                      		currentNetworkKeyInfo.SeqNumber.v[1] == nwkMAGICResSeq )
                  		{
                      		
                          		i++;
                          		PutNwkActiveKeyNumber(&i);
                      		
                      		break;
                  		}
                    }                   
                }
            }
        #endif


    }
    else
    {
        switch( inputPrimitive )
        {
            case ZDO_DATA_indication:
            {
                    #ifdef I_SUPPORT_BINDINGS
                    BOOL        discardMessage;
                    #endif
                    APS_ADDRESS_MAP currentAPSAddress1;
            
                    // Limitation - the AF command frame allows more than one transaction to be included
                    // in a frame, but we can only generate one response.  Therefore, we will support only
                    // one transaction per ZDO frame.
                    if ((params.ZDO_DATA_indication.ClusterId.Val & 0x8000) != 0x0000)
                    {
                        switch( params.ZDO_DATA_indication.ClusterId.Val )
                        {
                            #ifdef I_SUPPORT_BINDINGS
                                
                                case NWK_ADDR_rsp:
                                    discardMessage = FALSE;
                                   
                                    /* For Zigbee 2006: For End_device Binding to work, we must save 
                                     * the long/short address in APS table else the bind_req will
                                     * not have short address to use in its source binding table
                                    */
                                    currentAPSAddress1.shortAddr.v[0] = *(params.ZDO_DATA_indication.asdu+10);
                                    currentAPSAddress1.shortAddr.v[1] = *(params.ZDO_DATA_indication.asdu+11);

                                    currentAPSAddress1.longAddr.v[0] = *(params.ZDO_DATA_indication.asdu+2);
                                    currentAPSAddress1.longAddr.v[1] = *(params.ZDO_DATA_indication.asdu+3);
                                    currentAPSAddress1.longAddr.v[2] = *(params.ZDO_DATA_indication.asdu+4);
                                    currentAPSAddress1.longAddr.v[3] = *(params.ZDO_DATA_indication.asdu+5);
                                    currentAPSAddress1.longAddr.v[4] = *(params.ZDO_DATA_indication.asdu+6);
                                    currentAPSAddress1.longAddr.v[5] = *(params.ZDO_DATA_indication.asdu+7);
                                    currentAPSAddress1.longAddr.v[6] = *(params.ZDO_DATA_indication.asdu+8);
                                    currentAPSAddress1.longAddr.v[7] = *(params.ZDO_DATA_indication.asdu+9);
                                
                                    /* For Zigbee 2006:  If a new device with the same old longAddress address
                                    * joins the PAN, then make sure the old short address is no longer used and is 
                                    * overwritten with the new shortAddress & longAddress combo 
                                    */
                                    if( LookupAPSAddress(&currentAPSAddress1.longAddr) == TRUE)
                                    {
                                        for( i = 0; i < apscMaxAddrMapEntries; i++)
                                        {
                                            #ifdef USE_EXTERNAL_NVM
                                                GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                            #else
                                                GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                            #endif
                                            if (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&currentAPSAddress1.longAddr, 8 ))
                                            {
                                                /* overwrite the old with the new short/long address combo  */
                                                #ifdef USE_EXTERNAL_NVM
                                                    PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress1);
                                                #else
                                                    PutAPSAddress( &apsAddressMap[i], &currentAPSAddress1 );
                                                #endif
                                            }
                                        }
                                    
                                    }
                                    else
                                    {
                                        /* LongAddress was not in the APS Table, so just save it */
                                        APSSaveAPSAddress(&currentAPSAddress1);
                                    }
                                    
                                  
                                    // See if this is a NWK_ADDR_rsp for a NWK_ADDR_req that was sent for a bind operation.
                                    // Note - we are not doing a lot of error checking here.
                                    if (zdoStatus.flags.bits.bBinding &&
                                        /* For Zigbee 2006: These offsets are different since no MSG/KVP + Count */
                                        (*(params.ZDO_DATA_indication.asdu+1) == SUCCESS))
                                    {
                                        // See if one of our addresses has responded.  Note that the source and destination address
                                        // could be the same.
                                        if (!memcmp( (void *)(params.ZDO_DATA_indication.asdu+2), (void *)&(pBindInProgressInfo->sourceAddressLong.v[0]), 8 ))
                                        {
                                            // This is the response to the source address request
                                            /* For Zigbee 2006: These offsets are different since no MSG/KVP + Count */ 
                                            pBindInProgressInfo->sourceAddressShort.v[0] = *(params.ZDO_DATA_indication.asdu+10);
                                            pBindInProgressInfo->sourceAddressShort.v[1] = *(params.ZDO_DATA_indication.asdu+11);
                                            discardMessage = TRUE;
                                        }
                                        if (!memcmp( (void *)(params.ZDO_DATA_indication.asdu+2), (void *)&(pBindInProgressInfo->destinationAddressLong.v[0]), 8 ))
                                        {
                                            // This is the response to the destination address request
                                            pBindInProgressInfo->destinationAddressShort.v[0] = *(params.ZDO_DATA_indication.asdu+10);
                                            pBindInProgressInfo->destinationAddressShort.v[1] = *(params.ZDO_DATA_indication.asdu+11);
                                            discardMessage = TRUE;
                                        }
                                    }

                                    if (discardMessage)
                                    {
                                        ZDODiscardRx();
                                        break;
                                    }
                                    //break;
                            #endif
                            // These are ZDO responses that the application has requested.
                            // Send them back to the user.  The parameters are all in place.

                            return APSDE_DATA_indication;
                            break;

                            #ifdef I_SUPPORT_FREQUENCY_AGILITY
                                case MGMT_NWK_UPDATE_notify:
                                    {
                                        BYTE status;
                                        
                                        ZDOGet();       // Sequence number
                                        status = ZDOGet();
                                        if( status == SUCCESS )    // operation successful, valid response
                                        {
                                            DWORD_VAL   ScannedChannels;
                                            WORD_VAL    FailedTransmission;
                                            DWORD       ChannelMask = 0x00000080;
                                            ENERGY_DETECT_RECORD *EdRecordPtr = EdRecords;
                                            
                                            for(i = 0; i < 4; i++)
                                            {
                                                ScannedChannels.v[i] = ZDOGet();    
                                            }
                                            ZDOGet();       // total transmission
                                            ZDOGet();
                                            FailedTransmission.v[0] = ZDOGet();       
                                            FailedTransmission.v[1] = ZDOGet();
                                            ZDOGet();       // channel list count
                                            
                                            // find the EdRecords
                                            EdRecordPtr = locateEDRecord(params.ZDO_DATA_indication.SrcAddress.ShortAddr);
                                            if( EdRecordPtr == NULL )
                                            {
                                                EdRecordPtr = EdRecords;
                                                while(EdRecordPtr->next)
                                                {
                                                    EdRecordPtr = EdRecordPtr->next;
                                                }
                                                EdRecordPtr->next = (ENERGY_DETECT_RECORD *)SRAMalloc(sizeof(ENERGY_DETECT_RECORD));
                                                if( EdRecordPtr->next == NULL )
                                                {
                                                    // handle the failure here
                                                }
                                                EdRecordPtr = EdRecordPtr->next;
                                                EdRecordPtr->ScanDeviceAddr = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
                                                for(i = 0; i < 16; i++)
                                                {
                                                    EdRecordPtr->EnergyReading[i] = 0xFF;
                                                }
                                                EdRecordPtr->next = NULL;    
                                            }
                                            
                                            
                                            for(i = 0; i < 16; i++)
                                            {
                                                if( ScannedChannels.Val & ChannelMask )
                                                {
                                                    EdRecordPtr->EnergyReading[i] = ZDOGet();
                                                }
                                            }
                                            
                                            // now decide how to change channel
                                            if( FailedTransmission.Val > 0 )
                                            {
                                                BYTE channel = 0xFF;
                                                WORD maxRSSI = 0xFFFF;
                                                
                                                for(i = 0; i < 16; i++)
                                                {
                                                    WORD channelRSSI = 0;
                                                    EdRecordPtr = EdRecords;
                                                    while(EdRecordPtr)
                                                    {
                                                        if( EdRecordPtr->EnergyReading[i] > MAX_ENERGY_THRESHOLD )
                                                        {
                                                            channelRSSI = 0xFFFF;
                                                            break;
                                                        }
                                                        channelRSSI += EdRecordPtr->EnergyReading[i];
                                                        EdRecordPtr = EdRecordPtr->next;    
                                                    }
                                                    if( channelRSSI < maxRSSI )
                                                    {
                                                        maxRSSI = channelRSSI;
                                                        channel = i + 11;
                                                    }
                                                }
                                                
                                                // after getting out of the loop, channel will store the channel with the 
                                                // lowest combined RSSI reading on all records
                                                if( channel != phyPIB.phyCurrentChannel )
                                                {
                                                    ScannedChannels.Val = 0x00000001 << channel;
                                                    ZigBeeBlockTx();

                                                    TxBuffer[TxData++] = ZDOCounter++; // seq
                            
                                                    TxBuffer[TxData++] = 0x00; // channel mask
                                                    TxBuffer[TxData++] = 0; //0xF8;
                                                    TxBuffer[TxData++] = 0; //0xFF;
                                                    TxBuffer[TxData++] = 0x04;
                            
                                                    TxBuffer[TxData++] = 0xfe;  // scan duration

                           
                                                    params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val = 0xFFFF;
                                                    PrepareMessageResponse(MGMT_NWK_UPDATE_notify);

                                                    nextPrimitive = APSDE_DATA_request;                                                    
                                                }
                                                else
                                                {
                                                    nextPrimitive = NO_PRIMITIVE;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            nextPrimitive = NO_PRIMITIVE;
                                        }
                                        ZDODiscardRx();
                                    }
                                    break;
                        
                            #endif
                            default:
                                // These are ZDO responses that the application has requested.
                                // Send them back to the user.  The parameters are all in place.
                               #ifdef I_SUPPORT_BINDINGS
                                    if(!zdoStatus.flags.bits.bEndDeviceBinding && (params.ZDO_DATA_indication.ClusterId.Val == BIND_rsp)  && SentBindRequest)
                                    {
                                        SentBindRequest = 0;
                                        goto ret_indy;
                                    }
                                    else if(!zdoStatus.flags.bits.bEndDeviceBinding && (params.ZDO_DATA_indication.ClusterId.Val == UNBIND_rsp)  && SentBindRequest )
                                    {
                                        SentBindRequest = 0;
                                        goto ret_indy;
                                    }
                                #endif
                                
                                if (  (params.ZDO_DATA_indication.ClusterId.Val == UNBIND_rsp) ||
                                      (params.ZDO_DATA_indication.ClusterId.Val == BIND_rsp)  ||
                                       (params.ZDO_DATA_indication.ClusterId.Val == UNBIND_req) ||
                                       (params.ZDO_DATA_indication.ClusterId.Val == BIND_rsp)  )
                                 {
                                     goto contchecking; 
                                 }
                ret_indy:         return APSDE_DATA_indication;
                        }
                        break;
                    }

                    // should always be EP0 sourceEP                = params.ZDO_DATA_indication.SrcEndpoint;
contchecking:       sequenceNumber            = ZDOGet();
                switch (params.ZDO_DATA_indication.ClusterId.Val)
                {
                        case NWK_ADDR_req:
                        {
                                BYTE    tempMACAddrByte;
                                BYTE    oneByte;
                                
                                // Get the first parameter (IEEEAddr) and make sure that it is our address
                                for (i=0; i<8; i++)
                                {
                                    GetMACAddressByte( i, &tempMACAddrByte );
                                    oneByte = ZDOGet();
                                    if (tempMACAddrByte != oneByte)
                                    {
                                        goto Finished_NWK_ADDR_req;
                                    }
                                }
                                FinishAddressResponses( NWK_ADDR_rsp );

                                // This request is broadcast, so we must request an APS ACK on the response.
                                //params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
                                params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
                                nextPrimitive = APSDE_DATA_request;

Finished_NWK_ADDR_req:
                                ;
                            }
                            break;

                        case IEEE_ADDR_req:
                            if (IsThisMyShortAddr())
                            {
                                FinishAddressResponses( IEEE_ADDR_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            }
                            break;

                        case NODE_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = SUCCESS;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                ProfileGetNodeDesc( &TxBuffer[TxData] );
                                
                                #if defined(__C30__)
                                	TxBuffer[TxData+3] = TxBuffer[TxData+4];
                                	TxBuffer[TxData+4] = TxBuffer[TxData+5];
                                	TxBuffer[TxData+5] = TxBuffer[TxData+6];
                                	TxBuffer[TxData+6] = TxBuffer[TxData+8];
                                	TxBuffer[TxData+7] = TxBuffer[TxData+9];
									TxBuffer[TxData+8] = TxBuffer[TxData+10];
									TxBuffer[TxData+9] = TxBuffer[TxData+11];
                                #endif
                                
	                            TxData += sizeof_NODE_DESCRIPTOR;

                                PrepareMessageResponse( NODE_DESC_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            }
                            break;

                        case POWER_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = SUCCESS;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                ProfileGetNodePowerDesc( &TxBuffer[TxData] );
                                TxData += sizeof(NODE_POWER_DESCRIPTOR);

                                PrepareMessageResponse( POWER_DESC_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            }

                            break;

                        case SIMPLE_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if ( IsThisMyShortAddr())
                            {
                                BYTE                    endPoint;
                                

                                endPoint = ZDOGet();

                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                if ((endPoint > 0) && (endPoint <= 240))
                                {
                                    // Find the simple descriptor of the desired endpoint.  Note that we cannot just shove this
                                    // into a message buffer, because it may have extra padding in the cluster lists.
                                    i = 0;
                                    do
                                    {
                                        ProfileGetSimpleDesc( &simpleDescriptor, i );
                                        i++;
                                    }
                                    while ((simpleDescriptor.Endpoint != endPoint) &&
                                           (i <= NUM_DEFINED_ENDPOINTS));
                                    if (i == NUM_DEFINED_ENDPOINTS)
                                    {
                                        // Load result code
                                        TxBuffer[TxData++] = ZDO_NOT_ACTIVE;
                                        goto SendInactiveEndpoint;
                                    }

                                    // Load result code
                                    TxBuffer[TxData++] = SUCCESS;

                                    // Load our short address.
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    // Load length of simple descriptor.
                                    TxBuffer[TxData++] = SIMPLE_DESCRIPTOR_BASE_SIZE +
                                            ( (simpleDescriptor.AppInClusterCount)*2 ) + ( (simpleDescriptor.AppOutClusterCount) *2 );

                                    // Load the descriptor. Since we can't send extra padding in the cluster lists,
                                    // we can't load the descriptor as one big array.
                                    TxBuffer[TxData++] = simpleDescriptor.Endpoint;

                                    TxBuffer[TxData++] = simpleDescriptor.AppProfId.byte.LSB;
                                    TxBuffer[TxData++] = simpleDescriptor.AppProfId.byte.MSB;
                                    TxBuffer[TxData++] = simpleDescriptor.AppDevId.byte.LSB;
                                    TxBuffer[TxData++] = simpleDescriptor.AppDevId.byte.MSB;

                                    TxBuffer[TxData++] = simpleDescriptor.AppDevVer | (simpleDescriptor.AppFlags << 4);
                                    /* For Zigbee 2006: clusterID needs to now be WORD size not BYTE Size 3.25 */
                                    TxBuffer[TxData++] = simpleDescriptor.AppInClusterCount;
                                    memcpy( (void *)&TxBuffer[TxData], (void *)simpleDescriptor.AppInClusterList, simpleDescriptor.AppInClusterCount * sizeof(WORD_VAL) );
                                    TxData += simpleDescriptor.AppInClusterCount * sizeof(WORD_VAL);
                                    TxBuffer[TxData++] = simpleDescriptor.AppOutClusterCount;
                                    memcpy( (void *)&TxBuffer[TxData], (void *)simpleDescriptor.AppOutClusterList, simpleDescriptor.AppOutClusterCount * sizeof(WORD_VAL));
                                    TxData += simpleDescriptor.AppOutClusterCount * sizeof(WORD_VAL);

                                    PrepareMessageResponse( SIMPLE_DESC_rsp );
                                    nextPrimitive = APSDE_DATA_request;
                                }
                                else
                                {
                                    // Invalid or Inactive Endpoint

                                    // Load result code
                                    TxBuffer[TxData++] = ZDO_INVALID_EP;

SendInactiveEndpoint:
                                    // Load our short address.
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    // Indicate no descriptor
                                    TxBuffer[TxData++] = 0;

                                    PrepareMessageResponse( SIMPLE_DESC_rsp );
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            break;

                        case ACTIVE_EP_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                handleActiveEPReq();
                                nextPrimitive = APSDE_DATA_request;
                            }
                            break;

                        case MATCH_DESC_req:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            #if 1
                            if (IsThisMyShortAddr())
                            {
                                if( handleMatchDescReq() )
                                {
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            #endif
                            break;

                        case COMPLEX_DESC_req:
                        case USER_DESC_req:
                        case USER_DESC_set:
                            // Get NWKAddrOfInterest and make sure that it is ours
                            if (IsThisMyShortAddr())
                            {
                                #ifdef INCLUDE_OPTIONAL_SERVICE_DISCOVERY_REQUESTS
                                    // TODO: Right now this is not supported at all.  When added,
                                    // put full functionality code here and change status.

                                    // Skip over the header info
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;

                                    // Load our short address.
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                    nextPrimitive = APSDE_DATA_request;
                                #else

                                    // Skip over the header info
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;

                                    // Load our short address.
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
                                    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

                                    PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                    nextPrimitive = APSDE_DATA_request;
                                #endif

                            }
                            break;

                        case END_DEVICE_annce:
                            /* Zigbee 2006: support is now mandatory test case 3.25 */
                            {
                                APS_ADDRESS_MAP currentAPSAddress1;
                                
                                currentAPSAddress1.shortAddr.v[0] = ZDOGet();
                                currentAPSAddress1.shortAddr.v[1] = ZDOGet();

                                currentAPSAddress1.longAddr.v[0] = ZDOGet();
                                currentAPSAddress1.longAddr.v[1] = ZDOGet();
                                currentAPSAddress1.longAddr.v[2] = ZDOGet();
                                currentAPSAddress1.longAddr.v[3] = ZDOGet();
                                currentAPSAddress1.longAddr.v[4] = ZDOGet();
                                currentAPSAddress1.longAddr.v[5] = ZDOGet();
                                currentAPSAddress1.longAddr.v[6] = ZDOGet();
                                currentAPSAddress1.longAddr.v[7] = ZDOGet();
                                
                                /* For Zigbee 2006:  If a new device with the same old longAddress address
                                 * joins the PAN, then make sure the old short address is no longer used and is 
                                 * overwritten with the new shortAddress & longAddress combo 
                                */
                                if( LookupAPSAddress(&currentAPSAddress1.longAddr) == TRUE)
                                {
                                    for( i = 0; i < apscMaxAddrMapEntries; i++)
                                    {
                                        #ifdef USE_EXTERNAL_NVM
                                            GetAPSAddress( &currentAPSAddress,  apsAddressMap + i * sizeof(APS_ADDRESS_MAP) );
                                        #else
                                            GetAPSAddress( &currentAPSAddress,  &apsAddressMap[i] );
                                        #endif
                                        if (!memcmp( (void *)&currentAPSAddress.longAddr, (void *)&currentAPSAddress1.longAddr, 8 ))
                                        {
                                            /* overwrite the old with the new short/long address combo  */
                                            #ifdef USE_EXTERNAL_NVM
                                                PutAPSAddress( apsAddressMap + i * sizeof(APS_ADDRESS_MAP), &currentAPSAddress1);
                                            #else
                                                PutAPSAddress( &apsAddressMap[i], &currentAPSAddress1 );
                                            #endif
                                        }
                                    }
                                    
                                }
                                else
                                {
                                    /* LongAddress was not in the APS Table, so just save it */
                                    APSSaveAPSAddress(&currentAPSAddress1);
                                }
                             }
                            break;

                        
                        #ifdef I_SUPPORT_FREQUENCY_AGILITY
                        case SYSTEM_SERVER_DISCOVERY_req:
                            {
                                WORD_VAL ServerMask;
                                
                                ServerMask.v[0] = ZDOGet();
                                ServerMask.v[1] = ZDOGet();
                                
                                ServerMask.Val &= Config_Node_Descriptor.NodeServerMask.Val;
                                if( ServerMask.Val )
                                {
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                    TxBuffer[TxData++] = SUCCESS;
                                    TxBuffer[TxData++] = ServerMask.v[0];
                                    TxBuffer[TxData++] = ServerMask.v[1];
                                    PrepareMessageResponse( SYSTEM_SERVER_DISCOVERY_rsp );
                                    nextPrimitive = APSDE_DATA_request;
                                }
                            }
                            break;
                        #endif
                        

                        /* For Zigbee 2006 FFD can now participate in source bindings */
                        #if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER) || defined (I_AM_END_DEVICE)

                        #define BR_OFFSET_SOURCE_ADDRESS        0x00
                        #define BR_OFFSET_DEST_ADDR_MODE        0x0B
                        #define BR_OFFSET_DESTINATION_ADDRESS   0x0C

                        #ifdef SUPPORT_END_DEVICE_BINDING
                        case END_DEVICE_BIND_req:
                            nextPrimitive = ProcessEndDeviceBind( NULL );
                            break;
                        #endif

                        case BIND_req:
                            #if !defined(I_SUPPORT_BINDINGS)
                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( BIND_rsp );
                                nextPrimitive = APSDE_DATA_request;
                            #else
                                {
                                    SHORT_ADDR dstAddr;
                                    if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
                                    {
                                        if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
                                        {
                                            dstAddr = currentAPSAddress.shortAddr;
                                        }
                                        else
                                        {
                                            nextPrimitive = NO_PRIMITIVE;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        dstAddr = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
                                    }
                                        
                                    if (ProcessBindAndUnbind( BIND_FROM_EXTERNAL | BIND_NODES,
                                        (LONG_ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_SOURCE_ADDRESS],
                                        params.ZDO_DATA_indication.asdu[BR_OFFSET_DEST_ADDR_MODE],
                                        (ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_DESTINATION_ADDRESS] ))
                                    {
                                        // Skip over the header info
                                        ZigBeeBlockTx();
                                        TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                        TxBuffer[TxData++] = params.ZDO_BIND_req.Status;
                                        params.ZDO_DATA_indication.SrcAddress.ShortAddr = dstAddr;                                     
                                        PrepareMessageResponse( BIND_rsp );
                                        nextPrimitive = APSDE_DATA_request;
                                    }
                                    else
                                    {
                                        nextPrimitive = NO_PRIMITIVE;
                                    }
                                }
                            #endif
                            break;


                        case BIND_rsp:
                            #ifdef SUPPORT_END_DEVICE_BINDING
                                if( pFirstEndBindRequest &&
                                        pFirstEndBindRequest->shortAddr.Val == params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val) 
                                {
                                       /*  if any one fail, then the entire end_device_bind_rsp will be tagged accordingly */
                                        pFirstEndBindRequest->BindRspStatus                 = pFirstEndBindRequest->BindRspStatus | ZDOGet();
                                        pFirstEndBindRequest->flags.bits.bwaitForPairRsp    = 0;
                                        
                                        if(pFirstEndBindRequest->flags.bits.bAllRequestSent)    /* this will trigger edb_rsp */
                                        {
                                            pFirstEndBindRequest->flags.bits.bWaitForBindRsp    = 1;
                                        }
                                }
                            
                                if( pSecondEndBindRequest &&
                                    pSecondEndBindRequest->shortAddr.Val == params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val ) 
                                {
                                    /*  if any one fail, then the entire end_device_bind_rsp will be tagged accordingly */
                                    pSecondEndBindRequest->BindRspStatus          = pSecondEndBindRequest->BindRspStatus | ZDOGet();
                                    pSecondEndBindRequest->flags.bits.bwaitForPairRsp    = 0;
                                    
                                    if(pSecondEndBindRequest->flags.bits.bAllRequestSent) /* trigger edb_rsp */
                                    {
                                            pSecondEndBindRequest->flags.bits.bWaitForBindRsp    = 1;
                                    }
                                    
                                }
                            #endif
                            break;
                        
                        case UNBIND_rsp:
                            #ifdef SUPPORT_END_DEVICE_BINDING
                                if( pFirstEndBindRequest &&
                                    pFirstEndBindRequest->shortAddr.Val == params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val && 
                                    pFirstEndBindRequest->flags.bits.bWaitUnbindRsp == 1)
                                {
                                    pFirstEndBindRequest->flags.bits.bEDBinding = 1;
                                    pFirstEndBindRequest->flags.bits.bWaitUnbindRsp = 0;
                                    /* For Zigbee 2006:  Used to toggle between bind nodes or unbind nodes */
                                    if( ZDOGet() != 0x00 ) /* i.e. NO_ENTRY Found */
                                    {
                                        pFirstEndBindRequest->flags.bits.bBindDirection = BIND_NODES;
                                    }
                                    else  
                                    {
                                        pFirstEndBindRequest->flags.bits.bBindDirection = UNBIND_NODES;
                                    }
                                }
                                else if( pSecondEndBindRequest &&
                                    pSecondEndBindRequest->shortAddr.Val == params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val && 
                                    pSecondEndBindRequest->flags.bits.bWaitUnbindRsp == 1 )
                                {
                                    pSecondEndBindRequest->flags.bits.bEDBinding = 1;
                                    pSecondEndBindRequest->flags.bits.bWaitUnbindRsp = 0;
                                    /* For Zigbee 2006:  Used to toggle between bind nodes or unbind nodes */
                                    if( ZDOGet() != 0x00 ) /* i.e. NO_ENTRY Found */
                                    {
                                        pSecondEndBindRequest->flags.bits.bBindDirection = BIND_NODES;
                                    }
                                    else
                                    {
                                        pSecondEndBindRequest->flags.bits.bBindDirection = UNBIND_NODES;
                                    } 
                                }
                            
                            /* Zigbee 2006: This block is for the toogle function, when  all the 
                             * previous binds are being unbind, after the first unbind_req has passed 
                            */
                            if( pFirstEndBindRequest &&
                                    pFirstEndBindRequest->shortAddr.Val == params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val && 
                                    pFirstEndBindRequest->flags.bits.bEDBinding == 1) /* got our test unbind_rsp back */
                                    
                            {
                                    pFirstEndBindRequest->BindRspStatus     = pFirstEndBindRequest->BindRspStatus | ZDOGet();
                                    pFirstEndBindRequest->flags.bits.bwaitForPairRsp    = 0;
                                    if( pFirstEndBindRequest->flags.bits.bAllRequestSent)    /* trigger edb_rsp */
                                    {
                                            pFirstEndBindRequest->flags.bits.bWaitForBindRsp    = 1;
                                    }
                             }
                             else if( pSecondEndBindRequest &&
                                    pSecondEndBindRequest->shortAddr.Val == params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val && 
                                    pSecondEndBindRequest->flags.bits.bEDBinding == 1) /* got our test unbind_rsp back */
                             {
                                    pSecondEndBindRequest->BindRspStatus     = pSecondEndBindRequest->BindRspStatus | ZDOGet();
                                    pSecondEndBindRequest->flags.bits.bwaitForPairRsp    = 0;
                                    if(pSecondEndBindRequest->flags.bits.bAllRequestSent)      /* trigger edb_rsp */
                                    {
                                            pSecondEndBindRequest->flags.bits.bWaitForBindRsp    = 1;
                                    }
                             }
                            
                            #endif
                            break;

                        case UNBIND_req:
                            #if !defined(I_SUPPORT_BINDINGS) || (MAX_BINDINGS == 0)
                                // Skip over the header info
                                ZigBeeBlockTx();
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( UNBIND_rsp );
                                nextPrimitive =  APSDE_DATA_request;
                            #else
                            {
                                SHORT_ADDR dstAddr;
                                
                                if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
                                {
                                    if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
                                    {
                                        dstAddr = currentAPSAddress.shortAddr;
                                    }
                                    else
                                    {
                                        nextPrimitive = NO_PRIMITIVE;
                                        break;
                                    }
                                }
                                else
                                {
                                   dstAddr = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
                                }
                                if (ProcessBindAndUnbind( UNBIND_FROM_EXTERNAL | UNBIND_NODES,
                                    (LONG_ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_SOURCE_ADDRESS],
                                    params.ZDO_DATA_indication.asdu[BR_OFFSET_DEST_ADDR_MODE],
                                    (ADDR *)&params.ZDO_DATA_indication.asdu[BR_OFFSET_DESTINATION_ADDRESS] ))
                                {
                                    // Skip over the header info
                                    ZigBeeBlockTx();
                                    TxData = TX_DATA_START + MSG_HEADER_SIZE;
                                    TxBuffer[TxData++] = params.ZDO_UNBIND_req.Status;
                                    params.ZDO_DATA_indication.SrcAddress.ShortAddr = dstAddr;
                                    PrepareMessageResponse( UNBIND_rsp );
                                    nextPrimitive =  APSDE_DATA_request;
                                }
                                else
                                {
                                    nextPrimitive = NO_PRIMITIVE;
                                }
                            }
                            #endif

                            break;

                        #endif

                        case MGMT_NWK_DISC_req:
                        case MGMT_LQI_req:
                        case MGMT_RTG_req:
                        case MGMT_BIND_req:
                        case MGMT_DIRECT_JOIN_req:
                        case MGMT_LEAVE_req:
                            #ifdef INCLUDE_OPTIONAL_NODE_MANAGEMENT_SERVICES
                                // Right now this is not supported at all.  When added,
                                // put full functionality code here and change status.

                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                nextPrimitive = APSDE_DATA_request;
                            #else

                                // Skip over the header info
                                TxData = TX_DATA_START + MSG_HEADER_SIZE;

                                TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
                                PrepareMessageResponse( params.ZDO_DATA_indication.ClusterId.Val | 0x8000 );
                                nextPrimitive = APSDE_DATA_request;
                            #endif
                            break;


                        #ifdef I_SUPPORT_FREQUENCY_AGILITY
                            case MGMT_NWK_UPDATE_req:
                            {
                                DWORD_VAL ScanChannels;
                                BYTE ScanDuration;
                                
                                for(i = 0; i < 4; i++)
                                {
                                    ScanChannels.v[i] = ZDOGet();
                                }
                                ScanDuration = ZDOGet();
                                
                                if( ScanDuration < 0x06 && !params.ZDO_DATA_indication.WasBroadcast)
                                {
                                    // a energy scan is required
                                    if( nwkStatus.flags.bits.bScanRequestFromZDO )
                                    {
                                        nextPrimitive = NO_PRIMITIVE;
                                    }
                                    else
                                    {
                                        // store the current channel
                                        phyPIB.phyBackupChannel = phyPIB.phyCurrentChannel;
                                        nwkStatus.flags.bits.bScanRequestFromZDO = 1;
                                        
                                        zdoNwkNotifyAddr.Val = params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val;
                                        
                                        params.MLME_SCAN_request.ScanType = MAC_SCAN_ENERGY_DETECT;
                                        params.MLME_SCAN_request.ScanChannels.Val = ScanChannels.Val;
                                        params.MLME_SCAN_request.ScanDuration = ScanDuration;
 
                                        nextPrimitive = MLME_SCAN_request;
                                    }
                                }
                                else if( ScanDuration == 0xFE )
                                {
                                    DWORD channelMask = 1;
                                    i = 0;
                                    while((ScanChannels.Val & channelMask) == 0 )
                                    {
                                        channelMask <<= 1;
                                        i++;
                                    }
                                    phyPIB.phyCurrentChannel = i;
                                    PHYSetLongRAMAddr(0x200, (0x02 | (BYTE)((phyPIB.phyCurrentChannel-11)<<4)));
                                    PHYSetShortRAMAddr(PWRCTL,0x04);
                                    PHYSetShortRAMAddr(PWRCTL,0x00);
                                    
                                    nextPrimitive = NO_PRIMITIVE;       
                                }
                                else if( ScanDuration == 0xFF )
                                {
                                     ZDOGet();                          // nwkUpdateId
                                     nwkManagerAddr.v[0] = ZDOGet();   
                                     nwkManagerAddr.v[1] = ZDOGet();
                                     
                                     apsChannelMask.Val = ScanChannels.Val;
                                     
                                     nextPrimitive = NO_PRIMITIVE;
                                }
                                else
                                {
                                    ZigBeeBlockTx();
                                    
                                    TxBuffer[TxData++] = ZDOCounter++;
                                    TxBuffer[TxData++] = NWK_INVALID_REQUEST;
                                    
                                    for(i = 0; i < 4; i++)
                                    {
                                        TxBuffer[TxData++] = ScanChannels.v[i];
                                    }
                                    TxBuffer[TxData++] = 0; // total transmission
                                    TxBuffer[TxData++] = 0;
                                    
                                    TxBuffer[TxData++] = 0; // transmission failure
                                    TxBuffer[TxData++] = 0;
                                    
                                    TxBuffer[TxData++] = 0; // channel count
                                    
                                    PrepareMessageResponse( MGMT_NWK_UPDATE_notify ); 
                                    
                                    nextPrimitive = APSDE_DATA_request;
                                }
                               
                            }
                            break;
                        #endif
                        
                        default:
                            break;
                    }

                    ZDODiscardRx();
                }
                break;  // ZDO_DATA_indication

            #if defined (SUPPORT_END_DEVICE_BINDING)

            case ZDO_END_DEVICE_BIND_req:
                {
                    END_DEVICE_BIND_INFO    *bindInfo;
                    WORD                    *firstClusterPtr = NULL; 

                   BYTE k;              /* Skip APSCounter */ 
                      
                    nextPrimitive = NO_PRIMITIVE;
                    if ((bindInfo = (END_DEVICE_BIND_INFO *)SRAMalloc( sizeof(END_DEVICE_BIND_INFO) )) != NULL)
                    {
                        k = 1;  
                        
                                                
                        bindInfo->bindingTarget.v[0]    = TxBuffer[k++];
                        bindInfo->bindingTarget.v[1]    = TxBuffer[k++];
                            
                        for(i = 0; i < 8; i++)
                        {
                            bindInfo->longAddr.v[i]     = TxBuffer[k++];
                        }
                        
                        bindInfo->bindEP = TxBuffer[k++];   /* Skip Source Endppoint for now */

                        bindInfo->profileID.byte.LSB    = TxBuffer[k++];
                        bindInfo->profileID.byte.MSB    = TxBuffer[k++];
                        
                        /* Number of Input Clusters */
                        bindInfo->numInClusters         = TxBuffer[k++];

                        if (bindInfo->numInClusters != 0)
                        {
                            if ((firstClusterPtr = (WORD *)SRAMalloc( bindInfo->numInClusters * sizeof(WORD) )) == NULL)
                            {
                                nfree( bindInfo );
                                goto StopGettingBindInfo;
                            }
                            else
                                bindInfo->inClusterList = firstClusterPtr;
                        }
                        
                        /* load up Cluster list */
                        for (i=0; i<bindInfo->numInClusters; i++, firstClusterPtr++)
                        {
                            *firstClusterPtr = (WORD)TxBuffer[k++];
                            *firstClusterPtr += ((WORD)TxBuffer[k++])<<8;
                        }

                        /* Process the Output Cluster List */
                        bindInfo->numOutClusters        = TxBuffer[k++];
                        if (bindInfo->numOutClusters != 0)
                        {
                            if ((firstClusterPtr = (WORD *)SRAMalloc( bindInfo->numOutClusters * sizeof(WORD) )) == NULL)
                            {
                                if (bindInfo->inClusterList)
                                {
                                    nfree( bindInfo->inClusterList);
                                }
                                nfree( bindInfo );
                                goto StopGettingBindInfo;
                            }
                            else
                                bindInfo->outClusterList = firstClusterPtr;
                        }
                        for (i=0; i<bindInfo->numOutClusters; i++, firstClusterPtr++)
                        {
                            *firstClusterPtr = (WORD)TxBuffer[k++];
                            *firstClusterPtr += ((WORD)TxBuffer[k++])<<8;
                        }

                        bindInfo->flags.bits.bFromSelf = 1;
                        
                        /* Initialize the ZigBee 2006 added parameters properly for consistency */
                        bindInfo->flags.Val             = 0;
                        bindInfo->BindRspStatus         = SUCCESS;
                        bindInfo->NumReqSent            = 0;
                        bindInfo->status                = 0;
                        bindInfo->outIndex              = 0;
                        bindInfo->inIndex               = 0;
                        bindInfo->NumClusterMatched     = 0;
                        bindInfo->finalEDBRspSent       = 0;
                        
                        nextPrimitive = ProcessEndDeviceBind( bindInfo );
                        TxHeader = TX_HEADER_START;
                        TxData = TX_DATA_START;
                        ZigBeeUnblockTx();
                    }    
                    else
                    {
StopGettingBindInfo:                        
                        params.APSDE_DATA_confirm.Status = APS_INVALID_REQUEST;
                        ZigBeeUnblockTx();
                        nextPrimitive = APSDE_DATA_confirm; 
                    }
                        
                }
                break;

            #endif

            #if defined(I_SUPPORT_BINDINGS)

            case ZDO_BIND_req:
                if (ProcessBindAndUnbind( BIND_FROM_UPPER_LAYERS | BIND_NODES,
                    (LONG_ADDR *)&params.ZDO_BIND_req.SrcAddress,
                    params.ZDO_BIND_req.DstAddrMode,
                    (ADDR *)&params.ZDO_BIND_req.DstAddress ))
                {
                    // If successful, a fake APSDE_DATA_indication was created with the response.
                    nextPrimitive = APSDE_DATA_indication;
                }
                else
                {
                    nextPrimitive = NO_PRIMITIVE;
                }
                break;

            case ZDO_UNBIND_req:
                if (ProcessBindAndUnbind( UNBIND_FROM_UPPER_LAYERS | UNBIND_NODES,
                    (LONG_ADDR *)&params.ZDO_UNBIND_req.SrcAddress,
                    params.ZDO_UNBIND_req.DstAddrMode,
                    (ADDR *)&params.ZDO_UNBIND_req.DstAddress ))
                {
                    // If successful, a fake APSDE_DATA_indication was created with the response.
                    nextPrimitive = APSDE_DATA_indication;
                }
                else
                {
                    nextPrimitive = NO_PRIMITIVE;
                }
                break;
            #endif

			#ifdef I_SUPPORT_SECURITY
				#if !defined(I_AM_TRUST_CENTER)
        			case APSME_TRANSPORT_KEY_indication:
        			{
        				#if !defined(I_AM_COORDINATOR)
            		        BYTE beforeEdAnnc = FALSE;
            		        beforeEdAnnc = securityStatus.flags.bits.bAuthorization;
            				securityStatus.flags.bits.bAuthorization = 0;
        				#endif
        				#ifdef I_AM_RFD
            				PHYTasksPending.bits.PHY_AUTHORIZE = 0;
        				#endif

            			switch( params.APSME_TRANSPORT_KEY_indication.KeyType)
            			{
                			case 0x01:  // Network key
                			{
                    			BYTE activeNwkKeyIndex;
                    			BYTE KeyIndex;

                    			// handle trust center
                    			PutTrustCenterAddress(&params.APSME_TRANSPORT_KEY_indication.SrcAddr);

                    			// handle keys
                    			currentNetworkKeyInfo.NetKey = *(params.APSME_TRANSPORT_KEY_indication.Key);
                    			currentNetworkKeyInfo.SeqNumber.v[0] = params.APSME_TRANSPORT_KEY_indication.TransportKeyData.NetworkKey.KeySeqNumber;
                    			currentNetworkKeyInfo.SeqNumber.v[1] = nwkMAGICResSeq;
                    			for(i = 0; i < 16; i++)
                    			{
                        			if( currentNetworkKeyInfo.NetKey.v[i] != 0x00 )
                        			{
                            			break;
                        			}
                    			}

                    			if( i != 16 )
                    			{
                        			
                          			GetNwkActiveKeyNumber(&KeyIndex);

                        			/* Zigbee 2006: KeyIndex will be 0xff during authorization 
                        			 * so this first key will now become the active key
                        			 * otherwise the else part will be executed 
                        			*/
                        			if( KeyIndex != 0x01 && KeyIndex != 0x02 )
                        			{
                            			KeyIndex = 0x01;
                            			PutNwkActiveKeyNumber(&KeyIndex);
                            			#ifdef USE_EXTERNAL_NVM
                            				SetSecurityKey(0, currentNetworkKeyInfo);
                            			#else
                                			PutNwkKeyInfo( &networkKeyInfo, &currentNetworkKeyInfo );
                            			#endif
                        			}
                        			else
                        			{
                            			KeyIndex = (KeyIndex == 0x01) ? 1:0;
                            			/* Put key in the non-active slot 
                            			*/
                            			#ifdef USE_EXTERNAL_NVM
                            				SetSecurityKey(KeyIndex, currentNetworkKeyInfo);
                            			#else
                                			PutNwkKeyInfo( &networkKeyInfo[KeyIndex], &currentNetworkKeyInfo );
                                			
                            			#endif
                        			}
                    			}

                				#ifdef I_AM_ROUTER
                    				if ( macStatus.bits.allowBeacon == 0 )
                    				{
                        				params.NLME_START_ROUTER_request.BeaconOrder = MAC_PIB_macBeaconOrder;
                        				params.NLME_START_ROUTER_request.SuperframeOrder = MAC_PIB_macSuperframeOrder;
                        				params.NLME_START_ROUTER_request.BatteryLifeExtension = FALSE;
                        				return NLME_START_ROUTER_request;
                    				}
                				#endif
                    			break;
                			}

            			}
            			
            			#if !defined(I_AM_COORDINATOR)
                            if(!beforeEdAnnc)
            			        nextPrimitive = NO_PRIMITIVE;
            			    else
            			    {
                			    params.NLME_JOIN_confirm.Status = SUCCESS;
                			    params.NLME_JOIN_confirm.PANId = macPIB.macPANId;
            			        nextPrimitive = NLME_JOIN_confirm;
            			    }
            			#endif 
            			break;
        			}
				#endif // !I_AM_TRUST_CENTER

				#ifdef I_AM_TRUST_CENTER
        			case APSME_UPDATE_DEVICE_indication:
        			{
            			BOOL allowJoin = TRUE;
            			//  decide if trust center will accept the device to join
            			if( !allowJoin ) {
                			params.APSME_REMOVE_DEVICE_request.ChildAddress = params.APSME_UPDATE_DEVICE_indication.DeviceAddress;
                			params.APSME_REMOVE_DEVICE_request.ParentAddress = params.APSME_UPDATE_DEVICE_indication.SrcAddress;

                			nextPrimitive = APSME_REMOVE_DEVICE_request;
                			break;
            			}

            			// if allow to join, send network key to the router with security on
            			currentAPSAddress.longAddr = params.APSME_UPDATE_DEVICE_indication.DeviceAddress;
            			currentAPSAddress.shortAddr = params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress;
            			APSSaveAPSAddress(&currentAPSAddress);

            			// DestinationAddress overlap with Device address, no need to assign
        				#ifdef I_SUPPORT_SECURITY_SPEC
            				if( params.APSME_UPDATE_DEVICE_indication.Status == 0x00 )
            				{
                				for(i = 0; i < 16; i++)
                				{
                    				KeyVal.v[i] = 0;
                				}
                				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = 0x00;
            				}
            				else
            				{
                				BYTE activeNwkKeyIndex;

                				GetNwkActiveKeyNumber(&activeNwkKeyIndex);
                				#ifdef USE_EXTERNAL_NVM
                					KeyVal = plainSecurityKey[activeNwkKeyIndex-1].NetKey;
                					params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = plainSecurityKey[activeNwkKeyIndex-1].SeqNumber.v[0];
                				#else
                    				GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[activeNwkKeyIndex-1]);
    	            				KeyVal = currentNetworkKeyInfo.NetKey;
        	        				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
            					#endif
                				params.APSME_TRANSPORT_KEY_request.ParentAddress = params.APSME_UPDATE_DEVICE_indication.SrcAddress;
                				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = TRUE;
            				}
        				#else
                			params.APSME_TRANSPORT_KEY_request.ParentAddress = params.APSME_UPDATE_DEVICE_indication.SrcAddress;
                			// should not be this way by spec. However, have to do this way to pass ZCP. Sad.
                			// Unhappy customer can turn on I_SUPPORT_SECURITY_SPEC to strictly follow spec, but
                			// may not be compatible with certain certified ZigBee devices.
                			params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = TRUE;
            				#ifdef PRECONFIGURE_KEY
                				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                				for(i = 0; i < 16; i++)
                				{
                    				KeyVal.v[i] = 0x00;
                				}
            				#else
                				GetNwkActiveKeyNumber(&i);
                				#ifdef USE_EXTERNAL_NVM
                					KeyVal = plainSecurityKey[i-1].NetKey;
                					params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = plainSecurityKey[i-1].SeqNumber.v[0];
                				#else
                    				GetNwkKeyInfo(&currentNetworkKeyInfo, &networkKeyInfo[i-1]);
    	            				KeyVal = currentNetworkKeyInfo.NetKey;
        	        				params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
        	    				#endif
            				#endif

        				#endif
            			params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
            			params.APSME_TRANSPORT_KEY_request.KeyType = 0x01;
            			params.APSME_TRANSPORT_KEY_request._UseSecurity = TRUE;
            			
            			firstKeyHasBeenSent = FALSE;
            			nextPrimitive = APSME_TRANSPORT_KEY_request;

            			break;

        			}
				#endif      // if I_AM_TRUST_CENTER

				#if defined(I_AM_COORDINATOR) || defined(I_AM_ROUTER)
        			case APSME_REMOVE_DEVICE_indication:
        			{
            			params.NLME_LEAVE_request.DeviceAddress     = params.APSME_REMOVE_DEVICE_indication.ChildAddress;
            			params.NLME_LEAVE_request.RemoveChildren    = TRUE;
            			/* For Zigbee 2006:  Initialize the new Leave parameters */
            			params.NLME_LEAVE_request.Silent        = FALSE;
            			params.NLME_LEAVE_request.ReuseAddress  = FALSE;
            			params.NLME_LEAVE_request.Rejoin        = FALSE;
 
            			nextPrimitive = NLME_LEAVE_request;
            			break;
        			}
				#endif

				#ifdef I_AM_TRUST_CENTER
        			case APSME_REQUEST_KEY_indication:
        			{
            			switch( params.APSME_REQUEST_KEY_indication.KeyType)
            			{
                			case 0x01:  // network key
                			{
                    			BYTE ActiveKeyIndex;

                    			GetNwkActiveKeyNumber(&ActiveKeyIndex);
                    			#ifdef USE_EXTERNAL_NVM
									KeyVal = plainSecurityKey[ActiveKeyIndex-1].NetKey;
									params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = plainSecurityKey[ActiveKeyIndex-1].SeqNumber.v[0];
                    			#else
	                    			GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(networkKeyInfo[ActiveKeyIndex-1]));
    	                			KeyVal = currentNetworkKeyInfo.NetKey;
									params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
								#endif

                    			params.APSME_TRANSPORT_KEY_request.KeyType = params.APSME_REQUEST_KEY_indication.KeyType;
                    			params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                    			params.APSME_TRANSPORT_KEY_request.DestinationAddress = params.APSME_REQUEST_KEY_indication.SrcAddress;
                    			params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;

                    			nextPrimitive = APSME_TRANSPORT_KEY_request;
                    			break;

                			}
            			}

            			break;
        			}
				#endif  // I_AM_TRUST_CENTER

        		case APSME_SWITCH_KEY_indication:
        		{
            		zdoStatus.KeySeq = params.APSME_SWITCH_KEY_indication.KeySeqNumber;
            		zdoStatus.SwitchKeyTick = TickGet();
                    zdoStatus.flags.bits.bSwitchKey = 1;
            		break;
        		}


			#endif
            default:
                break;
        } // Input Primitive
    }
    return nextPrimitive;
}



/*********************************************************************
 * Function:        void FinishAddressResponses( WORD clusterID )
 *
 * PreCondition:    None
 *
 * Input:           clusterID - output cluster ID
 *
 * Output:          None
 *
 * Side Effects:    Message sent
 *
 * Overview:        This function finishes the address responses for the
 *                  NWK_ADDR_req and IEEE_ADDR_req clusters.
 *
 * Note:
 *
 * TODO: The spec is confusing on the IEEE_ADDR_rsp.  They mention in one place that we're
 * supposed to respond with the "16-bit IEEE addresses", but in the table for the
 * response it says 16-bit short address, which make the response the same
 * as for NWK_addr_rps.  And the Framework spec alludes to being able to request
 * both short and long addresses.  We'll send the short addresses, with code for
 * the long addresses left in the comments in case we need it later.
 ********************************************************************/
void FinishAddressResponses( WORD clusterID )
{
#ifndef I_AM_END_DEVICE
    BYTE        i;
    BYTE        count;
#endif
    BYTE        requestType;
    BYTE        startIndex;

    // Now get the rests of the paramters.
    requestType = ZDOGet();
    startIndex  = ZDOGet();          // 0-based or 1-based? assuming 0-based...

    if (requestType == SINGLE_DEVICE_RESPONSE)
    {
        // Skip over the header info
        
        TxData = TX_DATA_START + MSG_HEADER_SIZE;

        // Load status byte
        TxBuffer[TxData++] = SUCCESS;
  
        // Load our long address.
        GetMACAddress( &(TxBuffer[TxData]) );
        TxData += 8;

        // Load our short address.
        TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
        TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

        PrepareMessageResponse( clusterID );
    }
    else if (requestType == EXTENDED_RESPONSE)
    {
        #if defined(I_AM_END_DEVICE)
            
            // Skip over the header info
            TxData = TX_DATA_START + MSG_HEADER_SIZE;

            // Load status byte
            TxBuffer[TxData++] = SUCCESS;

            // Load our long address.
            GetMACAddress( &(TxBuffer[TxData]) );
            TxData += 8;

            // Load our short address.
            TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
            TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

            // Load NumAssocDev value
            // For end device, there are no associated device.  So we send 0 for the
            // number of associated devices and don't send StartIndex and the list
            // of associated devices.
            TxBuffer[TxData++] = 0;

            PrepareMessageResponse( clusterID );
        #else

            // Skip over the header info
            TxData = TX_DATA_START + MSG_HEADER_SIZE;

            // Load status byte
            TxBuffer[TxData++] = SUCCESS;

            // Load our long address.
            GetMACAddress( &(TxBuffer[TxData]) );
            TxData += 8;

            // Load our short address.
            TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
            TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

            // Load NumAssocDev value
            TxBuffer[TxData++] = currentNeighborTableInfo.numChildren;

            // Send the start index back
            TxBuffer[TxData++] = startIndex;

            if (startIndex < currentNeighborTableInfo.numChildren) // if 1-based, <=
            {
                // Send the associated devices list.
                i = 0;
                count = 0;
                pCurrentNeighborRecord = neighborTable;
                while ((count < MAX_SHORTADDS_TO_SEND) && (i < currentNeighborTableInfo.numChildren))
                {
                    do
                    {
                        GetNeighborRecord(&currentNeighborRecord, pCurrentNeighborRecord );
                        #ifdef USE_EXTERNAL_NVM
                            pCurrentNeighborRecord += sizeof(NEIGHBOR_RECORD);
                        #else
                            pCurrentNeighborRecord++;
                        #endif
                    }
                    while ( (!currentNeighborRecord.deviceInfo.bits.bInUse) ||
                            (currentNeighborRecord.deviceInfo.bits.Relationship != NEIGHBOR_IS_CHILD));

                    if (i >= startIndex)
                    {
                        TxBuffer[TxData++] = currentNeighborRecord.shortAddr.byte.LSB;
                        TxBuffer[TxData++] = currentNeighborRecord.shortAddr.byte.MSB;

                        count++;
                    }

                    i ++;   // if 1-based, move to before if statement
                }

            }
            PrepareMessageResponse( clusterID );
        #endif
    }
    else
    {
        // Load status byte
        TxBuffer[TxData++] = ZDO_INV_REQUESTTYPE;

        PrepareMessageResponse( clusterID );
    }
}

/*********************************************************************
 * Function:        BOOL IsThisMyShortAddr(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - The next two bytes at adsu match my short address
 *                  FALSE - The next two bytes at adsu do not match
 *
 * Side Effects:    None
 *
 * Overview:        This function determines if the next two bytes at
 *                  adsu match my short address or are the broadcast
 *                  address.
 *
 * Note:            The broadcast address is allowed as a match for
 *                  the MATCH_DESC_req.  That is the only one that is
 *                  allowed to be broadcast - all the others must be
 *                  unicast.
 ********************************************************************/

BOOL IsThisMyShortAddr(void)
{
    SHORT_ADDR  address;

    address.byte.LSB = ZDOGet();
    address.byte.MSB = ZDOGet();

    if ((address.Val == macPIB.macShortAddress.Val) ||
        (address.Val == 0xFFFF)
        #if !defined(I_AM_END_DEVICE)
            || (address.Val == 0xFFFC)
        #endif
        #if !defined(I_AM_RFD)
            || (address.Val == 0xFFFD)
        #endif
        )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*********************************************************************
 * Function:        END_DEVICE_BIND_INFO * GetOneEndDeviceBindRequest(
                            END_DEVICE_BIND_INFO * pRequestInfo )
 *
 * PreCondition:    None
 *
 * Input:           pRequestInfo - pointer to the end device bind request
 *                  information.  If this is null, the request is being
 *                  received from another device, and the information is
 *                  in the received message.  Otherwise, the information
 *                  must be extracted from the incoming message.
 *
 * Output:          Next ZigBee primitive
 *
 * Side Effects:    Numerous
 *
 * Overview:        This function retrieves end device binding information
 *                  from either the upper layers or an incoming message.
 *
 * Note:            None
 ********************************************************************/
#if defined SUPPORT_END_DEVICE_BINDING && (MAX_BINDINGS > 0)

END_DEVICE_BIND_INFO * GetOneEndDeviceBindRequest( END_DEVICE_BIND_INFO * pRequestInfo )
{
    WORD                    *firstClusterPtr = NULL;
    BYTE                    i;
    END_DEVICE_BIND_INFO    *pEndDeviceBindRequest;

    pEndDeviceBindRequest = NULL;

    if (pRequestInfo == NULL)
    {
        // We are getting this request from another device
        if ((pEndDeviceBindRequest = (END_DEVICE_BIND_INFO *)SRAMalloc( sizeof(END_DEVICE_BIND_INFO) )) != NULL)
        {
            pEndDeviceBindRequest->flags.Val             = 0;
            pEndDeviceBindRequest->shortAddr             = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
            pEndDeviceBindRequest->sequenceNumber        = sequenceNumber;
            // should always be EP0 pEndDeviceBindRequest->sourceEP              = params.ZDO_DATA_indication.SrcEndpoint;
            pEndDeviceBindRequest->inIndex               = 0;
            pEndDeviceBindRequest->outIndex              = 0;
            
            pEndDeviceBindRequest->BindRspStatus         = SUCCESS;
            pEndDeviceBindRequest->NumReqSent            = 0;
            pEndDeviceBindRequest->NumClusterMatched     = 0;
            
            pEndDeviceBindRequest->inClusterList         = NULL;
            pEndDeviceBindRequest->outClusterList        = NULL;

            pEndDeviceBindRequest->bindingTarget.v[0]    = ZDOGet();
            pEndDeviceBindRequest->bindingTarget.v[1]    = ZDOGet();
            
            /* Do some error checking here for robustness */
            if(pEndDeviceBindRequest->bindingTarget.Val  >=  0xfffc)
            {
                nfree( pEndDeviceBindRequest );
                goto BailFromGatheringInfo;
            }
                  
            for(i = 0; i < 8; i++)
            {
                pEndDeviceBindRequest->longAddr.v[i]    = ZDOGet();
            }
            pEndDeviceBindRequest->bindEP                = ZDOGet();
            pEndDeviceBindRequest->profileID.byte.LSB    = ZDOGet();
            pEndDeviceBindRequest->profileID.byte.MSB    = ZDOGet();
            pEndDeviceBindRequest->numInClusters         = ZDOGet();

            if (pEndDeviceBindRequest->numInClusters != 0)
            {
                if ((firstClusterPtr = (WORD *)SRAMalloc( pEndDeviceBindRequest->numInClusters * sizeof(WORD) )) == NULL)
                {
                    nfree( pEndDeviceBindRequest );
                    goto BailFromGatheringInfo;
                }
                else
                    pEndDeviceBindRequest->inClusterList = firstClusterPtr;
            }
            
            for (i=0; i<pEndDeviceBindRequest->numInClusters; i++, firstClusterPtr++)
            {
                *firstClusterPtr = (WORD)ZDOGet();
                *firstClusterPtr += ((WORD)ZDOGet())<<8;
            }

            pEndDeviceBindRequest->numOutClusters        = ZDOGet();
            if (pEndDeviceBindRequest->numOutClusters != 0)
            {
                if ((firstClusterPtr = (WORD *)SRAMalloc( pEndDeviceBindRequest->numOutClusters * sizeof(WORD) )) == NULL)
                {
                    if (pEndDeviceBindRequest->inClusterList)
                    {
                        nfree( pEndDeviceBindRequest->inClusterList);
                    }
                    nfree( pEndDeviceBindRequest );
                    goto BailFromGatheringInfo;
                }
                else
                    pEndDeviceBindRequest->outClusterList = firstClusterPtr;
            }
            for (i=0; i<pEndDeviceBindRequest->numOutClusters; i++, firstClusterPtr++)
            {
                *firstClusterPtr = (WORD)ZDOGet();
                *firstClusterPtr += ((WORD)ZDOGet())<<8;
            }

            pEndDeviceBindRequest->flags.bits.bFromSelf = 0;
        }
    }
    else
    {
        // We are getting this request from ourself
        pEndDeviceBindRequest = pRequestInfo;
        pEndDeviceBindRequest->flags.bits.bFromSelf = 1;
    }

BailFromGatheringInfo:
    return pEndDeviceBindRequest;
}
#endif

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE ProcessEndDeviceBind(
                        END_DEVICE_BIND_INFO *pRequestInfo )
 *
 * PreCondition:    sequenceNumber and sourceEP must be set
 *
 * Input:           pRequestInfo - pointer to the end device bind request
 *                  information.  If this is null, the request is being
 *                  received from another device, and the information is
 *                  in the received message.
 *
 * Output:          Next ZigBee primitive
 *
 * Side Effects:    Numerous
 *
 * Overview:        This function performs either the first half or the
 *                  second half of end device binding.  It can be invoked
 *                  either by receiving an END_DEVICE_BIND_req message
 *                  or by the upper layers of the application.  The first
 *                  time the routine is called, the bind information is
 *                  simply stored.  The second time it is called, the two
 *                  lists are checked for a match, and bindings are
 *                  created if possible.  The background processing will
 *                  check for timeout of the first message.
 *
 * Note:            None
 ********************************************************************/
#ifdef SUPPORT_END_DEVICE_BINDING

ZIGBEE_PRIMITIVE ProcessEndDeviceBind( END_DEVICE_BIND_INFO *pRequestInfo )
{

#if defined SUPPORT_END_DEVICE_BINDING && (MAX_BINDINGS > 0)

    WORD                    *firstClusterPtr;
    BYTE                    firstIndex;
    ZIGBEE_PRIMITIVE        nextPrimitive;
    WORD                    *secondClusterPtr;
    BYTE                    secondIndex;
    BYTE                    status;
    

    status = ZDO_NO_MATCH;
    nextPrimitive = NO_PRIMITIVE;


    if (pFirstEndBindRequest == NULL)
    {
        // We are receiving the first request.  Gather all of the information from
        // this request and store it for later.
        if ((pFirstEndBindRequest = GetOneEndDeviceBindRequest( pRequestInfo )) == NULL)
        {
            goto BailFromEndDeviceRequest;
        }
        // Set the timer and wait for the second request.
        pFirstEndBindRequest->lastTick = TickGet();
        pFirstEndBindRequest->flags.bits.bSendResponse = 0;
        
        // Set the background flag for processing end device binding.
        zdoStatus.flags.bits.bEndDeviceBinding = 1;
        return NO_PRIMITIVE;
    }
    else
    {
        // We can only process one request at a time!  So make sure we aren't really receiving
        // a new request.  We can tell by checking the bSendResponse flag for the first request.
        // If we are still trying to send a response, then we are not ready for a new request,
        // and we'll have to discard it.
        if (pFirstEndBindRequest->flags.bits.bSendResponse)
        {
            return NO_PRIMITIVE;
        }
        // We are receiving the second request.  Gather all of the information from
        // this request so we can check for matches.
        if ((pSecondEndBindRequest = GetOneEndDeviceBindRequest( pRequestInfo )) == NULL)
        {
            goto BailFromEndDeviceRequest;
        }

        // See if there are any matches between the first and the second requests.
        if (pFirstEndBindRequest->profileID.Val == pSecondEndBindRequest->profileID.Val)
        {
            // Check first request's input clusters against second request's output clusters
            for (firstIndex=0, firstClusterPtr = pFirstEndBindRequest->inClusterList;
                 firstIndex<pFirstEndBindRequest->numInClusters;
                 firstIndex++, firstClusterPtr++)
            {
                for (secondIndex=0, secondClusterPtr = pSecondEndBindRequest->outClusterList;
                     secondIndex<pSecondEndBindRequest->numOutClusters;
                     secondIndex++, secondClusterPtr++)
                {
                    if (*firstClusterPtr == *secondClusterPtr)
                    {
                        /* finish the entire loop so that a count of matched Cluster IDs can be obtained */
                        if(status != SUCCESS)
                        {
                            /* just keep the results of the 1st match found */
                            status = SUCCESS;
                            pSecondEndBindRequest->flags.bits.bSendUnbind = 1;
                            zdoStatus.flags.bits.bEDUnbind = 1;
                            matchCluster1 = *secondClusterPtr;
                        }
                        pFirstEndBindRequest->NumClusterMatched++; 
                        
                    }
                }
            }

            status = ZDO_NO_MATCH;
                
            // Check first request's output clusters against second request's input clusters
            for (firstIndex=0, firstClusterPtr = pFirstEndBindRequest->outClusterList;
                 firstIndex<pFirstEndBindRequest->numOutClusters;
                 firstIndex++, firstClusterPtr++)
            {
                for (secondIndex=0, secondClusterPtr = pSecondEndBindRequest->inClusterList;
                     secondIndex<pSecondEndBindRequest->numInClusters;
                     secondIndex++, secondClusterPtr++)
                {
                    if (*secondClusterPtr == *firstClusterPtr)
                    {
                        /* finish the entire loop so that a count of matched Cluster IDs can be obtained */
                        if(status != SUCCESS)
                        {
                            /* just keep the results of the 1st match found */
                            status = SUCCESS;
                            pFirstEndBindRequest->flags.bits.bSendUnbind = 1;
                            zdoStatus.flags.bits.bEDUnbind = 1;
                            matchCluster2 = *firstClusterPtr;
                        }
                        pSecondEndBindRequest->NumClusterMatched++; 
                    }
                }
            } 
        }
    }

BailFromEndDeviceRequest:
    // Send the responses to the two requestors.  Since each requires its own message, we must send one from
    // the background.  We'll end the second request here, and the first from the background, since pSecondEndBindRequest
    // is a local variable.  We'll clear zdoStatus.flags.bits.bEndDeviceBinding from the background after all of the
    // responses are sent.

    if (pFirstEndBindRequest != NULL)
    {
        pFirstEndBindRequest->status               = status;
        pFirstEndBindRequest->flags.bits.bSendResponse = 1;
    }
    if (pSecondEndBindRequest != NULL)
    {
        /* Don't send response immediately, wait until all the bind rsp  are in (testcase) */
        pSecondEndBindRequest->status                   = status;
        pSecondEndBindRequest->flags.bits.bSendResponse = 1;
    }
    /* Don't send response immediately, wait until all the bind rsp  are in (testcase) */
    nextPrimitive = NO_PRIMITIVE;
    return nextPrimitive;

#else

    // Skip over the header info
    TxData = TX_DATA_START + MSG_HEADER_SIZE;

    TxBuffer[TxData++] = ZDO_NOT_SUPPORTED;
    PrepareMessageResponse( END_DEVICE_BIND_rsp );
    return APSDE_DATA_request;

#endif

}

#endif

/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp(
                        END_DEVICE_BIND_INFO *pBindRequest, BYTE status )
 *
 * PreCondition:    End Device Binding should be in progress.
 *
 * Input:           *pBindRequest - pointer to the end device bind info
 *                  status - status of the end device bind
 *
 * Output:          Next primitive
 *
 * Side Effects:    None
 *
 * Overview:        This function either notifies the application of the
 *                  result of the end device bind, or sends a message to the
 *                  end device with the result.  It then deallocates the
 *                  memory that was allocated for the end device bind.
 *
 * Note:            Do to compiler limitations, we cannot set the
 *                  xxxBindRequest pointer passed in to 0.  Therefore,
 *                  the application must do the final free of the
 *                  xxxBindRequest memory.
 *
 ********************************************************************/
#ifdef SUPPORT_END_DEVICE_BINDING
ZIGBEE_PRIMITIVE Send_END_DEVICE_BIND_rsp( END_DEVICE_BIND_INFO *pBindRequest, BYTE status )
{
    ZIGBEE_PRIMITIVE    nextPrimitive;
    BYTE                *ptr;

    nextPrimitive = NO_PRIMITIVE;
    if (pBindRequest != NULL)
    {
        if (pBindRequest->flags.bits.bFromSelf)
        {
            // If we called Process_END_DEVICE_BIND_req from the upper layers, generate a fake
            // APSDE_DATA_indication to send back the answer.
            if (CurrentRxPacket == NULL)
            {
                if ((CurrentRxPacket = SRAMalloc(4)) != NULL)
                {
                    ptr = CurrentRxPacket;

                    // Load header information.
                    *ptr++ = pBindRequest->sequenceNumber;      // Transaction Sequence Number

                    // Load data.
                    *ptr = status;

                    // Populate the remainder of the parameters.
                    params.APSDE_DATA_indication.asduLength             = 4;
                    params.APSDE_DATA_indication.SecurityStatus         = FALSE;
                    params.APSDE_DATA_indication.asdu                   = CurrentRxPacket;
                    params.APSDE_DATA_indication.ProfileId.Val          = ZDP_PROFILE_ID;
                    params.APSDE_DATA_indication.SrcAddrMode            = APS_ADDRESS_16_BIT;
                    params.APSDE_DATA_indication.WasBroadcast           = FALSE;
                    params.APSDE_DATA_indication.SrcAddress.ShortAddr   = macPIB.macShortAddress;
                    params.APSDE_DATA_indication.SrcEndpoint            = EP_ZDO;
                    params.APSDE_DATA_indication.DstEndpoint            = EP_ZDO; // should always be EP0 pBindRequest->sourceEP;
                    params.APSDE_DATA_indication.ClusterId.Val          = END_DEVICE_BIND_rsp;

                    nextPrimitive = APSDE_DATA_indication;
                }
            }
        }
        else
        {
            // Skip over the header info
            ZigBeeBlockTx();
            TxData = TX_DATA_START + MSG_HEADER_SIZE;

            TxBuffer[TxData++] = status;

            // Load the message information.  We have to patch the destination address.
            sequenceNumber = pBindRequest->sequenceNumber;
            // should always be EP0 sourceEP = pBindRequest->sourceEP;
            PrepareMessageResponse( END_DEVICE_BIND_rsp );
            params.APSDE_DATA_request.DstAddress.ShortAddr  = pBindRequest->shortAddr;

            #ifdef ENABLE_DEBUG
            {
                BYTE    buffer[60];

                sprintf( (char *)buffer, (ROM char *) "Sending end device bind response %d to %04x.\r\n\0",
                    status, pBindRequest->shortAddr.Val );
                ConsolePutString( buffer );
            }
            #endif
            nextPrimitive = APSDE_DATA_request;
        }

    }

    return nextPrimitive;
}
#endif

/*********************************************************************
 * Function:        void PrepareMessageResponse( WORD clusterID )
 *
 * PreCondition:    Tx is not blocked, all data is loaded and TxData
 *                  is accurate.
 *
 * Input:           clusterID - cluster ID of the ZDO packet
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function prepares the APSDE_DATA_request
 *                  parameters as needed by the ZDO responses.
 *
 * Note:            None
 *
 ********************************************************************/
void PrepareMessageResponse( WORD clusterID )
{
    ZigBeeBlockTx();

    // Load the header information
    TxBuffer[TX_DATA_START]   = sequenceNumber;                      // Transaction Sequence Number of the request

    // We only get short addresses from the NWK layer.
    /* For ZigBee 2006 :  We need to verify address mode used by the
     * sender and respond accordingly, because if long address was used previous
     * version would fail here
     * Note:  The NLDE_indication changed this to a long address, so we must be 
     * consistent check for the address mode here again.
    */
    params.APSDE_DATA_request.DstAddrMode   = APS_ADDRESS_16_BIT;
    if( params.ZDO_DATA_indication.SrcAddrMode == APS_ADDRESS_64_BIT )
    {
        if( APSFromLongToShort(&params.ZDO_DATA_indication.SrcAddress.LongAddr) )
        {
            params.APSDE_DATA_request.DstAddress.ShortAddr = currentAPSAddress.shortAddr;
        }
    }
    else
    {
        params.APSDE_DATA_request.DstAddress.ShortAddr.Val = params.ZDO_DATA_indication.SrcAddress.ShortAddr.Val;
    }
            
    // params.APSDE_DATA_request.asduLength; in place with TxData
    params.APSDE_DATA_request.ProfileId.Val = ZDP_PROFILE_ID;
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS; 
	#ifdef I_SUPPORT_SECURITY
    	params.APSDE_DATA_request.TxOptions.Val = 0x01;
	#else
    	params.APSDE_DATA_request.TxOptions.Val = 0x00;
	#endif

    params.APSDE_DATA_request.SrcEndpoint   = EP_ZDO;   // should always be EP0 sourceEP;
    params.APSDE_DATA_request.DstEndpoint   = EP_ZDO;
    params.APSDE_DATA_request.ClusterId.Val     = clusterID;
}


void handleActiveEPReq(void)
{
    NODE_SIMPLE_DESCRIPTOR  simpleDescriptor;
    BYTE                    wasBroadcast;
    BYTE i;

    // Save the broadcast indication for later.
    wasBroadcast = params.ZDO_DATA_indication.WasBroadcast;

    // Skip over the header info
    TxData = TX_DATA_START + MSG_HEADER_SIZE;

    // Load status
    TxBuffer[TxData++] = SUCCESS;

    // Load our short address.
    TxBuffer[TxData++] = macPIB.macShortAddress.byte.LSB;
    TxBuffer[TxData++] = macPIB.macShortAddress.byte.MSB;

    // Load the endpoint count
    /* For ZigBee 2006: The number of endpoints should exclude EP0 */
    TxBuffer[TxData++] = NUM_USER_ENDPOINTS; 

    for (i=1; i<NUM_DEFINED_ENDPOINTS; i++)  /* For ZigBee 2006 - Do not include EP0 */
    {
        ProfileGetSimpleDesc( &simpleDescriptor, i );
        TxBuffer[TxData++] = simpleDescriptor.Endpoint;
    }

    PrepareMessageResponse( ACTIVE_EP_rsp );

    // This request may have been broadcast. If so, we must request an APS ACK on the response.
    if (wasBroadcast)
    {
        params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
    }    
}

BOOL handleMatchDescReq(void)
{
    BOOL                    checkProfileOnly;
    BYTE                    descIndex;
    WORD_VAL                *inClusterList;
    BYTE                    listIndex;
    BOOL                    match;
    BYTE                    numInClusters;
    BYTE                    numMatchingEPs;
    BYTE                    numOutClusters;
    WORD_VAL                *outClusterList;
    WORD_VAL                profileID;
    NODE_SIMPLE_DESCRIPTOR  *simpleDescriptor;
    BYTE                    wasBroadcast;
    BYTE                    i;

    // Save the broadcast indication for later.
    wasBroadcast = params.ZDO_DATA_indication.WasBroadcast;

    checkProfileOnly   = FALSE;
    inClusterList      = NULL;
    numMatchingEPs     = 0;
    outClusterList     = NULL;

    if ((simpleDescriptor = (NODE_SIMPLE_DESCRIPTOR *)SRAMalloc( sizeof(NODE_SIMPLE_DESCRIPTOR) )) == NULL)
    {
        goto BailFromMatchDesc;
    }

    profileID.byte.LSB = ZDOGet();
    profileID.byte.MSB = ZDOGet();
    numInClusters = ZDOGet();
    if (numInClusters)
    {
        if ((inClusterList = (WORD_VAL *)SRAMalloc( numInClusters * sizeof(WORD) )) == NULL)
        {
            goto BailFromMatchDesc;
        }
        for (i=0; i<numInClusters; i++)
        {
            inClusterList[i].v[0] = ZDOGet();
            inClusterList[i].v[1] = ZDOGet();
        }
    }
    numOutClusters = ZDOGet();
    if (numOutClusters)
    {
        if ((outClusterList = (WORD_VAL *)SRAMalloc( numOutClusters * sizeof(WORD) )) == NULL)
        {
            goto BailFromMatchDesc;
        }
        for (i=0; i<numOutClusters; i++)
        {
            outClusterList[i].v[0] = ZDOGet();
            outClusterList[i].v[1] = ZDOGet();
        }
    }

    if ((numInClusters == 0) && (numOutClusters == 0))
    {
        checkProfileOnly = TRUE;
    }

    // Set the data pointer to the matching endpoint list in the response message.
    TxData += 4 + MSG_HEADER_SIZE;

    // See if any of the input clusters match any of our input clusters, or if
    // any of the output clusters match any of our output clusters, in any of our
    // endpoints with a given profile ID.  Do not check the ZDO endpoint (0).
    for (i=1; i < NUM_DEFINED_ENDPOINTS; i++)
    {
        match = FALSE;
        ProfileGetSimpleDesc( simpleDescriptor, i );
        if (simpleDescriptor->AppProfId.Val == profileID.Val)
        {
            if (checkProfileOnly)
            {
                match = TRUE;
            }
            else
            {
                for (descIndex=0; descIndex<simpleDescriptor->AppInClusterCount; descIndex++)
                {
                    for (listIndex=0; listIndex<numInClusters; listIndex++)
                    {
                        /* for Zigbee 2006: Bug fix so that comparison will work */
                        if (inClusterList[listIndex].Val == simpleDescriptor->AppInClusterList[descIndex])
                        {
                            match = TRUE;
                            goto descrMatchFound; /* exit at the first match */
                        }
                    }
                }
                for (descIndex=0; descIndex<simpleDescriptor->AppOutClusterCount; descIndex++)
                {
                    for (listIndex=0; listIndex<numOutClusters; listIndex++)
                    {
                        /* for ZigBee 2006: Bug fix such that comparison will work */
                        if (outClusterList[listIndex].Val == simpleDescriptor->AppOutClusterList[descIndex])
                        {
                            match = TRUE;
                            goto descrMatchFound; /* exit at the first match */
                        }
                    }
                }
            }
        }
    }

descrMatchFound:
    if (match)
    {
        TxBuffer[TxData++] = simpleDescriptor->Endpoint;
        numMatchingEPs++;
    }

    // If there are no matching endpoints, do not send a response.
    if (!numMatchingEPs) goto BailFromMatchDesc;

    // Load the response message and send it.  TxData is already set from
    // above, so we'll use i to load up the initial part of the response.
    // Skip over the header info.
    i = TX_DATA_START + MSG_HEADER_SIZE;

    // Load status
    TxBuffer[i++] = SUCCESS;

    // Load our short address.
    TxBuffer[i++] = macPIB.macShortAddress.byte.LSB;
    TxBuffer[i++] = macPIB.macShortAddress.byte.MSB;

    // Load the number of matching endpoints. The endpoints themselves are already there.
    TxBuffer[i++] = numMatchingEPs;

    PrepareMessageResponse( MATCH_DESC_rsp );

    // This request may have been broadcast. If so, we must request an APS ACK on the response.
    if (wasBroadcast)
    {
        params.APSDE_DATA_request.TxOptions.bits.acknowledged = 1;
    }
    return TRUE;

BailFromMatchDesc:
    if (simpleDescriptor != NULL)
        nfree( simpleDescriptor );
    if (inClusterList != NULL)
        nfree( inClusterList );
    if (outClusterList != NULL)
        nfree( outClusterList );
        
    return FALSE;
                                
}



/*********************************************************************
 * Function:        BOOL ProcessBindAndUnbind( BIND_INFO bindInfo,
                    LONG_ADDR *sourceAddress, LONG_ADDR *destinationAddress )
 *
 * PreCondition:    If bindInfo.bFromUpperLayers is false, we are coming
 *                  from ZDO_DATA_indication, and asdu still points to the
 *                  beginning of the data area.
 *
 * Input:           bindInfo - the bind direction (bind/unbind) and if the
 *                      request is from upper layers or a received message.
 *
 * Output:          TRUE - process completed successfully, response ready
 *                  FALSE - response not generated
 *                  params.ZDO_BIND_req.Status updated
 *
 * Side Effects:    Binding created/destroyed.
 *
 * Overview:        This function performs a bind or unbind request.  The
 *                  request can be from either the upper layers or a
 *                  received message.
 *
 * Note:            The parameters for ZDO_BIND_req and ZDO_UNBIND_req
 *                  overlay each other, so we will just reference
 *                  ZDO_BIND_req parameters.
 *                  NOTE the spec is very vague on this function.  The
 *                  bind/unbind functions receive long addresses, but
 *                  bindings are created with short addresses. The spec
 *                  does not say how to obtain the short address.
 *
 *                  Coordinators may bind anyone to anyone.  Routers
 *                  may only bind themselves as the source.
 *
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)

BOOL ProcessBindAndUnbind( BYTE bindInfo, LONG_ADDR *sourceAddress, BYTE destAddrMode, ADDR *destinationAddress )
{
    int         i;
    BOOL        needInfo;
    SHORT_ADDR  tempSrcAddress;
    SHORT_ADDR  tempDstAddress;
    SHORT_ADDR  tempRequestorAddress;

    needInfo = FALSE;
    params.ZDO_BIND_req.Status = SUCCESS;
    tempRequestorAddress       = params.ZDO_DATA_indication.SrcAddress.ShortAddr;
    // Make sure that specified source and destination long addresses are
    // in our PAN.

    #if !defined (I_AM_COORDINATOR) && !defined (I_AM_ROUTER)
        if (NWKThisIsMyLongAddress( sourceAddress ))
        {
            // I am the source
            tempSrcAddress = macPIB.macShortAddress;
        }
        else
        {
            // Routers can only bind themselves as the source.
            params.ZDO_BIND_req.Status = BIND_NOT_SUPPORTED;
        }
    #else

        if ( NWKLookupNodeByLongAddr( sourceAddress ) == INVALID_NEIGHBOR_KEY )
        {
            if (NWKThisIsMyLongAddress( sourceAddress ))
            {
                // It is my address
                tempSrcAddress = macPIB.macShortAddress;
            }
            else
            {
                #if MAX_APS_ADDRESSES > 0
                if ( APSFromLongToShort(sourceAddress ) )
                {
                    tempSrcAddress = currentAPSAddress.shortAddr;
                }
                else
                #endif
                {
                    // Unknown source address.
                    tempSrcAddress.Val = 0xFFFF;
                    needInfo = TRUE;
                }
            }
        }
        else
        {
            tempSrcAddress = currentNeighborRecord.shortAddr;
        }
    #endif

    if( destAddrMode != APS_ADDRESS_GROUP )
    {
        if ( NWKLookupNodeByLongAddr( &destinationAddress->LongAddr ) == INVALID_NEIGHBOR_KEY )
        {
            if (NWKThisIsMyLongAddress( &destinationAddress->LongAddr ))
            {
                // It is my address
                tempDstAddress = macPIB.macShortAddress;
            }
            else
            {
                #if MAX_APS_ADDRESSES > 0
                if ( APSFromLongToShort( &(destinationAddress->LongAddr) ) )
                {
                    tempDstAddress = currentAPSAddress.shortAddr;
                }
                else
                #endif
                {
                    // Unknown destination address
                    tempDstAddress.Val = 0xFFFF;
                    needInfo = TRUE;
                }
            }
        }
        else
        {
            tempDstAddress = currentNeighborRecord.shortAddr;
        }
    } // if destAddMode
    else
    {
        tempDstAddress.Val = destinationAddress->ShortAddr.Val;
    }
    
    
    // Load the parameters in case we need them
    if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_EXTERNAL)
    {
        /* If the request is from the upper layers, the parameters are already in place.
         * Otherwise, load them.
        * Skip over the source address.
        */
        params.ZDO_BIND_req.SrcEndp         = *(params.ZDO_DATA_indication.asdu + 8);
        params.ZDO_BIND_req.ClusterID.v[0]  = *(params.ZDO_DATA_indication.asdu + 9);
        params.ZDO_BIND_req.ClusterID.v[1]  = *(params.ZDO_DATA_indication.asdu + 10);
        params.ZDO_BIND_req.DstAddrMode     = *(params.ZDO_DATA_indication.asdu + 11);

        params.ZDO_DATA_indication.asdu += 12; 
        

        // Skip over the destination address.
        if( params.ZDO_BIND_req.DstAddrMode != APS_ADDRESS_GROUP )
        {
            params.ZDO_DATA_indication.asdu += 8;
            params.ZDO_BIND_req.DstEndp = ZDOGet();
        }
        else
        {
            params.ZDO_BIND_req.DstEndp = 0xFE;
        }
    }

    // If we need more information, save off what we have and trigger background processing
    if (needInfo)
    {
        if ((pBindInProgressInfo != NULL) ||
            ((pBindInProgressInfo = (BIND_IN_PROGRESS_INFO *)SRAMalloc( sizeof(BIND_IN_PROGRESS_INFO) )) == NULL))
        {
            // We are already waiting for one binding; we cannot do two.
            // Or we could not allocate memory for the bind information.
            params.ZDO_BIND_req.Status = BIND_NOT_SUPPORTED;
            goto ReturnBindResult;
        }

        for (i=0; i<8; i++)
        {
            pBindInProgressInfo->sourceAddressLong.v[i]         = sourceAddress->v[i];
        }
        pBindInProgressInfo->sourceAddressShort                 = tempSrcAddress;
        pBindInProgressInfo->sourceEP                           = params.ZDO_BIND_req.SrcEndp;
        pBindInProgressInfo->cluster.Val                        = params.ZDO_BIND_req.ClusterID.Val;
        for (i=0; i<8; i++)
        {
            pBindInProgressInfo->destinationAddressLong.v[i]    = destinationAddress->v[i];
        }
        pBindInProgressInfo->destinationAddressShort            = tempDstAddress;
        pBindInProgressInfo->destinationEP                      = params.ZDO_BIND_req.DstEndp;
        pBindInProgressInfo->timeStamp                          = TickGet();
        pBindInProgressInfo->sequenceNumber                     = sequenceNumber;   // Note - this is garbage if it came from our upper layers. But it's not used.
        pBindInProgressInfo->status.val                         = 0;
        
        if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_EXTERNAL)
        {
            /* For Zigbee2006: ZDO_indication was over written so can't use here */
            pBindInProgressInfo->requestorAddress   = tempRequestorAddress;
        }
        else
        {
            // We don't really need this - we know it's from our upper layers.
            //pBindInProgressInfo->requestorAddress             = macPIB.macShortAddress;
        }

        // Set the bind direction.
        if ((bindInfo & BIND_DIRECTION_MASK) == BIND_NODES)
        {
            pBindInProgressInfo->status.bits.bBindNodes             = 1;
        }
     

        // Set where the bind request came from.
        if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_UPPER_LAYERS)
        {
            pBindInProgressInfo->status.bits.bFromUpperLayers       = 1;
        }

        // Set the status flags so we don't request addresses that we already have.
        if (tempSrcAddress.Val != 0xFFFF)
        {
            pBindInProgressInfo->status.bits.bSourceRequested       = 1;
        }
        if (tempDstAddress.Val != 0xFFFF)
        {
            pBindInProgressInfo->status.bits.bDestinationRequested  = 1;
        }

        zdoStatus.flags.bits.bBinding = 1;

        return FALSE;        
    }
    else
    {
        params.ZDO_BIND_req.SrcAddress.ShortAddr = tempSrcAddress;
        params.ZDO_BIND_req.DstAddress.ShortAddr = tempDstAddress;
    }

    if (params.ZDO_BIND_req.Status == SUCCESS)
    {
        if ((bindInfo & BIND_DIRECTION_MASK) == BIND_NODES)
        {
            if (APSAddBindingInfo( params.ZDO_BIND_req.SrcAddress.ShortAddr, params.ZDO_BIND_req.SrcEndp,
                params.ZDO_BIND_req.ClusterID, params.ZDO_BIND_req.DstAddress.ShortAddr, params.ZDO_BIND_req.DstEndp))
            {
                params.ZDO_BIND_req.Status = ZDO_TABLE_FULL;
            }
        }
        else
        {
            if (APSRemoveBindingInfo( params.ZDO_BIND_req.SrcAddress.ShortAddr, params.ZDO_BIND_req.SrcEndp,
                params.ZDO_BIND_req.ClusterID, params.ZDO_BIND_req.DstAddress.ShortAddr, params.ZDO_BIND_req.DstEndp))
            {
                params.ZDO_BIND_req.Status = ZDO_NO_ENTRY;
            }
        }
    }

ReturnBindResult:

    if ((bindInfo & BIND_SOURCE_MASK) == BIND_FROM_UPPER_LAYERS)
    {
        return SendUpBindResult( params.ZDO_BIND_req.Status, (bindInfo & BIND_DIRECTION_MASK) );
    }
    return TRUE;
}

#endif


/*********************************************************************
 * Function:        void SendBindAddressRequest( BYTE requestSource )
 *
 * PreCondition:    pBindInProgressInfo must be non-NULL and point to
 *                  valid information
 *
 * Input:           requestSource - if we need to request the source
 *                  address (TRUE) or the destination address (FALSE)
 *
 * Output:          None.
 *
 * Side Effects:    Message is loaded for transmission.
 *
 * Overview:        This function requests either the source or destination
 *                  address of a binding that is in progress.
 *
 * Note:            The NWK_ADDR_rsp must be captured.
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)

void SendBindAddressRequest( BYTE requestSource )
{
    BYTE i;
    
    // Send NWK_ADDR_req message
    ZigBeeBlockTx();

    TxBuffer[TxData++] = ZDOCounter++;

    // IEEEAddr
    if (requestSource)
    {
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = pBindInProgressInfo->sourceAddressLong.v[i];
        }
    }
    else
    {
        for(i = 0; i < 8; i++)
        {
            TxBuffer[TxData++] = pBindInProgressInfo->destinationAddressLong.v[i];
        }
    }

    // RequestType
    TxBuffer[TxData++] = 0x00;

    // StartIndex
    TxBuffer[TxData++] = 0x00;

    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
    params.APSDE_DATA_request.DstEndpoint = EP_ZDO;
    params.APSDE_DATA_request.DstAddress.ShortAddr.Val = 0xFFFF;

    params.APSDE_DATA_request.ProfileId.Val = ZDO_PROFILE_ID;
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
	#ifdef I_SUPPORT_SECURITY
    	params.APSDE_DATA_request.TxOptions.Val = 1;
	#else
    	params.APSDE_DATA_request.TxOptions.Val = 0;
	#endif
    params.APSDE_DATA_request.SrcEndpoint = EP_ZDO;
    params.APSDE_DATA_request.ClusterId.Val = NWK_ADDR_req;
}
#endif


/*********************************************************************
 * Function:        BOOL SendUpBindResult( BYTE status, BYTE bindNodes )
 *
 * PreCondition:    None
 *
 * Input:           status - binding status to send
 *                  bindNodes - 1 = bind, 0 = unbind
 *
 * Output:          TRUE - packet created
 *                  FALSE - packet not created
 *
 * Side Effects:    Fake APSDE message is created
 *
 * Overview:        This function creates a fake APSDE message in
 *                  response to a bind request made from the upper
 *                  layers.
 *
 * Note:            None
 ********************************************************************/
#if defined(I_SUPPORT_BINDINGS)

BOOL SendUpBindResult( BYTE status, BYTE bindNodes )
{
    BYTE    *ptr;

    // Create a fake APSDE_DATA_indication with the answer, as though it came from
    // a different node. This way the user can capture the response the same way for
    // both sources.
    if (CurrentRxPacket == NULL)
    {
        if ((CurrentRxPacket = SRAMalloc(4)) != NULL)
        {
            ptr = CurrentRxPacket;

            *ptr++ = 0;                                 // Transaction Sequence Number

            // Load data.
            *ptr = status;

            // Populate the remainder of the parameters.
            params.APSDE_DATA_indication.asduLength                 = 4;
            params.APSDE_DATA_indication.SecurityStatus             = FALSE;
            params.APSDE_DATA_indication.asdu                       = CurrentRxPacket;
            params.APSDE_DATA_indication.ProfileId.Val              = ZDP_PROFILE_ID;
            params.APSDE_DATA_indication.SrcAddrMode                = APS_ADDRESS_16_BIT;
            params.APSDE_DATA_indication.WasBroadcast               = FALSE;
            params.APSDE_DATA_indication.SrcAddress.ShortAddr       = macPIB.macShortAddress;
            params.APSDE_DATA_indication.SrcEndpoint                = EP_ZDO;
            params.APSDE_DATA_indication.DstEndpoint                = EP_ZDO;
            if (bindNodes)
            {
                params.APSDE_DATA_indication.ClusterId.Val          = BIND_rsp;
            }
            else
            {
                params.APSDE_DATA_indication.ClusterId.Val          = UNBIND_rsp;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}
#endif
