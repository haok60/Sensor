//*********************************************************************
//*
//* Software License Agreement
//*
//* The software supplied herewith by Microchip Technology Incorporated
//* (the ?Company?) for its PICmicro® Microcontroller is intended and
//* supplied to you, the Company?s customer, for use solely and
//* exclusively on Microchip PICmicro Microcontroller products. The
//* software is owned by the Company and/or its supplier, and is
//* protected under applicable copyright laws. All rights are reserved.
//* Any use in violation of the foregoing restrictions may subject the
//* user to criminal sanctions under applicable laws, as well as to
//* civil liability for the breach of the terms and conditions of this
//* license.
//*
//* THIS SOFTWARE IS PROVIDED IN AN ?AS IS? CONDITION. NO WARRANTIES,
//* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
//* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
//* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
//* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
//*
//*********************************************************************

// Created by ZENA(TM) Version 1.3.11.0, 10/11/2006, 15:01:54

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
    {0x00, 0x00},       // Manufacturer Code
    0x7F,               // Max Buffer Size
    {0x00, 0x00}        // Max Transfer Size
};

ROM NODE_POWER_DESCRIPTOR Config_Power_Descriptor =
{
    0x00, //Power mode: RxOn
    0x01, //Available power: Mains
    0x01, //Current power: Mains
    0x0c  //Fill in current power level
};

ROM NODE_SIMPLE_DESCRIPTOR Config_Simple_Descriptors[3] =
{
//--------------------------------------
// ZigBee Device Object Endpoint
// DO NOT MODIFY THIS DESCRIPTOR!!!
//--------------------------------------
    {
        EP_ZDO,
        {0x00, 0x00},  // ZDO Profile ID
        {0x00, 0x00},  // ZDO Device
        0x00,          // ZDO Version
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
    #ifdef ROUTER_EMB
        //input cluster
        5,
        {
            JOIN_CONFIRM_CLUSTER,
            STATE_NODE_CLUSTER,
            HTE_RESPONSE_CLUSTER,
            ACTOR_RESPONSE_CLUSTER,
            TRANSMIT_COUNTED_PACKETS_CLUSTER
        },
        //output cluster
        2,
        {
            HTE_REQUEST_CLUSTER,
            ACTOR_REQUEST_CLUSTER
        }
    #else
        //input cluster
        3,
        {
            HTE_REQUEST_CLUSTER,
            ACTOR_REQUEST_CLUSTER,
            CMR_REQUEST_CLUSTER
        },
        //output cluster
        5,
        {
            JOIN_CONFIRM_CLUSTER,
            STATE_NODE_CLUSTER,
            HTE_RESPONSE_CLUSTER,
            CMR_RESPONSE_CLUSTER,
            ACTOR_RESPONSE_CLUSTER,
         //   TRANSMIT_COUNTED_PACKETS_CLUSTER
        }
    #endif

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



