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
#include "project.h"
#include "debug.h" 
#include "tick.h"
#include "myLCD.h"
#include "M2M_SPI_Slave.h"
#include "crc16_ccitt.h"

#define M2M_SPI_ADDRESS 0xC0

CY_ISR_PROTO(SS_ISR);
#define MAX_DATA 22
#define DIM_QUEUE 100

uint8 rawData[DIM_QUEUE][MAX_DATA];
volatile uint8 ptrIn = 0;
uint8 ptrOut = 0;
uint8 isBusy;

union lcdData_t {
	struct display_data_t {
		uint8 address;
		uint8 len;
		uint8 type;
		uint8 val;
		uint8 data[16];
		uint8 crc1;
		uint8 crc2;
	} displayData;
	
	uint8 data[MAX_DATA];
} LcdData;

/*****************************************************************************\
*  Funzione:     hasElements()                                                *
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        ci sono elementi in coda se i due puntatori sono diversi     *
\*****************************************************************************/
uint8 hasElements(void) {
	// ci sono elementi in coda se i due puntatori sono diversi
	if (ptrIn != ptrOut) {
		return (1);
	} else {
		return (0);
	}
}

/*****************************************************************************\
*  Funzione:     isQueueNotFull()                                             *  	
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        posso accodare se il successivo elemento e' libero		      *
\*****************************************************************************/
uint8 isQueueNotFull(uint8 pIn) {
	uint8 val = (pIn + 1) % DIM_QUEUE; 
	// posso accodare se il successivo elemento e' libero
	if(val != ptrOut) {
		ptrIn = val;
		return (1);
	} else {
		return (0);
	}
}
 
/*****************************************************************************\
*  Funzione:     CY_ISR(SS_ISR)                                               *  	
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        														      *
\*****************************************************************************/
CY_ISR(SS_ISR) { 
	uint8 idx = 0;
	// questo blocco costa 50uS
	
	/* al master mettere un ritardo di CyDelayUs(100); tra una transazione SPI e l'altra. non meno di 100us altrimenti non prendo il dato.*/
	isBusy = 1;
	Pin_SPIF_Write(1);	
	if(isQueueNotFull(ptrIn)) {
		// accoda il dato ricevuto alla struttura di indice -> ptrIn 
		for(idx = 0;idx<MAX_DATA;idx++) {
			rawData[ptrIn][idx] = SPIS_M2M_ReadRxData();
		}
		SPIS_M2M_ClearRxBuffer();
		SPIS_M2M_ClearFIFO();
	}
	Pin_SPIF_Write(0);		
    Pin_SS_ClearInterrupt();
	isBusy = 0;
}

/*****************************************************************************\
*  Funzione:     M2M_SPI_Init()                         	                  *  	
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        														      *
\*****************************************************************************/
void M2M_SPI_Init(void) {
	DBG_PRINTF("[%s]\n",__func__);
	SPIS_M2M_Start();
	isr_ss_1_StartEx(SS_ISR);
}

/*****************************************************************************\
*  Funzione:     M2M_SPI_Slave_ApplicationPoll()							  *  	
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:        															  *
\*****************************************************************************/
void M2M_SPI_Slave_ApplicationPoll(void) {
	static uint8 isInitialized = 0;
	uint8 len = 0;
	uint8 dataIndex = 0;
	static uint8 lastPosition = 0;
	static uint16 crcErrorCounter = 0;
//    static uint8 spiByteCounter = 0;
//   static uint8 spiMaxByte = 0;
//    static uint8 spiData[100];
//	static uint8 divider = 0;
	uint16 crc;
	
	if(isInitialized == 0) {
		// Control_Reg_LV5239TAZ_Write(Control_Reg_LV5239TAZ_Read() & ~BIT2);
		M2M_SPI_Init();
		isInitialized = 1;
	}
	
	/*
	if(Pin_SS_Read()) { // trasferimento completato
		if(SPIS_M2M_GetRxBufferSize() == 16) { // ho tutti i caratteri
			len = SPIS_M2M_GetRxBufferSize();
			// Pin_SPIF_Write(1);
			len = SPIS_M2M_GetRxBufferSize();
			DBG_PRINTF("dati: %d\n",len);
			for(dataIndex = 0;dataIndex<len;dataIndex++) {
				DBG_PRINTF("0x%02X\n", SPIS_M2M_ReadRxData());
			}
			// Pin_SPIF_Write(0);
		}
	}
	*/
	if(isBusy == 0) {
		//if(tick_100ms(TICK_SPI)) {
			if(hasElements()) { // ci sono elementi in coda
				ptrOut++;	// primo elemento da prelevare
				ptrOut %= DIM_QUEUE; 
				memcpy(LcdData.data,rawData[ptrOut],MAX_DATA);
				
				/*
				DBG_PRINTF("%02d: ",ptrOut); // posizione del rpimo elemento
				DBG_PRINTF("%02X %02X %02X %02X |%02X %02X ",LcdData.displayData.address,
					LcdData.displayData.len,
					LcdData.displayData.type, // tipo carattere
					LcdData.displayData.val,
					LcdData.displayData.crc1,
					LcdData.displayData.crc2
				); // posizione
				DBG_PRINT_ARRAY(LcdData.displayData.data,16);
				*/
				// parser 
				
				crc=crc16ccitt_1d0f(LcdData.data,20);
				
		
				if (crc == (LcdData.displayData.crc1<<8|LcdData.displayData.crc2)) {
					//dati ricevuti validi
					switch(LcdData.displayData.address) {
						case M2M_SPI_ADDRESS: 	// -indirizzo per me
							switch(LcdData.displayData.val) {
								case 0:  // riga 0
								case 1:  // riga 1
									switch(LcdData.displayData.type) {
										case 0: // caratteri standard 
										 	// DBG_PRINTF("caratteri standard ");
											if(lastPosition != LcdData.displayData.val) {
												myLCD_Position(LcdData.displayData.val,0);
												lastPosition = LcdData.displayData.val;
											}
											myLCD_WriteDisplayLcd(LcdData.displayData.data,LcdData.displayData.len);
										
											// myLCD_Position(0,0);
											// myLCD_PrintString(LcdData.displayData.data);

											// queste due funzioni a seguire impiegano circa 2ms

											// myLCD_Position(0,0);  
											// myLCD_PrintString(rawData[ptrOut]);
						
											break;
										
										case 1: // caratteri custom	
											// DBG_PRINTF("caratteri custom ");
											Write_BarGraphs(LcdData.displayData.val,LcdData.displayData.data);
											break;
									} // switch(LcdData.displayData.type)
								break;	
							} // switch posizioni valide
						break;
					} //switch(LcdData.displayData.address)
				} else {
					crcErrorCounter++;
					DBG_PRINTF("| %02X%02X %04X |",LcdData.displayData.crc1,
					LcdData.displayData.crc2, crc);
					DBG_PRINTF("crc error %d",crcErrorCounter);
					DBG_PRINTF("\n");
				}
				
			}
		//}
	
		// Pin_SPIF_Write(0);
	} // hasElements
} // M2M_SPI_Slave_ApplicationPoll

/* [] END OF FILE */
