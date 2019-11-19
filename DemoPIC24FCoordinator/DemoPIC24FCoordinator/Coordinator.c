/* Microchip ZigBee2006 Residential Stack
 * Demo Coordinator
 *
 *******************************************************************************
 * FileName:        Coordinator.c
 * Date modified:   12/08/2013
 *
 * Microcontroller: PIC24FJ128GA306
 * Transceiver:     Microchip MRF24J40MB
 *
 * Editor:          MPLAB X IDE v1.80
 * Complier:        MCC30 v3.31
 *
 * Developer:       Nguyen Tien Dat
 * Class:           KSTN - DTVT - K54
 * Group:           WSAN
 * Organization:    Lab411 - Hanoi University of Technology
 *
 * Description:     Chuong trinh nay viet cho thiet bi de no tro thanh mot node
 *                  mang dong vai tro la mot Zigbee Coordinator. Sau khi duoc cap
 *                  nguon, Coordinator se tao mang va cho phep cac node mang khac
 *                  nhu Zigbee router, Zigbee End Device gia nhap mang. Ngoai ra
 *                  no cung co the duoc trang bi cac tinh nang khac cua mot cam
 *                  bien hoac cua mot actor.
 *
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
#ifdef I_SUPPORT_SECURITY
    #include "zSecurity.h"
    extern NETWORK_KEY_INFO    currentNetworkKeyInfo;
    #ifdef USE_EXTERNAL_NVM
        extern NETWORK_KEY_INFO plainSecurityKey[2];

        extern BOOL SetSecurityKey(INPUT BYTE index, INPUT NETWORK_KEY_INFO newSecurityKey);
    #endif
#endif

// If you are going to use UART module(s), include this file.
#if defined(USE_DEBUG) || defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM) || defined(USE_MINI_RADAR)
    #include "console.h"
#endif

// If you are going to use SHT module, include this file.
#if defined(USE_SHT10)
    #include "delay.h"
    #include "SHT1x.h"
#endif

//******************************************************************************
// Configuration Bits
//******************************************************************************

#if defined(__PIC24F__)
    // Configure by dat_a3cbq91
    _CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & LPCFG_OFF & ICS_PGx1 & WINDIS_OFF
            & FWDTEN_WDT_SW &  FWPSA_PR128 & WDTPS_PS512);
    _CONFIG2(IESO_ON & FNOSC_FRC & FCKSM_CSECME & OSCIOFCN_OFF & IOL1WAY_OFF & POSCMD_HS);
    _CONFIG3(BOREN_ON);
    // End by dat_a3cbq91
#elif defined(__dsPIC33F__) || defined(__PIC24H__)
    // Explorer 16 board
    _FOSCSEL(FNOSC_PRI)			// primary osc
    _FOSC(OSCIOFNC_OFF & POSCMD_XT)     // XT Osc
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

#define CLEAR           0
#define ActiveMode      0

//Define registers of MCU to control Leds
#if defined(USE_LED)
    #if defined(CHAMSOCLAN) || defined(THEODOIDONGVAT)
        #define WSAN_LED_TRIS 		TRISFbits.TRISF3
        #define WSAN_LED 		LATFbits.LATF3
    #endif

    #if defined(CANHBAOCHAYRUNG)
        #define WSAN_LED_TRIS 		TRISEbits.TRISE4
        #define WSAN_LED                LATEbits.LATE4
    #endif

    #define BLINK_LED_FORMED_NETWORK        4000
    #define BLINK_LED_NOT_FORMED_NETWORK    300
#endif

//In warning-low-power application
#if defined(USE_WARNING_LOW_POWER)
    //Define registers of MCU to control pins
    #define VOLTAGE_MEASURE_TRIS            TRISBbits.TRISB8
    #define VOLTAGE_MEASURE_ANALOG          ANSBbits.ANSB8
    #define SUPPLY_VOLT_ZENER_TRIS          TRISBbits.TRISB10
    #define SUPPLY_VOLT_ZENER               LATBbits.LATB10

    // trang thai: 0x03 --> muc nang luong co dau hieu can kiet
    #define LowPower                        0x03
    // Gia tri nguong canh bao het nang luong: 2,4V @ADC 12 bits
    #define ThresholdPower                  1350
    // So lan gui canh bao ve bo nhung truoc khi dung hoat dong cua khoi ADC
    #define MaxNumberWarningLowPower        2
    // Kenh ADC do dien ap nguon la AN8
    #define VOLTAGE_MEASURE_CHANNEL         0x0008
    // Buffer luu ket qua chuyen doi tuong tu-so dien ap nguon
    #define POWER_SUPPLY_BUFF               ADC1BUF8
#endif

//In warning-fire-forest application
#if defined(USE_MQ6_V1)
    //Define registers of MCU to control pins
    #define MQ6_MEASURE_TRIS                TRISBbits.TRISB1
    #define MQ6_MEASURE_ANALOG              ANSBbits.ANSB1

    // trang thai: 0x02 --> phat hien co khoi
    #define DetectingSmoke          0x02
    // Kenh ADC do dien ap tin hieu MQ6 la AN1
    #define MQ6_MEASURE_CHANNEL     0x0001
    // Buffer luu ket qua chuyen doi tuong tu-so dien ap tin hieu MQ6
    #define MQ6_BUFF                ADC1BUF1
#endif

#if defined(USE_WARNING_LOW_POWER) && defined(USE_MQ6_V1)
    // Gia tri cho biet module ADC dang do tin hieu tu cam bien khoi hay tu nguon cap
    #define Power_Mode_ADC  0b00
    #define MQ6_Mode_ADC    0b01
    #define Finish_Convert  0b10
#endif

#if defined(USE_MQ6_V2)
    // trang thai: 0x02 --> phat hien co khoi
    #define DetectingSmoke          0x02
#endif

//Dectecting object by PIR and Microwave sensor.
#if defined(USE_MINI_RADAR) || defined(USE_PIR)
    // 0x01 --> chua ket noi duoc voi MW sensor
    #define NoExistDevice           0x01
    // 0x06 --> da thiet lap duoc ket noi voi MW sensor
    #define ExistDevice             0x06
    // 0x04 --> phat hien co doi tuong dang xam nhap bang cam bien MW
    #define DetectingByMicroWave    0x04
    // 0x05 --> phat hien co doi tuong dang xam nhap bang cam bien PIR
    #define DetectingByPIR          0x05
#endif

//******************************************************************************
// Function Prototypes
//******************************************************************************

//functions that have been created and modified by dat_a3cbq91
void HardwareInit( void );
void ProcessNONZigBeeTasks(void);
void ProcessZigBeePrimitives(void);

//Node mang su dung ham SenOneByte de gui ban tin toi Router-EMB
void SendOneByte(BYTE data, WORD ClusterID);

#if defined(USE_SHT10)
    //Node mang gan sensor SHT su dung ham nay de gui du lieu ve Router-EMB
    void Send_HTE_ToRouterEmboard(WORD HTE_Cluster);

    //Node mang gan sensor SHT su dung ham nay de lay du lieu tu sensorSHT
    void LoadSHT10(void);
#endif

#ifdef USE_MINI_RADAR
    //Node mang gan sensor MW su dung ham nay de kiem tra da co ket noi voi MW chua?
    BOOL CheckMiniRadar(void);
#endif
//end by dat_a3cbq91

//created by Hoang Anh Son
#if defined (TRACE_ROUTE_SUPPORT)
    void SendRoutingTableToRouterEMB(void);
    void RouteDrawRequestToNextHop(BYTE MSB_dest_addr, BYTE LSB_dest_addr, BYTE NodeNumber, BYTE *pListOfNode);
    void RouteDrawReply(BYTE NodeNumber, BYTE *pListOfNode);
    //danglam
#endif
//end by Hoang Anh Son

extern void RemoveAllGroups(void);
extern BOOL LookupAPSAddress(LONG_ADDR *);
#if defined(I_SUPPORT_BINDINGS)
    extern void RemoveAllBindings(SHORT_ADDR);
#endif

extern BOOL APSSaveAPSAddress(APS_ADDRESS_MAP *AddressMap);

//******************************************************************************
// Application Variables
//******************************************************************************

ZIGBEE_PRIMITIVE    currentPrimitive;

SHORT_ADDR          destinationAddress;
#ifdef I_SUPPORT_SECURITY
    extern KEY_VAL	KeyVal;
    #ifdef USE_EXTERNAL_NVM
        extern WORD trustCenterLongAddr;
        extern NETWORK_KEY_INFO plainSecurityKey[2];
    #else
        extern ROM LONG_ADDR trustCenterLongAddr;
    #endif
#endif

//variables has modified by dat_a3cbq91
#if defined(USE_LED)
    WORD Led_Count = BLINK_LED_NOT_FORMED_NETWORK;
#endif

#if defined(USE_WARNING_LOW_POWER)
    WORD Energy_Level;//Luu tru ket qua do dien ap nguon @dat_a3cbq91
    BYTE NumberOfWarning = MaxNumberWarningLowPower;//So lan gui ban tin canh bao het nang luong, mac dinh la 2 lan.
#endif

#if defined(USE_MQ6_V1)
    WORD Mq6Signal;//Luu tru ket qua do tin hieu tu cam bien khoi MQ6 @dat_a3cbq91
    WORD ThresholdMQ6;//Bien nay cho phep tuy chinh nguong bao chay thong qua bo nhung
    WORD Threshold_0;//Bien luu gia tri nguong ban dau tuy thuoc vao moi truong

    BYTE k = 8;//bien nay duoc su dung de dem so lan do MQ6 tai moi thoi diem, mac dinh la 8 lan
#endif

#if defined(USE_WARNING_LOW_POWER) && defined(USE_MQ6_V1)
    WORD ADC_result;//Luu tru gia tri lay mau tu ADC module @dat_a3cbq91
#endif

#if defined(USE_SHT10)
    WORD humidity, temperature;//Luu tru gia tri do am, nhiet do lay tu SHT chua qua xu ly @dat_a3cbq91
#endif

#if defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM)
    volatile BYTE StatusPump;//bien nay dung de nhan thong tin phan hoi tu mach dieu khien may bom hoac dong ho bao chay.
                             //No cung chinh la du lieu se duoc gui toi bo nhung de xac nhan ket qua cua lenh dieu khien
#endif

/*
 * Bien nay cho phep tuy chinh chu ki gui thong tin xac nhan cua actor2 ve
 * Router-Emboar. Mac dinh la 5 phut gui mot lan.
 * Ngoai ra, bien nay cho phep tuy chinh chu ki gui du lieu nhiet do - do am ve
 * Router-EMB thong qua bo nhung khi nut mang o che do active. Mac dinh la 5
 * phut gui mot lan o che do active.
 */
#if defined(CHAMSOCLAN) || defined(CANHBAOCHAYRUNG)
    WORD CYCLE_SENDING;
#endif

//variable store state of device
#if defined(CHAMSOCLAN)
static union
{
    struct
    {
        WORD EnableGetDataHTE       : 1;//when timer complete, this flag is set to get humi - temp data from SHT sensor
        WORD HTE_Data_Ready         : 1;//when finished getting humi - temp data, this flag is set to be able to send data to RE.
        WORD PumpAckReceive         : 1;//when receive confirm from control pump board, this flag is set to be able to send response to RE.

        WORD LowPowerWarning        : 1;//when voltage drop below specific value, this flag is set to be able to send warning to RE
        WORD CompleteADC            : 1;//this flag use to identify that conversion of MQ6 signal and voltage signal are completed

        WORD DeviceMode             : 1;//this flag demonstrate what status of device: Active mode =0 or Save Energy mode = 1.

        WORD EnablePingToRE         : 1;//this flag use to inform to RE that ACTOR 1 or ACTOR2 is existing
        WORD PrintNeighborTable     : 1;

        WORD Reservation            : 8;
    } bits;
    WORD Val;
} WSANFlags;
#elif defined(CANHBAOCHAYRUNG)
static union
{
    struct
    {
        WORD EnableGetDataHTE       : 1;//when timer complete, this flag is set to get humi - temp data from SHT sensor
        WORD HTE_Data_Ready         : 1;//when finished getting humi - temp data, this flag is set to be able to send data to RE.
        WORD PumpAckReceive         : 1;//when receive confirm from control pump board, this flag is set to be able to send response to RE.

        WORD PrintNeighborTable     : 1;

        WORD OnTimeGetMQ6           : 1;//bien nay cho phep lay nguong ban dau sau 5 phut ke tu khi hoat dong
        WORD MQ6Warning             : 1;//when MQ6 detect smoke, this flag is set to be able to send warning to RE
        WORD DisableGetThresMq6     : 1;//when complete getting threshold of warning fire, this flag is set
        WORD LowPowerWarning        : 1;//when voltage drop below specific value, this flag is set to be able to send warning to RE
        WORD MQ6orVoltage           : 2;//this flag use to identify what signal is convert to digital
                                        //when we need to use both MQ6 sensor and monitor energy level
        WORD CompleteADC            : 1;//this flag use to identify that conversion of MQ6 signal and voltage signal are completed

        WORD DeviceMode             : 1;//this flag demonstrate what status of device: Active mode =0 or Save Energy mode = 1.

        WORD EnablePingToRE         : 1;//this flag use to inform to RE that ACTOR 1 or ACTOR2 is existing

        WORD Reservation            : 3;
    } bits;
    WORD Val;
} WSANFlags;
#else
static union
{
    struct
    {
        WORD PrintNeighborTable     : 1;

        WORD MicrowaveDetecting     : 1;//when sensor Microwave detect something, this flag is set to be able to send warning to RE.
        WORD PIRDetecting           : 1;//when sensor PIR detect something, this flag is set to be able to send warning to RE.
        WORD MicrowaveState         : 1;//when MCU identify Microwave Device, this flag is set

        WORD LowPowerWarning        : 1;//when voltage drop below specific value, this flag is set to be able to send warning to RE
        WORD CompleteADC            : 1;//this flag use to identify that conversion of MQ6 signal and voltage signal are completed

        WORD DeviceMode             : 1;//this flag demonstrate what status of device: Active mode =0 or Save Energy mode = 1.

        WORD EnablePingToRE         : 1;//this flag use to inform to RE that ACTOR 1 or ACTOR2 is existing

        WORD EnableReceiveMW        : 1;//this flag use to enable receive warning from Microwave sensor.
        WORD EnableReceivePIR       : 1;//this flag use to enable receive warning from PIR sensor.
        
        WORD Reservation            : 6;
    } bits;
    WORD Val;
} WSANFlags;
#endif

//Other variable
BYTE i;//bien i su dung nhu mot bien tam trong cac vong lap
WORD TimerCount = 0;//bien dem so lan ngat Timer
DWORD timer = TIMER_UNIT;//every 5s, the timer32 bit interrupt again.
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

    #ifndef THEODOIDONGVAT
        CYCLE_SENDING = 5 * ONE_MINUTE_VALUE;//default cycle: 5 minutes
        //CYCLE_SENDING = 6 * FIVE_SECONDS;//cycle: 30s
    #endif

    /* Initialize the UART2 to communicate with Computer or actor board */
    #if defined(USE_DEBUG) || defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM)
        ConsoleInit();
    #endif

    /* Initialize module UART1 to communicate with Miniradar sensor */
    #if defined(USE_MINI_RADAR)
        UART1Init();
    #endif

    #if defined(USE_DEBUG)
        #if defined(USE_MINI_RADAR)
            printf("Init UART1\r\n");
        #endif
        printf("Init UART2\r\n");
    #endif

    /* Initialize the hardware before initializing the ZigBee Stack */
    HardwareInit();
    #if defined(USE_DEBUG)
        printf("Init Hardware\r\n");
    #endif

    /* if using security force the use of key slot 0 in stack */
    #ifdef I_SUPPORT_SECURITY
    {
        BYTE i;
        i = 0xff;
        PutNwkActiveKeyNumber(&i);
    }
    #endif

    /* Initialize the ZigBee Stack */
    ZigBeeInit();
    currentPrimitive = NO_PRIMITIVE;
    #if defined (USE_DEBUG)
        printf("Init Zigbee Stack\r\n");
    #endif

    PHYSetShortRAMAddr(0x00, 0x0d);

    NWKClearNeighborTable();
    /* Refresh the tables anew each time the Node is booted up */
    #if defined(I_SUPPORT_BINDINGS)
        ClearBindingTable();
    #endif

    /* Clear the Group Table */
    RemoveAllGroups();
    #if defined(I_SUPPORT_BINDINGS)
        RemoveAllBindings(macPIB.macShortAddress);
    #endif
    // *************************************************************************
    // Perform any other initialization here
    // *************************************************************************
    #if defined (USE_DEBUG)
        printf( "****************************************\r\n" );
        printf( "R&D Zigbee Coordinator by WSAN-Lab411\r\n" );
        #if (RF_CHIP == MRF24J40)
            printf( "Transceiver-MRF24J40\r\n" );
        #else
            printf( "Transceiver-Unknown\r\n" );
        #endif
    #endif

    WSANFlags.Val = 0x0000;

    /* Enable interrupts to get the stack operational */
    RFIE = 1;

    while (1)
    {
        /* Clear the watch dog timer */
        CLRWDT();

        /* Process the current ZigBee Primitive */
        ZigBeeTasks( &currentPrimitive );

        /* Determine the next ZigBee Primitive */
        ProcessZigBeePrimitives();

        /* do any non ZigBee related tasks  */
        ProcessNONZigBeeTasks();

        /* use led to identify if device joined */
        #ifdef USE_PIR
        if(WSANFlags.bits.EnableReceivePIR == 0)
        {
            if(Led_Count == 0)
            {
                if(ZigBeeStatus.flags.bits.bNetworkFormed == 0)
                {
                    WSAN_LED = ~WSAN_LED;
                    Led_Count = BLINK_LED_NOT_FORMED_NETWORK;
                }
                else
                {
                    WSANFlags.bits.EnableReceivePIR = 1;
                    WSAN_LED = CLEAR;
                }
            }
            else
            {
                --Led_Count;
            }
        }

        #else
        if(Led_Count == 0)
        {
            WSAN_LED = ~WSAN_LED;

            if(ZigBeeStatus.flags.bits.bNetworkFormed)
                Led_Count = BLINK_LED_FORMED_NETWORK;
            else
                Led_Count = BLINK_LED_NOT_FORMED_NETWORK;
        }
        else
        {
            --Led_Count;
        }
        #endif
    }
}

void ProcessZigBeePrimitives(void)
{
    switch (currentPrimitive)
    {
        case NLME_DIRECT_JOIN_confirm:
            #if defined(USE_DEBUG)
                if (params.NLME_DIRECT_JOIN_confirm.Status == NWK_TABLE_FULL)
                {
                    printf( "Neighbor table is full.\r\n" );
                }
                else
                {
                    printf( "Direct join successful.\r\n" );
                }
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;

        case NLME_NETWORK_FORMATION_confirm:
            if (!params.NLME_NETWORK_FORMATION_confirm.Status)
            {
                #if defined(USE_DEBUG)
                    printf( "PAN " );
                    PrintWord(macPIB.macPANId.Val);
                    printf( " started successfully.\r\n" );
                #endif
                params.NLME_PERMIT_JOINING_request.PermitDuration = 0xFF;//to permit the joining of the network by new devices
                params.NLME_PERMIT_JOINING_request._updatePayload = TRUE;

                currentPrimitive = NLME_PERMIT_JOINING_request;
            }
            else
            {
                #if defined(USE_DEBUG)
                    PrintChar( params.NLME_NETWORK_FORMATION_confirm.Status );
                    printf( " Error forming network. Trying again...\r\n" );
                #endif
                currentPrimitive = NO_PRIMITIVE;
            }
            break;

        case NLME_PERMIT_JOINING_confirm:
            if (!params.NLME_PERMIT_JOINING_confirm.Status)
            {
                #if defined(USE_DEBUG)
                    printf( "Joining permitted.\r\n" );
                #endif
                currentPrimitive = NO_PRIMITIVE;

                #if defined(USE_WARNING_LOW_POWER)
                    AD1CON1bits.SAMP = SET;//start sampling for conversion
                #endif
            }
            else
            {
                #if defined(USE_DEBUG)
                    PrintChar( params.NLME_PERMIT_JOINING_confirm.Status );
                    printf( " Join permission unsuccessful. We cannot allow joins.\r\n" );
                #endif
                currentPrimitive = NLME_PERMIT_JOINING_request;
            }
            break;

        case NLME_ROUTE_DISCOVERY_confirm:
            #if defined(USE_DEBUG)
                if (!params.NLME_ROUTE_DISCOVERY_confirm.Status)
                {
                    printf( "..Route Reply Successful...\r\n" );
                }
                else
                {
                    PrintChar( params.NLME_ROUTE_DISCOVERY_confirm.Status );
                    printf( "..Route Reply not Successful..\r\n" );
                }
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;


        case NLME_JOIN_indication:
            #if defined(USE_DEBUG)
                printf( "Node " );
                PrintWord(params.NLME_JOIN_indication.ShortAddress.Val);
                printf( " With MAC Address " );
                //@dat_a3cbq91: Print MAC address of node has just joined
                for(i = 7; i ^ 0xFF; --i)
                {
                    PrintChar(params.NLME_JOIN_indication.ExtendedAddress.v[i]);
                }
                printf( " just joined.\r\n" );
            #endif

            /* For Zigbee 2006: If a new device with the same old longAddress address
             * joins the PAN, then make sure the old short address is no longer used and is
             * overwritten with the new shortAddress & longAddress combination
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
                }   /* end if */
            }   /* end address check block */

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
                                    GetNwkKeyInfo(&currentNetworkKeyInfo, (ROM void *)&(networkKeyInfo[i-1]));
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
                    #if defined(USE_DEBUG)
                        printf( "Node has left the network.\r\n" );
                    #endif
                }
                else
                {
                    #if defined(USE_DEBUG)
                        printf( "Another node has left the network.\r\n" );
                    #endif
                }
            }
            #ifdef I_SUPPORT_SECURITY
            {
                SHORT_ADDR		LeavingChildShortAddress;
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
            #if defined(USE_DEBUG)
                printf( "ZigBee Stack has been reset.\r\n" );
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;

        case NLME_LEAVE_confirm:
            #if defined(USE_DEBUG)
                PrintChar(params.NLME_LEAVE_confirm.Status);
                printf( " Device has left the network\r\n" );
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;

        case APSDE_DATA_confirm:
            #if defined(USE_DEBUG)
                if (params.APSDE_DATA_confirm.Status)
                {
                    printf( "Error " );
                    PrintChar( params.APSDE_DATA_confirm.Status );
                    printf( " sending message.\r\n" );
                }
                else
                {
                    printf( "Message sent successfully.\r\n" );
                }
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;

        case APSDE_DATA_indication:
            {
                BYTE        data;
                BYTE        dataLength = 0;
                BYTE        transByte = 0;

                currentPrimitive = NO_PRIMITIVE;

                switch (params.APSDE_DATA_indication.DstEndpoint)
                {
                    /* Process anything sent to ZDO */
                    case EP_ZDO:
                        {
                            dataLength = APLGet();//lay do dai du lieu @dat_a3cbq91
                            switch( params.APSDE_DATA_indication.ClusterId.Val )
                            {

                                // ********************************************************
                                // Put a case here to handle each ZDO response that we requested.
                                // ********************************************************

                                case BIND_rsp:
                                case UNBIND_rsp:
                                    data = APLGet();
                                    #if defined(USE_DEBUG)
                                        if (data == SUCCESS)
                                        {
                                            printf( "Binding/Unbinding done successfully.\r\n" );
                                        }
                                        else
                                        {
                                            PrintChar( data );
                                            printf( " Error doing binding/unbinding...\r\n" );
                                        }
                                    #endif
                                    break;

                                case NWK_ADDR_rsp:
                                    if (APLGet() == SUCCESS)
                                    {
                                        #if defined(USE_DEBUG)
                                            /* update our table when device receives the address it requested */
                                            printf( "Receiving NWK_ADDR_rsp.\r\n" );
                                        #endif
                                        // Skip over the IEEE address of the responder.
                                        for (data=0; data ^ 8; data++)
                                        {
                                            //gan lai dia chi MAC
                                            currentAPSAddress.longAddr.v[data] = APLGet();
                                            transByte++;
                                        }
                                        currentAPSAddress.shortAddr.v[0] = destinationAddress.byte.LSB = APLGet();
                                        currentAPSAddress.shortAddr.v[1] = destinationAddress.byte.MSB = APLGet();
                                        transByte += 2;
                                        APSSaveAPSAddress(&currentAPSAddress);
                                    }
                                    break;

                                /* Process any further ZDO responses here */
                                default:
                                    break;
                            }

                            // Read out the rest of the MSG in case there is another transaction.
                            for (; transByte<dataLength; transByte++)
                            {
                                APLGet();
                            }
                        }
                        break;

                    // ************************************************************************
                    // Place a case for each user defined endpoint.
                    // ************************************************************************

                    default:
                   /* In this project, every endpoint except Endpoint EP_ZDO is processed here */
                    {
                        switch (params.APSDE_DATA_indication.ClusterId.Val)
                        {
                            //user clusterID application
                            #if defined (TRACE_ROUTE_SUPPORT)
                                case ROUTE_DRAW_REQUEST_CLUSTER:
                                {
                                    BYTE i;
                                    BYTE MSB_dest_addr;
                                    BYTE LSB_dest_addr;
                                    BYTE NodeNumber;
                                    BYTE *pListOfNode;
                                    MSB_dest_addr = APLGet();//Lay ra dia chi dich can tim tuyen
                                    LSB_dest_addr = APLGet();
                                    NodeNumber    = APLGet();//So Node can di qua
                                    for (i = 0; i< NodeNumber; i++) pListOfNode[i] = APLGet();//Lay ra byte 0 dia chi MAC tat ca cac node di qua
                                    if ((macPIB.macShortAddress.v[1] == MSB_dest_addr)&&(macPIB.macShortAddress.v[0] == LSB_dest_addr)) RouteDrawReply(NodeNumber, pListOfNode);//Neu la node dich thi gui ket qua ve ROUTER_EMB
                                    else RouteDrawRequestToNextHop(MSB_dest_addr, LSB_dest_addr, NodeNumber, pListOfNode);//tiep tuc gui tin yeu cau ve tuyen den node tiep theo
                                }
                                break;
                            #endif

                            #if defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM)
                                case ACTOR_REQUEST_CLUSTER:
                                {
                                    //cau truc ban tin yeu cau bat bom tu router-emboard
                                    //------------------------------------------------------------------------------------
                                    // 0bSVVVVVVV
                                    // S = 0 --> tat, S = 1 --> bat
                                    // VVVVVVV --> van nao duoc bat/tat, dac biet, neu VVVVVVV = 0b1111111 thi tat ca ca van duoc bat/tat
                                    //------------------------------------------------------------------------------------
                                    data = APLGet();// lay lenh bat/tat bom tuoi
                                    ConsolePut(data);//gui lenh xuong mach actor
                                    SendOneByte(ACTOR_RESPONSE_CLUSTER,data);
                                    #if defined(USE_DEBUG)
                                        PrintChar(data);
                                        printf("\r\nJust sent command to actor board\r\n");
                                    #endif
                                }
                                break;
                            #endif

                            #if defined(USE_SHT10)
                                case HTE_REQUEST_CLUSTER://gui du lieu nhiet do do am ma emboard yeu cau
                                {
                                    //ban tin yeu cau lay du lieu nhiet do - do am tu bo nhung co dang: 0x00
                                    data = APLGet();
                                    LoadSHT10();//lay du lieu nhiet do - do am
                                    Send_HTE_ToRouterEmboard(HTE_RESPONSE_CLUSTER);
                                    #if defined(USE_DEBUG)
                                        printf("Just sent data for request of Router_EMB\r\n");
                                    #endif
                                }
                                break;

                                case RE_CONFIG_CYCLE_CLUSTER:
                                {
                                    data = APLGet();
                                    CYCLE_SENDING = data * 60;//data = 1 -> 5phut, data = 2 -> 10 phut,...
                                    TimerCount = 0;
                                    #ifdef USE_DEBUG
                                        printf("CYCLE_LEVEL = 0x");
                                        PrintWord(CYCLE_SENDING);
                                        printf("\r\n");
                                    #endif

                                }
                                break;
                            #endif

                            #if defined(USE_MQ6_V1)
                                case RE_ASSIGN_LEVEL_MQ6_CLUSTER:
                                {
                                    data = APLGet();
                                    ThresholdMQ6 = Threshold_0 + data * STEP_OF_THRES;
                                    #ifdef USE_DEBUG
                                        printf("MQ6_LEVEL = 0x");
                                        PrintWord(ThresholdMQ6);
                                        printf("\r\n");
                                    #endif
                                }
                                break;

                            #endif

                            default:
                                /* Catch all place for all none ZDO msgs not processed above */
                                #if defined(USE_DEBUG)
                                    PrintWord(params.APSDE_DATA_indication.ClusterId.Val);
                                    PrintChar(APLGet());
                                    printf(" Non-support ClusterID...\r\n");
                                #endif
                                break;
                        }
                        if (currentPrimitive != APSDE_DATA_request)
                            TxData = TX_DATA_START;//reset lai chi so TxData
                    }
                        break;
                }
                APLDiscardRx();
            }
            break;

        case APSME_ADD_GROUP_confirm:
        case APSME_REMOVE_GROUP_confirm:
        case APSME_REMOVE_ALL_GROUPS_confirm:
            #if defined(USE_DEBUG)
                printf("Perform Group Operation\r\n" );
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;

        /* if nothing to process first check to see if we are in startup seqence */
        case NO_PRIMITIVE:
            if (!ZigBeeStatus.flags.bits.bNetworkFormed)
            {
                if (!ZigBeeStatus.flags.bits.bTryingToFormNetwork)
                {
                    #if defined(USE_DEBUG)
                        printf( "Trying to start network...\r\n" );
                    #endif
                    params.NLME_NETWORK_FORMATION_request.ScanDuration          = 8;
                    params.NLME_NETWORK_FORMATION_request.ScanChannels.Val      = ALLOWED_CHANNELS;
                    params.NLME_NETWORK_FORMATION_request.PANId.Val             = NetworkID;
                    params.NLME_NETWORK_FORMATION_request.BeaconOrder           = MAC_PIB_macBeaconOrder;
                    params.NLME_NETWORK_FORMATION_request.SuperframeOrder       = MAC_PIB_macSuperframeOrder;
                    params.NLME_NETWORK_FORMATION_request.BatteryLifeExtension  = MAC_PIB_macBattLifeExt;
                    currentPrimitive = NLME_NETWORK_FORMATION_request;
                    break;
                }
            }
            else
            {
                if(!ZigBeeStatus.flags.bits.bHasBackgroundTasks)
                {
                    // ************************************************************************
                    // Place all processes that can send messages here.  Be sure to call
                    // ZigBeeBlockTx() when currentPrimitive is set to APSDE_DATA_request.
                    // ************************************************************************
                    #if defined(USE_MQ6_V1) || defined(USE_MQ6_V2)
                        //gui thong tin co khoi ve router-emboard
                        if(ZigBeeReady() && WSANFlags.bits.MQ6Warning)
                        {
                            WSANFlags.bits.MQ6Warning = CLEAR;
                            SendOneByte(DetectingSmoke,STATE_NODE_CLUSTER);//thong bao co khoi ve router-emboard
                            #if defined(USE_DEBUG)
                                printf("Sending warning fire-forest to RE\r\n");
                            #endif
                        }
                    #endif

                    #if defined(USE_WARNING_LOW_POWER)
                        //gui thong tin sap het nang luong ve router-emboard
                        if(ZigBeeReady() && WSANFlags.bits.LowPowerWarning)
                        {
                            WSANFlags.bits.LowPowerWarning = CLEAR;

                            if(NumberOfWarning == 0)
                            {
                                SUPPLY_VOLT_ZENER = CLEAR;//tat nguon cap cho diode zener
                                AD1CON1bits.ADON = CLEAR;//tat module ADC
                                #if defined(USE_LED)
                                    WSAN_LED_TRIS = 1;
                                #endif
                            }
                            else
                            {
                                --NumberOfWarning;
                                SendOneByte(LowPower,STATE_NODE_CLUSTER);
                                #if defined(USE_DEBUG)
                                    printf("Sending warning low-power to RE\r\n");
                                #endif
                            }
                        }
                    #endif

                    #if defined(USE_SHT10)
                        //gui thong tin nhiet do-do am dinh ky ve router-emboard
                        if(ZigBeeReady() && WSANFlags.bits.HTE_Data_Ready)
                        {
                            WSANFlags.bits.HTE_Data_Ready = CLEAR;
                            Send_HTE_ToRouterEmboard(HTE_AUTO_SEND_CLUSTER);
                            #if defined(USE_DEBUG)
                                printf("Sending period data to RE \r\n");
                            #endif
                        }
                    #endif

                    #if defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM)
                        if(ZigBeeReady() && WSANFlags.bits.PumpAckReceive)
                        {
                            WSANFlags.bits.PumpAckReceive = CLEAR;
                            SendOneByte(StatusPump,ACTOR_RESPONSE_CLUSTER);//bao ve Gateway da ra lenh dieu khien bat/tat van tuoi
                            #if defined(USE_DEBUG)
                                printf("Sending confirm of pump to RE\r\n");
                            #endif
                        }

                        if(ZigBeeReady() && WSANFlags.bits.EnablePingToRE)
                        {
                            WSANFlags.bits.EnablePingToRE = CLEAR;
                            SendOneByte(MAC_LONG_ADDR_BYTE0, JOIN_CONFIRM_CLUSTER);
                            #if defined(USE_DEBUG)
                                printf("Just ping to RE\r\n");
                            #endif
                        }
                    #endif

                    #if defined(USE_PIR)
                        if(ZigBeeReady() && WSANFlags.bits.PIRDetecting)
                        {
                            WSANFlags.bits.PIRDetecting = CLEAR;
                            SendOneByte(DetectingByPIR,STATE_NODE_CLUSTER);
                            #ifdef USE_DEBUG
                                printf("Sending warning strange-object PIR to RE\r\n");
                            #endif
                        }
                    #endif

                    #if defined(USE_MINI_RADAR)
                        //gui ban tin canh bao chuyen dong khi phat hien + ban tin kiem tra tinh trang hoat dong cua cam bien.
                        if(ZigBeeReady() && WSANFlags.bits.MicrowaveDetecting)
                        {
                            WSANFlags.bits.MicrowaveDetecting = CLEAR;
                            SendOneByte(DetectingByMicroWave,STATE_NODE_CLUSTER);
                            #ifdef USE_DEBUG
                                printf("Sending warning strange-object MW to RE\r\n");
                            #endif
                        }
                    #endif
                }
            }
            break;

        default:
            #if defined(USE_DEBUG)
                PrintChar( currentPrimitive );
                printf( " Unhandled primitive.\r\n" );
            #endif
            currentPrimitive = NO_PRIMITIVE;
            break;
    }

}

void ProcessNONZigBeeTasks(void)
{
    // *********************************************************************
    // Place any non-ZigBee related processing here.  Be sure that the code
    // will loop back and execute ZigBeeTasks() in a timely manner.
    // *********************************************************************
    #if defined(USE_SHT10)
        //Lay du lieu nhiet do-do am
        if(WSANFlags.bits.EnableGetDataHTE)
        {
            WSANFlags.bits.EnableGetDataHTE = CLEAR;
            LoadSHT10();

            #if defined(USE_DEBUG)
                printf("Temperature (hex): ");
                PrintWord(temperature);
                printf("\r\n");

                printf("Humidity (hex): ");
                PrintWord(humidity);
                printf("\r\n");
            #endif

            WSANFlags.bits.HTE_Data_Ready = SET;//cho phep gui du lieu dinh ki ve gateway
        }
    #endif

    #if defined(USE_WARNING_LOW_POWER) && defined(USE_MQ6_V1)
    if (WSANFlags.bits.MQ6orVoltage == Finish_Convert)
    {
        #if defined(USE_DEBUG)
            /* Hien thi ket qua tu cam bien khoi */
            printf("MQ6 signal (hex): ");
            PrintWord(Mq6Signal);
            printf("\r\n");

            /* Hien thi ket qua do dien ap nguon */
            printf("Voltage (hex): ");
            PrintWord(Energy_Level);
            printf("\r\n");
        #endif

        if(WSANFlags.bits.DisableGetThresMq6 == 0)
        {
            if(WSANFlags.bits.OnTimeGetMQ6 == 1)
            {
                Threshold_0 = Mq6Signal + 150;//lay nguong canh bao chay co ban.
                ThresholdMQ6 = Threshold_0;//gia tri nguong nay thay doi duoc dua tren gia tri nguong co ban.
                #if defined(USE_DEBUG)
                    printf("Theshold MQ6 signal (hex): ");
                    PrintWord(ThresholdMQ6);
                    printf("\r\n");
                #endif
                WSANFlags.bits.DisableGetThresMq6 = 1;
            }
        }
        else
        {
            if (Mq6Signal > ThresholdMQ6)
            {
                #if defined(USE_DEBUG)
                    printf("Detecting smoke\r\n");
                #endif
                WSANFlags.bits.MQ6Warning = SET;//kich hoat viec gui tin hieu ve router emboard
            }
        }

        if (Energy_Level > ThresholdPower)
        {
            #if defined(USE_DEBUG)
                printf("Detecting low power\r\n");
            #endif
            WSANFlags.bits.LowPowerWarning = SET;//kich hoat viec gui tin hieu ve router emboard
        }

        Mq6Signal = 0;//Clear this variable to reuse
        WSANFlags.bits.MQ6orVoltage = Power_Mode_ADC;//start with new cycle
    }
    #endif

    #if defined(USE_WARNING_LOW_POWER) && !defined(USE_MQ6_V1)
    if(WSANFlags.bits.CompleteADC)
    {
        #if defined(USE_DEBUG)
            printf("Voltage (hex): ");
            PrintWord(Energy_Level);
            printf("\r\n");
        #endif
        if (Energy_Level > ThresholdPower)
        {
            #if defined(USE_DEBUG)
                printf("Detecting low power\r\n");
            #endif
            WSANFlags.bits.LowPowerWarning = SET;
        }
        WSANFlags.bits.CompleteADC = CLEAR;
    }
    #endif

    #if !defined(USE_WARNING_LOW_POWER) && defined(USE_MQ6_V1)

    if(WSANFlags.bits.CompleteADC)
    {
        #if defined(USE_DEBUG)
            printf("MQ6 signal (hex): ");
            PrintWord(Mq6Signal);
            printf("\r\n");
        #endif

        if(WSANFlags.bits.DisableGetThresMq6 == 0)
        {
            if(OnTimeGetMQ6 == 1)
            {
                Threshold_0 = Mq6Signal + 150;
                ThresholdMQ6 = Threshold_0;
                #if defined(USE_DEBUG)
                    printf("Theshold MQ6 signal (hex): ");
                    PrintWord(ThresholdMQ6);
                    printf("\r\n");
                #endif
                WSANFlags.bits.DisableGetThresMq6 = 1;
            }
        }
        else
        {
            if (Mq6Signal > ThresholdMQ6)
            {
                #if defined(USE_DEBUG)
                    printf("Detecting smoke\r\n");
                #endif
                WSANFlags.bits.MQ6Warning = SET;
            }
        }

        Mq6Signal = 0;//Clear this variable to reuse

        WSANFlags.bits.CompleteADC = CLEAR;
    }
    #endif
    #if defined(USE_PIR)
    if(WSANFlags.bits.EnableReceivePIR)
    {
        if(PORTFbits.RF6)
            WSAN_LED = CLEAR;
        else
            WSAN_LED = SET;
    }
    #endif
}

/*******************************************************************************
HardwareInit

All port directioning and SPI must be initialized before calling ZigBeeInit().

For demonstration purposes, required signals are configured individually.
*******************************************************************************/
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

    /*Configure for other modules that designed by lab411*/
    // configure for led module
    #if defined(USE_LED)
        WSAN_LED_TRIS = 0;
        WSAN_LED = CLEAR;//Clear Led to a initialize state
    #endif

    // configure for ADC module
        //config for pin
    #if defined(USE_WARNING_LOW_POWER) || defined(USE_MQ6_V1)
        #if defined(USE_WARNING_LOW_POWER)
            //input analog at AN8
            VOLTAGE_MEASURE_ANALOG = SET;
            VOLTAGE_MEASURE_TRIS = 1;
            //Enable supply voltage to Zener Diode
            SUPPLY_VOLT_ZENER_TRIS = 0;
            SUPPLY_VOLT_ZENER = 1;
        #endif

        #if defined(USE_MQ6_V1)
            MQ6_MEASURE_ANALOG = SET;
            MQ6_MEASURE_TRIS = 1;
        #endif

        //configure ADC module
        AD1CON1 = 0x0470;//12-bit A/D operation
                        //Output format bit is absolute decimal result, unsigned, right-justified
                        //Internal counter ends sampling and starts conversion
                        //Sampling begins when SAMP bit is manually set

        AD1CON2 = 0x0800;//VDD is voltage reference
                        //AD1CON2 = 0x4800;//external positive voltage reference --> for measure signal from smoke sensor
                        //Conversion result is loaded into the buffer POWER_SUPPLY_BUFF
                        //Interrupt at the completion of conversion for each sample
                        //Always uses channel input selects for Sample A

        AD1CON3 = 0x1F02;//Sample Time = 31Tad, Tad = 3Tcy

        #if defined(USE_WARNING_LOW_POWER)
            AD1CHS = VOLTAGE_MEASURE_CHANNEL;//do dien ap dau tien
        #else
            AD1CHS = MQ6_MEASURE_CHANNEL;//do MQ6
        #endif

        AD1CON1bits.ADON = SET;//bat dau lay gia tri dien ap

        //config for ADC interrupt
        IFS0bits.AD1IF = CLEAR;
        IEC0bits.AD1IE = SET;
        IPC3bits.AD1IP = 6;
    #endif

    // configure for I2C software module to connect with SHT sensor.
    #if defined(USE_SHT10)
        // Make RG2, RG3 outputs.
        SCK_TRIS    =   CLEAR;
        DATA_TRIS   =   CLEAR;
    #endif

    // check Miniradar if it connect with this board target
    #ifdef USE_MINI_RADAR
        if(CheckMiniRadar())
        {
            WSANFlags.bits.MicrowaveState = 1;
            #if defined(USE_DEBUG)
                printf("Miniradar V2.0 found device\r\n");
            #endif
        }
        else
        {
            WSANFlags.bits.MicrowaveState = 0;
            #if defined(USE_DEBUG)
                printf("Motion Sensor not found\r\n");
            #endif
        }
    #endif

    // configure interface to connect with PIR
    #if defined(USE_PIR) || defined(USE_MQ6_V2)

        INTCON1bits.NSTDIS = 1;		//Interrupt nesting is disabled
        INTCON2bits.INT0EP = 0;//pos edge

        IPC0bits.INT0IP = 0b111;    // Interrupt priority level 7

        IFS0bits.INT0IF = CLEAR; 	   // Interrupt request has not occurred
        IEC0bits.INT0IE = SET;

        TRISFbits.TRISF6 = 1;   //RF6 connect to signal wire of PIR
        PORTFbits.RF6 = CLEAR;
    #endif
}

/*******************************************************************************
User Interrupt Handler

The stack uses some interrupts for its internal processing. Once it is done
checking for its interrupts, the stack calls this function to allow for any
additional interrupt processing.
*******************************************************************************/
//this routine is written by dat_a3cbq91
void _ISR __attribute__((interrupt, auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = CLEAR;
    #if defined(THEODOIDONGVAT)
        #if defined(USE_MINI_RADAR)
            if(WSANFlags.bits.EnableReceiveMW == CLEAR)
            {
                WSANFlags.bits.EnableReceiveMW = SET;
                printf("Cho phep nhan W\r\n");
            }
        #endif
    #else
    //if (SleepingStateTimerFlag) SleepTimer ++;
    TimerCount++;

    if (WSANFlags.bits.DeviceMode == ActiveMode)//Devicemode = 1 --> Save Energy.
    {
        if (TimerCount == CYCLE_SENDING)
        {
            TimerCount = CLEAR;

            //SET get temperature and humidity from SHT10 every (5*CYCLE_SENDING)s
            #if defined(USE_SHT10)
                WSANFlags.bits.EnableGetDataHTE = SET;
            #endif

            //start sampling for conversion to digital
            #if defined(USE_WARNING_LOW_POWER) && !defined(USE_MQ6_V1)
                AD1CON1bits.SAMP = SET;
            #endif

            #if defined(I_AM_ACTOR1) || defined(I_AM_ACTOR2)
                WSANFlags.bits.EnablePingToRE = SET;
            #endif

            #if defined(USE_MQ6_V1)
                WSANFlags.bits.OnTimeGetMQ6 = SET;
            #endif

        }

        //start sampling for conversion to digital
        #if defined(USE_MQ6_V1)
            AD1CON1bits.SAMP = SET;
        #endif
    }
    #endif
    //re-assigned for PR3:PR2
    PR3 = timer >> 16;
    PR2 = timer & 0xFFFF;

    //if you don't want print neighbor of this node, comment the following
    //WSANFlags.bits.PrintNeighborTable = SET;
}

#if defined(USE_MQ6_V1) || defined(USE_WARNING_LOW_POWER)
void _ISR __attribute__((interrupt, auto_psv)) _ADC1Interrupt(void)
{
    IFS0bits.AD1IF = CLEAR;
    //Lay gia tri chuyen doi ADC
    #if defined(USE_MQ6_V1) && defined(USE_WARNING_LOW_POWER)
        switch(WSANFlags.bits.MQ6orVoltage)
        {
            case Power_Mode_ADC:
                Energy_Level = POWER_SUPPLY_BUFF;//lay gia tri dien ap tu zener de suy ra VDD
                AD1CHS = MQ6_MEASURE_CHANNEL;//chuyen sang do mq6
                WSANFlags.bits.MQ6orVoltage = MQ6_Mode_ADC;
                break;

            case MQ6_Mode_ADC:
                if(k == 0)
                {
                    k = 8;
                    Mq6Signal = Mq6Signal >> 3;//chia 8 lay gia tri trung binh
                    AD1CHS = VOLTAGE_MEASURE_CHANNEL;//chuyen sang do voltage
                    #if defined(USE_LED)
                        WSAN_LED = ~WSAN_LED;
                    #endif
                    WSANFlags.bits.MQ6orVoltage = Finish_Convert;
                }
                else
                {
                    #if defined(USE_DEBUG)
                        ConsolePut(k+0x30);
                        printf("\r\n");
                    #endif
                    --k;
                    Mq6Signal = Mq6Signal + MQ6_BUFF;//lay gia tri dien ap tu mq6
                    AD1CON1bits.SAMP = SET;
                }
                break;

            default:
                break;
        }
    #endif

    #if defined(USE_MQ6_V1) && !defined(USE_WARNING_LOW_POWER)
        if(k == 0)
        {
            k = 8;
            Mq6Signal = Mq6Signal >> 3;//chia 8 lay gia tri trung binh
            #if defined(USE_LED)
                WSAN_LED = ~WSAN_LED;
            #endif
            WSANFlags.bits.CompleteADC = SET;
        }
        else
        {
            #if defined(USE_DEBUG)
                ConsolePut(k+0x30);
                printf("\r\n");
            #endif
            --k;
            Mq6Signal = Mq6Signal + MQ6_BUFF;//lay gia tri dien ap tu mq6
            AD1CON1bits.SAMP = SET;
        }
    #endif

    #if defined(USE_WARNING_LOW_POWER) && !defined(USE_MQ6_V1)
        Energy_Level = POWER_SUPPLY_BUFF;
        WSANFlags.bits.CompleteADC = SET;
    #endif
}
#endif

#if defined(USE_CONTROL_PUMP) || defined(USE_CONTROL_ALARM)
void _ISR __attribute__((interrupt, auto_psv)) _U2RXInterrupt(void)
{
    IFS1bits.U2RXIF = CLEAR;
    StatusPump = U2RXREG;
    if(StatusPump)
        WSANFlags.bits.PumpAckReceive = SET;
}
#endif

#if defined (USE_MINI_RADAR)
void _ISR __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = CLEAR;
    i = U1RXREG;
    #if defined(USE_DEBUG)
        ConsolePut(i);
        printf("\r\n");
    #endif
    //receive character W from Mini Radar Board -> detected motion
    if(i == 'W')
    {
        //mradar_count--;
        //if(mradar_count == 0)
        if(WSANFlags.bits.EnableReceiveMW)
        {
            WSANFlags.bits.MicrowaveDetecting = SET;
            WSANFlags.bits.EnableReceiveMW = CLEAR;
            //re-assigned for PR3:PR2
            PR3 = timer >> 16;
            PR2 = timer & 0xFFFF;
            #if defined(USE_DEBUG)
                printf("Khoi tao timer dem thoi gian\r\n");
            #endif
        }
    }
}
#endif

#if defined(USE_PIR)
void _ISR __attribute__((interrupt, auto_psv)) _INT0Interrupt(void)
{
    IFS0bits.INT0IF = CLEAR;
    if(WSANFlags.bits.EnableReceivePIR)
    {
        for(i = 0; i < 200; i++);//eliminate noise
        if(PORTFbits.RF6 == 0)
        {
            WSANFlags.bits.PIRDetecting = SET;
        }
    }
}
#endif

#if defined(USE_MQ6_V2)
void _ISR __attribute__((interrupt, auto_psv)) _INT0Interrupt(void)
{
    IFS0bits.INT0IF = CLEAR;
    for(i = 0; i < 200; i++);//eliminate noise
    if(PORTFbits.RF6 == 0)
    {
        WSANFlags.bits.MQ6Warning = SET;
    }
}
#endif
/*********************************************************************
 * Function:        void SendOneByte(BYTE data, WORD ClusterID)
 *
 * PreCondition:    Init OK
 *
 * Input:           state
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        Gui mot thong tin ve router-EMB. Thong tin nay co the la cac
 *                  trang thai chay rung, canh bao muc nang luong hoac la thong
 *                  xac nhan co mot node mang nao do vua gia nhap mang
 *
 * Note:
 ********************************************************************/
void SendOneByte(BYTE data, WORD ClusterID)
{
    //cau truc ban tin: SS
    TxBuffer[TxData++] = MAC_LONG_ADDR_BYTE0;
    TxBuffer[TxData++] = data;

    /* load parameters for APSDE_DATA_request primitive */
    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;

    /* load network address of router-emboard */
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = RouterEmboardAddrMSB;
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = RouterEmboardAddrLSB;

    params.APSDE_DATA_request.SrcEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.DstEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID;

    //params.APSDE_DATA_request.asduLength; TxData
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;

//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_FORCE;
//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    #ifdef I_SUPPORT_SECURITY
        params.APSDE_DATA_request.TxOptions.Val = Enable;
    #else
        params.APSDE_DATA_request.TxOptions.Val = CLEAR;
    #endif
    params.APSDE_DATA_request.TxOptions.bits.acknowledged = SET;
    params.APSDE_DATA_request.ClusterId.Val = ClusterID;

    ZigBeeBlockTx();
    currentPrimitive = APSDE_DATA_request;
}

#if defined (TRACE_ROUTE_SUPPORT)

/*********************************************************************
 * Function:        void RouteDrawRequestToNextHop(BYTE MSB_addr, BYTE LSB_addr, BYTE NodeNumber, BYTE *pListOfNode)
 *
 * PreCondition:
 *
 * Cluater:         REQUEST_ROUTE_CLUSTER
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        Gui yeu cau ve tuyen den nut co dia chi mang MSB_addr, LSB_addr
 *
 * Note: By Hoang Anh Son - K53
 ********************************************************************/
void RouteDrawRequestToNextHop(BYTE MSB_dest_addr, BYTE LSB_dest_addr, BYTE NodeNumber, BYTE *pListOfNode)
{
    BYTE i;
    NodeNumber ++;
    TxBuffer[TxData++] = MSB_dest_addr;//Dia chi Node dich
    TxBuffer[TxData++] = LSB_dest_addr;
    TxBuffer[TxData++] = NodeNumber;//So node di qua
    for (i = 0; i< NodeNumber; i++) TxBuffer[TxData++]= *pListOfNode;

    TxBuffer[TxData++] = MAC_LONG_ADDR_BYTE0; //Chen them byte0 dia chi MAC cua node
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
    params.APSDE_DATA_request.SrcEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.DstEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID;

    //params.APSDE_DATA_request.asduLength; TxData
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;//gioi han so hop ma du lieu duoc phep truyen qua, o day la 10 hop

//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_FORCE;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    #ifdef I_SUPPORT_SECURITY
        params.APSDE_DATA_request.TxOptions.Val = SET;
    #else
        params.APSDE_DATA_request.TxOptions.Val = CLEAR;//khong ho tro bao mat
    #endif
    params.APSDE_DATA_request.TxOptions.bits.acknowledged = SET;// Yeu cau ACK tu thiet bi thu
    params.APSDE_DATA_request.ClusterId.Val = ROUTE_DRAW_REQUEST_CLUSTER;

    ZigBeeBlockTx();//goi ham nay de ZigbeeIsReady() --> false
    currentPrimitive = APSDE_DATA_request;
}
/*********************************************************************
 * Function:        void RouteDrawReply(BYTE NodeNumber, BYTE *pListOfNode)
 *
 * PreCondition:
 *
 * Cluater:         REQUEST_ROUTE_CLUSTER
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        Gui yeu cau ve tuyen den nut co dia chi mang MSB_addr, LSB_addr
 *
 * Note: By Hoang Anh Son - K53
 ********************************************************************/
void RouteDrawReply(BYTE NodeNumber, BYTE *pListOfNode)
{
    BYTE i;
    NodeNumber ++;
    TxBuffer[TxData++] = NodeNumber;//So node di qua
    for (i = 0; i< NodeNumber - 1; i++) TxBuffer[TxData++]= *pListOfNode;

    TxBuffer[TxData++] = MAC_LONG_ADDR_BYTE0; //Chen them byte0 dia chi MAC cua node
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = RouterEmboardAddrMSB;
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = RouterEmboardAddrLSB;
    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;//0x02 : Ban tin Unicast, Set che do dia chi mang ( DstAddress) 16 bit va yeu cau co' DstEndPoint
    params.APSDE_DATA_request.SrcEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.DstEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID;

    //params.APSDE_DATA_request.asduLength; TxData
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;//gioi han so hop ma du lieu duoc phep truyen qua, o day la 10 hop

//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_FORCE;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
//    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    #ifdef I_SUPPORT_SECURITY
        params.APSDE_DATA_request.TxOptions.Val = SET;
    #else
        params.APSDE_DATA_request.TxOptions.Val = CLEAR;//khong ho tro bao mat
    #endif
    params.APSDE_DATA_request.TxOptions.bits.acknowledged = SET;// Yeu cau ACK tu thiet bi thu
    params.APSDE_DATA_request.ClusterId.Val = ROUTE_DRAW_REPLY_CLUSTER;

    ZigBeeBlockTx();//goi ham nay de ZigbeeIsReady() --> false
    currentPrimitive = APSDE_DATA_request;
}
#endif

#if defined(USE_SHT10)
/*********************************************************************
 * Function:        void Send_HTE_ToRouterEmboard(HTE_Cluster)
 *
 * PreCondition:
 *
 * Input:           clusterID
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        Gui du lieu nhiet do-do am-nang luong (HTE) ve
 *                  router-EMB.
 *
 * Note:
 ********************************************************************/
void Send_HTE_ToRouterEmboard(WORD HTE_Cluster)
{
    //nap du lieu nhiet do - do am - nang luong chua qua xu ly vao buffer TX
    TxBuffer[TxData++] = MAC_LONG_ADDR_BYTE0;
    TxBuffer[TxData++] = (BYTE) (temperature >> 8);
    TxBuffer[TxData++] = (BYTE) (temperature);
    TxBuffer[TxData++] = (BYTE) (humidity >> 8);
    TxBuffer[TxData++] = (BYTE) (humidity);
    #if defined(USE_WARNING_LOW_POWER)
        TxBuffer[TxData++] = (BYTE) (Energy_Level >> 8);
        TxBuffer[TxData++] = (BYTE) (Energy_Level);
    #else
        TxBuffer[TxData++] = 0x00;
        TxBuffer[TxData++] = 0x00;
    #endif

    params.APSDE_DATA_request.DstAddrMode = APS_ADDRESS_16_BIT;
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[1] = RouterEmboardAddrMSB;
    params.APSDE_DATA_request.DstAddress.ShortAddr.v[0] = RouterEmboardAddrLSB;

    params.APSDE_DATA_request.SrcEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.DstEndpoint = WSAN_Endpoint;
    params.APSDE_DATA_request.ProfileId.Val = MY_PROFILE_ID;//MY_PROFILE_ID = 0x0103

    //params.APSDE_DATA_request.asduLength; TxData
    params.APSDE_DATA_request.RadiusCounter = DEFAULT_RADIUS;//gioi han so hop ma du lieu duoc phep truyen qua, o day la 10 hop

    //params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_FORCE;
    //params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_SUPPRESS;
    params.APSDE_DATA_request.DiscoverRoute = ROUTE_DISCOVERY_ENABLE;

    #ifdef I_SUPPORT_SECURITY
        params.APSDE_DATA_request.TxOptions.Val = SET;
    #else
        params.APSDE_DATA_request.TxOptions.Val = CLEAR;//khong ho tro bao mat
    #endif
    params.APSDE_DATA_request.TxOptions.bits.acknowledged = SET;// Yeu cau ACK tu thiet bi thu
    params.APSDE_DATA_request.ClusterId.Val = HTE_Cluster;
    ZigBeeBlockTx();

    currentPrimitive = APSDE_DATA_request;
}

void LoadSHT10(void)
{
    BYTE error, checksum;

    error = 0;
    error += s_measure((BYTE*) &humidity, &checksum, HUMI); //measure humidity
    error += s_measure((BYTE*) &temperature, &checksum, TEMP); //measure temperature
    while (error != 0)
    {
        s_connectionreset(); //in case of an error: connection reset
        error += s_measure((BYTE*) &humidity, &checksum, HUMI); //measure humidity
        error += s_measure((BYTE*) &temperature, &checksum, TEMP); //measure temperature
    }
}
#endif

#ifdef USE_MINI_RADAR
/*********************************************************************
 * Function:     	void CheckMiniRadar()
 *
 * PreCondition:   UART1  ready to use
 *
 * Cluater:
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:
 *
 * Note:
 ********************************************************************/
BOOL CheckMiniRadar()
{
    unsigned long _i =0;
    BOOL ex =0;
    char Temp;

    //disable UART1 interrupt
    IEC0bits.U1RXIE = CLEAR;
    IFS0bits.U1RXIF = CLEAR;

    //ping to MiniRadar Board
    UART1Put('R');

    //Disable Watchdog Timer to avoid reset system before receive response from MiniRadar Board
    DISABLE_WDT();

    //if don't receive response from MiniRadar Board after time out, break wait
    while((!IFS0bits.U1RXIF)&&(_i <= 1000000)) _i++;

    //if receive (a) message from MiniRadar Board, check it if is 'R' character
    if(IFS0bits.U1RXIF)
    {
        IFS0bits.U1RXIF = CLEAR;
        Temp = U1RXREG;
        if(Temp == 'R') ex = 1;
        else ex = 0;
    }
    else
         ex = 0;

    //Re-enable Watchdog Timer.
    ENABLE_WDT();

    //Re-enable UART1 interrupt
    IEC0bits.U1RXIE = SET;
    IFS0bits.U1RXIF = CLEAR;
    return ex;
}
#endif
