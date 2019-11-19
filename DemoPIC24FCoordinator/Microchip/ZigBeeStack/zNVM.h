/*********************************************************************
 *
 *                  Zigbee non-volatile memory storage header
 *
 *********************************************************************
 * FileName:        zNVM.h
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
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti     10/15/04    Original Version
 * Nilesh Rajbharti     11/1/04     Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#ifndef _ZNVM_H_
#define _ZNVM_H_

#include "zigbee.h"
#include "zNWK.h"
#include "zAPS.h"
#include "zZDO.h"
#include "compiler.h"
//#if !defined(__C30__)
	#include <string.h>         // for memcpy-type functions
//#endif
/*******************************************************************************
Defines
*******************************************************************************/

#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
    #if (MAX_BINDINGS % 8) == 0
        #define BINDING_USAGE_MAP_SIZE  (MAX_BINDINGS/8)
    #else
        #define BINDING_USAGE_MAP_SIZE  (MAX_BINDINGS/8 + 1)
    #endif
#endif

#define NUM_NWK_KEYS    2

/*******************************************************************************
Typedefs
*******************************************************************************/

#if !defined(USE_EXTERNAL_NVM) || !defined(STORE_MAC_EXTERNAL)
    typedef ROM BYTE NVM_ADDR;
#endif

/*******************************************************************************
Variable Definitions
*******************************************************************************/

// MAC Address
#if defined(USE_EXTERNAL_NVM) && defined(STORE_MAC_EXTERNAL)
    extern WORD                         macLongAddrEE;
#else
    extern ROM LONG_ADDR                macLongAddr;
#endif

// Binding Information
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
    extern BINDING_RECORD               currentBindingRecord;
    #ifdef USE_EXTERNAL_NVM
        extern WORD                     apsBindingTable;
        extern WORD                     bindingTableUsageMap;
        extern WORD                     bindingTableSourceNodeMap;
        extern WORD                     bindingValidityKey;
        extern WORD                     pCurrentBindingRecord;
    #else
        extern ROM BINDING_RECORD       apsBindingTable[MAX_BINDINGS];
        extern ROM BYTE                 bindingTableUsageMap[BINDING_USAGE_MAP_SIZE];
        extern ROM BYTE                 bindingTableSourceNodeMap[BINDING_USAGE_MAP_SIZE];
        extern ROM WORD                 bindingValidityKey;
        extern ROM BINDING_RECORD       *pCurrentBindingRecord;
    #endif
#endif

#if defined(I_SUPPORT_GROUP_ADDRESSING)
    #ifdef USE_EXTERNAL_NVM
        extern WORD                    apsGroupAddressTable;
        extern WORD                    pCurrentGroupAddressRecord;
    #else
        extern ROM GROUP_ADDRESS_RECORD    apsGroupAddressTable[MAX_GROUP];
        extern ROM GROUP_ADDRESS_RECORD    *pCurrentGroupAddressRecord;
    #endif
#endif

// Neighbor Table Information
extern NEIGHBOR_RECORD                  currentNeighborRecord;
extern NEIGHBOR_TABLE_INFO              currentNeighborTableInfo;

#ifdef USE_EXTERNAL_NVM
    extern WORD                         neighborTable;
    extern WORD                         neighborTableInfo;
    extern WORD                         pCurrentNeighborRecord;
#else
    extern ROM NEIGHBOR_RECORD          neighborTable[MAX_NEIGHBORS];
    extern ROM NEIGHBOR_TABLE_INFO      neighborTableInfo;
    extern ROM NEIGHBOR_RECORD          *pCurrentNeighborRecord;
#endif

// Routing Information
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
    extern ROUTING_ENTRY                currentRoutingEntry;
    #ifdef USE_EXTERNAL_NVM
        extern WORD                     routingTable;
        extern WORD                     pCurrentRoutingEntry;
    #else
        extern ROM ROUTING_ENTRY        routingTable[ROUTING_TABLE_SIZE];
        extern ROM ROUTING_ENTRY        *pCurrentRoutingEntry;
    #endif
#endif

// APS Address Map Table
#if MAX_APS_ADDRESSES > 0
    #define apsMAGICValid               0x4e7b
    extern APS_ADDRESS_MAP              currentAPSAddress;
    #ifdef USE_EXTERNAL_NVM
        extern WORD                     apsAddressMapValidityKey;
        extern WORD                     apsAddressMap;
    #else
        extern ROM WORD                 apsAddressMapValidityKey;
        extern ROM APS_ADDRESS_MAP      apsAddressMap[MAX_APS_ADDRESSES];
    #endif
#endif


// Security Information
#if defined(I_SUPPORT_SECURITY)
    #define NUM_NWK_KEYS    2
    extern NETWORK_KEY_INFO             currentNetworkKeyInfo;
    #ifdef USE_EXTERNAL_NVM
        extern WORD                     nwkActiveKeyNumber;
        extern WORD                     networkKeyInfo;
        #if !(defined(I_AM_COORDINATOR) || defined(I_AM_TRUST_CENTER))
            extern WORD                 trustCenterLongAddr;
        #endif
    #else
        extern ROM BYTE                 nwkActiveKeyNumber;
        extern ROM NETWORK_KEY_INFO     networkKeyInfo[NUM_NWK_KEYS];
        #if !(defined(I_AM_COORDINATOR) || defined(I_AM_TRUST_CENTER))
            extern ROM LONG_ADDR        trustCenterLongAddr;
        #endif
    #endif
#endif



// Descriptors
#ifdef USE_EXTERNAL_NVM
    extern WORD                         configNodeDescriptor;
    extern WORD                         configPowerDescriptor;
    extern WORD                         configSimpleDescriptors;
    extern ROM NODE_DESCRIPTOR          Config_Node_Descriptor;
    extern ROM NODE_POWER_DESCRIPTOR    Config_Power_Descriptor;
    extern ROM NODE_SIMPLE_DESCRIPTOR   Config_Simple_Descriptors[];
#else
    extern ROM NODE_DESCRIPTOR          Config_Node_Descriptor;
    extern ROM NODE_POWER_DESCRIPTOR    Config_Power_Descriptor;
    extern ROM NODE_SIMPLE_DESCRIPTOR   Config_Simple_Descriptors[];
#endif

/*******************************************************************************
General Purpose Definitions
*******************************************************************************/

#ifdef USE_EXTERNAL_NVM

    #if defined(USE_EXTERNAL_NVM) && defined(STORE_MAC_EXTERNAL)
        #define GetMACAddress( x )              NVMRead( (BYTE *)x, macLongAddrEE, sizeof(LONG_ADDR) )
        #define GetMACAddressByte( y, x )       NVMRead( (BYTE *)x, macLongAddrEE + y, 1 )
        #define PutMACAddress( x )              NVMWrite( macLongAddrEE, (BYTE*)x, sizeof(LONG_ADDR) )
        #define PutMACAddressByte( y, x )       NVMWrite( macLongAddrEE + y, (BYTE*)x, 1 )
    #else
        #define GetMACAddress( x )              memcpypgm2ram( (BYTE *)x, (ROM void*)&macLongAddr, sizeof(LONG_ADDR) )
        #define GetMACAddressByte( y, x )       memcpypgm2ram( (BYTE *)x, (ROM void*)((int)&macLongAddr + (int)y), 1 )
    #endif

    #define ProfileGetNodeDesc(p)           NVMRead( (BYTE *)p, configNodeDescriptor, sizeof(NODE_DESCRIPTOR))
    #define ProfileGetNodePowerDesc(p)      NVMRead( (BYTE *)p, configPowerDescriptor, sizeof(NODE_POWER_DESCRIPTOR))
    #define ProfileGetSimpleDesc(p,i)       NVMRead( (BYTE *)p, configSimpleDescriptors + i * sizeof(NODE_SIMPLE_DESCRIPTOR), sizeof(NODE_SIMPLE_DESCRIPTOR))
    #define ProfilePutNodeDesc(p)           NVMWrite( configNodeDescriptor, (BYTE*)p, sizeof(NODE_DESCRIPTOR))
    #define ProfilePutNodePowerDesc(p)      NVMWrite( configPowerDescriptor, (BYTE*)p, sizeof(NODE_POWER_DESCRIPTOR))
    #define ProfilePutSimpleDesc(p,i)       NVMWrite( configSimpleDescriptors + i * sizeof(NODE_SIMPLE_DESCRIPTOR), (BYTE*)p, sizeof(NODE_SIMPLE_DESCRIPTOR))

    #define GetAPSAddress( x, y )           NVMRead( (BYTE *)x, y, sizeof(APS_ADDRESS_MAP) )
    #define GetAPSAddressValidityKey( x )   NVMRead( (BYTE *)x, apsAddressMapValidityKey, sizeof(WORD) )
    #define PutAPSAddress( x, y )           NVMWrite( x, (BYTE*)y, sizeof(APS_ADDRESS_MAP) )
    #define PutAPSAddressValidityKey( x )   NVMWrite( apsAddressMapValidityKey, (BYTE *)x, sizeof(WORD) )

    #define NeedNewBindingMapByte(b)        ((b&0x07) == 0)
    #define BindingBitMapBit(t)             (1 << (t&0x07))

    #define BindingIsUsed(y,t)              ((y & BindingBitMapBit(t)) != 0)
    #define MarkBindingUsed(y,t)            y |= BindingBitMapBit(t);
    #define MarkBindingUnused(y,t)          y &= ~BindingBitMapBit(t);

    #define ClearBindingTable()             ClearNVM( bindingTableUsageMap, BINDING_USAGE_MAP_SIZE )
    /*#define ClearBindingTable()           {ClearNVM( bindingTableUsageMap, sizeof(bindingTableUsageMap) ); \
    //                                      ClearNVM( apsBindingTable, sizeof(apsBindingTable) );} */
    #define GetBindingRecord( x, y )        NVMRead( (BYTE *)x, y, sizeof(BINDING_RECORD) )
    #define GetBindingSourceMap(x, y)       NVMRead( (BYTE *)x, bindingTableSourceNodeMap + (WORD)(y>>3), 1 )
    #define GetBindingUsageMap(x, y)        NVMRead( (BYTE *)x, bindingTableUsageMap + (WORD)(y>>3), 1 )
    #define GetBindingValidityKey( x )      NVMRead( (BYTE *)x, bindingValidityKey, sizeof(WORD) )
    #define PutBindingRecord( x, y )        NVMWrite( x, (BYTE *)y, sizeof(BINDING_RECORD) )
    #define PutBindingSourceMap(x, y)       NVMWrite( bindingTableSourceNodeMap + (WORD)(y>>3), (BYTE *)x, 1 )
    #define PutBindingUsageMap(x, y)        NVMWrite( bindingTableUsageMap + (WORD)(y>>3), (BYTE *)x, 1 )
    #define PutBindingValidityKey( x )      NVMWrite( bindingValidityKey, (BYTE *)x, sizeof(WORD) )

    #define GetNeighborRecord( x, y )       NVMRead( (BYTE *)x, y, sizeof(NEIGHBOR_RECORD) )
    #define GetNeighborTableInfo()          NVMRead( (BYTE *)&currentNeighborTableInfo, neighborTableInfo, sizeof(NEIGHBOR_TABLE_INFO))
    #define PutNeighborRecord( x, y )       NVMWrite( x, (BYTE *)y, sizeof(NEIGHBOR_RECORD) )
    #define PutNeighborTableInfo()          NVMWrite( neighborTableInfo, (BYTE *)&currentNeighborTableInfo, sizeof(NEIGHBOR_TABLE_INFO))

    #define GetRoutingEntry( x, y )         NVMRead( (BYTE *)x, y, sizeof(ROUTING_ENTRY) )
    #define PutRoutingEntry( x, y )         NVMWrite( x, (BYTE*)y, sizeof(ROUTING_ENTRY) )

    #define GetTrustCenterAddress(x)        NVMRead( (BYTE *)x, trustCenterLongAddr, sizeof(LONG_ADDR) )
    #define GetTrustCenterAddressByte(y, x) NVMRead( (BYTE *)x, trustCenterLongAddr + y, 1 )
    #define PutTrustCenterAddress( x )      NVMWrite( trustCenterLongAddr, (BYTE*)x, sizeof(LONG_ADDR) )

    #define GetNwkActiveKeyNumber(x)        NVMRead( (BYTE *)x, nwkActiveKeyNumber, 1 )
    #define PutNwkActiveKeyNumber(x)        NVMWrite( nwkActiveKeyNumber, (BYTE *)x, 1 )

    #define GetNwkKeyInfo(x, y)             NVMRead( (BYTE *)x, y, sizeof(NETWORK_KEY_INFO))
    #define PutNwkKeyInfo(x, y)             NVMWrite(x, (BYTE *)y, sizeof(NETWORK_KEY_INFO))

    #define GetGroupAddress(x, y)           NVMRead( (BYTE *)x, y, sizeof(GROUP_ADDRESS_RECORD))
    #define PutGroupAddress(x, y)           NVMWrite(x, (BYTE *)y, sizeof(GROUP_ADDRESS_RECORD))

#else

    #define GetMACAddress( x )              NVMRead( (BYTE *)x, (ROM void*)&macLongAddr, sizeof(LONG_ADDR) )
    #define GetMACAddressByte( y, x )       NVMRead( (BYTE *)x, (ROM void*)((int)&macLongAddr + (int)y), 1 )
    #define PutMACAddress( x )              NVMWrite((NVM_ADDR*)&macLongAddr, (BYTE*)x, sizeof(LONG_ADDR))

    #define ProfileGetNodeDesc(p)           NVMRead( (BYTE *)p, (ROM void*)&Config_Node_Descriptor, sizeof(NODE_DESCRIPTOR))
    #define ProfileGetNodePowerDesc(p)      NVMRead( (BYTE *)p, (ROM void*)&Config_Power_Descriptor, sizeof(NODE_POWER_DESCRIPTOR))
    #define ProfileGetSimpleDesc(p,i)       NVMRead( (BYTE *)p, (ROM void*)&(Config_Simple_Descriptors[i]), sizeof(NODE_SIMPLE_DESCRIPTOR))

    #define GetAPSAddress( x, y )           NVMRead( (BYTE *)x, (ROM void*)y, sizeof(APS_ADDRESS_MAP) )
    #define GetAPSAddressValidityKey( x )   NVMRead( (BYTE *)x, (ROM void*)&apsAddressMapValidityKey, sizeof(WORD) )
    #define PutAPSAddress( x, y )           NVMWrite( (NVM_ADDR*)x, (BYTE*)y, sizeof(APS_ADDRESS_MAP) )
    #define PutAPSAddressValidityKey( x )   NVMWrite( (NVM_ADDR *)&apsAddressMapValidityKey,(BYTE *)x, sizeof(WORD) )

    #define NeedNewBindingMapByte(b)        ((b&0x07) == 0)
    #define BindingBitMapBit(t)             (1 << (t&0x07))

    #define BindingIsUsed(y,t)              ((y & BindingBitMapBit(t)) != 0)
    #define MarkBindingUsed(y,t)            y |= BindingBitMapBit(t);
    #define MarkBindingUnused(y,t)          y &= ~BindingBitMapBit(t);

    //#define ClearBindingTable()             ClearNVM( (NVM_ADDR *)bindingTableUsageMap, BINDING_USAGE_MAP_SIZE )
    #define ClearBindingTable()           {ClearNVM( (NVM_ADDR *)bindingTableUsageMap, sizeof(bindingTableUsageMap) ); \
                                          ClearNVM( (NVM_ADDR *)apsBindingTable, sizeof(apsBindingTable) );}
    #define GetBindingRecord( x, y )        NVMRead( (BYTE *)x, (ROM void*)y, sizeof(BINDING_RECORD) )
    #define GetBindingSourceMap(x, y)       NVMRead( (BYTE *)x, (ROM void*)&bindingTableSourceNodeMap[y>>3], 1 )
    #define GetBindingUsageMap(x, y)        NVMRead( (BYTE *)x, (ROM void*)&bindingTableUsageMap[y>>3], 1 )
    #define GetBindingValidityKey( x )      NVMRead( (BYTE *)x, (ROM void*)&bindingValidityKey, sizeof(WORD) )
    #define PutBindingRecord( x, y )        NVMWrite( (NVM_ADDR *)x, (BYTE *)y, sizeof(BINDING_RECORD) )
    #define PutBindingSourceMap(x, y)       NVMWrite( (NVM_ADDR *)&bindingTableSourceNodeMap[y>>3], (BYTE *)x, 1 )
    #define PutBindingUsageMap(x, y)        NVMWrite( (NVM_ADDR *)&bindingTableUsageMap[y>>3], (BYTE *)x, 1 )
    #define PutBindingValidityKey( x )      NVMWrite( (NVM_ADDR *)&bindingValidityKey, (BYTE *)x, sizeof(WORD) )

    #define GetNeighborRecord( x, y )       NVMRead( (BYTE *)x, (ROM void*)y, sizeof(NEIGHBOR_RECORD) )
    #define GetNeighborTableInfo()          NVMRead( (BYTE *)&currentNeighborTableInfo,              \
                                                (ROM void*)&neighborTableInfo,              \
                                                 sizeof(NEIGHBOR_TABLE_INFO))
    #define PutNeighborRecord( x, y )       NVMWrite( (NVM_ADDR *)x, (BYTE *)y, sizeof(NEIGHBOR_RECORD) )
    #define PutNeighborTableInfo()          NVMWrite((NVM_ADDR*)&neighborTableInfo,         \
                                                (BYTE *)&currentNeighborTableInfo,           \
                                                sizeof(NEIGHBOR_TABLE_INFO))

    #define GetRoutingEntry( x, y )         NVMRead( (BYTE *)x, (ROM void*)y, sizeof(ROUTING_ENTRY) )
    #define PutRoutingEntry( x, y )         NVMWrite( (NVM_ADDR *)x, (BYTE*)y, sizeof(ROUTING_ENTRY) )

    #define GetTrustCenterAddress(x)        NVMRead( (BYTE *)x, (NVM_ADDR *)&trustCenterLongAddr, sizeof(LONG_ADDR) )
    #define GetTrustCenterAddressByte(y, x) NVMRead( (BYTE *)x, (NVM_ADDR *)((int)&trustCenterLongAddr + (int)y), 1 )
    #define PutTrustCenterAddress( x )      NVMWrite( (NVM_ADDR *)&trustCenterLongAddr, (BYTE *)x, sizeof(LONG_ADDR) )

    #define GetNwkActiveKeyNumber(x)        NVMRead(( BYTE *)x, (ROM void *)&nwkActiveKeyNumber, 1 )
    #define PutNwkActiveKeyNumber(x)        NVMWrite((NVM_ADDR *)&nwkActiveKeyNumber, (BYTE *)x, 1 )

    #define GetNwkKeyInfo(x, y)             NVMRead( (BYTE *)x,(ROM void *)y, sizeof(NETWORK_KEY_INFO) )
    #define PutNwkKeyInfo(x, y)             NVMWrite( (NVM_ADDR *)x, (BYTE *)y, sizeof(NETWORK_KEY_INFO) )

    #define GetGroupAddress(x, y)           NVMRead( (BYTE *)x, (ROM void *)y, sizeof(GROUP_ADDRESS_RECORD) )
    #define PutGroupAddress(x, y)           NVMWrite( (NVM_ADDR *)x, (BYTE *)y, sizeof(GROUP_ADDRESS_RECORD) )

#endif


/*******************************************************************************
Private Prototypes

These definitions are required for the above macros to work.  They should not be
used directly; only the definitions above should be used.
*******************************************************************************/
#if defined USE_EXTERNAL_NVM
    void ClearNVM( WORD dest, WORD count );
    BYTE NVMalloc( WORD size, WORD *location );
    void NVMRead( BYTE *dest, WORD src, BYTE count );
    BYTE NVMInit( void );
    void NVMWrite( WORD dest, BYTE *src, BYTE count );
#else
    void ClearNVM( NVM_ADDR *dest, WORD count );
    #define NVMRead(dest, src, count)   memcpypgm2ram(dest, src, count)
    void NVMWrite( NVM_ADDR * dest, BYTE *src, BYTE count );
#endif


#endif

