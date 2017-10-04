/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-01-15
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
    
#include <project.h>
#include <stdio.h>

void MOD_SysLog_Init(void);
    
#define NUM_SERIALI 2
#define BUFSIZE 20
#define UART0 0

    
enum{
	PARSE_SOF = 0,
	PARSE_LEN ,
	PARSE_CMD,
	PARSE_SUBCMD,
	PARSE_ADD,
    PARSE_CONFIGURATION,
	PARSE_CHM,
    PARSE_CLOCK_PARAM,
    PARSE_HB_PARAM,
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
    uint16 length;
	uint16 checksum;
    uint8 timeout;
};

/******************************/

extern char string[];
void sprintfWrite(void);
/******************************/
// opzione con sprintf e  sprintfWrite() per evitare la nanoliob e la printf.

#if (DEBUG_UART_ENABLED == YES)
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


    #define DBG_PRINT_ARRAY(a,len)     do\
                                        {\
                                            uint32 i;\
                                            for(i = 0u; i < (len); i++) {\
                                                sprintf(string,"%02X ", *(a+i));\
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
 void Debug_Start(void);
void UART_DEBUG_PARSER_Task( void );
 uint8 verbose_Mode; // interruttore usato per attvare/disattivare il printf
 void printLine(void);
void printStarLine(void);
/*******************************************************************************/
#endif
/* [] END OF FILE */
