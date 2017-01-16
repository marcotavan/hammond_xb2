/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef _COMMON_H_
#define _COMMON_H_
    
#define FALSE                   (0u)
#define TRUE                    (1u)
    
#ifndef BIT0    
	#define BIT0    1
	#define BIT1    2
	#define BIT2    4
	#define BIT3    8
	#define BIT4    16
	#define BIT5    32
	#define BIT6    64
	#define BIT7    128
	#define BIT8    256
	#define BIT9    512
	#define BIT10   1024
	#define BIT11   2048
	#define BIT12   4096
	#define BIT13   8192
	#define BIT14   16384
	#define BIT15   32768
#endif


#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#endif

/* [] END OF FILE */
