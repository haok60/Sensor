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
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#ifndef _zZDO_H_
#define _zZDO_H_

// NOTE: No validity checks are performed by this code.  Use ZENA to ensure
// proper generation of additional constants.


// ******************************************************************************
// Constants and Enumerations

#define EP_ZDO                              0x00
#define EXTENDED_RESPONSE                   0x01    // IEEE/NWK_addr_req parameter
#define NO_CLUSTER                          0x00
#define NO_OTHER_DESCRIPTOR_AVAILABLE       0x00
#define SIMPLE_DESCRIPTOR_BASE_SIZE         8
#define SINGLE_DEVICE_RESPONSE              0x00    // IEEE/NWK_addr_req parameter
#define ZDO_PROFILE_ID                      0x0000
#define ZDO_PROFILE_ID_MSB                  0x00
#define ZDO_PROFILE_ID_LSB                  0x00
#define ZDP_PROFILE_ID                      ZDO_PROFILE_ID
#define ZDP_PROFILE_ID_MSB                  ZDO_PROFILE_ID_MSB
#define ZDP_PROFILE_ID_LSB                  ZDO_PROFILE_ID_LSB

// Error Codes
#define ZDO_INV_REQUESTTYPE                 0x80
#define ZDO_DEVICE_NOT_FOUND                0x81
#define ZDO_INVALID_EP                      0x82
#define ZDO_NOT_ACTIVE                      0x83
#define ZDO_NOT_SUPPORTED                   0x84
#define ZDO_TIMEOUT                         0x85
#define ZDO_NO_MATCH                        0x86
#define ZDO_TABLE_FULL                      0x87
#define ZDO_NO_ENTRY                        0x88

// For backwards compatibility
#define END_DEVICE_BIND_TIMEOUT             0x85
#define END_DEVICE_BIND_NO_MATCH            0x86

typedef enum _ZDO_CLUSTER
{
    NWK_ADDR_req = 0x0000,
    IEEE_ADDR_req,
    NODE_DESC_req,
    POWER_DESC_req,
    SIMPLE_DESC_req,
    ACTIVE_EP_req,
    MATCH_DESC_req,
    COMPLEX_DESC_req = 0x0010,
    USER_DESC_req,
    DISCOVERY_CACHE_req,    // M
    END_DEVICE_annce,
    USER_DESC_set,
    SYSTEM_SERVER_DISCOVERY_req, // new in 2006 M
    DISCOVERY_STORE_req,
    NODE_DESC_STORE_req,
    POWER_DESC_STORE_req,
    ACTIVE_EP_STORE_req,
    SIMPLE_DESC_STORE_req,
    REMOVE_NODE_CACHE_req,
    FIND_NODE_CACHE_req,        // M
    END_DEVICE_BIND_req = 0x0020,
    BIND_req,
    UNBIND_req,
    MGMT_NWK_DISC_req = 0x0030,
    MGMT_LQI_req,
    MGMT_RTG_req,
    MGMT_BIND_req,
    MGMT_LEAVE_req,
    MGMT_DIRECT_JOIN_req,
    MGMT_PERMIT_JOINING_req,
    MGMT_CACHE_req,
    MGMT_NWK_UPDATE_req,
    NWK_ADDR_rsp = 0x8000,
    IEEE_ADDR_rsp,
    NODE_DESC_rsp,
    POWER_DESC_rsp,
    SIMPLE_DESC_rsp,
    ACTIVE_EP_rsp,
    MATCH_DESC_rsp,
    COMPLEX_DESC_rsp = 0x8010,
    USER_DESC_rsp,
    DISCOVERY_CACHE_rsp,
    USER_DESC_conf = 0x8014,
    SYSTEM_SERVER_DISCOVERY_rsp,
    DISCOVERY_STORE_rsp,
    NODE_DESC_STORE_rsp,
    POWER_DESC_STORE_rsp,
    ACTIVE_EP_STORE_rsp,
    SIMPLE_DESC_STORE_rsp,
    REMOVE_NODE_CACHE_rsp,
    FIND_NODE_CACHE_rsp,
    END_DEVICE_BIND_rsp = 0x8020,
    BIND_rsp,
    UNBIND_rsp,
    MGMT_NWK_DISC_rsp = 0x8030,
    MGMT_LQI_rsp,
    MGMT_RTG_rsp,
    MGMT_BIND_rsp,
    MGMT_LEAVE_rsp,
    MGMT_DIRECT_JOIN_rsp,
    MGMT_PERMIT_JOINING_rsp,
    MGMT_CACHE_rsp,
    MGMT_NWK_UPDATE_notify
} ZDO_CLUSTER;


// -----------------------------------------------------------------------------
// Descriptor size calculations

#define NUM_DEFINED_ENDPOINTS   (1+NUM_USER_ENDPOINTS)

// Determine ZDO Input and Output Cluster counts and the sizes for the
// simple descriptor structure.
#ifdef INCLUDE_OPTIONAL_NODE_MANAGEMENT_SERVICES
    #define OPTIONAL_NODE_MANAGEMENT_SERVICES_INPUT_CLUSTERS        6
    #define OPTIONAL_NODE_MANAGEMENT_SERVICES_OUTPUT_CLUSTERS       6
#else
    #define OPTIONAL_NODE_MANAGEMENT_SERVICES_INPUT_CLUSTERS        0
    #define OPTIONAL_NODE_MANAGEMENT_SERVICES_OUTPUT_CLUSTERS       0
#endif
#ifdef INCLUDE_OPTIONAL_SERVICE_DISCOVERY_RESPONSES
    #define SERVICE_DISCOVERY_RESPONSES_INPUT_CLUSTERS     5
    #define SERVICE_DISCOVERY_RESPONSES_OUTPUT_CLUSTERS    4
#else
    #define SERVICE_DISCOVERY_RESPONSES_INPUT_CLUSTERS     0
    #define SERVICE_DISCOVERY_RESPONSES_OUTPUT_CLUSTERS    0
#endif
#ifdef SUPPORT_END_DEVICE_BINDING
    #define END_DEVICE_BINDING_INPUT_CLUSTERS                       1
    #define END_DEVICE_BINDING_OUTPUT_CLUSTERS                      1
#else
    #define END_DEVICE_BINDING_INPUT_CLUSTERS                       0
    #define END_DEVICE_BINDING_OUTPUT_CLUSTERS                      0
#endif
#if defined(I_SUPPORT_BINDINGS)
    #define BINDING_INPUT_CLUSTERS                                  2
    #define BINDING_OUTPUT_CLUSTERS                                 2
#else
    #define BINDING_INPUT_CLUSTERS                                  0
    #define BINDING_OUTPUT_CLUSTERS                                 0
#endif
#define ZDO_BASE_INPUT_CLUSTERS                                     7
#define ZDO_BASE_OUTPUT_CLUSTERS                                    7

#define ZDO_INPUT_CLUSTERS  (OPTIONAL_NODE_MANAGEMENT_SERVICES_INPUT_CLUSTERS +       \
                             SERVICE_DISCOVERY_RESPONSES_INPUT_CLUSTERS +    \
                             END_DEVICE_BINDING_INPUT_CLUSTERS +                      \
                             BINDING_INPUT_CLUSTERS +                                 \
                             ZDO_BASE_INPUT_CLUSTERS )
#if (MY_MAX_INPUT_CLUSTERS > ZDO_INPUT_CLUSTERS)
    #define MAX_INPUT_CLUSTERS  MY_MAX_INPUT_CLUSTERS
#else
    #define MAX_INPUT_CLUSTERS  ZDO_INPUT_CLUSTERS
#endif

#define ZDO_OUTPUT_CLUSTERS  (OPTIONAL_NODE_MANAGEMENT_SERVICES_OUTPUT_CLUSTERS +       \
                              SERVICE_DISCOVERY_RESPONSES_OUTPUT_CLUSTERS +    \
                              END_DEVICE_BINDING_OUTPUT_CLUSTERS +                      \
                              BINDING_OUTPUT_CLUSTERS +                                 \
                              ZDO_BASE_OUTPUT_CLUSTERS )
#if (MY_MAX_OUTPUT_CLUSTERS > ZDO_OUTPUT_CLUSTERS)
    #define MAX_OUTPUT_CLUSTERS MY_MAX_OUTPUT_CLUSTERS
#else
    #define MAX_OUTPUT_CLUSTERS ZDO_OUTPUT_CLUSTERS
#endif

// Verify that the above cluster sizes will not make the simple
// descriptors exceed the maximum size.
#define SIMPLE_DESCRIPTOR_BASE_SIZE         8
#define apscMaxDescriptorSize               64          // Define by ZigBee v1.0
#if (MAX_INPUT_CLUSTERS + MAX_OUTPUT_CLUSTERS + SIMPLE_DESCRIPTOR_BASE_SIZE) > apscMaxDescriptorSize
    #error An endpoint may contain at most 54 input and output clusters.
#endif


// ******************************************************************************
// Macros

#ifdef USE_EXTERNAL_NVM
    #define ZDOGetCapabilityInfo(x)         NVMRead( x, configNodeDescriptor + LOC_NodeMACCapabilityFlags, 1 )
    #define ZDOPutCapabilityInfo(x)         NVMWrite( configNodeDescriptor + LOC_NodeMACCapabilityFlags, (void *)x, 1 )
#else
    #define ZDOGetCapabilityInfo(x)         NVMRead( x, (ROM void*)(&Config_Node_Descriptor.NodeMACCapabilityFlags), 1 )
    #define ZDOPutCapabilityInfo(x)         NVMWrite((NVM_ADDR*)(&Config_Node_Descriptor.NodeMACCapabilityFlags), (void *)x, 1 )
#endif

// ******************************************************************************
// Data Structures

typedef struct _NODE_DESCRIPTOR
{
    unsigned char   NodeLogicalType     : 3;
    unsigned char   filler              : 5;
    unsigned char   NodeAPSFlags        : 3;
    unsigned char   NodeFrequencyBand   : 5;

    BYTE NodeMACCapabilityFlags;
    WORD_VAL NodeManufacturerCode;
    BYTE NodeMaxBufferSize;
    WORD_VAL NodeMaxIncomingTransferSize;
    WORD_VAL NodeServerMask;
    //WORD_VAL NodeMaxOutgoingTransferSize;   /* Zigbee 2006: These are gone */
    //BYTE NodeDescriptorCapability;
} NODE_DESCRIPTOR;
#define sizeof_NODE_DESCRIPTOR 10

#define LOC_NodeMACCapabilityFlags  2       // Offset for external NVM


typedef struct _NODE_POWER_DESCRIPTOR
{
    unsigned char NodeCurrentPowerMode           : 4;
    unsigned char NodeAvailablePowerSources      : 4;
    unsigned char NodeCurrentPowerSource         : 4;
    unsigned char NodeCurrentPowerSourceLevel    : 4;
} NODE_POWER_DESCRIPTOR;


typedef struct _NODE_SIMPLE_DESCRIPTOR
{
    BYTE            Endpoint;

    WORD_VAL        AppProfId;
    WORD_VAL        AppDevId;

    unsigned char    AppDevVer   :4;
    unsigned char    AppFlags    :4;

    BYTE            AppInClusterCount;
    WORD            AppInClusterList[MAX_INPUT_CLUSTERS];
    BYTE            AppOutClusterCount;
    WORD            AppOutClusterList[MAX_OUTPUT_CLUSTERS];
} NODE_SIMPLE_DESCRIPTOR;


typedef struct _ZDO_STATUS
{
    union _ZDO_STATUS_flags
    {
        BYTE    Val;
        struct _ZDO_STATUS_bits
        {
            // Background Task Flags
            BYTE    bEndDeviceBinding   : 1;
            BYTE    bBinding            : 1;
            BYTE    bEDUnbind           : 1;
            #ifdef I_SUPPORT_SECURITY
                BYTE    bSwitchKey      : 1;
            #endif
            #ifdef I_SUPPORT_FREQUENCY_AGILITY
                BYTE    bNwkUpdateEDScan    : 1;
            #endif
            // Status Flags
        } bits;
    } flags;
    
    #ifdef I_SUPPORT_SECURITY
        BYTE KeySeq;
        TICK SwitchKeyTick;
    #endif
} ZDO_STATUS;
#define ZDO_BACKGROUND_TASKS    0x04

// ******************************************************************************
// Function Prototypes

BOOL                ZDOHasBackgroundTasks( void );
void                ZDOInit( void );
ZIGBEE_PRIMITIVE    ZDOTasks(ZIGBEE_PRIMITIVE inputPrimitive);

#endif

