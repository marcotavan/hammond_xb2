/*******************************************************************************
* File Name: Debug.c
*
* Version: 1.0
*
* Description:
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
#include <project.h>
#include "cytypes.h"
#include "debug.h"
#include "common.h"
#include "Internal_EEPROM.h"
#include "life_clock.h"
#include "parametri.h"
#include "spif_psoc5.h"
#include "masterReset.h"

char string[256];
// sprintf(str, "uart_zw_buffer_size:%d ", uart_zw_buffer_size);

#if 0

struct syslog_t slog[MAX_LOG];
DBG_PRINTF("ciao %d",n);
DBG_LOG("ciao %d",n);


void syslog(char *id, char *str)
{
    // sprintf(slog[0].id,id);
    // sprintf(slog[0].str,str);
    // sprintf(slog[0].str, "uart_zw_buffer_size:%d ", uart_zw_buffer_size);
}

#endif

/*
struct _FIFO_ fifo[NUM_SERIALI];

struct logIndex_t logIndex;

uint16 str_Index = 0;
uint8 canaleLibero = 1;
*/

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

/*****************************************************************************\
*  Funzione:     UART0_PARSER_Task(void)                                      *
*  Argomenti:    				      				                          *
*  Restituisce:  Nessuno                                                      *
*  Scopo:  UART0_PARSER_Task fa qualcosa ricevendo caratteri dalla seriale    *
\*****************************************************************************/
void UART0_PARSER_Task( void )
{
    // uint8 index;
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
                    DBG_PRINT_ARRAY(cc_configuration_data,cc_configuration_data_index);
                    DBG_PRINTF("\n");
					fifo[UART0].rxStatus = PARSE_CHM;
                    FL_CC_CONFIG_SAVE = 1;
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
                            case 1:
                                // eeprom inspector
                                internal_eeprom_inspector(0,0x20);
                            break;
                            
                            case 2:
                            {
                                DBG_PRINTF("Lifetime clock ");
                                fifo[UART0].rxStatus = PARSE_CLOCK_PARAM;
                            }
                            break;
                            
                            case 3:
                            { // portato in CC COnfiguration
                                DBG_PRINTF("EEP MARKER RESET\n");
                                 hardResetFunc();
                                /*
                                if(EEPROM_UpdateTemperature() == CYRET_SUCCESS)
                                {
                                    // EEPROM_ByteWritePos(uint8 dataByte, uint8 rowNumber, uint8 byteNumber) 
                                    // EEPROM_ByteWrite(0, MARKER_EEPROM_ROW, MARKER_EEPROM_POSITION);
                                    task[0] = EEP_RESET_MARKER;
                                    eeprom_task(task);
                                    SoftwareReset(SR12);
                                }
                                */
                            }
                            break;
                            
                            case 4:
                            {
                                DBG_PRINTF("HeartBeat LED ");
                                fifo[UART0].rxStatus = PARSE_HB_PARAM;
                            }
                            break;
                            
                            case 5:
                            { // portato in cc configuration
                                DBG_PRINTF("BOARD SOFTWARE RESET\n");
                                SoftwareReset(SR11);
                            }
                            break;
                            
                            case 6:
                            { // portato in CC COnfiguration
                                DBG_PRINTF("EEP MARKER RESET\n");

                                if(EEPROM_UpdateTemperature() == CYRET_SUCCESS)
                                {
                                    task[0] = EEP_RESET_MARKER;
                                    eeprom_task(task);
                                    SoftwareReset(SR12);
                                }
                            }
                            break;
                                
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
            
            case PARSE_HB_PARAM:
            {
                switch(fifo[UART0].c)
                {
                    case 0:
                    DBG_PRINTF("OFF\n");
                    stato.heartbeat_led = LOCK; // resta qua dentro per sempre
                    break;
                    
                    default:
                    // DBG_PRINTF("1\n");
                    DBG_PRINTF("ON\n");
                    stato.heartbeat_led = ON; // riparte
                    break;
                }
                    
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
            }
            break;
            
            case PARSE_CLOCK_PARAM:
            {
                switch(fifo[UART0].c)
                {
                    case 1:
                    // DBG_PRINTF("1\n");
                    DBG_PRINTF("ON\n");
                    // clock.print_enable = 1;
                    printClock(ON);
                    break;
                    
                    case 2:
                    DBG_PRINTF("OFF\n");
                    // clock.print_enable = 0;
                    printClock(OFF);
                    // DBG_PRINTF("2\n");
                    break;
                    
                    case 3:
                    DBG_PRINTF("SAVE\n");
                    FL_CLOCK_SAVE = 1;
                    // DBG_PRINTF("2\n");
                    break;
                    
                    default:
                    break;
                }
                    
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
            }
            break;
            
			case PARSE_CHM:
			{
				if (fifo[UART0].c == fifo[UART0].checksum)
				{
					// ok
					// DBG_PRINTF("checkusum torna\n");
				}
				else
				{
					// DBG_PRINTF("il checksum NON torna\n");
				}

				// fai lo stesso quello che ti si [ detto

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
    
    if (FL_CC_CONFIG_SAVE != 0)
    {
		#warning "Marco: il passaggio dei parametri alla funzione set_parametro e' cambiato. Attenzione in caso di debug"
		// setto il parametro e scrivo in eeprom
        FL_CC_CONFIG_SAVE = 0;
		set_parametro(
			cc_configuration_data[0],	// Parameter Number
			/* Default */ (cc_configuration_data[1] & CONFIGURATION_SET_LEVEL_DEFAULT_BIT_MASK),	// Default
			/* Size */ (cc_configuration_data[1] & CONFIGURATION_SET_LEVEL_SIZE_MASK),				// Size
			&cc_configuration_data[2]	// Configuration Values 1..4 (according to Size)
		);
    }
    
    if(FL_CLOCK_SAVE != 0) // da spostare in un FLAG
    { // salva l'ora attuale
        FL_CLOCK_SAVE = 0;
        
        task[0] = EEP_SET_CLOCK;
        eeprom_task(task);
        
    }
}

/**********************************************************************************************************************************/

#if 0
CY_ISR_PROTO(ISR_TX_UART_InterruptHandler);
CY_ISR_PROTO(ISR_RX_UART_InterruptHandler);

/*****************************************************************************\
*  Funzione:     CY_ISR(ISR_UART_InterruptHandler)                         *
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        Interrupt RX uart                                            *
\*****************************************************************************/
CY_ISR(ISR_TX_UART_InterruptHandler)
{
    //spif_on();
    /*
    if ((str_Index++) < (syslog[logIndex.out].len-1))
    {
        // butto fuori i caratteri successivi fino alla fine
        UART_PutChar(syslog[logIndex.out].str[str_Index]); 
    }
    else
    {
        // avrei finito ma ... in coda c'e' ancora roba?
        str_Index = 0;    
        
        if (logIndex.out != logIndex.in)
        {
            // forse c'e' ancora qualcosa da spedire
            logIndex.out++;
            if (logIndex.out == MAXLOG) logIndex.out = 0;
            // e spedisco il primo carattere
            UART_PutChar(syslog[logIndex.out].str[str_Index]); 
        }
        else
        {
            canaleLibero = 1;
        }
    }
    */
    //spif_off();
    
}

/*****************************************************************************\
*  Funzione:     CY_ISR(ISR_UART_InterruptHandler)                         *
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        Interrupt RX uart                                            *
\*****************************************************************************/
CY_ISR(ISR_RX_UART_InterruptHandler)
{
    fifo[UART0].rxBuf[fifo[UART0].in++] = UART_GetChar();    /* Get it */
}
#endif

void MOD_SysLog_Init(void)
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
    
    verbose_Mode = OFF; // interruttore usato per attvare/disattivare il printf
    
    fifo[UART0].checksum = 0xff;
    fifo[UART0].in = 0;
	fifo[UART0].out = 0;
	fifo[UART0].rxStatus = PARSE_SOF;
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
