/*********************************************************************
 *
 *                  ZigBee non-volatile memory storage
 *
 *********************************************************************
 * FileName:        zNVM.c
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
 * Nilesh Rajbharti     7/12/04 Rel 0.9
 * Nilesh Rajbharti     11/1/04 Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 *********************************************************************

 This file contains all the routines necessary to access data that
 should be stored in non-volatile memory.

 These routines store the neighbor and binding tables in flash program
 memory.  If these tables are moved to a different type of memory, only
 this file and zNVM.H need to be changed.

 *****************************************************************************/
#if !defined(__C30__)
	#include <string.h>
#endif
// Uncomment ENABLE_DEBUG line to enable debug mode for this file.
// Or you may also globally enable debug by defining this macro
// in zigbee.def file or from compiler command-line.
#ifndef ENABLE_DEBUG
//#define ENABLE_DEBUG
#endif

// Enable this to print out information if multiple attempts are required
// to write to external NVM
#ifdef ENABLE_DEBUG
    #define PRINT_MULTIPLE_WRITE_ATTEMPTS
#endif

#include "zigbee.h"
#include "sralloc.h"
#include "MSPI.h"
#include "zNVM.h"
#include "zNWK.h"
#include "zAPS.h"

#if defined( ENABLE_DEBUG ) || defined( PRINT_MULTIPLE_WRITE_ATTEMPTS )
    #include "console.h"
#endif

#ifdef I_SUPPORT_SECURITY
	#include "zSecurity.h"
#endif

#if defined(__C30__)
    #include <string.h>
#endif

#if defined(__18F4620) && !defined(USE_EXTERNAL_NVM)
    #define ERASE_BLOCK_SIZE        (64ul)
    #define WRITE_BLOCK_SIZE        (64ul)
#elif !defined(USE_EXTERNAL_NVM)
    /*  If you are storing your nonvolatile memory internally and you are using
        a device other than the PIC18F4620, it may have different erase and
        write block sizes.  Determine the block sizes for your device, add the
        #defines for them here, and comment out the error line below. */
    #error "Verify ERASE_BLOCK_SIZE and WRITE_BLOCK_SIZE for target PIC"
#endif

//******************************************************************************
// Compilation options
//******************************************************************************

// The definitions for ERASE_BLOCK_SIZE and WRITE_BLOCK_SIZE have been moved
// to ZENA and zigbee.def, since they can change based on the selected processor.

// To help reduce the number of times program memory is written, enable this
// option to see if the program memory already matches the values we are trying
// to write to it.
#define CHECK_BEFORE_WRITE

// The definition for VERIFY_WRITE has been moved to ZENA and zigbee.def.  Use it
// if you would like a verification performed on program memory writes.  Note that
// this option will also perform the CHECK_BEFORE_WRITE function.  Be aware that
// if there is a problem with the device where it is unable to successfully
// perform the program memory write, and infinite loop will occur.

#define VERIFY_WRITE

// If we are using separate SPI's for the transceiver and a serial EE, redefine
// the SPI routines.
#if defined(USE_EXTERNAL_NVM) && !defined(EE_AND_RF_SHARE_SPI)
    #define SPIPut( x )     EE_SPIPut( x )
    #define SPIGet()        EE_SPIGet()
#endif

//******************************************************************************
// Constants
//******************************************************************************

#ifdef USE_EXTERNAL_NVM

    #define SPISelectEEPROM()   {EEPROM_nCS = 0;}
    #define SPIUnselectEEPROM() {EEPROM_nCS = 1;}

    #define SPIREAD     0x03
    #define SPIWRITE    0x02
    #define SPIWRDI     0x04
    #define SPIWREN     0x06
    #define SPIRDSR     0x05
    #define SPIWRSR     0x01

    #define WIP_MASK    0x03  //0x01

#endif



//******************************************************************************
// RAM and ROM Variables
//******************************************************************************

#ifdef PRINT_MULTIPLE_WRITE_ATTEMPTS
    int flag = 0;
#endif

// Our MAC address.
#define MAC_ADDRESS_ROM_LOCATION        0x00002A
#if defined(USE_EXTERNAL_NVM) && defined(STORE_MAC_EXTERNAL)
    WORD macLongAddrEE;
    BYTE macLongAddrByte[8] = {MAC_LONG_ADDR_BYTE0, MAC_LONG_ADDR_BYTE1, MAC_LONG_ADDR_BYTE2, MAC_LONG_ADDR_BYTE3,
                             MAC_LONG_ADDR_BYTE4, MAC_LONG_ADDR_BYTE5, MAC_LONG_ADDR_BYTE6, MAC_LONG_ADDR_BYTE7};
#else
    #if defined(__C18__)
    #pragma romdata macaddress=MAC_ADDRESS_ROM_LOCATION
    #endif
    ROM LONG_ADDR macLongAddr = {{MAC_LONG_ADDR_BYTE0, MAC_LONG_ADDR_BYTE1, MAC_LONG_ADDR_BYTE2, MAC_LONG_ADDR_BYTE3,
                             MAC_LONG_ADDR_BYTE4, MAC_LONG_ADDR_BYTE5, MAC_LONG_ADDR_BYTE6, MAC_LONG_ADDR_BYTE7}
    };
    #if defined(__C18__)                             
    #pragma romdata
    #endif
#endif


#ifndef USE_EXTERNAL_NVM
    // Since we are using program memory for storage with no hardcoded
    // memory address, it is possible that these memory regions may be
    // placed in between executable code. In that case, when we update this
    // memory, we would erase part of executable code.
    // To avoid that there are two ERASE_BLOCK_SIZE filler block around the storage.
    // This will make sure that we are well away from executable code.
    // We don't need this if we are using an external EEPROM
    ROM BYTE filler[ERASE_BLOCK_SIZE] = {0x00};     // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
#endif


// Binding table information.
#if defined(I_SUPPORT_BINDINGS) || defined(SUPPORT_END_DEVICE_BINDING)
    BINDING_RECORD      currentBindingRecord;       // RAM copy of current records.

    #ifdef USE_EXTERNAL_NVM
        WORD                bindingValidityKey;
        WORD                apsBindingTable;
        WORD                bindingTableUsageMap;
        WORD                bindingTableSourceNodeMap;
        WORD                pCurrentBindingRecord;     // To remember current table entry pointer.
    #else
        ROM WORD            bindingValidityKey = {0x0000};
        ROM BINDING_RECORD  apsBindingTable[MAX_BINDINGS] = {0x00};           // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
        ROM BYTE            bindingTableUsageMap[BINDING_USAGE_MAP_SIZE] = {0x00};      // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
        ROM BYTE            bindingTableSourceNodeMap[BINDING_USAGE_MAP_SIZE] = {0x00}; // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
        ROM BINDING_RECORD  *pCurrentBindingRecord; // To remember current table entry pointer.
    #endif
#endif


#if defined(I_SUPPORT_GROUP_ADDRESSING)
    #ifdef USE_EXTERNAL_NVM
        WORD                apsGroupAddressTable;
        WORD                pCurrentGroupAddressRecord;
    #else
        ROM GROUP_ADDRESS_RECORD    apsGroupAddressTable[MAX_GROUP];
        ROM GROUP_ADDRESS_RECORD    *pCurrentGroupAddressRecord;
    #endif
#endif

// Neighbor Table information
NEIGHBOR_RECORD             currentNeighborRecord;                      // Node information.
NEIGHBOR_TABLE_INFO         currentNeighborTableInfo;               // Info about the neighbor table and the node's children.

#ifdef USE_EXTERNAL_NVM
    WORD                    neighborTableInfo;
    WORD                    neighborTable;
    WORD                    pCurrentNeighborRecord;
#else
    ROM NEIGHBOR_TABLE_INFO neighborTableInfo = {0x00};         // Initialize to something other than the valid key.
    ROM NEIGHBOR_RECORD     neighborTable[MAX_NEIGHBORS] = {0x00};  // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
    ROM NEIGHBOR_RECORD     *pCurrentNeighborRecord;
#endif


// Routing information
#if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
    ROUTING_ENTRY currentRoutingEntry;              // RAM copy of the current routing entry

    #ifdef USE_EXTERNAL_NVM
        WORD routingTable;
        WORD pCurrentRoutingEntry;       // Pointer to the current routing entry in ROM
    #else
        ROM ROUTING_ENTRY routingTable[ROUTING_TABLE_SIZE] = {0x00};   // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
        ROM ROUTING_ENTRY *pCurrentRoutingEntry;    // Pointer to the current routing entry in ROM
    #endif
#endif


// Descriptors
#ifdef USE_EXTERNAL_NVM
    WORD configNodeDescriptor;
    WORD configPowerDescriptor;
    WORD configSimpleDescriptors;
#endif


// APS Address Map Table
#if MAX_APS_ADDRESSES > 0
    APS_ADDRESS_MAP     currentAPSAddress;
    #ifdef USE_EXTERNAL_NVM
        WORD apsAddressMapValidityKey;
        WORD apsAddressMap;
    #else
        ROM WORD            apsAddressMapValidityKey = {0x0000};
        ROM APS_ADDRESS_MAP apsAddressMap[MAX_APS_ADDRESSES];
    #endif
#endif

// Security Information
#if defined(I_SUPPORT_SECURITY)
    NETWORK_KEY_INFO    currentNetworkKeyInfo;
    #ifdef USE_EXTERNAL_NVM
        WORD nwkActiveKeyNumber;
        WORD networkKeyInfo;
        WORD trustCenterLongAddr;
    #else
        ROM BYTE nwkActiveKeyNumber;
        ROM NETWORK_KEY_INFO    networkKeyInfo[NUM_NWK_KEYS];
        #if defined(I_AM_COORDINATOR) || defined(I_AM_TRUST_CENTER)
            ROM LONG_ADDR trustCenterLongAddr = {TRUST_CENTER_LONG_ADDR_BYTE0, TRUST_CENTER_LONG_ADDR_BYTE1, TRUST_CENTER_LONG_ADDR_BYTE2, TRUST_CENTER_LONG_ADDR_BYTE3,
                                                 TRUST_CENTER_LONG_ADDR_BYTE4, TRUST_CENTER_LONG_ADDR_BYTE5, TRUST_CENTER_LONG_ADDR_BYTE6, TRUST_CENTER_LONG_ADDR_BYTE7};
        #else
        	ROM LONG_ADDR trustCenterLongAddr;
        #endif
    #endif
#endif


#ifndef USE_EXTERNAL_NVM
    // If we are storing to program memory, make sure the next code
    // section does not fall in the same page as our memory.
    ROM BYTE filler2[ERASE_BLOCK_SIZE] = {0x00};    // Does not need initializing, but the HI-TECH compiler will not place it in ROM memory otherwise
#endif


//******************************************************************************
// Function Prototypes
//******************************************************************************

#if defined( USE_EXTERNAL_NVM ) && (defined( VERIFY_WRITE ) || defined( CHECK_BEFORE_WRITE ))
    BYTE ComparePage( WORD dest,  BYTE *src, WORD count );
#endif

/*********************************************************************
 * Function:        BYTE NVMInit( void )
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          0 - success
 *                  1 - not enough space available
 *
 * Side Effects:    None
 *
 * Overview:        This routine allocates chunks of nonvolatile memory for
 *                  all the nonvolatile tables.  If there is not enough space,
 *                  a non-zero value is returned.  Otherwise, 0 is returned.
 *
 * Note:            This routine needs to be called before attempting
 *                  to read or write nonvolatile tables when external storage
 *                  is used.  This function is not required if the tables are
 *                  stored internally.  A stub is provided for compatibility.
 ********************************************************************/

#ifdef USE_EXTERNAL_NVM

    BYTE NVMInit( void )
    {
        BYTE    *memBlock;
        BYTE    result = 0;

        SPIUnselectEEPROM();

        CLRWDT();

        #ifdef ENABLE_DEBUG
            ConsolePutROMString((ROM char * const)"NVMInit started...\r\n");
        #endif

        #if defined(USE_EXTERNAL_NVM) && defined(STORE_MAC_EXTERNAL)
            result |= NVMalloc(sizeof (LONG_ADDR), &macLongAddrEE );
            #ifdef ZCP_DEBUG
                PutMACAddress(macLongAddrByte);
            #endif
        #endif

        #if defined(I_SUPPORT_BINDINGS)
            result |= NVMalloc( sizeof(WORD), &bindingValidityKey );
            result |= NVMalloc( sizeof(BINDING_RECORD) * MAX_BINDINGS, &apsBindingTable );
            result |= NVMalloc( BINDING_USAGE_MAP_SIZE, &bindingTableUsageMap );
            result |= NVMalloc( BINDING_USAGE_MAP_SIZE, &bindingTableSourceNodeMap );
        #endif

        #if defined(I_SUPPORT_GROUP_ADDRESSING)
            result |= NVMalloc( sizeof(APS_GROUP_RECORD) * MAX_GROUP, &apsGroupAddressTable);
        #endif

        result |= NVMalloc( sizeof(NEIGHBOR_TABLE_INFO), &neighborTableInfo );
        result |= NVMalloc( sizeof(NEIGHBOR_RECORD) * MAX_NEIGHBORS, &neighborTable );

        #if defined(I_SUPPORT_ROUTING) && !defined(USE_TREE_ROUTING_ONLY)
            result |= NVMalloc( sizeof(ROUTING_ENTRY) * ROUTING_TABLE_SIZE, &routingTable );
        #endif


        result |= NVMalloc( sizeof(NODE_DESCRIPTOR), &configNodeDescriptor );
        result |= NVMalloc( sizeof(NODE_POWER_DESCRIPTOR), &configPowerDescriptor );
        // NOTE - the simple descriptor for the ZDO has been removed in later specs, so the "+1" will go away.
        result |= NVMalloc( sizeof(NODE_SIMPLE_DESCRIPTOR) * (NUM_USER_ENDPOINTS+1), &configSimpleDescriptors );


        #if MAX_APS_ADDRESSES > 0
            result |= NVMalloc( sizeof(WORD), &apsAddressMapValidityKey );
            result |= NVMalloc( sizeof(APS_ADDRESS_MAP) * MAX_APS_ADDRESSES, &apsAddressMap );
        #endif

        #if defined(I_SUPPORT_SECURITY)
            result |= NVMalloc( sizeof(BYTE), &nwkActiveKeyNumber );
            result |= NVMalloc( sizeof(NETWORK_KEY_INFO) * NUM_NWK_KEYS, &networkKeyInfo );
            #if !(defined(I_AM_COORDINATOR) || defined(I_AM_TRUST_CENTER))
                result |= NVMalloc( sizeof(LONG_ADDR), &trustCenterLongAddr );
            #endif
        #endif

        if (!result)
        {
            #ifdef ENABLE_DEBUG
                ConsolePutROMString((ROM char * const)"Initializing EE...\r\n");
            #endif
            // If the MAC Address is stored externally, then the user is responsible
            // for programming it.  They may choose to preprogram the EEPROM, or program
            // it based on other input.  It should be programmed with the PutMACAddress() macro.


            // Initialize the trust center address
            #if defined(I_SUPPORT_SECURITY) && (defined(I_AM_COORDINATOR) || defined(I_AM_TRUST_CENTER))
                if ((memBlock = SRAMalloc( 8 )) == NULL)
                {
                    result = 1;
                }
                else
                {
                    int i = 0;
    
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE0;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE1;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE2;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE3;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE4;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE5;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE6;
                    memBlock[i++] = TRUST_CENTER_LONG_ADDR_BYTE7;
    
                    NVMWrite( trustCenterLongAddr, memBlock, 8 );
                    SRAMfree( memBlock );
                }
            #endif

            // Initialize the descriptors using the ROM copy.
            if ((memBlock = SRAMalloc( sizeof(NODE_DESCRIPTOR) )) == NULL)
            {
                result = 1;
            }
            else
            {
                memcpypgm2ram( memBlock, (void *)&Config_Node_Descriptor, sizeof(NODE_DESCRIPTOR) );
                NVMWrite( configNodeDescriptor, memBlock, sizeof(NODE_DESCRIPTOR) );
                SRAMfree( memBlock );
            }

            if ((memBlock = SRAMalloc( sizeof(NODE_POWER_DESCRIPTOR) )) == NULL)
            {
                result = 1;
            }
            else
            {
                memcpypgm2ram( memBlock, (void *)&Config_Power_Descriptor, sizeof(NODE_POWER_DESCRIPTOR) );
                NVMWrite( configPowerDescriptor, memBlock, sizeof(NODE_POWER_DESCRIPTOR) );
                SRAMfree( memBlock );
            }

            if ((memBlock = SRAMalloc( sizeof(NODE_SIMPLE_DESCRIPTOR) )) == NULL)
            {
                result = 1;
            }
            else
            {
                // NOTE - Currently, a simple descriptor is needed for the ZDO endpoint.  When this requirement
                // goes away, take off the "+1".
                int     i;

                for (i=0; i<NUM_USER_ENDPOINTS+1; i++)
                {
                    memcpypgm2ram( memBlock, (void *)Config_Simple_Descriptors + i * sizeof(NODE_SIMPLE_DESCRIPTOR), sizeof(NODE_SIMPLE_DESCRIPTOR) );
                    NVMWrite( configSimpleDescriptors + (WORD)i * (WORD)sizeof(NODE_SIMPLE_DESCRIPTOR), memBlock, sizeof(NODE_SIMPLE_DESCRIPTOR) );
                }
                SRAMfree( memBlock );
            }
        }

        #ifdef ENABLE_DEBUG
            ConsolePutROMString((ROM char * const)"NVMInit complete.\r\n");
        #endif

        CLRWDT();

        return result;
    }

#else

    BOOL NVMInit( void )
    {
        return 0;
    }

#endif

/*********************************************************************
 * Function:        void NVMRead( BYTE *dest, WORD  src, BYTE count )
 *
 * PreCondition:    None
 *
 * Input:           *dest - pointer to the location in RAM where the first
 *                      source byte is to be stored
 *                  src - index to the location in EEPROM of the first
 *                      byte to be read
 *                  count - the number of bytes to be read
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine reads the specified number of bytes
 *                  from an external EEPROM and stores it into RAM.
 *
 * Note:            This routine cannot be called while the transceiver
 *                  is being accessed, and vice versa.
 *
 *                  BE SURE THE nCS LINE IS INITIALIZED IN THE
 *                  APPLICATION CODE.
 ********************************************************************/
#ifdef USE_EXTERNAL_NVM

    void NVMRead (BYTE *dest, WORD  src, BYTE count)
    {
        #if !defined(__C30__)
        BYTE oldGIEH;
        #endif
        
        #ifdef ENABLE_DEBUG
            ConsolePut('r');
            PrintChar( (BYTE)(((WORD)src>>8)&0xFF) );
            PrintChar( (BYTE)((WORD)src&0xFF) );
            ConsolePut('-');
            PrintChar( count );
        #endif
        #if !defined(__C30__)
            oldGIEH = 0;
            if ( INTCONbits.GIEH )
            {
                oldGIEH = 1;
            }
            INTCONbits.GIEH = 0;
        #endif

        SPISelectEEPROM();
        SPIPut( SPIREAD );
        SPIPut( (BYTE)(((WORD)src>>8) & 0xFF) );
        SPIPut( (BYTE)((WORD)src & 0xFF) );
        while( count )
        {
            *dest = SPIGet();
            #ifdef ENABLE_DEBUG
            #endif
            dest++;
            count--;
        }
        SPIUnselectEEPROM();

        #if !defined(__C30__)
            if (oldGIEH)
            {
                INTCONbits.GIEH = 1;
            }
        #endif

        #ifdef ENABLE_DEBUG
            ConsolePutROMString((ROM char * const)"\r\n");
        #endif
    }
#else
    #define NVMRead(dest, src, count)   memcpypgm2ram(dest, src, count)
#endif


/*********************************************************************
 * Function:        EXTERNAL EEPROM
 *                      BYTE ComparePage( WORD dest,  BYTE *src, WORD count )
 *
 * PreCondition:    None
 *
 * Input:           dest - index to the location in EEPROM where the first
 *                      source byte is to be stored
 *                  *src - pointer to the location in RAM of the first
 *                      byte to be written to ROM
 *                  count - the number of bytes to be checked
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine determines if the bytes in external
 *                  EEPROM in the current page already match the value to write.
 *
 * Note:            Some devices have a limited number of write cycles.
 *                  When using these devices, ensure that the number
 *                  of write cycles over the lifetime of the application
 *                  will not exceed the specification.  It is also
 *                  recommended that the CHECK_BEFORE_WRITE option is
 *                  enabled to further reduce the number of write cycles.
 ********************************************************************/

#if defined( USE_EXTERNAL_NVM ) && (defined( VERIFY_WRITE ) || defined( CHECK_BEFORE_WRITE ))

    BYTE ComparePage( WORD dest,  BYTE *src, WORD count )
    {
        BYTE    oneByte;
        BYTE    result;

        result = 0;
        do
        {
            NVMRead( &oneByte, dest, 1 );
            #ifdef ENABLE_DEBUG
                ConsolePutROMString((ROM char * const)"Want ");
                PrintChar(*src);
                ConsolePutROMString((ROM char * const)"Got ");
                PrintChar(oneByte);
                ConsolePut( ' ' );
            #endif
            if (oneByte != *src)
            {
                #ifdef PRINT_MULTIPLE_WRITE_ATTEMPTS
                    if (flag)
                    {
                        ConsolePut('(');
                        PrintChar( (BYTE)(((WORD)dest>>8)&0xFF) );
                        PrintChar( (BYTE)((WORD)dest&0xFF) );
                        ConsolePut('-');
                        PrintChar( *src );
                        ConsolePut('-');
                        PrintChar( oneByte );
                        ConsolePut(')');
                    }
                #endif
                return 1;   // Mismatch
            }
            src++;
            dest++;
            count--;
        }
        while (count && (dest & (EEPROM_PAGE_SIZE-1)));

        return 0;   // Match
    }

#endif

/*********************************************************************
 * Function:        EXTERNAL EEPROM
 *                      void NVMWrite( WORD dest, BYTE *src, BYTE count )
 *
 * PreCondition:    None
 *
 * Input:           dest - index to the location in EEPROM where the first
 *                      source byte is to be stored
 *                  *src - pointer to the location in RAM of the first
 *                      byte to be written to ROM
 *                  count - the number of bytes to be written
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine writes the specified number of bytes
 *                  into program memory.
 *
 * Note:            Some devices have a limited number of write cycles.
 *                  When using these devices, ensure that the number
 *                  of write cycles over the lifetime of the application
 *                  will not exceed the specification.  It is also
 *                  recommended that the CHECK_BEFORE_WRITE option is
 *                  enabled to further reduce the number of write cycles.
 *
 *                  BE SURE THE nCS LINE IS INITIALIZED IN THE
 *                  APPLICATION CODE.
 ********************************************************************/

#ifdef USE_EXTERNAL_NVM

    void NVMWrite( WORD dest, BYTE *src, BYTE count )
    {
        BYTE    bytesOnPage;
        BYTE    bytesOnPageCounter;
    	#if !defined(__C30__)
        	BYTE    oldGIEH;
    	#endif
        BYTE    *srcCounter;
        BYTE    status;
        WORD    writeStart;

        if (!count)
        {
            return;
        }

        #if !defined(__C30__)
            oldGIEH = 0;
            if ( INTCONbits.GIEH )
            {
                oldGIEH = 1;
            }
            INTCONbits.GIEH = 0;
        #endif


        // Make sure the chip is unlocked.
        SPISelectEEPROM();                  // Enable chip select
        SPIPut( SPIWRSR );                  // Send WRSR - Write Status Register opcode
        SPIPut( 0x00 );                     // Write the status register
        SPIUnselectEEPROM();                // Disable Chip Select

        #ifdef ENABLE_DEBUG
            ConsolePut('w');
            PrintChar( (BYTE)(((WORD)dest>>8)&0xFF) );
            PrintChar( (BYTE)((WORD)dest&0xFF) );
            ConsolePut('-');
            PrintChar( count );
        #endif

        writeStart = dest;
        while (count)
        {
            bytesOnPage = EEPROM_PAGE_SIZE - (writeStart & (EEPROM_PAGE_SIZE-1));
            if (bytesOnPage > count)
            {
                bytesOnPage = count;
            }

            #ifdef PRINT_MULTIPLE_WRITE_ATTEMPTS
                flag = 0;
            #endif
            CLRWDT();

            #if defined(VERIFY_WRITE)
                while (ComparePage( writeStart, src, bytesOnPage ))
            #elif defined(CHECK_BEFORE_WRITE)
                if (ComparePage( writeStart, src, bytesOnPage ))
            #endif
            {
                #ifdef PRINT_MULTIPLE_WRITE_ATTEMPTS
                    flag = 1;
                #endif
                SPISelectEEPROM();                          // Enable chip select
                SPIPut( SPIWREN );                          // Transmit the write enable instruction
                SPIUnselectEEPROM();                        // Disable Chip Select to enable Write Enable Latch

                SPISelectEEPROM();                          // Enable chip select
                SPIPut( SPIWRITE );                         // Transmit write instruction
                SPIPut( (BYTE)((writeStart>>8) & 0xFF) );   // Transmit address high byte
                SPIPut( (BYTE)((writeStart) & 0xFF) );      // Trabsmit address low byte

                // Loop until the required number of bytes have been written or
                // until the maximum number of bytes per write cycle have been written
                bytesOnPageCounter = bytesOnPage;
                srcCounter = src;
                do
                {
                    SPIPut (*srcCounter++);                    // Write the source byte
                    bytesOnPageCounter--;
                }
                while (bytesOnPageCounter);

                // Disable chip select to start write cycle.  We'll let it finish in the background if we can.
                SPIUnselectEEPROM();

                // Wait for the write to complete.  We have to wait here, because we can't
                // do a read of the memory while the write is in progress.
                do
                {
                    SPISelectEEPROM();                  // Enable chip select
                    SPIPut( SPIRDSR );                  // Send RDSR - Read Status Register opcode
                    status = SPIGet();;                 // Read the status register
                    SPIUnselectEEPROM();                // Disable Chip Select
                }
                while (status & WIP_MASK);
            }

            count -= bytesOnPage;
            writeStart += bytesOnPage;
            src = &src[bytesOnPage];
        }

        #if !defined(__C30__)
            if (oldGIEH)        // If interrupts were enabled before this function
            {
                INTCONbits.GIEH = 1;       // re-enable them
            }
        #endif

        #ifdef ENABLE_DEBUG
            ConsolePut('.');
            ConsolePutROMString((ROM char * const)"\r\n");
        #endif
    }

#endif

/*********************************************************************
 * Function:        PROGRAM MEMORY
 *                      void NVMWrite( NVM_ADDR *dest, BYTE *src, BYTE count )
 *
 * PreCondition:    None
 *
 * Input:           *dest - pointer to the location in ROM where the first
 *                      source byte is to be stored
 *                  *src - pointer to the location in RAM of the first
 *                      byte to be written to ROM
 *                  count - the number of bytes to be written
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine writes the specified number of bytes
 *                  into program memory.
 *
 * Note:            Some devices have a limited number of write cycles.
 *                  When using these devices, ensure that the number
 *                  of write cycles over the lifetime of the application
 *                  will not exceed the specification.  It is also
 *                  recommended that the CHECK_BEFORE_WRITE option is
 *                  enabled to further reduce the number of write cycles.
 ********************************************************************/

#ifndef USE_EXTERNAL_NVM

    void NVMWrite( NVM_ADDR *dest, BYTE *src, BYTE count )
    {
    NVM_ADDR *pEraseBlock;
    BYTE    *memBlock;
    BYTE *pMemBlock;
    BYTE writeIndex;
    BYTE writeStart;
    BYTE writeCount;
    BYTE oldGIEH;
    DWORD oldTBLPTR;

    #if defined(VERIFY_WRITE)
        while (memcmppgm2ram( src, (ROM void *)dest, count ))
    #elif defined(CHECK_BEFORE_WRITE)
        if (memcmppgm2ram( src, (ROM void *)dest, count ))
    #endif
        {
            if ((memBlock = SRAMalloc( ERASE_BLOCK_SIZE )) == NULL)
                return;

            #if 0
                ConsolePutROMString( (ROM char * const)"NVMWrite at " );
                PrintChar( (BYTE)(((WORD)dest>>8)&0xFF) );
                PrintChar( (BYTE)((WORD)dest&0xFF) );
                ConsolePutROMString( (ROM char * const)" count " );
                PrintChar( count );
                ConsolePutROMString( (ROM char * const)"\r\n" );
            #endif

            // First, get the nearest "left" erase block boundary
            pEraseBlock = (NVM_ADDR*)((long)dest & (long)(~(ERASE_BLOCK_SIZE-1)));
            writeStart = (BYTE)((BYTE)dest & (BYTE)(ERASE_BLOCK_SIZE-1));

            while( count )
            {
                // Now read the entire erase block size into RAM.
                NVMRead(memBlock, (ROM void*)pEraseBlock, ERASE_BLOCK_SIZE);

                // Erase the block.
                // Erase flash memory, enable write control.
                EECON1 = 0x94;

                oldGIEH = 0;
                if ( INTCONbits.GIEH )
                    oldGIEH = 1;
                INTCONbits.GIEH = 0;

                #if defined(MCHP_C18)
                    TBLPTR = (unsigned short long)pEraseBlock;
                #elif defined(HITECH_C18)
                    TBLPTR = (void*)pEraseBlock;
                #endif

                CLRWDT();

                EECON2 = 0x55;
                EECON2 = 0xaa;
                EECON1bits.WR = 1;
                NOP();

                EECON1bits.WREN = 0;

                oldTBLPTR = TBLPTR;

                if ( oldGIEH )
                    INTCONbits.GIEH = 1;

                // Modify 64-byte block of RAM buffer as per what is required.
                pMemBlock = &memBlock[writeStart];
                while( writeStart < ERASE_BLOCK_SIZE && count )
                {
                    *pMemBlock++ = *src++;

                    count--;
                    writeStart++;
                }

                // After first block write, next start would start from 0.
                writeStart = 0;

                // Now write entire 64 byte block in one write block at a time.
                writeIndex = ERASE_BLOCK_SIZE / WRITE_BLOCK_SIZE;
                pMemBlock = memBlock;
                while( writeIndex )
                {

                    oldGIEH = 0;
                    if ( INTCONbits.GIEH )
                        oldGIEH = 1;
                    INTCONbits.GIEH = 0;

                    TBLPTR = oldTBLPTR;

                    // Load individual block
                    writeCount = WRITE_BLOCK_SIZE;
                    while( writeCount-- )
                    {
                        TABLAT = *pMemBlock++;

                        TBLWTPOSTINC();
                    }

                    // Start the write process: reposition tblptr back into memory block that we want to write to.
                    #if defined(MCHP_C18)
                        _asm tblrdpostdec _endasm
                    #elif defined(HITECH_C18)
                        asm(" tblrd*-");
                    #endif

                    // Write flash memory, enable write control.
                    EECON1 = 0x84;

                    CLRWDT();

                    EECON2 = 0x55;
                    EECON2 = 0xaa;
                    EECON1bits.WR = 1;
                    NOP();
                    EECON1bits.WREN = 0;

                    // One less block to write
                    writeIndex--;

                    TBLPTR++;

                    oldTBLPTR = TBLPTR;

                    if ( oldGIEH )
                        INTCONbits.GIEH = 1;
                }

                // Go back and do it all over again until we write all
                // data bytes - this time the next block.
        #if !defined(WIN32)
                pEraseBlock += ERASE_BLOCK_SIZE;
        #endif
            }

            SRAMfree( memBlock );
        }
    }

#endif


/*********************************************************************
 * Function:        EXTERNAL EEPROM
 *                      void ClearNVM( WORD dest, WORD count )
 *
 * PreCondition:    None
 *
 * Input:           dest - index to the location in ROM where to
 *                      start clearing memory
 *                  count - the number of bytes to be cleared
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine clears the specified number of bytes
 *                  of program memory.
 *
 * Note:            Some devices have a limited number of write cycles.
 *                  When using these devices, ensure that the number
 *                  of write cycles over the lifetime of the application
 *                  will not exceed the specification.  It is also
 *                  recommended that the CHECK_BEFORE_WRITE option is
 *                  enabled to further reduce the number of write cycles.
 *
 *                  This function is not very efficient.  If this function
 *                  is called in time-critical code, it should be
 *                  reworked to clear the maximim number of bytes possible.
 ********************************************************************/
#ifdef USE_EXTERNAL_NVM

    void ClearNVM( WORD dest, WORD count )
    {
        BYTE    dummy = 0;
        WORD    i;
        BYTE    oldByte;

        #ifdef ENABLE_DEBUG
            ConsolePut('c');
        #endif

        for (i=0; i<count; i++)
        {

    #if defined(VERIFY_WRITE)
            NVMRead( &oldByte, dest, 1 );
            while (oldByte)
    #elif defined(CHECK_BEFORE_WRITE)
            NVMRead( &oldByte, dest, 1 );
            if (oldByte)
    #endif
            {
                NVMWrite( dest, &dummy, 1 );
    #if defined(VERIFY_WRITE)
                NVMRead( &oldByte, dest, 1 );
    #endif
            }
            dest++;
            CLRWDT();
        }
    }

#endif


/*********************************************************************
 * Function:        PROGRAM MEMORY
 *                      void ClearNVM( NVM_ADDR *dest, WORD count )
 *
 * PreCondition:    None
 *
 * Input:           *dest - pointer to the location in ROM where to
 *                      start clearing memory
 *                  count - the number of bytes to be cleared
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine clears the specified number of bytes
 *                  of program memory.
 *
 * Note:            Some devices have a limited number of write cycles.
 *                  When using these devices, ensure that the number
 *                  of write cycles over the lifetime of the application
 *                  will not exceed the specification.  It is also
 *                  recommended that the CHECK_BEFORE_WRITE option is
 *                  enabled to further reduce the number of write cycles.
 *
 *                  This function is not very efficient.  If this function
 *                  is called in time-critical code, it should be
 *                  reworked to clear the maximim number of bytes possible.
 ********************************************************************/
#ifndef USE_EXTERNAL_NVM

    void ClearNVM( NVM_ADDR *dest, WORD count )
    {
        BYTE    dummy = 0;
        WORD    i;

        for (i=0; i<count; i++)
        {
    #if defined(VERIFY_WRITE)
            while (memcmppgm2ram( &dummy, (rom void *)dest, 1 ))
    #elif defined(CHECK_BEFORE_WRITE)
            if (memcmppgm2ram( &dummy, (rom void *)dest, 1 ))
    #endif
            {
                NVMWrite( dest, &dummy, 1 );
            }
            dest++;
            CLRWDT();
        }
    }

#endif

/*********************************************************************
 * Function:        BYTE NVMalloc( WORD size, WORD *location )
 *
 * PreCondition:    None
 *
 * Input:           size - the number of bytes required
 *
 * Output:          *location - location of allocated space
 *                  return code:    0 - success
 *                                  1 - not enough space available
 *
 * Side Effects:    None
 *
 * Overview:        This routine returns the index of a chunk of nonvolatile
 *                  memory of the required size.
 *
 * Note:            This routine needs to be called for each section
 *                  of nonvolatile memory before that memory is accessed.
 ********************************************************************/
#ifdef USE_EXTERNAL_NVM

    BYTE NVMalloc( WORD size, WORD *location )
    {
        static WORD nextEEPosition = 0;
        if ((nextEEPosition + size) > EXTERNAL_NVM_BYTES)
        {
            return 1;
        }

        *location = nextEEPosition;
        nextEEPosition += size;
        #ifdef ENABLE_DEBUG
            ConsolePut('(');
            PrintChar((unsigned int)nextEEPosition >> 8);
            PrintChar((unsigned int)nextEEPosition & 0xFF);
            ConsolePut(')');
        #endif
        return 0;
    }

#endif
