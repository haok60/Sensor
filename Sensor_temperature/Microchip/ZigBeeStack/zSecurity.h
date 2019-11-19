/*********************************************************************
 *
 *                  Security Generic Header File
 *
 *********************************************************************
 * FileName:        zSecurity.h
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
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 ********************************************************************/
#if 0
#ifdef I_SUPPORT_SECURITY
#ifndef _ZSECURITY_H
#define _ZSECURITY_H

#include "ZigbeeTasks.h"
#include "Zigbee.def"
#include "zPHY.h"
#include "zMAC.h"
#include "MSPI.h"
#include "generic.h"
#include "sralloc.h"
#include "zAPS.h"


typedef struct _SECURITY_STATUS
{
	union _SECURITY_STATUS_FLAGS
	{
		BYTE	Val;
		struct _SECURITY_STATUS_bits
		{
			BYTE	bAuthorization 		: 1;
			BYTE	nwkSecureAllFrames	: 1;
			BYTE	nwkAllFresh			: 1;
		}bits;
	} flags;
} SECURITY_STATUS;

typedef enum _SECURITY_ERROR
{
	NO_ERROR = 0x00,
	ERR_INVALID_PARAMETER,
	NO_MASTER_KEY,
	INVALID_CHALLENGE,
	INVALID_SKG,
	INVALID_MAC,
	INVLAID_KEY,
	TIMEOUT,
	BAD_FRAME
} SECURITY_ERROR;



typedef enum _SECURITY_LEVEL
{
	None = 0x00,
	MIC_32,
	MIC_64,
	MIC_128,
	ENC,
	ENC_MIC_32,
	ENC_MIC_64,
	ENC_MIC_128
} SECRUITY_LEVEL;

typedef enum _KEY_IDENTIFIER
{
	ID_LinkKey = 0x00,
	ID_NetworkKey,
	ID_KeyTransportKey,
	ID_KeyLoadKey
} KEY_IDENTIFIER;


typedef enum _LAYER_INDEX
{
	LAYER_MAC,
	LAYER_NETWORK,
	LAYER_APL
} LAYER_INDEX;


typedef union _SECURITY_CONTROL
{
	BYTE Val;
	struct _SC_bits
	{
		BYTE SecurityLevel : 3;
		BYTE KeyIdentifier : 2;
		BYTE ExtendedNonce : 1;
		BYTE filler : 2;
	} bits;
} SECURITY_CONTROL;


#if (RF_CHIP == MRF24J40)
    #include "zSecurity_MRF24J40.h"
#endif

#endif
#endif
#endif

