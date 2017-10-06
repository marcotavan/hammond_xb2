/* ========================================
 *
 * Copyright Marco Tavan, 2017
 * V1.0 01/09/2017 
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* Includes */
#include "project.h"
#include "M2M_SPI_Master.h"
#include "debug.h"
#include "tick.h"
// #include "myLCD.h"

#define DESELECT 	0
#define ADDRESS_0 	1
#define ADDRESS_1 	2
#define ADDRESS_2 	3

#define M2M_SPI_ADDRESS 0xC0
#define MAX_DATA 		20

enum rows_e {
	ROW_0,	
	ROW_1	
};

enum SPI_states_e {
	SPI_IDLE,
	SPI_START,
	SPI_WRITE_DATA,
	SPI_WAIT_BUSY,
	SPI_WAIT_BUSY_OE
};

enum fontType_e {
	LCD_STANDARD,
	LCD_CUSTOM
};
	
void M2M_SPI_Init(void) {
	DBG_PRINTF("[%s]\n",__func__);
    Pin_SS_M2M_Write(1);
	SPIM_M2M_Start();
}
			
uint8 M2MSpiWriteData(uint8 address, uint8 *data, uint8 len) {
    uint8 spiByteCounter = 0;
	address = address;
	
	// sprintf(str,"address: 0x%02X, ",address);
	// DBG_PRINT_TEXT(str);
	// sprintf(str,"data: 0x%02X,0x%02X,0x%02X\n",data[0],data[1],data[2]);
	// DBG_PRINT_ARRAY(data,20);
	
	Pin_SS_M2M_Write(0);
    // CyDelayUs(1);
	for(spiByteCounter = 0;spiByteCounter<len;spiByteCounter++) {
        SPIM_M2M_WriteTxData(data[spiByteCounter]);
    }
	
	while(!(SPIM_M2M_ReadTxStatus() & SPIM_M2M_STS_SPI_IDLE)) ;
	
    // CyDelayUs(10);
	Pin_SS_M2M_Write(1); 
	return 0;
}

void M2M_Write_LCD(uint8 type, uint8 position, uint8*data) {
    uint8 spiData[MAX_DATA];
	
	spiData[0] = M2M_SPI_ADDRESS;
	spiData[1] = 16;		// dataLen
	spiData[2] = type;		// type write data
	spiData[3] = position;	// pos

	memset(&spiData[4],data[0],16);	// filla 16
	M2MSpiWriteData(0,spiData,MAX_DATA);		// spedisce 20
}

void M2M_SPI_Master_ApplicationPoll(void){
	static uint8 isInitialized = 0;
    static uint8 spiByteCounter = 0;
    static uint8 spiMaxByte = 0;

	static uint16 divider = 0;
	static uint8 counter = 0;
	
	if(isInitialized == 0) {
		M2M_SPI_Init();
		spiByteCounter = 0b00100000;
		isInitialized = 1;
	}
	
	if(tick_100ms(TICK_M2M_SPI)) {
        
    } // tick_10ms
	

	if(tick_10ms(TICK_M2M_SPI)) {
		// DBG_PRINTF("1 ");
		divider++;
		if(divider == 30) {
			divider = 0;
			
			spiByteCounter++;
			// possibile struttura: |ADDRESS|LEN|TYPE|VAL|DATA x 16|
			
			M2M_Write_LCD(ROW_0,LCD_STANDARD,&spiByteCounter);
			
			if(spiByteCounter == 0x7F) spiByteCounter = 0b00100000;
			
			CyDelayUs(100);
			
			M2M_Write_LCD(ROW_1,LCD_CUSTOM,&counter);
			counter++;
		}
	}
}

// EOF