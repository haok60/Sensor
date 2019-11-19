// *****************************************************************************
//                                                                              
//  Software License Agreement                                                  
//                                                                              
//  Copyright (c) 2004-2008 Microchip Technology Inc.                           
//  All rights reserved.                                                        
//                                                                              
//  Microchip licenses to you the right to use, copy and distribute Software    
//  only when embedded on a Microchip microcontroller or digital signal         
//  controller and used with a Microchip radio frequency transceiver, which     
//  are integrated into your product or third party product (pursuant to the    
//  sublicense terms in the accompanying license agreement).  You may NOT       
//  modify or create derivative works of the Software.                          
//                                                                              
//  If you intend to use this Software in the development of a product for      
//  sale, you must be a member of the ZigBee Alliance.  For more                
//  information, go to www.zigbee.org.                                          
//                                                                              
//  You should refer to the license agreement accompanying this Software for    
//  additional information regarding your rights and obligations.               
//                                                                              
//  SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     
//  KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY          
//  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A      
//  PARTICULAR PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE         
//  LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY,           
//  CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY       
//  DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY     
//  INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST      
//  PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS,              
//  TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT     
//  LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.                    
//                                                                              
//  8/20/08                                                                     
//                                                                              
// *****************************************************************************

// Created by ZENA(TM) Version 3.0.0.0, 8/8/2012, 9:03:25

#include "zigbee.def"
#include "zNWK.h"
#include "zZDO.h"

ROM NODE_DESCRIPTOR Config_Node_Descriptor =
{
    0x00,               // ZigBee Coordinator
    0x00,               // (reserved)
    0x00,               // (APS Flags, not currently used)
    0x08,               // Frequency Band 2400
    MY_CAPABILITY_INFO, // Capability Information
    {{0x00, 0x00}},       // Manufacturer Code
    0x7F,               // Max Buffer Size
    {{0x00, 0x00}},     // Max Transfer Size
                        // Primary Discovery Cache
                        // Primary Binding Table Cache
    {{0x14, 0x00}}     // Server Mask Bits
};

ROM NODE_POWER_DESCRIPTOR Config_Power_Descriptor =
{
    0x00, //Power mode: RxOn
    0x01, //Available power: Mains 
    0x01, //Current power: Mains
    0x0c  //Fill in current power level
};

ROM NODE_SIMPLE_DESCRIPTOR Config_Simple_Descriptors[2] =
{
//--------------------------------------
// ZigBee Device Object Endpoint
// DO NOT MODIFY THIS DESCRIPTOR!!!
//--------------------------------------
    {
        EP_ZDO,
        {{0x00, 0x00}},  // ZDO Profile ID
        {{0x00, 0x00}},  // ZDO Device
        0x00,            // ZDO Version
        NO_OTHER_DESCRIPTOR_AVAILABLE,
        ZDO_INPUT_CLUSTERS,
        { NWK_ADDR_req, IEEE_ADDR_req, NODE_DESC_req, POWER_DESC_req,
          SIMPLE_DESC_req, ACTIVE_EP_req, MATCH_DESC_req
                    , END_DEVICE_BIND_req
                , BIND_req, UNBIND_req
        },
        ZDO_OUTPUT_CLUSTERS,
        { NWK_ADDR_rsp, IEEE_ADDR_rsp, NODE_DESC_rsp, POWER_DESC_rsp,
          SIMPLE_DESC_rsp, ACTIVE_EP_rsp, MATCH_DESC_rsp
                    , END_DEVICE_BIND_rsp
                , BIND_rsp, UNBIND_rsp
        }
    }
,
//--------------------------------------
    {
        WSAN_Endpoint,
        {{MY_PROFILE_ID_LSB,MY_PROFILE_ID_MSB}},
        {{FULL_DEV_ID_LSB,FULL_DEV_ID_MSB}},
        FULL_DEV_VER,
        NO_OTHER_DESCRIPTOR_AVAILABLE,
        //input cluster @dat_a3cbq91
        5,
        { 
            ACTOR_REQUEST_CLUSTER,
            HTE_REQUEST_CLUSTER,
            RE_CONFIG_CYCLE_CLUSTER,
            RE_ASSIGN_LEVEL_MQ6_CLUSTER,
            ROUTE_DRAW_REQUEST_CLUSTER
        },
        //output cluster @dat_a3cbq91
        5,
        {
            JOIN_CONFIRM_CLUSTER,
            STATE_NODE_CLUSTER,
            ACTOR_RESPONSE_CLUSTER,
            HTE_RESPONSE_CLUSTER,
            HTE_AUTO_SEND_CLUSTER,
            ROUTE_DRAW_REPLY_CLUSTER
        }
    }
};

ROM _Config_NWK_Mode_and_Params Config_NWK_Mode_and_Params =
{
    nwkcProtocolVersion,        //Protocol Version
    MY_STACK_PROFILE_ID,        //Stack Profile ID
    MAC_PIB_macBeaconOrder,     //Beacon Order
    MAC_PIB_macSuperframeOrder, //Superframe Order
    MAC_PIB_macBattLifeExt,     //Battery Life Extension
    PROFILE_nwkSecurityLevel,   //Security Level
    ALLOWED_CHANNELS            //Channels to scan
};



