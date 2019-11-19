/* Microchip ZigBee2006 Residential Stack
 * Demo Router
 *
 *******************************************************************************
 * FileName:        Router.c
 * Date modified:   14/08/2013
 *
 * Microcontroller: PIC24FJ128GA306
 * Transceiver:     Microchip MRF24J40
 *
 * Editor:          MPLAB X IDE v1.50
 * Complier:        MCC30 v3.31
 *
 * Developer:       Nguyen Tien Dat
 * Class:           KSTN - DTVT - K54
 * Group:           WSAN
 * Organization:    Lab411 - Hanoi University of Technology
 *
 * Description:     Chuong trinh nay thuc hien setup mot thiet bi tro thanh mot
 *                  node mang dong vai tro la mot Zigbee Router. Chuong trinh
 *                  cung duoc thiet ke de tuong tac voi cac thiet bi khac trong
 *                  mang theo chuan Zigbee (Zigbee Router hoac Zigbee End Device).
 *                  Cac hoat dong do bao gom: cho phep gia nhap/khong gia nhap
 *                  mang, yeu cau ban tin tu mot thiet bi hoac mot nhom thiet bi
 *                  trong mang, gui ban tin toi mot thiet bi hoac mot nhom cac
 *                  thiet bi trong mang.
 *
 * Note:            Xem lich su ma nguon trong file Readme.txt
*******************************************************************************/
//******************************************************************************
// Header Files
//******************************************************************************
// Include the main ZigBee header file.
#include "zAPL.h"
#include "zNVM.h"
#include "zPHY_MRF24J40.h"
#include "console.h"
#include "p24FJ128GA306.h"
#include "delay.h"


//******************************************************************************
// Configuration Bits
//******************************************************************************
#if defined(__PIC24F__)
    // Configure by dat_a3cbq91
    _CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & LPCFG_OFF & ICS_PGx1 & WINDIS_OFF & FWDTEN_WDT_SW &  FWPSA_PR128 & WDTPS_PS512);
    _CONFIG2(IESO_ON & FNOSC_FRC & FCKSM_CSECME & OSCIOFCN_OFF & IOL1WAY_OFF & POSCMD_HS);
    _CONFIG3(BOREN_ON);
    // End by dat_a3cbq91
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
    // Explorer 16 board
    _FOSCSEL(FNOSC_PRI)			// primary osc
    _FOSC(OSCIOFNC_OFF & POSCMD_XT)	// XT Osc
    _FWDT(FWDTEN_OFF)			// Disable Watchdog timer
    // JTAG should be disabled as well
#elif defined(__dsPIC30F__)
    // dsPICDEM 1.1 board
    _FOSC(XT_PLL16)		// XT Osc + 16X PLL
    _FWDT(WDT_OFF)		// Disable Watchdog timer
    _FBORPOR(MCLR_EN & PBOR_OFF & PWRT_OFF)
#else
    #error Other compilers are not yet supported.
#endif

//******************************************************************************
// Constants                        //defined application service   @dat_a3cbq91
//******************************************************************************
//Define for useful constants
#define SET             1
#define SaveEnergyMode  1
#define ENABLE          1
#define CLEAR           0
#define ActiveMode      0
#define DISABLE         0

//Define registers of MCU to control Leds
#if defined(USE_LED)
    
        #define WSAN_LED_TRIS 		TRISFbits.TRISF3
        #define WSAN_LED 		LATFbits.LATF3
  
    #define BLINK_LED_JOIN              4000
    #define BLINK_LED_NOT_JOIN          300
#endif



//******************************************************************************
// Function Prototypes
//******************************************************************************
//functions that have been created and modified by dat_a3cbq91
void HardwareInit( void );
// ProcessNONZigBeeTasks(void);
void ProcessZigBeePrimitives(void);

//Node mang su dung ham SenOneByte de gui ban tin toi Router-EMB
void SendOneByte(WORD ClusterID, BYTE MSB, BYTE LSB, BYTE cmd);
void SendRouteDrawRequest(BYTE MSB_dest_addr, BYTE LSB_dest_addr);

//******************************************************************************
// Application Variables
//******************************************************************************
NETWORK_DESCRIPTOR  *currentNetworkDescriptor;
NETWORK_DESCRIPTOR  *NetworkDescriptor;
ZIGBEE_PRIMITIVE    currentPrimitive;
BYTE                orphanTries;

BYTE    i;
volatile BYTE counter = CLEAR, EnableSendCmd = DISABLE;
BYTE Addr_MSB, Addr_LSB, Cmd,ServiceCmd, ContentCmd;
BYTE AllowJoin = SET;
extern NWK_STATUS nwkStatus;
WORD TimerCount = 0;//bien dem so lan ngat Timer
WORD ADD;
DWORD timer = TIMER_UNIT;//every 5s, the timer32 bit interrupt again.
#if defined(USE_LED)
    WORD Led_Count = BLINK_LED_NOT_JOIN;
#endif
#ifdef USE_COMPUTER
    volatile BYTE Val[6] = {0,0,0,0,0,0};
#endif

WORD len_data;
BYTE test,test_1;// bien luu gia tri hien tai va truoc do cua du lieu
//end by dat_a3cbq91

//******************************************************************************
//******************************************************************************
//******************************************************************************
int main(void)
{
    /*******************************create by lab411****************************/
    OSCCON = 0x2001;//current osc HS, new osc FRC

    CLKDIV = 0x0000;//Fcy = Fosc/2

    /************Maping Pins************/
    //unlock registers
    asm volatile ("mov #OSCCONL, w1  \n"
        "mov #0x46, w2     \n"
        "mov #0x57, w3     \n"
        "mov.b w2, [w1]    \n"
        "mov.b w3, [w1]    \n"
        "bclr OSCCON, #6");

    // INT1 >>> RP21
    RPINR0bits.INT1R = 21;

    /*------SPI1------*/
    // SCK1 >> RP26
    RPINR20bits.SCK1R = 26;
    RPOR13bits.RP26R  = 8;
    // SDO1 >> RP19
    RPOR9bits.RP19R   = 7;
    // SDI1 >> RP27
    RPINR20bits.SDI1R = 27;

    /*------SPI2------*/
    // SCK2 >> RP3
    RPINR22bits.SCK2R = 3;
    RPOR1bits.RP3R    = 11;
    //SDO2 >> RP2
    RPOR1bits.RP2R    = 10;
    // SDI2 >> RP4
    RPINR22bits.SDI2R = 4;

    /*------UART1-----*/
    // RX1 >> RP22
    RPINR18bits.U1RXR = 22;
    // TX1 >> RP25
    RPOR12bits.RP25R  = 3;

    /*------UART2-----*/
    // RX2 >> RP24
    RPINR19bits.U2RXR = 24;
    // TX2 >> RP23
    RPOR11bits.RP23R  = 5;

    //lock register
    asm volatile ("mov #OSCCONL, w1  \n"
        "mov #0x46, w2     \n"
        "mov #0x57, w3     \n"
        "mov.b w2, [w1]    \n"
        "mov.b w3, [w1]    \n"
        "bset OSCCON, #6");
    /*********************************end by lab411*****************************/
    CLRWDT();
    ENABLE_WDT();

    NetworkDescriptor = NULL;
    orphanTries = 3;
    // If you are going to send data to a terminal, initialize the UART.
    ConsoleInit();
    //test lora
    UART1Init();

    // Initialize the hardware - must be done before initializing ZigBee.
    HardwareInit();
    printf("Init Hardware\r\n");

    // Initialize the ZigBee Stack.
    ZigBeeInit();
    printf("Init Zigbee\r\n");

    // *************************************************************************
    // Perform any other initialization here
    // *************************************************************************
    printf("R&D ZigBee-Router EMB by WSAN-Lab411\r\n");
    #if (RF_CHIP == MRF24J40)
        printf("Transceiver-MRF24J40\r\n");
    #else
        printf("Transceiver-Unknown\r\n" );
    #endif

    // Enable interrupts to get everything going.
    RFIE = ENABLE;

    /* Start the network anew each time Node is booted up */
    NWKClearNeighborTable();
    #if defined(I_SUPPORT_BINDING)
        ClearBindingTable();
    #endif
    printf("1\r\n");
    /* Clear the Group Table */
    RemoveAllGroups();
    printf("2\r\n");
    #if defined(I_SUPPORT_BINDING)
        RemoveAllBindings(macPIB.macShortAddress);
    #endif

    /* Clearing nwk status flags */
    nwkStatus.flags.Val = 0x00;

    /* Initialize for primitive */
    currentPrimitive = NO_PRIMITIVE;
    

    while (1)
    {
        /* Clear the watch dog timer */
        CLRWDT();

        /* Determine which is the next ZigBee Primitive to operate on */
        ZigBeeTasks( &currentPrimitive );

        /* Process the next ZigBee Primitive */
        ProcessZigBeePrimitives();
        
        /* do any non ZigBee related tasks and then go back to ZigBee tasks */
      // ProcessNONZigBeeTasks();
        
        /* use led to identify if device joined */
       #if defined (USE_LED)
            if(Led_Count == 0)
            {
                WSAN_LED=~WSAN_LED;

                if(ZigBeeStatus.flags.bits.bNetworkJoined)
                    Led_Count = BLINK_LED_JOIN;
                else
                    Led_Count = BLINK_LED_NOT_JOIN;
            }
            else
            {
                --Led_Count;
            }
        #endif
    }
    return 1;
}

void ProcessZigBeePrimitives(void)
{
    switch (currentPrimitive)
    {
        case NLME_ROUTE_DISCOVERY_confirm:
            if (!params.NLME_ROUTE_DISCOVERY_confirm.Status)
            {
                printf("Route Reply OK\r\n" );
            }
            else
            {
                PrintChar( params.NLME_ROUTE_DISCOVERY_confirm.Status );
                printf(" Route Reply Failed\r\n" );
            }
            currentPrimitive = NO_PRIMITIVE;
            break;

        case NLME_NETWORK_DISCOVERY_confirm:
            currentPrimitive = NO_PRIMITIVE;

            switch(params.NLME_NETWORK_DISCOVERY_confirm.Status)
            {
                case 0x00:
                    printf("Number of Zigbee network have been find: ");
                    PrintChar(params.NLME_NETWORK_DISCOVERY_confirm.NetworkCount);
                    printf("\r\n");

                    // Save the descriptor list pointer so we can destroy it later.
                    NetworkDescriptor = params.NLME_NETWORK_DISCOVERY_confirm.NetworkDescriptor;

                    // Select a network to try to join.  We're not going to be picky right now...
                    currentNetworkDescriptor = NetworkDescriptor;

SubmitJoinRequest:
                    params.NLME_JOIN_request.PANId = currentNetworkDescriptor->PanID;

                    printf("Prepare join to network: 0x");
                    PrintWord(params.NLME_JOIN_request.PANId.Val);
                    printf("\r\n");

                    if(params.NLME_JOIN_request.PANId.Val != 0x1AAA)
                    {
                        currentNetworkDescriptor = currentNetworkDescriptor->next;
                        if(currentNetworkDescriptor)
                        {
                            goto SubmitJoinRequest;
                        }
                        else
                        {
                            printf("Not found desire network 0x1AAA\r\n");

                            ZigBeeStatus.flags.bits.bNetworkJoined = 0;
                            ZigBeeStatus.flags.bits.bTryingToJoinNetwork = 0;
                        }
                    }
                    else
                    {
                        printf("Found desire network 0x1AAA\r\n");

                        params.NLME_JOIN_request.JoinAsRouter   = TRUE;
                        params.NLME_JOIN_request.RejoinNetwork  = FALSE;
                        params.NLME_JOIN_request.PowerSource    = MAINS_POWERED;
                        params.NLME_JOIN_request.RxOnWhenIdle   = TRUE;
                        params.NLME_JOIN_request.MACSecurity    = FALSE;
                        params.NLME_JOIN_request.ExtendedPANID = currentNetworkDescriptor->ExtendedPANID;

                        currentPrimitive = NLME_JOIN_request;

                        printf("Network(s) found. Trying to join " );
                        PrintWord(params.NLME_JOIN_request.PANId.Val);
                        printf(" | ");
                        for(i=7;i ^ 0xFF;--i)
                            PrintChar(currentNetworkDescriptor->ExtendedPANID.v[i]);
                        printf("\r\n");
                    }
                    break;

                case 0xEA:
                    if (!params.NLME_NETWORK_DISCOVERY_confirm.NetworkCount)
                    {
                        printf("No networks found. Trying again!\r\n" );
                    }
                    break;

                default:
                    PrintChar( params.NLME_NETWORK_DISCOVERY_confirm.Status );
                    printf(" Error finding network. Trying again...\r\n" );
                    break;
            }
            break;

        case NLME_JOIN_confirm:
            if (!params.NLME_JOIN_confirm.Status)
            {
                printf("Join to this network OK!\r\n" );

                // Free the network descriptor list, if it exists. If we joined as an orphan, it will be NULL.
                while (NetworkDescriptor)
                {
                    currentNetworkDescriptor = NetworkDescriptor->next;//tro toi bang mo ta mang tim thay tiep theo
                    nfree( NetworkDescriptor );//giai phong bang mo ta mang truoc do
                    NetworkDescriptor = currentNetworkDescriptor;
                }

                // Start routing capability.
                params.NLME_START_ROUTER_request.BeaconOrder = MAC_PIB_macBeaconOrder;
                params.NLME_START_ROUTER_request.SuperframeOrder = MAC_PIB_macSuperframeOrder;
                params.NLME_START_ROUTER_request.BatteryLifeExtension = FALSE;
                currentPrimitive = NLME_START_ROUTER_request;
            }
            else
            {
                currentPrimitive = NO_PRIMITIVE;
                printf("Status ");
                PrintChar( params.NLME_JOIN_confirm.Status );

                // If we were trying as an orphan, see if we have some more orphan attempts.
                if (ZigBeeStatus.flags.bits.bTryOrphanJoin)
                {
                    // If we tried to join as an orphan, we do not have NetworkDescriptor, so we do
                    // not have to free it.
                    printf(". Could not join as orphan. " );
                    orphanTries--;
                    /* For Zigbee 2006 09/17/07, we now need to do an orphan join after a reset, but
                     * we choose not to do this forever, and quit after a few of retries
                     */
                    if (orphanTries)
                    {
                        printf("Trying join as orphan again!\r\n" );
                    }
                    else
                    {
                        orphanTries = 3;
                        printf("Must now try as a new node...\r\n" );
                        ZigBeeStatus.flags.bits.bTryOrphanJoin = CLEAR;
                    }
                }
                else
                {
                    printf(". Could not join selected network: " );
                    PrintWord(currentNetworkDescriptor->PanID.Val);

                    currentNetworkDescriptor = currentNetworkDescriptor->next;
                    //if have other networks
                    if (currentNetworkDescriptor)
                    {
                        printf(". Trying next discovered network: " );
                        PrintWord(currentNetworkDescriptor->PanID.Val);
                        printf("\r\n");
                        goto SubmitJoinRequest;
                    }
                    //neu chi tim thay mot mang duy nhat
                    else
                    {
                        //Ran out of descriptors.  Free the network descriptor list, and fall
                        // through to try discovery again.
                        printf(". Cleaning up and retrying discovery!\r\n" );
                        while (NetworkDescriptor)
                        {
                            currentNetworkDescriptor = NetworkDescriptor->next;
                            nfree( NetworkDescriptor );
                            NetworkDescriptor = currentNetworkDescriptor;
                        }
                    }
                }
            }
            break;

        case NLME_START_ROUTER_confirm:
            if (!params.NLME_START_ROUTER_confirm.Status)
            {
                printf("Router Started! Enabling joins...\r\n" );
                params.NLME_PERMIT_JOINING_request.PermitDuration = 0xFF;   // No Timeout
                params.NLME_PERMIT_JOINING_request._updatePayload = TRUE;
                currentPrimitive = NLME_PERMIT_JOINING_request;
            }
            else
            {
                printf("Router start Failed:" );
                PrintChar( params.NLME_JOIN_confirm.Status );
                printf(". We cannot route frames\r\n" );
                currentPrimitive = NLME_START_ROUTER_request;//request again
            }
            break;

        case NLME_PERMIT_JOINING_confirm:
            if (!params.NLME_PERMIT_JOINING_confirm.Status)
            {
                printf("Current Network Address is: ");
                PrintWord(macPIB.macShortAddress.Val);
                printf("\r\n");
                if(macPIB.macShortAddress.Val != 0x0001)
                {
                    // Thiet lap lai dia chi mang cua Router_EMB @dat_a3cbq91
                    macPIB.macShortAddress.v[1] = NetworkAddrMSB_EMB;
                    macPIB.macShortAddress.v[0] = NetworkAddrLSB_EMB;
                    // Set ShortAddr on Transceiver !
                    PHYSetShortRAMAddr(0x03, macPIB.macShortAddress.v[0]);
                    PHYSetShortRAMAddr(0x04, macPIB.macShortAddress.v[1]);
                    printf("Network Address has just re-assigned is 0x0001\r\n");
                }
                currentPrimitive = NO_PRIMITIVE;
            }
            else
            {
                printf("Join permission Failed: " );
                PrintChar( params.NLME_PERMIT_JOINING_confirm.Status );
                printf(". We cannot allow joins\r\n" );
                currentPrimitive = NLME_PERMIT_JOINING_request;//request again
            }
            break;

        case NLME_JOIN_indication:
            printf( "Node " );
            PrintWord(params.NLME_JOIN_indication.ShortAddress.Val);
            printf( " With MAC Address " );
            //@dat_a3cbq91: Print MAC address of node has just joined
            for(i = 7; i ^ 0xFF; --i)
            {
                PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[i]);
            }
            printf( " just joined.\r\n" );
            /* For Zigbee 2006: If a new device with the same old longAddress address
            * joins the PAN, then make sure the old short address is no longer used and is
            * overwritten with the new shortAddress & longAddress combination
            */
            /* Neu mot thiet bi da tung la thanh vien cua mang, can phai dam bao
             * la dia chi mang khong duoc dung nua va cap (dia chi mang & dia chi MAC)
             * se duoc ghi lai trong bang @dat_a3cbq91
             */
            {  /* same long address check block */
                APS_ADDRESS_MAP currentAPSAddress1;
                currentAPSAddress1.shortAddr   =   params.NLME_JOIN_indication.ShortAddress;
                currentAPSAddress1.longAddr    =   params.NLME_JOIN_indication.ExtendedAddress;

                if(LookupAPSAddress(&params.NLME_JOIN_indication.ExtendedAddress) )
                {

                    for( i = 0; i ^ apscMaxAddrMapEntries; i++)
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
                    }   /* end for loop */
                }       /* end if */
            }           /* end address check block */
            #ifdef I_SUPPORT_SECURITY
                #ifdef I_AM_TRUST_CENTER
                {
                    BOOL AllowJoin = TRUE;
                    // decide if you allow this device to join
                    if( !AllowJoin )
                    {
                        // no need to set deviceAddress, since it is overlap with NLME_JOIN_indication
                        //params.NLME_LEAVE_request.DeviceAddress = params.NLME_JOIN_indication.ExtendedAddress;
                        params.NLME_LEAVE_request.RemoveChildren = TRUE;
                        currentPrimitive = NLME_LEAVE_request;
                        break;
                    }

                    #ifdef I_SUPPORT_SECURITY_SPEC
                        if( params.NLME_JOIN_indication.secureJoin )
                        {
                            BYTE i;
                            for(i = 0; i < 16; i++)
                            {
                                    KeyVal.v[i] = 0;
                            }
                            params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;

                        }
                        else
                        {
                            BYTE i;
                            GetNwkActiveKeyNumber(&i);
                            #ifdef USE_EXTERNAL_NVM
                                currentNetworkKeyInfo = plainSecurityKey[i-1];
                            #else
                                GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(NetworkKeyInfo[i-1]));
                            #endif
                            params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
                        }
                    #else
                        #ifdef PRECONFIGURE_KEY
                        {
                            BYTE i;
                            for(i = 0; i < 16; i++)
                            {
                                KeyVal.v[i] = 0;
                            }
                            params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                            params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                            params.APSME_TRANSPORT_KEY_request._UseSecurity = TRUE;
                        }
                        #else
                            if( params.NLME_JOIN_indication.secureJoin )
                            {
                                BYTE i;
                                for(i = 0; i < 16; i++)
                                {
                                    KeyVal.v[i] = 0;
                                }
                                params.APSME_TRANSPORT_KEY_request.Key = &KeyVal;
                                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = 0;
                                params.APSME_TRANSPORT_KEY_request._UseSecurity = TRUE;
                            }
                            else
                            {
                                BYTE i;
                                GetNwkActiveKeyNumber(&i);
                                #ifdef USE_EXTERNAL_NVM
                                    currentNetworkKeyInfo = plainSecurityKey[i-1];
                                #else
                                    GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(NetworkKeyInfo[i-1]));
                                #endif
                                params.APSME_TRANSPORT_KEY_request.Key = &(currentNetworkKeyInfo.NetKey);
                                params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.KeySeqNumber = currentNetworkKeyInfo.SeqNumber.v[0];
                                params.APSME_TRANSPORT_KEY_request._UseSecurity = FALSE;
                            }
                        #endif
                    #endif
                    params.APSME_TRANSPORT_KEY_request.KeyType = ID_NetworkKey;
                    params.APSME_TRANSPORT_KEY_request.DestinationAddress = params.NLME_JOIN_indication.ExtendedAddress;
                    params.APSME_TRANSPORT_KEY_request.TransportKeyData.NetworkKey.UseParent = FALSE;
                    currentPrimitive = APSME_TRANSPORT_KEY_request;
                }
                #else
                    #ifdef I_SUPPORT_SECURITY_SPEC
                        params.APSME_UPDATE_DEVICE_request.Status = (params.NLME_JOIN_indication.secureJoin ) ? 0x00 : 0x01;
                    #else
                        #ifdef PRECONFIGURE_KEY
                            params.APSME_UPDATE_DEVICE_request.Status = 0x00;
                        #else
                            params.APSME_UPDATE_DEVICE_request.Status = 0x01;
                        #endif
                    #endif
                    params.APSME_UPDATE_DEVICE_request.DeviceShortAddress = params.NLME_JOIN_indication.ShortAddress;
                    params.APSME_UPDATE_DEVICE_request.DeviceAddress = params.NLME_JOIN_indication.ExtendedAddress;
                    GetTrustCenterAddress(&params.APSME_UPDATE_DEVICE_request.DestAddress);
                    for(i=0; i < 8; i++)
                        params.APSME_UPDATE_DEVICE_request.DestAddress.v[i] = 0xaa;

                    currentPrimitive = APSME_UPDATE_DEVICE_request;
                #endif
            #else
                currentPrimitive = NO_PRIMITIVE;
            #endif
            break;

        case NLME_LEAVE_indication:
            {
                LONG_ADDR myLongAddr;

                GetMACAddress(&myLongAddr);
                if(!memcmppgm2ram( &params.NLME_LEAVE_indication.DeviceAddress, &myLongAddr, 8 ))
                {
                    printf("We have left the network\r\n" );
                }
                else
                {
                    printf("Another node has left the network\r\n" );
                }
            }
            #ifdef I_SUPPORT_SECURITY
            {
                SHORT_ADDR	LeavingChildShortAddress;
                if( !APSFromLongToShort(&params.NLME_LEAVE_indication.DeviceAddress) )
                {
                    currentPrimitive = NO_PRIMITIVE;
                    break;
                }
                LeavingChildShortAddress = currentAPSAddress.shortAddr;

                #ifdef I_AM_TRUST_CENTER
                    params.APSME_UPDATE_DEVICE_indication.Status = 0x02;
                    params.APSME_UPDATE_DEVICE_indication.DeviceAddress = params.NLME_LEAVE_indication.DeviceAddress;
                    GetMACAddress(&params.APSME_UPDATE_DEVICE_indication.SrcAddress);
                    params.APSME_UPDATE_DEVICE_indication.DeviceShortAddress = LeavingChildShortAddress;
                    currentPrimitive = APSME_UPDATE_DEVICE_indication;
                    break;
                #else
                    params.APSME_UPDATE_DEVICE_request.Status = 0x02;
                    GetTrustCenterAddress(&params.APSME_UPDATE_DEVICE_request.DestAddress);
                    params.APSME_UPDATE_DEVICE_request.DeviceShortAddress = LeavingChildShortAddress;
                    currentPrimitive = APSME_UPDATE_DEVICE_request;
                    break;
                #endif
            }

            #else
                currentPrimitive = NO_PRIMITIVE;
            #endif
            break;

        case NLME_RESET_confirm:
            printf("ZigBee Stack has been reset\r\n" );
            /* For Zigbee 2006 The Specs require that node needs to
             * try joining as an orphan first after each reset,
             * see Mandatory test 3.9
            */
            ZigBeeStatus.flags.bits.bTryOrphanJoin = SET;

            currentPrimitive = NO_PRIMITIVE;
            break;

        case NLME_LEAVE_confirm:
            PrintChar(params.NLME_LEAVE_confirm.Status);
            printf(" Leaving the Zigbee network\r\n" );

            currentPrimitive = NO_PRIMITIVE;
            break;

        case APSDE_DATA_indication:
            {
                currentPrimitive = NO_PRIMITIVE;

                switch (params.APSDE_DATA_indication.DstEndpoint)
                {
                    // ************************************************************************
                    // Place a case for each user defined endpoint.
                    // ************************************************************************
                    case WSAN_Dst:
                    {
                        switch(params.APSDE_DATA_indication.ClusterId.Val)
                        {
                            case SLEEP_CONFIRM_CLUSTER://1
                                printf("#VL:");
                                PrintChar(APLGet());
                                printf("\r\n");
                                break;

                            case WAKE_CONFIRM_CLUSTER://2
                                printf("#WC:");
                                PrintChar(APLGet());
                                printf("\r\n");
                                break;

                            case STATE_NODE_CLUSTER://3
                                printf("#SN:");
                                PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                PrintChar(APLGet());//MM
                                PrintChar(APLGet());//SS
                                printf("\r\n");
                                break;

                            case STATUS_RESPONSE://4
                            {
                                printf ("#SC:");
                                PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                PrintChar(APLGet());//Byte0 dia chi MAC
                                PrintChar(APLGet());//STATUS
                                printf ("\r\n");
                            }
                            break;

                            case NONE_SUPPORT://5
                            {
                                printf ("#SC:");
                                PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                PrintChar(APLGet());//Byte0 dia chi MAC
                                PrintChar(APLGet());//Byte0 dia chi MAC
                                printf ("None support!\r\n");
                            }
                            break;

                            case JOIN_CONFIRM_CLUSTER://6
                                printf("#JN:");
                                ADD=params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val;
                                PrintWord(ADD);
                                //printf("---");
                                PrintChar(APLGet()); //MM
                                APLGet();//Rejected byte
                                printf("\r\n");
                                #if defined(USE_LED)
                                    WSAN_LED = ~WSAN_LED;
                                #endif
                                break;

                            case HTE_RESPONSE_CLUSTER://7
                                {
                                    printf("#RD:");
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                    PrintChar(APLGet());//Byte0 dia chi MAC
                                    PrintChar(APLGet());//nhiet do
                                    PrintChar(APLGet());
                                    PrintChar(APLGet());//do am
                                    PrintChar(APLGet());
                                    PrintChar(APLGet());//dien ap
                                    PrintChar(APLGet());
                                    
                                    printf("\r\n");
                                    #if defined(USE_LED)
                                        WSAN_LED = ~WSAN_LED;
                                    #endif
                                }
                                break;
                            case MAX_RESPONSE_CLUSTER://7
                                {
                                    printf("#AS:");
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                    PrintChar(APLGet());//Byte0 dia chi MAC
                                    PrintChar(APLGet());//anhsang
                                    PrintChar(APLGet());                                   
                                    printf("\r\n");
                                    #if defined(USE_LED)
                                        WSAN_LED = ~WSAN_LED;
                                    #endif
                                }
                                break;
                            case MAX_AUTO_SEND_CLUSTER:
                            {
                                printf("#SS:");
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                    PrintChar(APLGet());//Byte0 dia chi MAC
                                    PrintChar(APLGet());//anhsang
                                    PrintChar(APLGet());                                   
                                    printf("\r\n");
                                    break;
                            }
                            case HTE_AUTO_SEND_CLUSTER://8
                                {
                                    printf("#AD:");
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                    PrintChar(APLGet());//Byte0 dia chi MAC
                                    PrintChar(APLGet());//nhiet do
                                    PrintChar(APLGet());
                                    PrintChar(APLGet());//do am
                                    PrintChar(APLGet());
                                    PrintChar(APLGet());//dien ap
                                    PrintChar(APLGet());
                                    printf("\r\n");
                                    }
                                break;
                                /******************Camera command********************/
                                case CMR_RESPONSE_CLUSTER://9
                                {
                                    printf("#RC:");// khai bao moi
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                    PrintChar(APLGet());//Byte0 dia chi MAC
                                    PrintChar(APLGet());//2 byte do dai anh
                                    PrintChar(APLGet());
                                    PrintChar(APLGet());
                                    for(i=0;i<80;i++){  //nhan data anh
                                            test=APLGet();
                                            PrintChar(test);
                                            if(test_1==0xFF && test==0xD9)break;
                                            test_1=test;
                                          };
                                    printf("\r\n");
                                    #if defined(USE_LED)
                                       WSAN_LED = ~WSAN_LED;
                                    #endif
                               
                                }
                                break;
                                case CMR_AUTO_SEND_CLUSTER://12
                                {
                                    printf("#RA:");// khai bao moi
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);
                                    PrintChar(APLGet());//Byte0 dia chi MAC
                                    PrintChar(APLGet());//2 byte do dai anh
                                    PrintChar(APLGet());
                                    PrintChar(APLGet());
                                    for(i=0;i<80;i++){  //nhan data anh
                                            test=APLGet();
                                            PrintChar(test);
                                            if(test_1==0xFF && test==0xD9)break;
                                            test_1=test;
                                          };
                                    printf("\r\n");
                                    #if defined(USE_LED)
                                       WSAN_LED = ~WSAN_LED;
                                    #endif
                               
                                }
                                break;
                            /* Place other Cluster.ID cases here */
                                case ACTOR_RESPONSE_CLUSTER://10
                                {
                                    printf("#OK:");
                                    PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);//Dia chi mang node gui
                                    PrintChar(APLGet());//Byte0 dia chi MAC ndoe gui
                                    PrintChar(APLGet());//Ma xac nhan cua Actor
                                    printf("\r\n");
                                    #if defined(USE_LED)
                                       WSAN_LED = ~WSAN_LED;
                                    #endif
                                }
                                break;

                                case ROUTE_DRAW_REPLY_CLUSTER://11
                                    {
                                        BYTE i;
                                        BYTE NodeNumber;
                                        BYTE *pListNode;
                                        printf("#TR:");
                                        PrintWord(params.APSDE_DATA_indication.SrcAddress.ShortAddr.Val);//Dia chi mang node gui
                                        NodeNumber = APLGet();//Lay ro so node mang goi tin di qua
                                        PrintChar(NodeNumber);
                                        for (i = 0; i<NodeNumber;i++) PrintChar(APLGet());
                                        printf("\r\n");
                                    }
                                    break;

                            default:
                                break;
                        }   /* switch end*/

                        if( currentPrimitive != APSDE_DATA_request )
                            TxData = TX_DATA_START;
                    }           /* if msg */
                        break;

                    default:
                        break;
                }
                APLDiscardRx();
            }
            break;

        case APSDE_DATA_confirm:
            if (params.APSDE_DATA_confirm.Status)
            {
                PrintChar(params.APSDE_DATA_confirm.Status);
                printf(" Error sending message\r\n");
            }
            else
            {
                printf("Sending message OK!\r\n" );
            }
            currentPrimitive = NO_PRIMITIVE;
            break;

        case APSME_ADD_GROUP_confirm:
        case APSME_REMOVE_GROUP_confirm:
        case APSME_REMOVE_ALL_GROUPS_confirm:
            printf("Perform Group Operation\r\n" );
            currentPrimitive = NO_PRIMITIVE;
            break;

        case NO_PRIMITIVE:
            //if the router has not joined any network, do the following
            if (AllowJoin &&!ZigBeeStatus.flags.bits.bNetworkJoined)
            {
                //if the router has not tried to join any network
                if (!ZigBeeStatus.flags.bits.bTryingToJoinNetwork)
                {
                    //if the router want to join as an orphan
                    if (ZigBeeStatus.flags.bits.bTryOrphanJoin)
                    {
                        printf("Trying to join network as an orphan...\r\n" );

                        params.NLME_JOIN_request.ScanDuration     = 8;
                        params.NLME_JOIN_request.ScanChannels.Val = ALLOWED_CHANNELS;
                        params.NLME_JOIN_request.JoinAsRouter     = TRUE;
                        params.NLME_JOIN_request.RejoinNetwork    = 0x01;
                        params.NLME_JOIN_request.PowerSource      = MAINS_POWERED;
                        params.NLME_JOIN_request.RxOnWhenIdle     = TRUE;
                        params.NLME_JOIN_request.MACSecurity      = FALSE;

                        params.NLME_JOIN_request.ExtendedPANID = currentNetworkDescriptor->ExtendedPANID;
                        currentPrimitive = NLME_JOIN_request;
                    }
                    else
                    {
                        printf("Trying to join network as a new device!\r\n" );
                        params.NLME_NETWORK_DISCOVERY_request.ScanDuration          = 8;
                        params.NLME_NETWORK_DISCOVERY_request.ScanChannels.Val      = ALLOWED_CHANNELS;
                        currentPrimitive = NLME_NETWORK_DISCOVERY_request;
                    }
                }
            }
            else
            {
                #ifdef USE_COMPUTER
                if (ZigBeeReady() & (!ZigBeeStatus.flags.bits.bHasBackgroundTasks))
                {
                    if(EnableSendCmd)
                    {
                        Addr_MSB = GetHexByte(Val[0],Val[1]);
                        Addr_LSB = GetHexByte(Val[2],Val[3]);
                        Cmd = GetHexByte(Val[4],Val[5]);

                        printf("Command: ");
                        PrintChar(Addr_MSB);
                        PrintChar(Addr_LSB);
                        PrintChar(Cmd);
                        printf("\r\n");

                        //Neu Buffer[2] = 0x00 -> yeu cau du lieu nhiet do - do am
                        //Neu Buffer[2] != 0x00 -> yeu cau bat van
                                                //for send broadcast messages
                        if ((Addr_MSB == 0xff)&&(Addr_LSB == 0xff))
                        {
                            ContentCmd = Cmd & 0x1F;//take content of cmd
                            ServiceCmd = Cmd >> 5;//take kind of cmd

                            switch (ServiceCmd)
                            {
                                case 1:
                                    SendOneByte(SLEEP_SYN_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;
                                    #ifdef USE_USART
                                        printf("Sleep request OK\r\n");
                                    #endif
                                    break;

                                case 2:
                                    #ifdef USE_EXTERNAL_NVM
                                        pCurrentRoutingEntry = routingTable;   //+ (WORD)neighborIndex * (WORD)sizeof(NEIGHBOR_RECORD);
                                    #else
                                        pCurrentRoutingEntry = &(routingTable[0]);
                                    #endif
                                    printf("#RT:");
                                    PrintChar(macPIB.macShortAddress.v[1]);
                                    PrintChar(macPIB.macShortAddress.v[0]);
                                    for ( i=0; i < ROUTING_TABLE_SIZE-RESERVED_ROUTING_TABLE_ENTRIES; i++ )
                                    {
                                        GetRoutingEntry(&currentRoutingEntry,pCurrentRoutingEntry);
                                        if ((currentRoutingEntry.destAddress.byte.MSB !=0xff))
                                        {
                                            PrintChar(currentRoutingEntry.destAddress.byte.MSB);
                                            PrintChar(currentRoutingEntry.destAddress.byte.LSB);
                                            PrintChar(currentRoutingEntry.nextHop.byte.MSB);
                                            PrintChar(currentRoutingEntry.nextHop.byte.LSB);
                                        }
                                        #ifdef USE_EXTERNAL_NVM
                                            pCurrentRoutingEntry += (WORD)sizeof(ROUTING_ENTRY);
                                        #else
                                            pCurrentRoutingEntry++;
                                        #endif
                                    }
                                    printf("\r\n");
                                    SendOneByte(ROUTING_TABLE_READ_CLUSTER, Addr_MSB, Addr_LSB, Cmd);
                                    EnableSendCmd = CLEAR;
                                    #ifdef USE_USART
                                        printf("Read Routing Table request OK");
                                    #endif
                                    break;

                                case 0x:
                                    SendOneByte(RE_CONFIG_CYCLE_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    break;

                                case 5:
                                    SendOneByte(RE_ASSIGN_LEVEL_MQ6_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    break;

                                default:
                                    break;
                            }
                        }
                        else
                        {
                            //for send unicast message
                            switch (Cmd)
							{
							case 0x00://Lay nhiet do - do am sensor
								SendOneByte(HTE_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,Cmd);
							break;
							case 0xF1://Yeu cau ve tuyen
							SendRouteDrawRequest(Addr_MSB, Addr_LSB);
							break;
                            default://Gui lenh cho Actor
                            SendOneByte(ACTOR_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,Cmd);
							break;
                            EnableSendCmd = CLEAR;
							}
                        }
                    }
                }//end computer
                #else
                if (ZigBeeReady() & (!ZigBeeStatus.flags.bits.bHasBackgroundTasks))
                {
                    if(EnableSendCmd)
                    {
                        //for send broadcast messages
                        if ((Addr_MSB == 0xff)&&(Addr_LSB == 0xff))
                        {
                            ContentCmd = Cmd & 0x0F;//take content of cmd
                            ServiceCmd = Cmd & 0xF0;//take kind of cmd

                            switch (ServiceCmd)
                            {
                                //che do tiet kiem nang luong: tat - ngu luan phien
                                case 0x00:
                                    //Neu ContentCmd = 0x01 thi chuyen tu che do active --> tiet kiem nang luong.
                                    //Neu ContentCmd = 0x02 thi chuyen tu che do tiet kiem nang luong --> active.
                                    SendOneByte(SLEEP_SYN_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;

                                    if(ContentCmd == 0x01)
                                        printf("Sleep request OK\r\n");
                                    if(ContentCmd == 0x02)
                                    	printf("Normal Mode requested OK\r\n");
                                    break;

                                //thay doi chu ki gui du lieu nhiet do do am trong che do active.
                                case 0x10:
                                    SendOneByte(RE_CONFIG_CYCLE_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;
                                    break;

                                //thay doi nguong so sanh cam bien MQ6_V1
                                case 0x20:
                                    SendOneByte(RE_ASSIGN_LEVEL_MQ6_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;
                                    break;
                                case 0x30:
                                    SendOneByte(HTE_REQUEST_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;
                                    break;
                                case 0x60:
                                    SendOneByte(MAX_REQUEST_CLUSTER, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;
                                    break;

                                default:
                                    SendOneByte(REQUEST_NODE, Addr_MSB, Addr_LSB, ContentCmd);
                                    EnableSendCmd = CLEAR;
                                    break;
                           }
                        }
                        else
                        {
                            switch(Cmd)
                            {
                                case 0x00:
                                    SendOneByte(HTE_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,Cmd);//Lay nhiet do - do am sensor
                                    break;
                                case 0x01:
                                    SendOneByte(RESET_NODE, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x40:// khai bao moi
                                    SendOneByte(CMR_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,Cmd);//Lay anh sensor                                   
                                    break;
                                    ////////////////////////////////////////////////
                                    //CMD retasking: 0x1*
                                    //* = (0Y1Y2Y3)b
                                    //Y3: SHT10; Y2:PTC06; Y1: Che do gui dinh ki////Y=0:tat, 1:bat
                                case 0x50://000:tat het cac chuc nang
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x51://001
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x52://010
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x53://011
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x54://100
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x55://101
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x56://110
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x57://111
                                    SendOneByte(RETASK_FUNCTION, Addr_MSB,Addr_LSB,Cmd);
                                    break;
                                case 0x58://lay thong tin node mang
                                    SendOneByte(STATUS_REQUEST, Addr_MSB,Addr_LSB,Cmd);//thong bao trang thai node mang
                                    break;
                                case 0x60:
                                    SendOneByte(MAX_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,Cmd);//Lay gia tri anh sang
                                    break;
                                ////////////////////////////////////////////////   
                                case 0x70:
                                    printf("c$");
                                       
                                case 0xF1:
                                    SendRouteDrawRequest(Addr_MSB, Addr_LSB);	//Yeu cau ve tuyen
                                    break;

                                default:
                                    SendOneByte(ACTOR_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,Cmd);//Gui lenh cho Actor
                                    break;
                            }

                            EnableSendCmd = CLEAR;
                        }
                    }
                }
                #endif
            }
            break;

        default:
            PrintChar( currentPrimitive );
            printf(" Unhandled primitive\r\n" );
            currentPrimitive = NO_PRIMITIVE;
            break;
    }
}

void HardwareInit(void)
{
    /*Configure digital pins @ added by dat_a3cbq91*/
    ANSE = 0x0000;
    ANSG = 0x0000;
    ANSD = 0x0000;

    /*Configure for SPI communication*/
    #if(CLOCK_FREQ < 1000000)
        //SPI1 for RF transceiver
        SPI1CON1 = 0b0000000100111111;      // CLOCK_FREQ as SPI CLOCK
        SPI1STAT = 0x8000;

        //SPI2 for EEPROM
        SPI2CON1 = 0b0000000100111111;      // CLOCK_FREQ as SPI CLOCK
        SPI2STAT = 0x8000;
    #else
        //comment by dat_a3cbq91
        /* SPI1 for RF transceiver */
        SPI1CON1 = 0b0000000100111110;  //PIC24FJ128GA306 is Master, MRF24J40 is Slaver
                                        //Internal SPI clock is enabled
                                        //SDO1 pin is controlled by the module
                                        //Communication is byte-wide (8 bits)
                                        //Input data is sampled at the middle of data output time
                                        //Serial output data changes on transition from active clock state to Idle clock state
                                        //~SS1 pin is not used by the module; pin is controlled by the port function
                                        //Idle state for the clock is a low level; active state is a high level
                                        //Secondary prescale 1:1
                                        //Primary prescale 4:1 -> CLOCK_FREQ/2/4 as SPI CLOCK

        //Enables module and configures SCKx, SDOx, SDIx and ~SSx as serial port pins
        //Interrupt when the last data in the receive buffer is read; as a result, the buffer is empty (SRXMPT bit is set)
        SPI1STAT = 0x8000;

        /* SPI2 for EEPROM */
        //same as configuration SPI1 module
        SPI2CON1 = 0b0000000100111110;
        SPI2STAT = 0x8000;
    #endif

    #ifdef USE_EXTERNAL_NVM
    	EEPROM_nCS	= 1;
    	EEPROM_nCS_TRIS	= 0;
    	IFS2bits.SPI2IF = 1;
    #endif

    PHY_RESETn = 0;
    PHY_RESETn_TRIS = 0;
    PHY_CS = 1;
    PHY_CS_TRIS = 0;

    RFIF = 0;
    RFIE = 1;

    if(RF_INT_PIN == 0)
    {
        RFIF = 1;
    }
    #if defined(USE_LED)
        WSAN_LED_TRIS = 0;
        WSAN_LED = CLEAR;//Clear Led to a initialize state
    #endif

}

/*********************************************************************
 * Function:        void SendOnceByte(BYTE data, WORD ClusterID)
 *
 * PreCondition:    Init OK
 *
 * Input:           data, CluaterID
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        Gui trang thai ve router-EMB. Cac trang thai co the
 *                  la tat/bat bom, chay rung, canh bao muc nang luong.
 *
 * Note:
 ********************************************************************/

void SendOneByte(WORD ClusterID, BYTE MSB, BYTE LSB, BYTE cmd)
{
    TxBuffer[TxData++] = cmd;
    ZigBeeBlockTx();

    /* load parameters for APSDE_DATA_request primitive */
    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;

    /* load network address of router-emboard */
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = MSB;
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = LSB;

    params.APSDE_DATA_request.SrcEndpoint = WSAN_Src;
    params.APSDE_DATA_request.DstEndpoint = WSAN_Dst;
    params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID;

    //params.APSDE_DATA_request.asduLength; TxData
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;

//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_FORCE;
//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    #ifdef I_SUPPORT_SECURITY
        params.APSDE_DATA_request.TxOptions.Val = Enable;
    #else
        params.APSDE_DATA_request.TxOptions.Val = DISABLE;
    #endif
    params.APSDE_DATA_request.TxOptions.bits.acknowledged = ENABLE;

    params.APSDE_DATA_request.ClusterId.Val = ClusterID;
    
    currentPrimitive = APSDE_DATA_request;
}

void SendRouteDrawRequest(BYTE MSB_dest_addr, BYTE LSB_dest_addr)
{
    BYTE i;
    TxBuffer[TxData++] = MSB_dest_addr;//Dia chi Node dich
    TxBuffer[TxData++] = LSB_dest_addr;
    TxBuffer[TxData++] = 0x00;
    for (i = 0; i < ROUTING_TABLE_SIZE-RESERVED_ROUTING_TABLE_ENTRIES; i++ )//Gui den forward Node
    {
        GetRoutingEntry(&currentRoutingEntry,pCurrentRoutingEntry);
        if ((MSB_dest_addr == currentRoutingEntry.destAddress.byte.MSB)&&(LSB_dest_addr == currentRoutingEntry.destAddress.byte.LSB)&&(currentRoutingEntry.status == ROUTE_ACTIVE))
        {
            params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = currentRoutingEntry.nextHop.byte.MSB;
            params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = currentRoutingEntry.nextHop.byte.LSB;
            break;
        }
    }

    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;//0x02 : Ban tin Unicast, Set che do dia chi mang ( DstAddress) 16 bit va yeu cau co' DstEndPoint
    params.APSDE_DATA_request.SrcEndpoint = WSAN_Src;
    params.APSDE_DATA_request.DstEndpoint = WSAN_Dst;
    params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID;

    //params.APSDE_DATA_request.asduLength; TxData
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;//gioi han so hop ma du lieu duoc phep truyen qua, o day la 10 hop

//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_FORCE;
//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    #ifdef I_SUPPORT_SECURITY
        params.APSDE_DATA_request.TxOptions.Val = ENABLE;
    #else
        params.APSDE_DATA_request.TxOptions.Val = DISABLE;//khong ho tro bao mat
    #endif
    params.APSDE_DATA_request.TxOptions.bits.acknowledged = ENABLE;// Yeu cau ACK tu thiet bi thu
    params.APSDE_DATA_request.ClusterId.Val = ROUTE_DRAW_REQUEST_CLUSTER;

    ZigBeeBlockTx();//goi ham nay de ZigbeeIsReady() --> false
    currentPrimitive = APSDE_DATA_request;
}


/*void _ISR __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void)
{
    WORD k;
    IFS1bits.U2RXIF = CLEAR;
     i = U2RXREG;
     if(i=='1'){
         k=ADD;
      printf("lenh gui\n");
        Addr_MSB=  (ADD >> 8);
        Addr_LSB=  k & 0x00FF;
        PrintChar(Addr_MSB);
        PrintChar(Addr_LSB);
    SendOneByte(HTE_REQUEST_CLUSTER, Addr_MSB,Addr_LSB,0x00);
    printf("lenh end\n");
                  }
}*/
void _ISR __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void)
{
    BYTE data;
    data = U2RXREG;
    IFS1bits.U2RXIF = CLEAR;
//    if (data == '$')
    if (data == 'x') counter = CLEAR;
    //else if(data=='c') printf("doam");
    else
    {
        switch(counter)
        {
            case 0:
                Addr_MSB = data;
                ++counter;
                break;
            case 1:
                Addr_LSB = data;
                ++counter;
                break;
            case 2:
                Cmd = data;
                EnableSendCmd = ENABLE;
                ++counter;
                break;

            default:
                counter = CLEAR;
                #if defined USE_USART
                    printf("Command Error\r\n");
                #endif
                break;
        }
    }
}
/* End by Hoang Anh Son */
void _ISR __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void)
{
    BYTE data;
    data = U1RXREG;
    IFS0bits.U1RXIF = CLEAR;
    ConsolePut(data);
    
}