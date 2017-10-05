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

void M2M_SPI_Init(void) {
	DBG_PRINTF("[%s]\n",__func__);
	SPIS_M2M_Start();
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
	
	if(Pin_SS_Read()) { // trasferimento completato
		if(SPIS_M2M_GetRxBufferSize() == 16) { // ho tutti i caratteri
			len = SPIS_M2M_GetRxBufferSize();
			Pin_SPIF_Write(1);
			len = SPIS_M2M_GetRxBufferSize();
			DBG_PRINTF("dati: %d\n",len);
			for(dataIndex = 0;dataIndex<len;dataIndex++) {
				DBG_PRINTF("0x%02X\n", SPIS_M2M_ReadRxData());
			}
			// Pin_SPIF_Write(0);
		}
	}
}

/* [] END OF FILE */
