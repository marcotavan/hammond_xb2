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

CY_ISR_PROTO(SS_ISR);
#define MAX_DATA 20
#define DIM_QUEUE 100

uint8 rawData[DIM_QUEUE][MAX_DATA];
volatile uint8 ptrIn = 0;
uint8 ptrOut = 0;

union lcdData_t {
	struct display_data_t {
		uint8 address;
		uint8 len;
		uint8 type;
		uint8 val;
		uint8 data[16];
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
	// Pin_SPIF_Write(1);	
	if(isQueueNotFull(ptrIn)) {
		// accoda il dato ricevuto alla struttura di indice -> ptrIn 
		for(idx = 0;idx<MAX_DATA;idx++) {
			rawData[ptrIn][idx] = SPIS_M2M_ReadRxData();
		}
		SPIS_M2M_ClearRxBuffer();
		SPIS_M2M_ClearFIFO();
	}
	// Pin_SPIF_Write(0);		
    Pin_SS_ClearInterrupt();
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
//    static uint8 spiByteCounter = 0;
//   static uint8 spiMaxByte = 0;
//    static uint8 spiData[100];
//	static uint8 divider = 0;
	
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
	
	if(hasElements()) { // ci sono elementi in coda

		ptrOut++;	// primo elemento da prelevare
		ptrOut %= DIM_QUEUE; 
		memcpy(LcdData.data,rawData[ptrOut],MAX_DATA);
		
		DBG_PRINTF("%02d: ",ptrOut); // posizione del rpimo elemento
		DBG_PRINT_ARRAY(LcdData.data,MAX_DATA);
		DBG_PRINTF("\n");

		
		
		// queste due funzioni a seguire impiegano circa 2ms
		//Pin_SPIF_Write(1);	
		// myLCD_Position(0,0);  
		// myLCD_PrintString(rawData[ptrOut]);
		// Pin_SPIF_Write(0);	
	}
	
}

/* [] END OF FILE */
