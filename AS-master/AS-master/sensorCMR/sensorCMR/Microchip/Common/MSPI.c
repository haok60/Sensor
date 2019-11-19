/*********************************************************************
 *
 *                  Master SPI routintes
 *
 *********************************************************************
 * FileName:        MSPI.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Complier:        MCC18 v1.00.50 or higher
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
 * Nilesh Rajbharti     7/12/04 Original
 * Nilesh Rajbharti     11/1/04 Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#include "MSPI.h"
#include "Zigbee.def"
#include "Compiler.h"

#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24H__)

	void RF_SPIPut(BYTE v)
	{
    	BYTE dummy;
    
	    RF_SSPIF_BIT = 0;
    	dummy = SPI1BUF;
	    RF_SSPBUF_REG = v;
        while(RF_SSPIF_BIT == 0) {}
	}

	BYTE RF_SPIGet(void)
	{
    	RF_SPIPut(0x00);
	    return RF_SSPBUF_REG; 
	}

	void EE_SPIPut(BYTE v)
	{
		BYTE dummy;
		EE_SSPIF_BIT = 0;
		dummy = EE_SSPBUF_REG; 
		EE_SSPBUF_REG = v;
		while(EE_SSPIF_BIT == 0 );
	}

	BYTE EE_SPIGet(void)
	{
		EE_SPIPut(0x00);
		return EE_SSPBUF_REG; 
	}


#elif defined(__18CXX)


	#if defined(USE_EXTERNAL_NVM) && !defined(EE_AND_RF_SHARE_SPI)

    	void RF_SPIPut(BYTE v)
	    {
    	    RF_SSPIF_BIT = 0;

        	do
        	{
            	RF_WCOL_BIT = 0;
	            RF_SSPBUF_REG = v;
    	    }
        	while (RF_WCOL_BIT);
	        while(RF_SSPIF_BIT == 0){}
    	}

	    BYTE RF_SPIGet(void)
    	{
        	RF_SPIPut(0x00);
	        return RF_SSPBUF_REG;
    	}


	    void EE_SPIPut(BYTE v)
    	{
        	EE_SSPIF_BIT = 0;

	        do
    	    {
        	    EE_WCOL_BIT = 0;
            	EE_SSPBUF_REG = v;
	        }
    	    while (EE_WCOL_BIT);
        	while(EE_SSPIF_BIT == 0){}
	    }

    	BYTE EE_SPIGet(void)
	    {
    	    EE_SPIPut(0x00);
        	return EE_SSPBUF_REG;
	    }

	#else


        void SPIPut(BYTE v)
        {
            SSPIF_BIT = 0;
    
            do
            {
                WCOL_BIT = 0;
                SSPBUF_REG = v;
            }
            while (WCOL_BIT);
            while(SSPIF_BIT == 0){}
        }
    
        BYTE SPIGet(void)
        {
            SPIPut(0x00);
            return SSPBUF_REG;
        }

		// The ZMD44101 requires SS to be driven high between reading bytes,
		// so these functions are not used and should not be compiled.
		#if !defined(USE_ZMD44101)
			void SPIGetArray(BYTE *buffer, BYTE len)
			{
			    while( len-- )
        			*buffer++ = SPIGet();
			}

			void SPIPutArray(BYTE *buffer, BYTE len)
			{
		    	while( len-- )
        			SPIPut(*buffer++);
			}
		#endif
	#endif

#else
    #error Unknown processor.  See Compiler.h
#endif

