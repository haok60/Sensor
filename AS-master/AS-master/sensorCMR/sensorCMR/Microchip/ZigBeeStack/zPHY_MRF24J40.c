/*********************************************************************
 *
 *                  PHY for the MRF24J40
 *
 *********************************************************************
 * FileName:        zPHY_MRF24J40.c
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
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/

#include "ZigbeeTasks.h"
#include "Zigbee.def"
#include "zigbee.h"
#include "zPHY.h"
#include "zMAC.h"
#include "zNWK.h"
#include "MSPI.h"
#include "zNVM.h"
#include "generic.h"
#include "sralloc.h"
#include "console.h"
#include "zSecurity.h"
#include "Compiler.h"
#if (RF_CHIP == MRF24J40) || (RF_CHIP == UZ2400)


// If we are using separate SPI's for the transceiver and a serial EE, redefine
// the SPI routines.
#if defined(USE_EXTERNAL_NVM) && !defined(EE_AND_RF_SHARE_SPI)
    #define SPIPut( x )     RF_SPIPut( x )
    #define SPIGet()        RF_SPIGet()
#endif


// ******************************************************************************
// Data Structures

typedef struct _RX_DATA
{
    unsigned char size :7;
    unsigned char inUse :1;
} RX_DATA;


// ******************************************************************************
// Variable Definitions

extern CURRENT_PACKET       currentPacket;
volatile INT_SAVE           IntStatus;
extern MAC_TASKS_PENDING    macTasksPending;
extern BYTE                 nwkSecurityLevel;
volatile MAC_FRAME_CONTROL  pendingAckFrameControl;
PHY_ERROR                   phyError;
PHY_PIB                     phyPIB;
volatile PHY_PENDING_TASKS  PHYTasksPending;
extern BYTE                 RxBuffer[RX_BUFFER_SIZE];
volatile RX_DATA            RxData;
BYTE                        TRXCurrentState;
volatile TX_STAT            TxStat;
SAVED_BITS                  savedBits;
BYTE                        SecurityLevel_ZIGBEE_2_IEEE(BYTE);
extern ROUTE_DST_INFO       routeDstInfo[ROUTE_DST_INFO_SIZE];

#if (RX_BUFFER_SIZE > 256)
    extern volatile WORD    RxWrite;
    extern WORD             RxRead;
#else
    extern volatile BYTE    RxWrite;
    extern BYTE             RxRead;
#endif


// ******************************************************************************
// Function Prototypes

void UserInterruptHandler(void);
#ifdef USE_SAVE_ENERGY_MODE
void MRF24J40Sleep(void);
void MRF24J40Wake(void);
void WakeUp_Tranceiver(void);
void Sleep_Tranceiver(void);
#endif

/*********************************************************************
 * Function:        BYTE PHYGet(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          The next byte from the transceiver read buffer
 *
 * Side Effects:    Read buffer pointer is incremented
 *
 * Overview:        This function returns the next byte from the
 *                  transceiver read buffer.
 *
 * Note:            None
 ********************************************************************/

BYTE PHYGet(void)
{
    BYTE toReturn;

    toReturn = RxBuffer[RxRead++];

    if(RxRead == (BYTE)RX_BUFFER_SIZE)
    {
        RxRead = 0;
    }

    return toReturn;
}

/*********************************************************************
 * Function:        BOOL PHYHasBackgroundTasks( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          TRUE - PHY layer has background tasks to run
 *                  FALSE - PHY layer does not have background tasks
 *
 * Side Effects:    None
 *
 * Overview:        Determines if the PHY layer has background tasks
 *                  that need to be run.
 *
 * Note:            None
 ********************************************************************/

BYTE PHYHasBackgroundTasks(void)
{
    //PHY might want to check to verify that we are not in the middle of
    // a transmission before allowing the user to turn off the TRX
    return PHYTasksPending.Val;
}

/*********************************************************************
 * Function:        void PHYInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Timer 1 is turned on; the interrupt is disabled.
 *
 * Overview:        This routine initializes all PHY layer data
 *                  constants.  It also initializes the transceiver.
 *
 * Note:            None
 ********************************************************************/

void PHYInit(void)
{
        BYTE i;
        WORD j;

        PHY_RESETn_TRIS = 0;

        RxData.inUse        = 0;
        PHYTasksPending.Val = 0;

        CLRWDT();

        /* Do a hard reset */
        PHY_RESETn = 0;
        for(j=0;j< (WORD)300;j++){}
            
        PHY_RESETn = 1;
        for(j=0;j<(WORD)300;j++){}
            
        /* Do a soft reset - power mgrmnt, baseband, and mac resets */
        PHYSetShortRAMAddr(SOFTRST, 0x07);
        do
        {
            i = PHYGetShortRAMAddr(SOFTRST);
        }while((i & 0x07) != (BYTE)0x00);
        for(j=0; j < (WORD)1000; j++) {};

      
        /* Do a soft reset - power mgrmnt, baseband, and mac resets */
        PHYSetShortRAMAddr(SOFTRST, 0x07);
        do
        {
            i = PHYGetShortRAMAddr(SOFTRST);
        }while((i & 0x07) != (BYTE)0x00);
        for(j=0; j < (WORD)1000; j++) {};

        /* flush the RX fifo */
        //PHYSetShortRAMAddr(RXFLUSH,0x01);
        PHYSetShortRAMAddr(RXFLUSH,0x61);


        /* Program the short MAC Address, 0xffff */
        PHYSetShortRAMAddr(SADRL,0xFF);
        PHYSetShortRAMAddr(SADRH,0xFF);
        PHYSetShortRAMAddr(PANIDL,0xFF);
        PHYSetShortRAMAddr(PANIDH,0xFF);

        /* Program Long MAC Address, 0xaaaaaaaaaaaaaaaaaa*/
        PHYSetShortRAMAddr(EADR0, MAC_LONG_ADDR_BYTE0);
        PHYSetShortRAMAddr(EADR1, MAC_LONG_ADDR_BYTE1);
        PHYSetShortRAMAddr(EADR2, MAC_LONG_ADDR_BYTE2);
        PHYSetShortRAMAddr(EADR3, MAC_LONG_ADDR_BYTE3);
        PHYSetShortRAMAddr(EADR4, MAC_LONG_ADDR_BYTE4);
        PHYSetShortRAMAddr(EADR5, MAC_LONG_ADDR_BYTE5);
        PHYSetShortRAMAddr(EADR6, MAC_LONG_ADDR_BYTE6);
        PHYSetShortRAMAddr(EADR7, MAC_LONG_ADDR_BYTE7);


        /* setup */
        PHYSetLongRAMAddr(RFCTRL2,0x80);   /* RF Optimization */

        /* set the power level to max level */
        PHYSetOutputPower( PA_LEVEL );

        /* program RSSI ADC with 2.5 MHz clock */
        PHYSetLongRAMAddr(RFCTRL6,0x90);     /* RF Optimization */
        
        PHYSetLongRAMAddr(RFCTRL7,0x80);     /* RF Optimization */

        /* Program CCA mode using RSSI */
        PHYSetShortRAMAddr(BBREG2,0x80);    /* Set CCA mode = ED and threshold = b0000 */
        
        /* Enable the packet RSSI */
        PHYSetShortRAMAddr(BBREG6,0x40);    /* Append RSSI value in RxFIFO */
        
        /* Program CCA, RSSI threshold values */
        PHYSetShortRAMAddr(RSSITHCCA,0x60);

        #ifdef I_AM_FFD
            PHYSetShortRAMAddr(ACKTMOUT,0xB9);
        #endif

        /* Set up device to operate over wider temps */
        PHYSetLongRAMAddr(RFCTRL0, 0x03);
        PHYSetLongRAMAddr(RFCTRL1, 0x02);
        
        /* Configure to use external PA LNA */
        #if defined(USE_EXT_PA_LNA) && defined(__C30__)
            PHYSetLongRAMAddr(TESTMODE, 0x0F);
        #endif
        
        /* Set interrupt mask, handle only Rx and Tx for now */
        #ifdef I_SUPPORT_SECURITY_SPEC
            PHYSetShortRAMAddr(INTMSK, 0b11100110);
        #else
            PHYSetShortRAMAddr(INTMSK, 0b11110110);  /* this is what we currently use */
        #endif

        PHYSetLongRAMAddr(RFCTRL1,0x01);/* set optimal VCO current value */
        
        PHYSetShortRAMAddr(TXPEMISP, 0x95);
        
        /* Transmit ON time setting = 0x6 */
        PHYSetShortRAMAddr(FFOEN, 0x98);  /* Tx and Rx FIFO output enable signal - via state machine */
        
        /* Set sleep clk div = 1 */
        PHYSetLongRAMAddr(SCLKDIV, 0x01);
        
        /* System clock (20MHZz) recovery time = 0x5f */
        PHYSetShortRAMAddr(SLPACK, 0x5f);
        
        /* VCO control equals 1 */
        PHYSetLongRAMAddr(RFCTRL8, 0x10);  /* RF Optimization */
        
        /* waiting until state machine is in Rx Mode */
        PHYSetShortRAMAddr(TXPEMISP, 0x95);
               
       
        /* wait until state machine is in Rx Mode */
        do
        {
            i = PHYGetLongRAMAddr(RFSTATE);
        }
        while((i&0xE0) != 0xA0);
}
#ifdef USE_SAVE_ENERGY_MODE
/*********************************************************************
 * Function:        void MRF24J40Sleep(void)
 *
 * PreCondition:    BoardInit (or other initialzation code is required)
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    the ZigBee stack is initialized
 *
 * Overview:        This function initializes the ZigBee stack and is required
 *                  before stack operations can be performed
 ********************************************************************/
void MRF24J40Sleep(void)
{
    
    /* keep track of the status of the radio */
    #if defined(I_AM_RFD)
        ZigBeeStatus.flags.bits.bRadioIsSleeping = 1;
    #endif
    
    //clear the WAKE pin in order to allow the device to go to sleep
    PHY_WAKE = 0;
    
    // make a power management reset to ensure device goes to sleep
    PHYSetShortRAMAddr(SOFTRST, 0x04);
    
    //;write the registers required to place the device in sleep
    PHYSetShortRAMAddr(TXBCNINTL, 0x80);
    PHYSetShortRAMAddr(RXFLUSH,   0x60);
    PHYSetShortRAMAddr(SLPACK,    0x80);
}
/*********************************************************************
 * Function:        void MRF24J40Wake(void)
 *
 * PreCondition:    BoardInit (or other initialzation code is required)
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    the ZigBee stack is initialized
 *
 * Overview:        This function initializes the ZigBee stack and is required
 *                  before stack operations can continue
 ********************************************************************/
void MRF24J40Wake(void)
{
    BYTE results;
    TICK failureTimer;
    TICK currentDifference;
     
    //;wake up the device
    PHY_WAKE = 1;

    failureTimer = TickGet();

    while(1)
    {
        currentDifference = TickGet();
            
        currentDifference.Val = TickGetDiff(currentDifference,failureTimer);
        
        if(currentDifference.Val > 6250)
        {
            break;
        }
    
        results = PHYGetShortRAMAddr(ISRSTS);
        if((results & 0x40))
        {
            break;
        }
    }
    
    while(1)
    {
        currentDifference = TickGet();
        
        currentDifference.Val = TickGetDiff(currentDifference, failureTimer);
        
        if( currentDifference.Val > 6250 )
        {
            break;
        }
        
        results = PHYGetLongRAMAddr(RFSTATE);
        if( (results & 0xE0) == 0xA0 )
        {
            break;
        }
    }
   
   /* Keep track of radio status - now awake */
   #if defined(I_AM_RFD)        
    ZigBeeStatus.flags.bits.bRadioIsSleeping = 0;
   #endif
}
#endif
/*********************************************************************
 * Function:        ZIGBEE_PRIMITIVE PHYTasks(ZIGBEE_PRIMITIVE inputPrimitive)
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
 * Note:            If a message is waiting in the receive buffer, it
 *                  can only be sent to the upper layers if the input
 *                  primitive is NO_PRIMITIVE and the Tx path is not
 *                  blocked.
 ********************************************************************/

ZIGBEE_PRIMITIVE PHYTasks(ZIGBEE_PRIMITIVE inputPrimitive)
{
    
    if( RF_INT_PIN == 0 )
    {
        RFIE = 1;
        RFIF = 1;
    }
    
    MLME_SET_macPANId_hw();
    
    if(inputPrimitive == NO_PRIMITIVE)
    {
        /* manage background tasks here */
        if(ZigBeeTxUnblocked)   
        {

            if(PHYTasksPending.bits.PHY_RX)
            {
                BYTE packetSize;
                if(CurrentRxPacket != NULL)
                {
                    
                    return NO_PRIMITIVE;
                }

                packetSize = RxBuffer[RxRead];
                params.PD_DATA_indication.psdu = SRAMalloc(packetSize);

                if(params.PD_DATA_indication.psdu == NULL)
                {
                    phyError.failedToMallocRx = 1;
                    PHYTasksPending.bits.PHY_RX = 0;
                    
                    return NO_PRIMITIVE;
                }

                /* save the packet head somewhere so that it can be freed later */
                if(CurrentRxPacket == NULL)
                {
                    CurrentRxPacket = params.PD_DATA_indication.psdu;

                    params.PD_DATA_indication.psduLength = packetSize;
                    RxRead++;
                    if(RxRead == (BYTE)RX_BUFFER_SIZE)
                    {
                        RxRead = 0;
                    }

                    while(packetSize--)
                    {
                        *params.PD_DATA_indication.psdu++ = PHYGet();
                    }

                    /* reset the psdu to the head of the alloc-ed RAM, just happens to me CurrentRxPacket */
                    params.PD_DATA_indication.psdu = CurrentRxPacket;

				    #if !defined(__C30__)
                        /* disable interrupts before checking to see if this was the
                            last packet in the FIFO so that if we get a packet(interrupt) after the check,
                            but before the clearing of the bit then the new indication will not
                            get cleared */
    
                        savedBits.bGIEH = INTCONbits.GIEH;
                        INTCONbits.GIEH = 0;
    
				    #endif
                    if(RxRead == RxWrite)
                    {
                        PHYTasksPending.bits.PHY_RX = 0;
                    }

					#if !defined(__C30__)
                    	INTCONbits.GIEH = savedBits.bGIEH;
					#endif

                    return PD_DATA_indication;
                }
                else
                {
                    nfree(params.PD_DATA_indication.psdu);
                    return NO_PRIMITIVE;
                }
            }
        }
        else
        {
            return NO_PRIMITIVE;
        }

    }
    return NO_PRIMITIVE;
}

/*********************************************************************
 * Function:        void PHYSetLongRAMAddr(WORD address, BYTE value)
 ********************************************************************/

void PHYSetLongRAMAddr(WORD address, BYTE value)
{
    #if defined(__C30__)
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
    	PHY_CS = 0;
    	SPIPut((((BYTE)(address>>3))&0b01111111)|0x80);
        SPIPut((((BYTE)(address<<5))&0b11100000)|0x10);
        SPIPut(value);
        PHY_CS = 1;
        RFIE = IntStatus.CCP2IntF;
    #else	
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
        PHY_CSn = 0;
        NOP();
        SPIPut((((BYTE)(address>>3))&0b01111111)|0x80);
        SPIPut((((BYTE)(address<<5))&0b11100000)|0x10);
        SPIPut(value);
        NOP();
        PHY_CSn = 1;
        RFIE = IntStatus.CCP2IntF;
    #endif    
}

/*********************************************************************
 * Function:        void PHYSetShortRAMAddr(WORD address, BYTE value)
 ********************************************************************/

void PHYSetShortRAMAddr(BYTE address, BYTE value)
{
    #if defined(__C30__)
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
    	PHY_CS = 0;
    	SPIPut(((address<<1)&0b01111111)|0x01);
        SPIPut(value);
        PHY_CS = 1;
        RFIE = IntStatus.CCP2IntF;
    #else
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
        PHY_CSn = 0;
        NOP();
        SPIPut(((address<<1)&0b01111111)|0x01);
        SPIPut(value);
        NOP();
        PHY_CSn = 1;
        RFIE = IntStatus.CCP2IntF;
    #endif    
}

/*********************************************************************
 * Function:        void PHYGetShortRAMAddr(WORD address, BYTE value)
 ********************************************************************/

BYTE PHYGetShortRAMAddr(BYTE address)
{
    BYTE toReturn;
    	
    #if defined(__C30__)
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
    	PHY_CS = 0;
    	SPIPut((address<<1)&0b01111110);
        toReturn = SPIGet();
        PHY_CS = 1;
        RFIE = IntStatus.CCP2IntF;
    #else	
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
        PHY_CSn = 0;
        NOP();
        SPIPut((address<<1)&0b01111110);
        toReturn = SPIGet();
        NOP();
        PHY_CSn = 1;
        RFIE = IntStatus.CCP2IntF;
    #endif    
    return toReturn;
}

/*********************************************************************
 * Function:        void PHYGetLongRAMAddr(WORD address, BYTE value)
 ********************************************************************/

BYTE PHYGetLongRAMAddr(WORD address)
{
    BYTE toReturn;
    
    #if defined(__C30__)
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
    	PHY_CS = 0;
        SPIPut(((address>>3)&0b01111111)|0x80);
        SPIPut(((address<<5)&0b11100000));
        toReturn = SPIGet();
        PHY_CS = 1;	
        RFIE = IntStatus.CCP2IntF;
    #else    
        IntStatus.CCP2IntF = RFIE;
        RFIE = 0;
        PHY_CSn = 0;
        NOP();
        SPIPut(((address>>3)&0b01111111)|0x80);
        SPIPut(((address<<5)&0b11100000));
        toReturn = SPIGet();
        NOP();
        PHY_CSn = 1;
        RFIE = IntStatus.CCP2IntF;
    #endif    
    return toReturn;
}

/*********************************************************************
 * Function:        void PHYSetOutputPower( BYTE power)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine sets the output power of the transceiver.
 *                  Refer to the transceiver datasheet for the value
 *                  to pass to this function for the desired power level.
 *
 * Note:            None
 ********************************************************************/
void PHYSetOutputPower( BYTE power)
{
/*   PA_LEVEL determiens output power of transciever
        Default output power is 0 dBm. Summation of “large” and “small” tuning decreases
        output power
    PA_LEVEL:
        [7:6] -> large scale tuning
              00: 0 dB
              01: -10 dB
              10: -20 dB
              11: -30 dB
        [5:3] -> small scale tuning
              000: 0 dB
              001: -1.25 dB
              010: -2.5 dB
              011: -3.75 dB
              100: -5 dB
              101: -6.25 dB
              110: -7.5 dB
              111: -8.75 dB
        [2:0] -> 000
*/	  
#if defined(AODV_POWER_CONTROL)
		printf("RFCTRL3 (power) = ");
		PrintChar(power);
        printf("\r\n");		   
#endif
        PHYSetLongRAMAddr( RFCTRL3, power );
}

//******************************************************************************
// ISR
//******************************************************************************

#if defined(__18CXX)
    #if defined(HITECH_C18)
        void interrupt HighISR(void)
    #else
        #pragma interruptlow HighISR
        void HighISR(void)
    #endif
#else
    void _ISRFAST __attribute__((interrupt, auto_psv)) _INT1Interrupt(void)
#endif
{
    volatile BYTE    CheckInterrupt;
    BYTE    TxStatus;

    // Check if our interrupt came from the INT pin on the MRF24J40
    if(RFIF)
    {
        if(RFIE)
        {
            // Clear interrupt
            RFIF = 0;

            // Reading this interrupt register will clear the interrupts
            CheckInterrupt = PHYGetShortRAMAddr(0x31);
            

            /* Check the Transmit interrupt first */
            if (CheckInterrupt & 0x01)
            {
                TxStat.finished = 1;
                //Transmit FIFO release interrupt
                // The release status will be set to ok if an ACK was required and
                // was received successfully

                TxStatus = PHYGetShortRAMAddr(0x24);
                if( TxStat.cipher == 0)
                {
                    if (TxStatus & 0x20)
                    {
                        // Channel Busy, CSMA-CA failure
                        TxStat.success = 0;
                    }
                    else if (TxStatus & 0x01)
                    {
                        //Failure- No ack back
                        TxStat.success = 0;
                        TxStat.no_ack = 1;
                    }
                }
                TxStat.cipher = 0;
                if( (TxStatus & 0x21) == 0 )
                {
                    BYTE Pending = PHYGetShortRAMAddr(0x1b);
                    if(( currentPacket.info.bits.RX_association && (Pending & 0b00000100 )) ||
                        currentPacket.info.bits.disassociation )
                    {
                        macTasksPending.bits.bSendUpMACConfirm = 1;
                    }
                    if( currentPacket.info.bits.data_request == 1 )
                    {
                        pendingAckFrameControl.word.Val = 0;
                        pendingAckFrameControl.bits.FrameType = MAC_FRAME_TYPE_ACK;
                        if( Pending & 0b00010000 )
                        {
                            pendingAckFrameControl.bits.FramePending = 1;
                        } else {
                            PHYTasksPending.bits.PHY_DATA_REQUEST = 0;
                        }
                    }
                    //success
                    TxStat.success = 1;
                }
                PHYTasksPending.bits.PHY_TX = 0;

            }

			#ifdef I_SUPPORT_SECURITY_SPEC
            if( CheckInterrupt & 0x10 )
            {
                // security interrupt from MAC layer
                BYTE i;
                BYTE loc;
                BYTE KeySeq;
                BYTE FrameControlMSB;
                BYTE FrameControlLSB;
                BYTE SecurityLevel = SwitchTable[nwkSecurityLevel]; //SecurityLevel_ZIGBEE_2_IEEE(nwkSecurityLevel);
                PHYTasksPending.bits.PHY_SECURITY = 1;

                FrameControlLSB = PHYGetLongRAMAddr((WORD)0x301);
                FrameControlMSB = PHYGetLongRAMAddr((WORD)0x302);

                loc = 0x04;

                i = FrameControlMSB & 0b0000001100;
                if( i != 0 ) // has destination PANID
                {
                    loc += 2;
                    if( i == 0b00001000 ) // short address
                    {
                        loc += 2;
                    }
                    else if( i == 0b00001100 ) // long address
                    {
                        loc += 8;
                    }
                }


                i = FrameControlMSB & 0b11000000;
                if( i != 0 ) // has destination PANID
                {
                    if( (FrameControlLSB & 0b01000000) == 0x00 )
                    {
                        loc += 2;
                    }
                    if( i == 0b10000000 ) // short address
                    {
                        loc += 2;
                    }
                    else if( i == 0b11000000 ) // long address
                    {
                        loc += 8;
                    }
                }

                
                loc += 4;

                KeySeq = PHYGetLongRAMAddr((WORD)(0x300) + loc);
                for(i = 0; i < 2; i++)
                {
                    #ifdef USE_EXTERNAL_NVM
						if( plainSecurityKey[i].SeqNumber.v[0] == KeySeq &&
							plainSecurityKey[i].SeqNumber.v[1] == nwkMAGICResSeq )
                    #else
                        GetNwkKeyInfo(&currentNetworkKeyInfo, &(networkKeyInfo[i]));
						if( currentNetworkKeyInfo.SeqNumber.v[0] == KeySeq &&
							currentNetworkKeyInfo.SeqNumber.v[1] == nwkMAGICResSeq)
					#endif
                    {
                        break;
                    }
                }

                if( i == 2 )
                {
                    // cannot find the key, ignore
                    PHYSetShortRAMAddr(SECCR0, 0x80);   // cannot find the key, ignore
                }
                else {
                    WORD myloc = 0x2B0;
					#ifdef USE_EXTERNAL_NVM
						BYTE KeyIndex = i;
					#endif
                    for(i = 0; i < 16; i++)
                    {
						#if defined(USE_EXTERNAL_NVM)
							PHYSetLongRAMAddr(myloc++, plainSecurityKey[KeyIndex].NetKey.v[i];
						#else
							PHYSetLongRAMAddr(myloc++, currentNetworkKeyInfo.NetKey.v[i]);
						#endif
                    }
                    // set security level and trigger the decryption
                    PHYSetShortRAMAddr(SECCR0, SecurityLevel << 3 | 0x40);
                }
            }
			#endif
			
			/* check and process the Rx interrupt */
            if (CheckInterrupt & 0x08)
            {
                BYTE_VAL ack_status;
                BYTE count;
                BYTE counter;
                BYTE_VAL w;
                //static BYTE rc = 0;

                #if (RX_BUFFER_SIZE > 256)
                        #error "Rx buffer must be <= 256"
                #else
                        #define BUFFER_CAST BYTE
                        BYTE RxBytesRemaining;
                        BYTE OldRxWrite;
                #endif

                //rc++;
				#ifdef I_SUPPORT_SECURITY
                	PHYTasksPending.bits.PHY_SECURITY = 0;
				#endif

                ack_status.Val = 0;
                count = 0;
                counter = 0x00;

                /* Process errata #2 RXMAC here */
                /* Hold off further recieving of bytes until the entire packet has been read */
                PHYSetShortRAMAddr(BBREG1, 0x04);
                
                // Receive ok interrupt
                if(RxData.inUse == 0)
                {
                    RxData.size = PHYGetLongRAMAddr ((WORD)(0x300 + counter++));
                    RxData.inUse=1;
                }

                OldRxWrite = RxWrite;
                if(RxWrite < RxRead)
                {
                    RxBytesRemaining = (BUFFER_CAST)(RxRead - RxWrite - 1);
                }
                else
                {
                    RxBytesRemaining = (BUFFER_CAST)(RX_BUFFER_SIZE - 1 - RxWrite + RxRead);
                }

                w.Val = RxData.size;
                
                /* this is less then because we need one extra byte for the length (which worst case would make it equivent to less then or equal to )*/
                if(w.Val < RxBytesRemaining)
                {
                    MAC_FRAME_CONTROL mfc;

                    /* there is room in the buffer */
                    RxData.inUse = 0;

                    /* add the packet */
                    RxBuffer[RxWrite++]=w.Val;

                    if(RxWrite==(BYTE)RX_BUFFER_SIZE)
                    {
                        RxWrite = 0;
                    }

                    while(w.Val--)
                    {
                        //Note: I am counting on the fact that RxWrite doesn't get incremented here anymore such that the ACK packet doesn't get written into the Buffer and the RxWrite doesn't get modified.
                        if (w.Val == 1)
                        {
                            // The last two bytes will be the 16-bit CRC.  Instead, we will read out the RSSI value
                            // for the link quality and stick it in the second to last byte.  We still need to read the
                            // last byte of the CRC to trigger the device to flush the packet.
                            RxBuffer[RxWrite] = PHYGetLongRAMAddr ((WORD)(0x300 + counter+3));

                            counter++;
                        }
                        else
                        {
                            RxBuffer[RxWrite] = PHYGetLongRAMAddr ((WORD)(0x300 + counter++));
                        }

                        if(count==0)
                        {
                            //if the frame control indicates that this packet is an ack

                            mfc.word.byte.LSB=RxBuffer[RxWrite];

                            if(mfc.bits.FrameType == 0b010)
                            {
                                //it was an ack then set the ack_status.bits.b0 to 1 showing it was an ack
                                ack_status.bits.b0 = 1;
                            }
                        }
                        else if(count==2)
                        {
                            //if we are reading the sequence number and the packet was an ack
                            if(ack_status.bits.b0)
                            {

                                if ((macTasksPending.bits.packetPendingAck) &&
                                    (RxBuffer[RxWrite] == currentPacket.sequenceNumber))
                                {
                                    // If this is the ACK we've been waiting for, set the flag to
                                    // send up the confirm and save the Frame Control.
                                    macTasksPending.bits.bSendUpMACConfirm = 1;
                                    pendingAckFrameControl = mfc;
                                }
                                RxWrite = OldRxWrite;
                                goto DoneReceivingPacket;
                            }
                        }

                        count++;
                        RxWrite++;
                        //roll buffer if required
                        if(RxWrite==(BYTE)RX_BUFFER_SIZE)
                        {
                            RxWrite = 0;
                        }
                    }

                    if(RxWrite==0)
                    {
                        w.Val = RxBuffer[RX_BUFFER_SIZE-1];
                    }
                    else
                    {
                        w.Val = RxBuffer[RxWrite - 1];
                    }

                    #if 0
                    if(PHYGetShortRAMAddr(RXSR) & 0x08 )
                    {
                        /* crc failed.  Erase packet from the array */
                        RxWrite = OldRxWrite;
                        // Flush the RX FIFO
                        PHYSetShortRAMAddr (RXFLUSH, 0x01);
                    }
                    else
                    #endif
                    {
                        ZigBeeStatus.flags.bits.bHasBackgroundTasks = 1;
                        PHYTasksPending.bits.PHY_RX = 1;
                    }
                }
                else
                {
                    RxWrite = OldRxWrite;
                    ZigBeeStatus.flags.bits.bRxBufferOverflow = 1;
                }

DoneReceivingPacket: 

                PHYSetShortRAMAddr(RXFLUSH, 0x01);
                PHYSetShortRAMAddr(BBREG1, 0x00);

                NOP();
            }
            
         }

    }
    
    #if defined(__18CXX)   
        if(INTCONbits.TMR0IF)
        {
            if(INTCONbits.TMR0IE)
            {
                /* there was a timer overflow */
                INTCONbits.TMR0IF = 0;
                timerExtension1++;
    
                if(timerExtension1 == 0)
                {
                    timerExtension2++;
                }
            }
        }
        
        UserInterruptHandler();
    #endif

}

/************************************/
/*        Interrupt Vectors         */
/************************************/

#if defined(MCHP_C18)
#pragma code highVector=0x08
void HighVector (void)
{
    _asm goto HighISR _endasm
}
#pragma code /* return to default code section */
#endif

#if defined(MCHP_C18)
#pragma code lowhVector=0x18
void LowVector (void)
{
    _asm goto HighISR _endasm
}
#pragma code /* return to default code section */
#endif


#else
    #error Please link the appropriate PHY file for the selected transceiver.
#endif      // RF_CHIP == MRF24J40
#if defined(USE_SAVE_ENERGY_MODE)
/*********************************************************************
 * Function:        void Sleep_Tranceiver(void)
 *
 * PreCondition:
 *
 * Cluater:         
 *
 * Output:          Put tranceiver into Sleep mode
 * Note: By Hoang Anh Son - K53
 ********************************************************************/
void Sleep_Tranceiver(void)
{
PHYSetLongRAMAddr(TESTMODE, 0x08);//Disable PA, LNA
PHYSetShortRAMAddr(GPIODIR, 0x03);
PHYSetShortRAMAddr(GPIO, 0x00);
MRF24J40Sleep();//Put MRF24J40 chip into Sleep Mode
}
/*********************************************************************
 * Function:        void WakeUp_Tranceiver(void)
 *
 * PreCondition:
 *
 * Cluater:         
 *
 * Output:          Wake Up Tranceiver from Sleep mode
 * Note: By Hoang Anh Son - K53
 ********************************************************************/
void WakeUp_Tranceiver(void)
{
MRF24J40Wake();//Wake Up MRF24J40 chip from Sleep Mode
PHYSetShortRAMAddr(GPIO,    0x00);//Enable PA, LNA
PHYSetShortRAMAddr(GPIODIR, 0x00);
PHYSetLongRAMAddr(TESTMODE, 0x0F); 
}
#endif
