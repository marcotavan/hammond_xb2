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
#include <project.h>
#include "cytypes.h"
#include "debug.h"
#include "D:\progetti C\cypress\Psoc5\git/usr.h" // -> see usr.h.sample into firmware directory

char string[256];

uint8 cc_configuration_data[10];    // array di 10 elementi della cc configuration
uint8 cc_configuration_data_index;

#if (DEBUG_UART_ENABLED == YES)

    #if defined(__ARMCC_VERSION)

        /* For MDK/RVDS compiler revise fputc() for the printf() */
        struct __FILE
        {
            int handle;
        };

        enum
        {
            STDIN_HANDLE,
            STDOUT_HANDLE,
            STDERR_HANDLE
        } ;

        FILE __stdin = {STDIN_HANDLE};
        FILE __stdout = {STDOUT_HANDLE};
        FILE __stderr = {STDERR_HANDLE};

        int fputc(int ch, FILE *file)
        {
            int ret = EOF;

            switch( file->handle )
            {
                case STDOUT_HANDLE:
                    UART_PutChar(ch);
                    ret = ch ;
                    break ;

                case STDERR_HANDLE:
                    ret = ch ;
                    break ;

                default:
                    file = file;
                    break ;
            }
            return ret ;
        }

#elif defined (__ICCARM__)      /* IAR */

/* For IAR compiler revise __write() function for printf functionality */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    size_t nChars = 0;

    if (buffer == 0)
    {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return (0);
    }

    for (/* Empty */; size != 0; --size)
    {
        UART_PutChar(*buffer++);
        ++nChars;
    }

    return (nChars);
}

#else  /* (__GNUC__)  GCC */

/* For GCC compiler revise _write() function for printf functionality */
        int _write(int file, char *ptr, int len) {
            int i;
            file = file;
            // if (verbose_Mode != 0) {
                for (i = 0; i < len; i++) {
                    UART_DBG_PutChar(*ptr++);
                }
			// }
            return len;
        }
    #endif  /* (__ARMCC_VERSION) */
#endif /* (DEBUG_UART_ENABLED == YES) */

/**********************************************************************************************************************************/

void sprintfWrite(void) {
    uint8 i;
    for (i = 0; i <= 255; i++) {
        if (string[i] == '\0') break;   // EOS     
        UART_DBG_PutChar(string[i]);
    }
}

/*****************************************************************************\
*  Funzione:     UART0_PARSER_Task(void)                                      *
*  Argomenti:    				      				                          *
*  Restituisce:  Nessuno                                                      *
*  Scopo:  UART0_PARSER_Task fa qualcosa ricevendo caratteri dalla seriale    *
\*****************************************************************************/
void UART0_PARSER_Task( void )
{
    //controlla lo stato del buffer di ricezione

	if(UART_DBG_GetRxBufferSize() > 0u)    {
        fifo[UART0].rxBuf[fifo[UART0].in++] = UART_DBG_GetChar();    /* Get it */
    }

	if (fifo[UART0].in != fifo[UART0].out) { // hasdata
		fifo[UART0].c = fifo[UART0].rxBuf[fifo[UART0].out++];

        // DBG_PRINTF("echo carattere ricevuto %c\n",fifo[UART0].c);
        fifo[UART0].timeout = 50; // mezzo secondo di timeout ricaricato ad ogni pacchetto
        
		switch(fifo[UART0].rxStatus) {
			case PARSE_SOF:	// 01
				if (fifo[UART0].c == 1)	{
					fifo[UART0].rxStatus = PARSE_LEN;
				}
				break;

			case PARSE_LEN:	// LEN
					fifo[UART0].length = fifo[UART0].c;
					fifo[UART0].rxStatus = PARSE_CMD;
	            break;

			case PARSE_CMD:	// C0
                if (fifo[UART0].length)	{
					fifo[UART0].length--;
					fifo[UART0].checksum ^= fifo[UART0].c;
					if (fifo[UART0].c == 0xC0) {
						fifo[UART0].rxStatus = PARSE_SUBCMD;
					}
					else {
						fifo[UART0].rxStatus = PARSE_IDLE;
					}
				}
	 			break;

			case PARSE_SUBCMD:	// AD
				if (fifo[UART0].length) {
					fifo[UART0].length--;
					fifo[UART0].checksum ^= fifo[UART0].c;
					if (fifo[UART0].c == 0xAD) {
						fifo[UART0].rxStatus = PARSE_ADD;
					}
                    else if (fifo[UART0].c == 0xC1) {
						fifo[UART0].rxStatus = PARSE_CONFIGURATION;
                        cc_configuration_data_index = 0;
					}
					else {
						fifo[UART0].rxStatus = PARSE_IDLE;
					}
				}
				break;

			case PARSE_ADD:
				if (fifo[UART0].length) {
					fifo[UART0].length--;
					fifo[UART0].checksum ^= fifo[UART0].c;

					if(fifo[UART0].length == 0) {	// il pacchetto [ finito
						fifo[UART0].rxStatus = PARSE_CHM;
					}
					else {	// ce altro da parsare
						fifo[UART0].rxStatus = PARSE_CHM;
					}

					if (fifo[UART0].c !=0) {
						// DBG_PRINTF("Filtro l'indirizzo %02X all'indice %d\n",c,filtro.indice);
                        // DBG_PRINTF("carattere ricevuto: %c\n",fifo[UART0].c);
					}
				}
				break;
            
            
			case PARSE_CHM: 
				if (fifo[UART0].c == fifo[UART0].checksum) {
					// ok
					// DBG_PRINTF("checkusum torna\n");
				}
				else {
					// DBG_PRINTF("il checksum NON torna\n");
				}

				// fai lo stesso quello che ti si [ detto

                fifo[UART0].timeout = 0;
				fifo[UART0].rxStatus = PARSE_IDLE;
			// break;

			default:
				fifo[UART0].in = 0;
				fifo[UART0].out = 0;
				fifo[UART0].rxStatus = PARSE_SOF;
				break;
		}
	}
}

/**********************************************************************************************************************************/

void Debug_Init(void)
{
    // ISR_RX_UART_StartEx(ISR_RX_UART_InterruptHandler);
    // ISR_TX_UART_StartEx(ISR_TX_UART_InterruptHandler);
    
/* 
    logIndex.in = 0;
    logIndex.out = 0;
*/
    
    // Rx_DBG_0_SetDriveMode(GPIO_0_DM_STRONG);  // imposta la porta in uscita
    
    // qui settare il pin in |RESISTIVE OPULLUP 
    UART_DBG_Start();
    
    fifo[UART0].checksum = 0xff;
    fifo[UART0].in = 0;
	fifo[UART0].out = 0;
	fifo[UART0].rxStatus = PARSE_SOF;
	
	DBG_PRINT_TEXT  ("\r\n");
    DBG_PRINT_TEXT  ("\r\n");
    printStarLine();
    DBG_PRINT_TEXT  ("=             Evaluation Board 001: CSxxxx.xxx  \r\n");
    // DBG_PRINTF      ("=     bootloader version: 0x%04X   \r\n", (uint16)Bootloader_GetMetadata(Bootloader_GET_BTLDR_APP_VERSION,0));
    // DBG_PRINTF      ("=           Product type: 0x%04X   \r\n", (uint16)Bootloader_GetMetadata(Bootloader_GET_BTLDB_APP_ID,0));
    // DBG_PRINTF      ("=            App Version: 0x%04X   \r\n", (uint16)Bootloader_GetMetadata(Bootloader_GET_BTLDB_APP_VERSION,0));   
    // DBG_PRINTF      ("=     Custom App Version: 0x%04X   \r\n", (uint16)Bootloader_GetMetadata(Bootloader_GET_BTLDB_APP_CUST_ID,0));   
    DBG_PRINTF      ("=     PSOC CREATOR 4.1.1: %s @ %s, %s    \r\n", __DATE__,__TIME__,USRMAIL);
	printStarLine();
    DBG_PRINT_TEXT  ("\r\n"); 
}

void printLine(void)
{
    DBG_PRINTF      ("-----------------------------------------------------------------------------------------\r\n");  
}

void printStarLine(void)
{
    DBG_PRINT_TEXT  ("*****************************************************************************************\r\n");
}

/* [] END OF FILE */
