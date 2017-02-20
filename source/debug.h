/*******************************************************************************
* File Name: Debug.h
*
* Version: 1.0
*
* Description:
*  Provides debug API.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

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
                                            \
                                            for(i = 0u; i < (len); i++)\
                                            {\
                                                sprintf(string,"%02X ", *(a+i));\
                                            }\
                                        } while (0);\
                                        sprintfWrite()

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
 void MOD_SysLog_Init(void);
void UART_DEBUG_PARSER_Task( void );
 uint8 verbose_Mode; // interruttore usato per attvare/disattivare il printf
 void printLine(void);
void printStarLine(void);
/*******************************************************************************/
#endif
/* [] END OF FILE */
