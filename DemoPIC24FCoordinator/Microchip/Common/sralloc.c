/*****************************************************************************
 *
 *              Simple SRAM Dynamic Memory Allocation
 *
 *****************************************************************************
 * FileName:        sralloc.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Compiler:        C18 02.20.00 or higher
 *                  C30 2.05 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         Microchip Technology Incorporated
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
 * This is a simple dynamic memory allocation module. The following are the
 * supported services:
 *
 * unsigned char * NEAR SRAMalloc(NEAR unsigned char nBytes)
 * void SRAMfree(unsigned char * NEAR pSRAM)
 * void SRAMInitHeap(void)
 *
 * This version of the dynamic memory allocation limits the segment size
 * to 126 bytes. This is specifically designed such to enable better
 * performance by limiting pointer manipulation.
 *
 *
 * How it works:
 * The model is based on a simple form of a linked list. A block of memory
 * refered to as the dynamic heap is split into segments. Each segment
 * has a single byte header that references the next segment in the list
 * as well as indicating whether the segment is allocated. Consiquently
 * the reference implicitly identifies the length of the segment.
 *
 * This method also enables the possibility of allowing a large number
 * of memory allocations. The maximum is limited by the defined heap size.
 *
 * SRAMalloc() is used to split or merge segments to be allocated.
 * SRAMfree() is used to release segments.
 *
 * Example:
 *  ----------
 *  |  0x7F  |  0x200   Header Seg1
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |  0x89  |  0x27F   Header Seg2 (allocated)
 *  |        |
 *  |        |
 *  |  0x77  |  0x288   Header Seg3
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |  0x00  |  0x2FF   Tail
 *  ----------
 *
 *
 *  Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0
 *
 *  Alloc   ------------- reference to next Header --------------
 *
 *
 * Recomendations:
 * Although this model will allow dynamic allocation down to a single byte,
 * doing so sacrifices performance. With more segments within the heap, more
 * time is required to attempt to allocate memory. Plus every segment requires
 * a header byte; therefore, smaller segments require more memory. There is
 * also the possibility of fragmentation, which could ultimately doom an
 * application by reducing the largest allocatable block of memory. Thus the
 * recomendation is to allocate at least 8 bytes of memory.
 *
 *
 *
 * Author               Date        Version     Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Ross Fosler          05/25/03    v1.03       ... First release
 * Nilesh Rajbharti     7/14/04                 Modified for Zigbee stack.
 * Nilesh Rajbharti     11/1/04                 Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY             11/27/06 Microchip ZigBee Stack v1.0-3.7
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 *****************************************************************************/
#include "zigbee.def"
#include "Compiler.h"
#include "Generic.h"
#include "Console.h"

/********************************************************************
Compilation options
********************************************************************/
// Enable this switch to output debug messages
//#define ENABLE_DEBUG

// Enabled this switch if 16-bit math is going to be performed during
// an interrupt, and the math temporary variables are not being saved
// on the stack.  If in doubt, enable this switch.
#if !defined(__C30__)
	#define MAKE_INTERRUPT_SAFE
#endif

/********************************************************************
Constants
********************************************************************/
#define NEAR

#define _MAX_SEGMENT_SIZE   127
#define _MAX_HEAP_SIZE  MAX_HEAP_SIZE-1


/*********************************************************************
 * Segment header data type
 ********************************************************************/
typedef union _SALLOC
{
    unsigned char byte;
    struct _BITS
    {
        unsigned count:7;
        unsigned alloc:1;
    }bits;
}SALLOC;


/*********************************************************************
 * Reserve the memory heap
 ********************************************************************/
#if defined(MCHP_C18)
    // NOTE - The starting location here must align with the linker script.
    #pragma udata MultiBankHeap=HEAP_LOCATION
    unsigned char _uDynamicHeap[MAX_HEAP_SIZE];
    #pragma udata
#elif defined(HITECH_C18)
    #pragma psect bss=MulitBankHeap
    unsigned char _uDynamicHeap[MAX_HEAP_SIZE];
#endif

/*********************************************************************
 * Private function declarations
 ********************************************************************/
BOOL _SRAMmerge(SALLOC * NEAR pSegA);


/*********************************************************************
 * Function:        unsigned char * SRAMalloc(unsigned char length)
 *
 * PreCondition:    A memory block must be allocated in the linker,
 *                  and the memory headers and tail must already be
 *                  set via the function SRAMInitHeap().
 *
 * Input:           unsigned char nBytes - Number of bytes to allocate.
 *
 * Output:          unsigned char * - A pointer to the requested block
 *                  of memory.
 *
 * Side Effects:
 *
 * Overview:        This functions allocates a chunk of memory from
 *                  the heap. The maximum segment size for this
 *                  version is 126 bytes. If the heap does not have
 *                  an available segment of sufficient size it will
 *                  attempt to create a segment; otherwise a NULL
 *                  pointer is returned. If allocation is succeessful
 *                  then a pointer to the requested block is returned.
 *
 * Note:            The calling function must maintain the pointer
 *                  to correctly free memory at runtime.
 ********************************************************************/
#if defined(__C30__)
	#include <stdlib.h>
#endif

#undef MAKE_INTERRUPT_SAFE

unsigned char * NEAR SRAMalloc(NEAR unsigned char nBytes)
{
	#if defined(__C30__)
		return (unsigned char *)malloc((size_t)nBytes);
	#else
	
    	SALLOC * NEAR       pHeap;
		#ifdef MAKE_INTERRUPT_SAFE
		    BYTE                saveGIEH;
		#endif
	    NEAR SALLOC         segHeader;
    	NEAR unsigned char  segLen;
	    SALLOC * NEAR       temp;


    	#ifdef MAKE_INTERRUPT_SAFE
        	saveGIEH = 0;
	        if (INTCONbits.GIEH)
    	        saveGIEH = 1;
        	INTCONbits.GIEH = 0;
	    #endif


    	#ifdef ENABLE_DEBUG
        	ConsolePutROMString( (ROM char * const)"\r\nAlloc " );
	        PrintChar(nBytes);
    	#endif

	    // Do not allow allocation above the max minus one bytes.  Also, do
    	// not allow a zero length packet.  Could cause problems.
	    if ((nBytes > (_MAX_SEGMENT_SIZE - 1))  ||
    	    (nBytes == 0))
    	{
        	#ifdef MAKE_INTERRUPT_SAFE
            	INTCONbits.GIEH = saveGIEH;
	        #endif
    	    return (0);
	    }

    	// Init the pointer to the heap
	    pHeap = (SALLOC *)_uDynamicHeap;

    	while (1)
	    {
    	    #ifdef ENABLE_DEBUG
        	    ConsolePutROMString( (ROM char * const)" check " );
            	PrintChar( (BYTE)((WORD)pHeap>>8) );
	            PrintChar( (BYTE)((WORD)pHeap&0xff) );
    	    #endif

        	// Get the header of the segment
	        segHeader = *pHeap;

    	    // Extract the segment length from the segment
        	segLen = segHeader.bits.count - 1;

	        // A null segment indicates the end of the table
    	    if (segHeader.byte == 0)
        	{
            	#ifdef ENABLE_DEBUG
                	ConsolePutROMString( (ROM char * const)" TableEnd\r\n" );
	            #endif
    	        #ifdef MAKE_INTERRUPT_SAFE
        	        INTCONbits.GIEH = saveGIEH;
            	#endif
	            return (0);
    	    }

        	// If this segment is not allocated then attempt to allocate it
	        if (!(segHeader.bits.alloc))
    	    {
        	    // If the free segment is too small then attempt to merge
            	if (nBytes > segLen)
	            {
    	            #ifdef ENABLE_DEBUG
        	            ConsolePutROMString( (ROM char * const)" merge" );
            	    #endif
                	// If the merge fails them move on to the next segment
	                if (!(_SRAMmerge(pHeap)))
    	            {
        	            pHeap +=(WORD)(segHeader.bits.count);
            	    }
                	// Fall through so we can check the newly created segment.
	            }
    	        else

        	    // If the segment length matches the request then allocate the
            	// header and return the pointer
	            if (nBytes == segLen)
    	        {
        	        // Allocate the segment
            	    (*pHeap).bits.alloc = 1;

                	// Return the pointer to the caller
	                #ifdef MAKE_INTERRUPT_SAFE
    	                INTCONbits.GIEH = saveGIEH;
        	        #endif
            	    #ifdef ENABLE_DEBUG
                	    ConsolePutROMString( (ROM char * const)" Found\r\n" );
	                #endif
    	            return ((unsigned char *)(pHeap + 1));
        	    }

            	// Else create a new segment
	            else
    	        {
        	        // Reset the header to point to a new segment
            	    (*pHeap).byte = nBytes + 0x81;

                	// Remember the pointer to the first segment
	                temp = pHeap + 1;

    	            // Point to the new segment
        	        pHeap += (WORD)(nBytes + 1);

            	    // Insert the header for the new segment
                	(*pHeap).byte = segLen - nBytes;

	                // Return the pointer to the user
    	            #ifdef MAKE_INTERRUPT_SAFE
        	            INTCONbits.GIEH = saveGIEH;
            	    #endif
                	#ifdef ENABLE_DEBUG
                    	ConsolePutROMString( (ROM char * const)" Found\r\n" );
	                #endif
    	            return ((unsigned char *) temp);
        	    }
	        }

    	    // else set the pointer to the next segment header in the heap
        	else
	        {
    	        pHeap += (WORD)segHeader.bits.count;
        	}
	    }

    	#ifdef MAKE_INTERRUPT_SAFE
        	INTCONbits.GIEH = saveGIEH;
	    #endif
    	#ifdef ENABLE_DEBUG
        	ConsolePutROMString( (ROM char * const)" ???\r\n" );
	    #endif
    	return (0);
	#endif    
}

/*********************************************************************
 * Function:        void SRAMfree(unsigned char * pSRAM)
 *
 * PreCondition:    The pointer must have been returned from a
 *                  previously allocation via SRAMalloc().
 *
 * Input:           unsigned char * pSRAM - pointer to the allocated
 *
 * Output:          void
 *
 * Side Effects:
 *
 * Overview:        This function de-allocates a previously allocated
 *                  segment of memory.
 *
 * Note:            The pointer must be a valid pointer returned from
 *                  SRAMalloc(); otherwise, the segment may not be
 *                  successfully de-allocated, and the heap may be
 *                  corrupted.
 ********************************************************************/
void SRAMfree(unsigned char * NEAR pSRAM)
{
	#if defined(__C30__)
		return free((void *)pSRAM);
	#endif
	
    // Release the segment
    (*(SALLOC *)(pSRAM - 1)).bits.alloc = 0;
    #ifdef ENABLE_DEBUG
        ConsolePutROMString( (ROM char * const)"\r\nFree " );
        PrintChar((*(SALLOC *)(pSRAM - 1)).byte - 1);
        ConsolePutROMString( (ROM char * const)" at " );
        PrintChar( (BYTE)((WORD)(pSRAM - 1)>>8) );
        PrintChar( (BYTE)((WORD)(pSRAM - 1)&0xff) );
    #endif
}

/*********************************************************************
 * Function:        void SRAMInitHeap(void)
 *
 * PreCondition:
 *
 * Input:           void
 *
 * Output:          void
 *
 * Side Effects:
 *
 * Overview:        This function initializes the dynamic heap. It
 *                  inserts segment headers to maximize segment space.
 *
 * Note:            This function must be called at least one time.
 *                  And it could be called more times to reset the
 *                  heap.
 ********************************************************************/
void SRAMInitHeap(void)
{
#if !defined(__C30__)	
    unsigned char * NEAR pHeap;
    NEAR unsigned int count;

    pHeap = _uDynamicHeap;
    count = _MAX_HEAP_SIZE;

    while (1)
    {
        if (count > _MAX_SEGMENT_SIZE)
        {
            *pHeap = _MAX_SEGMENT_SIZE;
            pHeap += _MAX_SEGMENT_SIZE;
            count = count - _MAX_SEGMENT_SIZE;
        }
        else
        {
            *pHeap = count;
            *(pHeap + count) = 0;
            return;
        }
    }
#endif
}

/*********************************************************************
 * Function:        unsigned char _SRAMmerge(SALLOC * NEAR pSegA)
 *
 * PreCondition:
 *
 * Input:           SALLOC * NEAR pSegA - pointer to the first segment.
 *
 * Output:          usnigned char - returns the length of the
 *                  merged segment or zero if failed to merge.
 *
 * Side Effects:
 *
 * Overview:        This function tries to merge adjacent segments
 *                  that have not been allocated. The largest possible
 *                  segment is merged if possible.
 *
 * Note:
 ********************************************************************/
 #if !defined(__C30__)
BOOL _SRAMmerge(SALLOC * NEAR pSegA)
{
    SALLOC * NEAR pSegB;
    NEAR SALLOC uSegA, uSegB, uSum;

    // Init the pointer to the heap
    pSegB = pSegA + (WORD)(*pSegA).bits.count;

    // Extract the headers for faster processing
    uSegA = *pSegA;
    uSegB = *pSegB;

    // Quit if the tail has been found
    if (uSegB.byte == 0)
    {
        return (FALSE);
    }

    // If either segment is allocated then do not merge
    if (uSegA.bits.alloc || uSegB.bits.alloc)
    {
        return (FALSE);
    }

    // If the first segment is max then nothing to merge
    if (uSegA.bits.count == _MAX_SEGMENT_SIZE)
    {
        return (FALSE);
    }

    // Get the sum of the two segments
    uSum.byte = uSegA.byte + uSegB.byte;


    // If the sum of the two segments are > than the largest segment
    // then create a new segment equal to the max segment size and
    // point to the next segments
    if ((uSum.byte) > _MAX_SEGMENT_SIZE)
    {
        (*pSegA).byte = _MAX_SEGMENT_SIZE;
        pSegA += _MAX_SEGMENT_SIZE; //(*pSeg1).byte;
//        pSegB += uSegB.byte; //(*pSeg2).byte ;
//        (*pSegA).byte = pSegB - pSegA;
        (*pSegA).byte = uSum.byte - _MAX_SEGMENT_SIZE;

        return (TRUE);
    }
    // Else combine the two segments into one segment and
    // do not adjust the pointers to the next segment
    else
    {
        (*pSegA).byte = uSum.byte;
        return (TRUE);
    }
}
#endif
