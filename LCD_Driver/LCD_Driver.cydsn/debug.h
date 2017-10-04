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

#ifndef _DEBUG_H_
#define _DEBUG_H_
    
#include "project.h"	
#include <stdio.h>

#define TRUE 	1
#define FALSE 	0

#define NUM_SERIALI 2
#define BUFSIZE 20
#define UART0 0

#ifndef BIT0
	#define BIT0    1u
	#define BIT1    2u
	#define BIT2    4u
	#define BIT3    8u
	#define BIT4    16u
	#define BIT5    32u
	#define BIT6    64u
	#define BIT7    128u
	#define BIT8    256u
	#define BIT9    512u
	#define BIT10   1024u
	#define BIT11   2048u
	#define BIT12   4096u
	#define BIT13   8192u
	#define BIT14   16384u
	#define BIT15   32768u
#endif
	
enum{
	PARSE_SOF = 0,
	PARSE_LEN ,
	PARSE_CMD,
	PARSE_SUBCMD,
	PARSE_ADD,
    PARSE_CONFIGURATION,
	PARSE_CHM,
	PARSE_IDLE
};

    
struct _FIFO_ {
    volatile unsigned char    in;
    volatile unsigned char    out;
    unsigned char outLen;
    volatile unsigned char    rxStatus;
    volatile unsigned char    txStatus;
    unsigned char    txBuf[BUFSIZE];
    unsigned char    rxBuf[BUFSIZE];
    unsigned char c;
    uint16_t length;
	uint16_t checksum;
    uint8 timeout;
};

/******************************/
   
extern char string[];
void sprintfWrite(void);
/******************************/
// opzione con sprintf e  sprintfWrite() per evitare la nanoliob e la printf.

#if (DEBUG_UART_ENABLED == YES)
    #define DBG_PRINT_BINARY(a)         do\
                                        {\
											uint8 i; \
											sprintf(string,"0b");\
											sprintfWrite();\
											for(i=0;i<8;i++){\
												if(a&(1<<(7-i))) {\
		                                           sprintf(string,"1");\
													sprintfWrite();\
												} else { \
													sprintf(string,"0");\
													sprintfWrite();\
												}\
											}\
                                        } while (0);\
                                        
	
	#define DBG_PRINT_TEXT(a)           do\
                                        {\
                                            sprintf(string,a);\
                                        } while (0);\
                                        sprintfWrite()

    #define DBG_PRINT_DEC(a)         do\
                                        {\
                                           sprintf(string,"%02d ", a);\
                                        } while (0);\
                                        sprintfWrite()


    #define DBG_PRINT_HEX(a)         do\
                                        {\
                                           sprintf(string,"%08X ", a);\
                                        } while (0);\
                                        sprintfWrite()


    #define DBG_PRINT_ARRAY(a,len)      do\
                                        {\
											uint32 i;\
                                        	for(i = 0u; i < (len); i++) {\
                                        		sprintf(string, "%02X ", *(a+i));\
		                                        sprintfWrite();\
											}\
										} while (0);

    #define DBG_PRINTF(...)          (sprintf(string,__VA_ARGS__));\
                                        sprintfWrite()
    
    
#else
    #define DBG_PRINT_TEXT(a)
    #define DBG_PRINT_DEC(a)
    #define DBG_PRINT_HEX(a)
    #define DBG_PRINT_ARRAY(a,b)
    #define DBG_PRINTF(...)
#endif /* (DEBUG_UART_ENABLED == YES) */

/*******************************************************************************/
 struct _FIFO_ fifo[NUM_SERIALI];
 void Debug_Init(void);
 void UART0_PARSER_Task(void);
 void printLine(void);
void printStarLine(void);
/*******************************************************************************/
#endif
/* [] END OF FILE */
