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

#include <project.h>
#include "cytypes.h"
#include "debug.h"
#include "EepromManager.h"
#include "midiLibrary.h"
#include "VB3_midi_map.h"
#include "ButtonScanner.h"

char string[256];
// sprintf(str, "uart_zw_buffer_size:%d ", uart_zw_buffer_size);



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
        int _write(int file, char *ptr, int len)
        {
            
            int i;
            file = file;
            if (verbose_Mode != 0)
            {

                // spif_on();
                for (i = 0; i < len; i++)
                {
                    UART_DBG_PutChar(*ptr++);
                }
				
                // spif_off();
                
                /*
                memcpy(syslog[logIndex.in].str,ptr,len);
                syslog[logIndex.in].len = len;
                
                logIndex.in++; // incremento per la prossima volta in ingresso
                if(logIndex.in == MAXLOG) logIndex.in = 0; // circolare grazie
                
                // start se posso 
                // if (logIndex.out != logIndex.in)
                if (canaleLibero)
                {
                    canaleLibero = 0;
                    UART_PutChar(syslog[logIndex.out].str[0]); // butto fuori il primo carattere
                    // e alscio che sia l'isr a getire l'uscita di tutto il mondo
                }
                */


            }
            
            return len;
        }

    #endif  /* (__ARMCC_VERSION) */

#endif /* (DEBUG_UART_ENABLED == YES) */

/**********************************************************************************************************************************/

void sprintfWrite(void)
{
    uint8 i;
    
    for (i = 0; i <= 255; i++)
    {
        if (string[i] == '\0') break;   // EOS     
        UART_DBG_PutChar(string[i]);

    }
}

void CommandParser(uint8 *data, uint8 len)
{
    uint8 command = data[0];
    len--;
    
    uint8 parameter[len];
    memcpy(parameter,&data[1],len);
    
    DBG_PRINTF("%s: len:%d\n",__func__,len);
    
    switch(command)    {
        case 0xFF: {
            // erboot, erase all, ecc
            switch (parameter[0]) {
                case 1: {
                    CySoftwareReset();
                } break;
                
                case 0xA5: {
                    DBG_PRINTF("EEPROM_EraseSector 0 ...");
                    EEPROM_EraseSector(0);
                    DBG_PRINTF("done\n");
                } break;
            }
        } break;
            
        case 0xF0: {
            // relativi alla memoria
            switch (parameter[0]) {
                case 1: {
                    internal_eeprom_inspector(0,80);
                }
                break;
            }
        } break;
            
        case 0xF1: {
            // relativi all amemoria
            internal_eeprom_inspector(0,parameter[0]);
        } break;
                
        case 0xC0: {
            // send control change
            sendControlChange(parameter[0],parameter[1],MIDI_CHANNEL_1);
        } break;
        
        case 0xB0: {
        // button test
            ButtonCommand(parameter[0],parameter[1]);
        } break;
        
        case 0xB1: {
        // button test
            switch (parameter[0]) {
                case 1: {
                    InitSwitchButtons();
                }
                break;
            }
        } break;
            
        default:
            break;
    }
}

/*****************************************************************************\
*  Funzione:     UART0_PARSER_Task(void)                                      *
*  Argomenti:    				      				                          *
*  Restituisce:  Nessuno                                                      *
*  Scopo:  UART0_PARSER_Task fa qualcosa ricevendo caratteri dalla seriale    *
\*****************************************************************************/
void UART_DEBUG_PARSER_Task( void )
{
    uint8 i;
    // uint8 *ps;
    // ps = (uint8 *) &clock;
    
	/* questo TASK conta esattamente TIME_DELAY ms */
	/*
	 * 01 05 C0 AD 28 F0 02 EE	// Filter: SOF LEN PAYLOAD CHM C0 comando AD address 28 F0 02
	 * 01 03 C0 AD 00 EE 		// Filter: SOF LEN PAYLOAD CHM C0 comando free
	 * 01 03 C0 AD 28 EE		// Filter: SOF LEN PAYLOAD CHM C0 comando AD address 28
	 */

	// uint16_t delay;
    
    /*
    if (UART_ReadRxStatus() & (UART_RX_STS_PAR_ERROR|UART_RX_STS_STOP_ERROR|UART_RX_STS_OVERRUN|UART_RX_STS_SOFT_BUFF_OVER))
    {
        spif_n8(UART_ReadRxStatus());   
    }
    */
    
/*
	DBG_PRINTF("\r\n");
	DBG_PRINTF("\r\n");
	DBG_PRINTF("===============================================================================\r\n");
	DBG_PRINTF("NO FRAMEs. Comandi disponibili:\n");
	DBG_PRINTF("\r\n");
	DBG_PRINTF("Filter: SOF LEN PAYLOAD CHM C0 comando AD address 28\n");
	DBG_PRINTF("\t01 03 C0 AD 28 EE\n");
	DBG_PRINTF("\r\n");

	DBG_PRINTF("Filter: SOF LEN PAYLOAD CHM C0 comando AD address 28 F0 Funzione 02\n");
	DBG_PRINTF("\t01 05 C0 AD 28 F0 02 EE\n");
	DBG_PRINTF("\r\n");
	DBG_PRINTF("Filter: SOF LEN PAYLOAD CHM C0 comando 00 non filtrare\n");
	DBG_PRINTF("\t01 03 C0 AD 00 EE\n");
	DBG_PRINTF("\r\n");
	DBG_PRINTF("===============================================================================\r\n");
*/
    
        //controlla lo stato del buffer di ricezione
    
	#ifndef PRODUCTION_TEST
    if(UART_DBG_GetRxBufferSize() > 0u)
    {
        fifo[UART0].rxBuf[fifo[UART0].in++] = UART_DBG_GetChar();    /* Get it */
    }
	#endif // PRODUCTION_TEST


	if (fifo[UART0].in != fifo[UART0].out) // hasdata
	{
		fifo[UART0].c = fifo[UART0].rxBuf[fifo[UART0].out++];

        // DBG_PRINTF("echo carattere ricevuto %c\n",fifo[UART0].c);
        fifo[UART0].timeout = 50; // mezzo secondo di timeout ricaricato ad ogni pacchetto
        
		switch(fifo[UART0].rxStatus)
		{
			case PARSE_SOF:	// 01
			//sof
			if (fifo[UART0].c == 1)
			{
                
				fifo[UART0].rxStatus = PARSE_LEN;
			}
			break;

			case PARSE_LEN:	// LEN
            {
				fifo[UART0].length = fifo[UART0].c;
				fifo[UART0].rxStatus = PARSE_CMD;
            }				
            break;

			case PARSE_CMD:	// C0
			{
                if (fifo[UART0].length)
				{
					fifo[UART0].length--;
					fifo[UART0].checksum ^= fifo[UART0].c;
					if (fifo[UART0].c == 0xC0)
					{
						fifo[UART0].rxStatus = PARSE_SUBCMD;
					}
					else
					{
						fifo[UART0].rxStatus = PARSE_IDLE;
					}
				}
            }
			break;


			case PARSE_SUBCMD:	// AD
            {
                // DBG_PRINTF("PARSE_SUBCMD\n");
				if (fifo[UART0].length)
				{
					fifo[UART0].length--;
					fifo[UART0].checksum ^= fifo[UART0].c;
					if (fifo[UART0].c == 0xAD)
					{
						fifo[UART0].rxStatus = PARSE_ADD;
					}
                    else if (fifo[UART0].c == 0xC1)
					{
						fifo[UART0].rxStatus = PARSE_CONFIGURATION;
                        cc_configuration_data_index = 0;
					}
					else
					{
						fifo[UART0].rxStatus = PARSE_IDLE;
					}
				}
            }
			break;

            case PARSE_CONFIGURATION:
            {
                // DBG_PRINTF("length: %d, *: %d\n",fifo[UART0].length, cc_configuration_data_index);
                
                // checksum
                fifo[UART0].checksum ^= fifo[UART0].c;
                
                // colleziono
                cc_configuration_data[cc_configuration_data_index] = fifo[UART0].c;  
                
                // vado un po' avanti
                cc_configuration_data_index++;     
                
                if (fifo[UART0].length)
			    {
                    fifo[UART0].length--;
                }                   
                
				if(fifo[UART0].length != 0)
				{	// ce altro da parsare
					fifo[UART0].rxStatus = PARSE_CONFIGURATION; // resto qua
				}
                else
				{	// il pacchetto [ finito
                    DBG_PRINTF("Scrivo l'array ricevuto per CONFIGURATION: ");
                    // DBG_PRINT_ARRAY(cc_configuration_data,cc_configuration_data_index);
                    
                    for(i=0;i<cc_configuration_data_index;i++)
                    {
                        DBG_PRINTF("%02X ",cc_configuration_data[i]);
                    }
                    
                    DBG_PRINTF("\n");
					fifo[UART0].rxStatus = PARSE_CHM;
				}
            }
            break;
            
			case PARSE_ADD:
            {
				if (fifo[UART0].length)
				{
					fifo[UART0].length--;
					fifo[UART0].checksum ^= fifo[UART0].c;

					if(fifo[UART0].length == 0)
					{	// il pacchetto [ finito
						fifo[UART0].rxStatus = PARSE_CHM;
					}
					else
					{	// ce altro da parsare
						fifo[UART0].rxStatus = PARSE_CHM;
					}

					if (fifo[UART0].c !=0)
					{
//    						DBG_PRINTF("Filtro l'indirizzo %02X all'indice %d\n",c,filtro.indice);
                        // DBG_PRINTF("carattere ricevuto: %c\n",fifo[UART0].c);
                        
                        switch(fifo[UART0].c)
                        {   
                            default:
                            break;
                        }
                        
					}
					else
					{

                    }
				}
            }
			break;
            
			case PARSE_CHM:
			{
				if (fifo[UART0].c == fifo[UART0].checksum)
				{
					// ok
					// DBG_PRINTF("checkusum torna\n");
                    // esegui
                    
				}
				else
				{
					// DBG_PRINTF("il checksum NON torna\n");
				}

				// fai lo stesso quello che ti si [ detto
                CommandParser(cc_configuration_data,cc_configuration_data_index);

                fifo[UART0].timeout = 0;
				fifo[UART0].rxStatus = PARSE_IDLE;
			}
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

void MOD_SysLog_Init(void)
{
    
    // Rx_1_Write(1); // attiva internal PullUp per evitare che la macchina seriale si incarti 
    
    // qui settare il pin in |RESISTIVE PULLUP 
    UART_DBG_Start();
    
    fifo[UART0].checksum = 0xff;
    fifo[UART0].in = 0;
	fifo[UART0].out = 0;
	fifo[UART0].rxStatus = PARSE_SOF;
    DBG_PRINT_TEXT  ("\r\n");
    DBG_PRINT_TEXT  ("\r\n");    
    DBG_PRINTF("[%s]\n",__func__);
    printStarLine();
    DBG_PRINT_TEXT  ("=            HAMMOND XB2 MIDI KEYBOARD 0.0 \r\n");
    // DBG_PRINTF      ("=     bootloader version: 0x%04X   \r\n", Bootloader_GetMetadata(Bootloader_GET_BTLDR_APP_VERSION,0));
    // DBG_PRINTF      ("=           Product type: 0x%04X   \r\n", Bootloader_GetMetadata(Bootloader_GET_BTLDB_APP_ID,0));
    // DBG_PRINTF      ("=            App Version: 0x%04X   \r\n", Bootloader_GetMetadata(Bootloader_GET_BTLDB_APP_VERSION,0));   
    // DBG_PRINTF      ("=     Custom App Version: 0x%04X   \r\n", Bootloader_GetMetadata(Bootloader_GET_BTLDB_APP_CUST_ID,0));   
    DBG_PRINTF      ("=  Compile Date and Time: %s %s    \r\n", __DATE__,__TIME__);
    printStarLine();
    DBG_PRINT_TEXT  ("\r\n"); 
}

void printLine(void)
{
    DBG_PRINTF      ("--------------------------------------------------------------------------------\r\n");  
}

void printStarLine(void)
{
    DBG_PRINT_TEXT  ("********************************************************************************\r\n");
}

/* [] END OF FILE */
