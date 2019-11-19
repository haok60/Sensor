/*

ZigBee2006 Compliant Platform Profile Definition File

This ZigBee Profile header file provides all of the constants needed to
implement a device that is conforms to the ZigBee Home Control, Lighting
profile.

Mandatory Clusters shall be used in all devices and require all attributes
to be full implemented with the designated data type.

Optional Clusters may be unsupported.  However, if a cluster is supported,
then all of that cluster's attributes shall be supported.

Refer to the ZigBee Specification for the Mandatory and Optional clusters
for each device.


 *********************************************************************
 * FileName:        zTest.h
 * Dependencies:
 * Processor:       PIC18F/PIC24F
 * Complier:        MCC18 v3.20 or higher
 * Complier:        MCC30 v3.10 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC® microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC microcontroller products.
 *
 * You may not modify or create derivatives works of the Software.
 *
 * If you intend to use the software in a product for sale, then you must
 * be a member of the ZigBee Alliance and obtain any licenses required by
 * them.  For more information, go to www.zigbee.org.
 *
 * The software is owned by the Company and/or its licensor, and is
 * protected under applicable copyright laws. All rights are reserved.
 *
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
*/

#ifndef _ZTEST_H_
#define _ZTEST_H_

#define MY_PROFILE_ID                       0x0103
#define MY_PROFILE_ID_MSB                   0x01
#define MY_PROFILE_ID_LSB                   0x03

#define TESTZCP_PROFILE_ID_MSB              0x7f
#define TESTZCP_PROFILE_ID_LSB              0x01

#define TEST_DRIVER_DEV_ID      0x0000   //test driver
#define TEST_DRIVER_DEV_ID_MSB  0x00
#define TEST_DRIVER_DEV_ID_LSB  0x00
#define TEST_DRIVER_DEV_VER     0x00

#define KVP_DEV_ID      0x5555   //KVP device under test
#define KVP_DEV_ID_MSB  0x55
#define KVP_DEV_ID_LSB  0x55
#define KVP_DEV_VER     0x00

#define MSG_DEV_ID        0xaaaa   //MSG device under test
#define MSG_DEV_ID_MSB    0xaa
#define MSG_DEV_ID_LSB    0xaa
#define MSG_DEV_VER       0x00

#define FULL_DEV_ID       0xffff   //Full device under test
#define FULL_DEV_ID_MSB   0xff
#define FULL_DEV_ID_LSB   0xff
#define FULL_DEV_VER      0x00



#define MY_STACK_PROFILE_ID                 0x01
#define PROFILE_nwkSecurityLevel            0x05        // TODO: should be 5 when security supported
#define PROFILE_nwkSecureAllFrames          0x01

#define PROFILE_nwkMaxChildren              20
#define PROFILE_nwkMaxDepth                 5
#define PROFILE_nwkMaxRouters               6


#define PROFILE_MinBindings                     15
#define PROFILE_MinCoordinatorNeighbors         24
#define PROFILE_MinRouterNeighbors              24
#define PROFILE_MinEndDeviceNeighbors           1
#define PROFILE_MinRouteDiscoveryTableSize      4
#define PROFILE_MinRoutingTableSize             8
#define PROFILE_MinReservedRoutingTableEntries  8

//******************************************************************************
// Distributed Address Assignment Constants
//
// These should be calculated manually and placed here.  They are calculated by
// the following formulae.  CSKIP values should be generated until the max
// depth is reached or until CSKIP equals 0.
//
//  Cskip(d) =  if PROFILE_nwkMaxRouters is 1 =
//                  1 + Cm * (Lm - d - 1)
//              otherwise =
//                  1 + Cm - Rm - (Cm * Rm^(Lm - d - 1))
//                  ------------------------------------
//                                1 - Rm
//  where
//      Cm = PROFILE_nwkMaxChildren
//      Lm = PROFILE_nwkMaxDepth
//      Rm = PROFILE_nwkMaxRouters
//      d  = depth of node in the network

#define CSKIP_DEPTH_0                       0x143D
#define CSKIP_DEPTH_1                       0x035D
#define CSKIP_DEPTH_2                       0x008D
#define CSKIP_DEPTH_3                       0x0015
#define CSKIP_DEPTH_4                       0x0001
#define CSKIP_DEPTH_5                       0x0000

//******************************************************************************
//WSAN Clusters
#define JOIN_CONFIRM_CLUSTER			0x0005
#define STATE_NODE_CLUSTER			0x0006
#define ACTOR_REQUEST_CLUSTER			0x0007
#define ACTOR_RESPONSE_CLUSTER  		0x0008
#define HTE_REQUEST_CLUSTER			0x0009
#define HTE_RESPONSE_CLUSTER			0x000A
#define SAVE_ENERGY_SYN_CLUSTER                 0x000B
#define SAVE_ENERGY_CONFIRM_CLUSTER		0x000C
#define ROUTING_TABLE_READ_CLUSTER		0x000D
#define SEND_ROUTING_TABLE_CONFIRM_CLUSTER      0x000E
#define HTE_AUTO_SEND_CLUSTER                   0x000F
#define ACTIVE_SYN_CLUSTER 			0x0010
#define ACTIVE_CONFIRM_CLUSTER			0x0011
#define RE_CONFIG_CYCLE_CLUSTER                 0x0012
#define RE_ASSIGN_LEVEL_MQ6_CLUSTER             0x0013
#define ROUTE_DRAW_REQUEST_CLUSTER		0x0014
#define ROUTE_DRAW_REPLY_CLUSTER		0x0015

#define REQUEST_NODE                            0x0016//cho ban tin broadcast
#define RESPONSE_NODE                           0x0017//cho ban tin broadcast
//CMR
#define CMR_REQUEST_CLUSTER			0x0019
#define CMR_RESPONSE_CLUSTER			0x001A
#define CMR_AUTO_SEND_CLUSTER                   0x001F
#define PACKET_REQUEST_CLUSTER                  0x0020
#define PACKET_RESPONSE_CLUSTER                 0x0021
////RETASK_REQUEST
#define STATUS_REQUEST                              0x0022
#define STATUS_RESPONSE                             0x0023
#define RETASK_FUNCTION                             0x0024
#define NONE_SUPPORT                                0x0025
//Reset
#define RESET_NODE                                  0x0026
//temp
#define STATUSTEMP_REQUEST                      0x0027
#define STATUSTEMP_RESPONSE                     0x0028
//******************************************************************************
// Mandatory Clusters

#define MANAGE_NO_DATA_ATTRIBUTE_CLUSTER			0x0000	//Manage no data attribute
#define TRANSMIT_COUNTED_PACKETS_CLUSTER			0x0001	//Transmit Counted packets
#define	RESET_PACKET_COUNT_CLUSTER				0x0002	//Reset packet count
#define RETRIEVE_PACKET_COUNT_CLUSTER				0x0003	//Retrieve packet count
#define PACKET_COUNT_RESPONSE_CLUSTER				0x0004	//Packet count response
#define BUFFER_TEST_REQUEST_CLUSTER				0x001C	//Buffer test request
#define MANAGE_8BIT_INTEGER_ATTRIBUTES_CLUSTER                  0x0038	//Mange 8-bit integer attributes
#define BUFFER_TEST_RESPONSE_CLUSTER				0x0054	//Buffer test response
#define MANAGE_16BIT_INTEGER_ATTRIBUTES_CLUSTER                 0x0070	//Manage 16-bit integer attributes
#define MANAGE_SEMI_PRECISION_ATTRIBUTES_CLUSTER                0x008C	//Manage semi-precision attribute
#define FREEFORM_MSG_REQUEST_CLUSTER				0xa0A8	//Freeform MSG request
#define MANAGE_TIME_ATTRIBUTES_CLUSTER				0x00C4	//Manage time atributes	
#define FREEFORM_MSG_RESPONSE_CLUSTER				0xe000	//Freeform MSG response
#define MANAGE_STRING_ATTRIBUTES_CLUSTER			0x00FF	//Manage string attributes
#define REVERSE_FREEFORM_MSG_RESPONSE_CLUSTER                   0x00e0  //Reverse Freeform MSG response



//******************************************************************************
// Mandatory Cluster Attributes

//------------------------------------------------------------------------------
// Cluster MANAGE_NO_DATA_ATTRIBUTE_CLUSTER
#define NoDataAttr                          0x0000  //No data test attribute

//------------------------------------------------------------------------------
// Cluster MANAGE_8BIT_INTEGER_ATTRIBUTES
#define UInt8Attr						0x1C71	// default 0x12
#define Int8Attr						0x38E2	// default 0x80

//------------------------------------------------------------------------------
// Cluster MANAGE_16BIT_INTEGER_ATTRIBUTES
#define UInt16Attr						0x5553	// default 0x1234
#define Int16Attr						0x71C4	// default 0x8000

//------------------------------------------------------------------------------
// Cluster MANAGE_SEMI_PRECISION_INTEGER_ATTRIBUTES
#define SemiPrecAttr						0x8E35	// default 0x0412

//------------------------------------------------------------------------------
// Cluster MANAGE_TIME_ATTRIBUTES
#define AbsTimeAttr						0xAAA6	// default 0x12345678
#define RelTimeAttr						0xC717	// default 0xabcdef01

//------------------------------------------------------------------------------
// Cluster MANAGE_STRING_ATTRIBUTES
#define CharStringAttr						0xE388	// default 0x0441424344
#define OctetStringAttr						0xFFFF	// default 0x0412345678



//******************************************************************************
// Check User Assignments

// Check size of routing table
#if defined(I_SUPPORT_ROUTING)
    #if ROUTING_TABLE_SIZE < PROFILE_MinRoutingTableSize
        //#error Routing Table size too small.  Check Profile description.
    #endif
    #if RESERVED_ROUTING_TABLE_ENTRIES < PROFILE_MinReservedRoutingTableEntries
        //#error Reserved Routing Table entries too small.  Check Profile description.
    #endif
#endif

// Check size of neighbor table
#if defined(I_AM_COORDINATOR)
    #if MAX_NEIGHBORS < PROFILE_MinCoordinatorNeighbors
        #error Neighbor Table size too small.  Check Profile description.
    #endif
#elif defined (I_AM_ROUTER)
    #if MAX_NEIGHBORS < PROFILE_MinRouterNeighbors
        #error Neighbor Table size too small.  Check Profile description.
    #endif
#else
    #if MAX_NEIGHBORS < PROFILE_MinEndDeviceNeighbors
        #error Neighbor Table size too small.  Check Profile description.
    #endif
#endif

// Check size of route discovery table
#if defined(I_AM_ROUTER) || defined(I_AM_COORDINATOR)
    #if ROUTE_DISCOVERY_TABLE_SIZE < PROFILE_MinRouteDiscoveryTableSize
        #error Route Discovery Table size too small.  Check Profile description.
    #endif
#endif

// Check size of binding table
#if defined(I_AM_COORDINATOR)
    #if MAX_BINDINGS < PROFILE_MinBindings
        //#error Binding Table size too small.  Check Profile description.
    #endif
#endif

// If coordinator, make sure that end device binding is supported.
#if defined(I_AM_COORDINATOR)
    #ifndef SUPPORT_END_DEVICE_BINDING
        // #error This Profile requires that ZigBee coordinators support End Device Binding.
    #endif
#endif

//with Source Binding make sure EDB and BIND work together
#if defined(SUPPORT_END_DEVICE_BINDING) && !defined(I_SUPPORT_BINDINGS)
	#error Binding && End Device Binding must be selected together.
#endif

//with Source Binding make sure EDB and BIND work together
#if !defined(SUPPORT_END_DEVICE_BINDING) && defined(I_SUPPORT_BINDINGS)
	#error Binding && End Device Binding must be selected together.
#endif

#endif
