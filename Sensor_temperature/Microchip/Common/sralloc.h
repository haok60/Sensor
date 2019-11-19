/*****************************************************************************
 *
 *              Simple SRAM Dynamic Memory Allocation
 *
 *****************************************************************************
 * FileName:        sralloc.c
 * Dependencies:
 * Processor:       PIC18 / PIC24 / dsPIC33
 * Compiler:        C18 02.20.00 or higher
 *                  MCC30 v2.05 or higher
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
 *
 * Nilesh Rajbharti     11/1/04 Pre-release version
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
 * DF/KO/YY				11/27/06 Microchip ZigBee Stack v1.0-3.7 
 * DF/KO/YY				01/12/07 Microchip ZigBee Stack v1.0-3.8
 * DF/KO/YY             02/26/07 Microchip ZigBee Stack v1.0-3.8.1
 *****************************************************************************/


#ifndef NULL
#define NULL    0
#endif


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
unsigned char * SRAMalloc(unsigned char nBytes);



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
void SRAMfree(unsigned char * pSRAM);

#define nfree(p) {                                   \
                    SRAMfree( (unsigned char *)p ); \
                    p = 0L;                       \
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
void SRAMInitHeap(void);
