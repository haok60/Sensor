/*********************************************************************
 *
 *                  ZigBee Tasks File
 *
 *********************************************************************
 * FileName:        ZigBeeTasks.c
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
 ********************************************************************/

#include "Compiler.h"
#include "generic.h"
#include "zigbee.def"
#include "ZigbeeTasks.h"
#include "zNVM.h"
#include "zPHY.h"
#include "zMAC.h"
#include "zNWK.h"
#include "zAPS.h"
#if defined(AODV_ORIGINAL)
#include "zZDO.h"		  
#endif
#include "MSPI.h"
#include "sralloc.h"
#include "SymbolTime.h"

BYTE TxBuffer[TX_BUFFER_SIZE];
#if defined(__C30__)
	volatile BYTE RxBuffer[RX_BUFFER_SIZE];
#else
	#pragma udata RX_BUFFER=RX_BUFFER_LOCATION
		volatile BYTE RxBuffer[RX_BUFFER_SIZE];
	#pragma udata
#endif

BYTE                    * CurrentRxPacket;
PARAMS                  params;
BYTE                    RxRead;
volatile BYTE           RxWrite;
BYTE                    TxData;
BYTE                    TxHeader;
volatile ZIGBEE_STATUS  ZigBeeStatus;

void ZigBeeInit(void)
{
	#if !defined(__C30__)
    	SRAMInitHeap();
	#endif
//    printf("0!!");
    #ifdef USE_EXTERNAL_NVM
        NVMInit();
    #endif

    MACInit();
    NWKInit();
    APSInit(); 
	#if defined(AODV_ORIGINAL)
    ZDOInit();				 
	#endif
    TxHeader = 127;
    TxData = 0;
    RxWrite = 0;
    RxRead = 0;

    // Set up the interrupt to read in a data packet.
    // set to capture on falling edge
    #if !defined(__C30__)    
        #if (RF_CHIP == UZ2400) || (RF_CHIP == MRF24J40)
            CCP2CON = 0b00000100;
        #endif
    #endif

    // Set up the interrupt to read in a data packet.
    #if (RF_CHIP==UZ2400) || (RF_CHIP == MRF24J40)
        RFIF = 0;
        RFIE = 1;
    #endif
    InitSymbolTimer();

    ZigBeeStatus.nextZigBeeState = NO_PRIMITIVE;
    CurrentRxPacket = NULL;
}

BOOL ZigBeeTasks( ZIGBEE_PRIMITIVE *command )
{
    ZigBeeStatus.nextZigBeeState = *command;

    if(RF_INT_PIN == 0)
    {
        RFIF = 1;
    }
    
    do        /* need to determine/modify the exit conditions */
    {
        CLRWDT();
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = PHYTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = MACTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = NWKTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = APSTasks(ZigBeeStatus.nextZigBeeState);
        }
        if(ZigBeeStatus.nextZigBeeState == NO_PRIMITIVE)
        {
            ZigBeeStatus.nextZigBeeState = ZDOTasks(ZigBeeStatus.nextZigBeeState);
        }
        switch(ZigBeeStatus.nextZigBeeState)
        {
            // Check for the primitives that are handled by the PHY.
            case PD_DATA_request:
            case PLME_CCA_request:
            case PLME_ED_request:
            case PLME_SET_request:
            case PLME_GET_request:
            case PLME_SET_TRX_STATE_request:
                ZigBeeStatus.nextZigBeeState = PHYTasks(ZigBeeStatus.nextZigBeeState);
                break;

            // Check for the primitives that are handled by the MAC.
            case PD_DATA_indication:
            case PD_DATA_confirm:
            case PLME_ED_confirm:
            case PLME_GET_confirm:
            case PLME_CCA_confirm:
            case PLME_SET_TRX_STATE_confirm:
            case PLME_SET_confirm:
            case MCPS_DATA_request:
            case MCPS_PURGE_request:
            case MLME_ASSOCIATE_request:
            case MLME_ASSOCIATE_response:
            case MLME_DISASSOCIATE_request:
            case MLME_GET_request:
            case MLME_GTS_request:
            case MLME_ORPHAN_response:
            case MLME_RESET_request:
            case MLME_RX_ENABLE_request:
            case MLME_SCAN_request:
            case MLME_SET_request:
            case MLME_START_request:
            case MLME_SYNC_request:
            case MLME_POLL_request:
                ZigBeeStatus.nextZigBeeState = MACTasks(ZigBeeStatus.nextZigBeeState);
                break;

            // Check for the primitives that are handled by the NWK.
            case MCPS_DATA_confirm:
            case MCPS_DATA_indication:
            case MCPS_PURGE_confirm:
            case MLME_ASSOCIATE_indication:
            case MLME_ASSOCIATE_confirm:
            case MLME_DISASSOCIATE_indication:
            case MLME_DISASSOCIATE_confirm:
            case MLME_BEACON_NOTIFY_indication:
            case MLME_GET_confirm:
            case MLME_GTS_confirm:
            case MLME_GTS_indication:
            case MLME_ORPHAN_indication:
            case MLME_RESET_confirm:
            case MLME_RX_ENABLE_confirm:
            case MLME_SCAN_confirm:
            case MLME_COMM_STATUS_indication:
            case MLME_SET_confirm:
            case MLME_START_confirm:
            case MLME_SYNC_LOSS_indication:
            case MLME_POLL_confirm:
            case NLDE_DATA_request:
            case NLME_NETWORK_DISCOVERY_request:
            case NLME_NETWORK_FORMATION_request:
            case NLME_PERMIT_JOINING_request:
            case NLME_START_ROUTER_request:
            case NLME_JOIN_request:
            case NLME_DIRECT_JOIN_request:
            case NLME_LEAVE_request:
            case NLME_RESET_request:
            case NLME_SYNC_request:
            case NLME_GET_request:
            case NLME_SET_request:
            case NLME_ROUTE_DISCOVERY_request:
                ZigBeeStatus.nextZigBeeState = NWKTasks( ZigBeeStatus.nextZigBeeState );
                break;

            // Check for the primitives that are handled by the APS.
            case NLDE_DATA_confirm:
            case NLDE_DATA_indication:
            case APSDE_DATA_request:
            case APSME_BIND_request:
            case APSME_UNBIND_request:
            case APSME_ESTABLISH_KEY_request:
            case APSME_ESTABLISH_KEY_confirm:
            case APSME_ESTABLISH_KEY_response:
            case APSME_TRANSPORT_KEY_request:
            case APSME_UPDATE_DEVICE_request:
            case APSME_REMOVE_DEVICE_request:
            case APSME_REQUEST_KEY_request:
            case APSME_SWITCH_KEY_request:
            case APSME_ADD_GROUP_request:
            case APSME_REMOVE_GROUP_request:
            case APSME_REMOVE_ALL_GROUPS_request:
                ZigBeeStatus.nextZigBeeState = APSTasks( ZigBeeStatus.nextZigBeeState );
                break;
                
            /* Cases that are handled by the ZDO */
            case ZDO_DATA_indication:
            case ZDO_BIND_req:
            case ZDO_UNBIND_req:
            case ZDO_END_DEVICE_BIND_req:
            case APSME_TRANSPORT_KEY_indication:
            case APSME_ESTABLISH_KEY_indication:
            case APSME_UPDATE_DEVICE_indication:
            case APSME_REMOVE_DEVICE_indication:
            case APSME_REQUEST_KEY_indication:
            case APSME_SWITCH_KEY_indication:
                ZigBeeStatus.nextZigBeeState = ZDOTasks( ZigBeeStatus.nextZigBeeState );
                break;


            // Check for the primitives that are returned to the user.
            case APSDE_DATA_confirm:
            case NLME_NETWORK_DISCOVERY_confirm:
            case NLME_NETWORK_FORMATION_confirm:
            case NLME_PERMIT_JOINING_confirm:
            case NLME_START_ROUTER_confirm:
            case NLME_JOIN_confirm:
            case NLME_DIRECT_JOIN_confirm:
            case NLME_LEAVE_confirm:
            case NLME_RESET_confirm:
            case NLME_SYNC_confirm:
            case NLME_GET_confirm:
            case NLME_SET_confirm:
            case NLME_JOIN_indication:
            case NLME_LEAVE_indication:
            case NLME_SYNC_indication:
            case NLME_ROUTE_DISCOVERY_confirm:
            case APSDE_DATA_indication:
            case APSME_BIND_confirm:
            case APSME_UNBIND_confirm:
            case APSME_ADD_GROUP_confirm:
            case APSME_REMOVE_GROUP_confirm:
            case APSME_REMOVE_ALL_GROUPS_confirm:
                *command = ZigBeeStatus.nextZigBeeState;
                if (ZDOHasBackgroundTasks() || APSHasBackgroundTasks() || NWKHasBackgroundTasks() ||
                    MACHasBackgroundTasks() || PHYHasBackgroundTasks())
                {
                    ZigBeeStatus.flags.bits.bHasBackgroundTasks = 1;
                    return TRUE;
                }
                else
                {
                    ZigBeeStatus.flags.bits.bHasBackgroundTasks = 0;
                    return FALSE;
                }
                break;
                
            default: 
                break;
        }
    } while (ZigBeeStatus.nextZigBeeState != NO_PRIMITIVE);
    *command = ZigBeeStatus.nextZigBeeState;
    if (ZDOHasBackgroundTasks() || APSHasBackgroundTasks() || NWKHasBackgroundTasks() ||
        MACHasBackgroundTasks() || PHYHasBackgroundTasks())
    {
        ZigBeeStatus.flags.bits.bHasBackgroundTasks = 1;
        return TRUE;
    }
    else
    {
        ZigBeeStatus.flags.bits.bHasBackgroundTasks = 0;
        return FALSE;
    }
}
#if defined(AODV_POWER_CONTROL)			
float convertIndexPtoValueP(BYTE p){
          float a =0;
          switch((p&0xc0)>>6){
          case 0: a+=00;       
          break;
          case 1: a+=-10;
          break; 
          case 2: a+=-20;
          break; 
          case 3: a+=-30;
          break;                              
          }
          switch((p&0x38)>>3){
          case 0: a+=0;
          break;
          case 1: a+=-1.25;
          break;
          case 2: a+=-2.5;
          break;
          case 3: a+=-3.75;
          break;
          case 4: a+=-5;
          break;
          case 5: a+=-6.25;
          break;
          case 6: a+=-7.5;
          break;
          case 7: a+=-8.75;
          break;                             
          }
          return a;
	}

BYTE convertValuePtoIndexP(float a){
          
          BYTE *p,c =0x00,d;
          p = (BYTE*)&a;
          p+=3;
          *p = (*p)&0x7f;
          d = (BYTE)a;
        //  printf("--> %x\n",c);
          if(d >=30)      c|=0xc0;    /*11*/ 
           else if(d>=20) c|=0x80;    /*10*/
           else if(d>=10) c|=0x40;    /*01*/
           else           c|=0x00;    /*00*/          
        //   printf("--> %x\n",c);
          switch((c&0xc0)>>6){
          case 0: a -= 0;
          break;  
          case 1: a -= 10;
          break; 
          case 2: a -= 20;   
          break; 
          case 3: if(a >= 40) return (c|0x38);
                  else a -=30;
          break;                              
          }
         // printf("--> %f\n",a);
          if(a >=8.75)       c|=0x38; /*111*/
           else if(a>=7.5)   c|=0x30; /*110*/
           else if(a >=6.25) c|=0x28; /*101*/
           else if(a >=5)    c|=0x20; /*100*/
           else if(a >=3.75) c|=0x18; /*011*/
           else if(a >=2.5)  c|=0x10; /*010*/
           else if(a >=1.25) c|=0x08; /*001*/
           else              c|=0x00; /*000*/        
                                      
        //   printf("--> %x\n",c); 
          return c;
          
     }      

BYTE PA_LEVEL_CONTROL(BYTE pt, BYTE lqi, BYTE lqi_threshold, BYTE offset){ //@cuong can do dac thuc te de xem moi quan he.
    
     //    A send a packet to B
     //    B measures LQI and uses LQI_threshold to calculate Pt
     //    rule:
     //    if Pt reduces 5db, LQI will reduce 20 units.
     //    another?                
        
     return convertValuePtoIndexP(convertIndexPtoValueP(pt)+5*((lqi_threshold + offset - lqi)/20 -1));
      // return 2;
     }

//float getmetrics(BYTE *pathcost,BYTE remainEnergy,BYTE Ptsent,BYTE Ptdes, BYTE LQIhear, BYTE gama) { //@cuong test can than cho ra cac so tuyen dep.
float getmetrics(BYTE LQIhear,BYTE Tpass, BYTE Ptdes) { //@Son53 tinh metric
  
     float metriclink;
			metriclink = (LQI_THRESHOLD*1.0/LQIhear) + Tpass/Tcharge; 
            Ptdes = 31-((Ptdes&0xFC)>>3); // de dua ve muc nang luong quy uoc tu muc 0-31.
            metriclink += 1/Ptdes
//           // giai quyet cong thuc (1- Ptsent/255)*pow(exp(-1.0/(255*heso-Ptsent)),5)
//     if(Ptsent > 179) 	   metriclink += 0;//Lay nhung so lieu nay o dau???
//     else if(Ptsent > 178) metriclink += 0.0239248;
//     else if(Ptsent > 177) metriclink += 0.11798;
//	 else if(Ptsent > 176) metriclink += 0.223745;
//     else if(Ptsent > 173) metriclink += 0.392027;
//     else if(Ptsent > 166) metriclink += 0.643433;
//     else if(Ptsent > 157) metriclink += 0.79946;
//     else if(Ptsent > 132) metriclink += 0.899766;
//     else if(Ptsent > 100) metriclink += 0.939661;      
//     else if(Ptsent > 100) metriclink += 0.972655;     
     return metriclink;          
}

void Pack(float f,BYTE *p)
	{   /*
	    uint32_t p;
	    uint32_t sign;
	    if (f < 0) { sign = 1; f = -f; }
	    else { sign = 0; }
	    p = ((((uint32_t)f)&0x7fff)<<16) | (sign<<31); // whole part and sign
	    p |= (uint32_t)(((f - (int)f) * 65536.0f))&0xffff; // fraction
	    return p; */
	       
	    p[0] = *((BYTE*)(&f))    	;	 
	    p[1] = *(((BYTE*)(&f)) + 1)	;  
	    p[2] = *(((BYTE*)(&f)) + 2)	;  
	    p[3]=  *(((BYTE*)(&f)) + 3)	;  
	    
	}

float unPack(BYTE* p)
	{   /*
	    float f = ((p>>16)&0x7fff); // whole part
	    f += (p&0xffff) / 65536.0f; // fraction
	    if (((p>>31)&0x1) == 0x1) { f = -f; } // sign bit set
	    return f; */
	    float f;
	    *((BYTE*)(&f))    		=p[0];	 
	    *(((BYTE*)(&f)) + 1)	=p[1];  
	    *(((BYTE*)(&f)) + 2)	=p[2];  
	    *(((BYTE*)(&f)) + 3)	=p[3];
	    return f;
	}

void printfloat(float f)
	{	
		BYTE i,b=0;
		while(f>=1){f=f/10.0;b++;};
		if(b==0) printf("0,");
		for(i=0;i<10;i++)
		{	
			f=f*10.0;
			ConsolePut(CharacterArray[(BYTE)((unsigned int)f)]);
			f=f-(unsigned int)f;
			if(b==(i+1)) printf(",");
		}
	}
#endif
