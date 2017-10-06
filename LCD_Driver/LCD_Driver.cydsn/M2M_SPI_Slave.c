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

volatile uint8 spiData[160];
struct display_data_t {
	uint8 data[20];
	
};

volatile uint8 ptrIn = 0;
uint8 ptrOut = 0;

#define DIM_QUEUE 100
struct display_data_t displayData[DIM_QUEUE];

uint8 hasElements(void) {
	if (ptrIn != ptrOut) {
		return (1);
	} else {
		return (0);
	}
	
}

uint8 isQueueNotFull(uint8 pIn) {
	uint8 val = (pIn + 1) % DIM_QUEUE; 
	
	if(val != ptrOut) {
		ptrIn = val;
		return (1);
	} else {
		return (0);
	}
}
 

CY_ISR_PROTO(SS_ISR);

CY_ISR(SS_ISR) { 
	uint8 idx = 0;
	// decidere al posizione di ptrIn
	if(isQueueNotFull(ptrIn)) {
		Pin_SPIF_Write(1);	
		// accoda il dato all'indice  -> ptrIn 
		for(idx = 0;idx<16;idx++) {
			displayData[ptrIn].data[idx] = SPIS_M2M_ReadRxData();
		}
		Pin_SPIF_Write(0);	
	}
	
    Pin_SS_ClearInterrupt();
}

void M2M_SPI_Init(void) {
	DBG_PRINTF("[%s]\n",__func__);
	SPIS_M2M_Start();
	isr_ss_1_StartEx(SS_ISR);
}

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
		
		DBG_PRINTF("%2d: ",ptrOut); // posizione del rpimo elemento
		DBG_PRINT_ARRAY(displayData[ptrOut].data,16);
		DBG_PRINTF("\n");
	}
	
}

/* [] END OF FILE */
