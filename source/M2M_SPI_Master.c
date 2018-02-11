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
#include "crc16_ccitt.h"
// #include "customLcd.h"

#define DESELECT 	0
#define ADDRESS_0 	1
#define ADDRESS_1 	2
#define ADDRESS_2 	3

#define M2M_SPI_ADDRESS_SYSTEM 0x5A
#define M2M_SPI_ADDRESS 0xC0
#define MAX_DATA 		22

static uint8 isInitialized = 0;
static char str[200];

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


uint8 text1[16] = {'a','b','c','d',
				  'e','f','g','h',
				  'i','j','k','l',
	  	 	 	  'm','n','o','p'};
	
uint8 text2[16] = {'0','1','2','3',
					'4','5','6','7',
					'8','9','A','B',
					'C','D','E','F'};
/********************************************************************************************************************/
void SendResetToDisplayBoard(void) {
    uint8 spiData[MAX_DATA];
	uint16 crc;
	
	spiData[0] = M2M_SPI_ADDRESS_SYSTEM;
	spiData[1] = 16;		// dataLen
	spiData[2] = 0xA0;		// type write data
	spiData[3] = 1;			// pos = val

	memset(&spiData[4],0,16);	// filla 16
	crc = crc16ccitt_1d0f(spiData,20);
	spiData[20] = (crc >> 8) & 0xff;
	spiData[21] = crc & 0xff;
	M2MSpiWriteData(0,spiData,MAX_DATA);		// spedisce 20
}

void M2M_SPI_Init(void) {
	DBG_PRINTF("[%s]\n",__func__);
    Pin_SS_M2M_Write(1);
	SPIM_M2M_Start();
	SendResetToDisplayBoard();
	isInitialized = 1;
}
	
uint8 M2MSpiWriteData(uint8 address, uint8 *data, uint8 len) {
    uint8 spiByteCounter = 0;
	address = address;
	/*
	DBG_PRINTF("[%s] ",__func__);
	sprintf(str,"data: 0x%02X,0x%02X,0x%02X\n",data[0],data[1],data[2]);
	DBG_PRINT_ARRAY(data,22);
	DBG_PRINTF("\n");
	*/
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

void M2M_Write_command(uint8 type) {
    uint8 spiData[MAX_DATA];
	uint16 crc;
	
	spiData[0] = M2M_SPI_ADDRESS+1;
	spiData[1] = 0;		// dataLen
	spiData[2] = type;		// type write data

	crc = crc16ccitt_1d0f(spiData,3);
	spiData[3] = (crc >> 8) & 0xff;
	spiData[4] = crc & 0xff;
	M2MSpiWriteData(0,spiData,5);		// spedisce 20
}
					  // ROW_0, LCD_STANDARD,  *text
void M2M_Write_LCD(uint8 position, uint8 type, uint8 *data) {
	#define DATA_LEN 16
    uint8 spiData[MAX_DATA];
	uint16 crc;
	
	spiData[0] = M2M_SPI_ADDRESS;
	spiData[1] = DATA_LEN;		// dataLen
	spiData[2] = position;		
	spiData[3] = type;	

	memcpy(&spiData[4],data,16);	// filla 16
	crc = crc16ccitt_1d0f(spiData,20);
	spiData[20] = (crc >> 8) & 0xff;
	spiData[21] = crc & 0xff;
	M2MSpiWriteData(0,spiData,MAX_DATA);		// spedisce 20
}

void M2M_SPI_Master_ApplicationPoll(void){

    static uint8 spiByteCounter = 0;
    static uint8 spiMaxByte = 0;

	static uint16 divider = 0;
	static uint8 counter = 0;
	static uint8 toggle = 0;
	
	if(isInitialized == 0) {
		M2M_SPI_Init();
		// spiByteCounter = 0b00100000;
	}
	/*
	if(tick_100ms(TICK_M2M_SPI)) {
        
    } // tick_10ms
	
	
	if(tick_10ms(TICK_M2M_SPI)) {
		// DBG_PRINTF("1 ");
		divider++;
		if(divider == 30) {
			divider = 0;
			
			spiByteCounter++;
			// possibile struttura: |ADDRESS|LEN|TYPE|VAL|DATA x 16|
			
			// M2M_Write_LCD(ROW_0,LCD_STANDARD,&spiByteCounter);
			
			if(spiByteCounter == 0x7F) spiByteCounter = 0b00100000;
			
			// CyDelayUs(100);
			
			// M2M_Write_LCD(ROW_1,LCD_CUSTOM,&counter);
			// counter++;
		}
	}
	*/
	/*
	if(tick_1s(TICK_M2M_SPI)) {
		toggle = ~toggle;
		if(toggle) {
			M2M_Write_LCD(ROW_1,LCD_STANDARD,text1);		// testo scritto nella riga bassa
		} else {
			M2M_Write_LCD(ROW_1,LCD_STANDARD,text2);		// testo scritto nella riga bassa
		}
	}
	*/
}

static uint8 prev_bargraph[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

void ResetBarGraphs() {
	DBG_PRINTF("%s\n",__func__);
	uint8 prevBarValue[16];
	uint8 tmpdata[16];

	memcpy(prevBarValue,prev_bargraph,sizeof(prevBarValue)); // salvo la posizione dei bargraps

	memset(prev_bargraph,0x0,sizeof(prev_bargraph));	// cancello i precedenti per far commutare la fujnzione successiva
	memset(tmpdata,0xff,sizeof(tmpdata));				// preparo un array temporaneo da inviare per cancellare il display
	lockBargraphs = 0; 									// libero il semaforo
	Write_BarGraphs(tmpdata);							// scrivo un blank del display
	Write_BarGraphs(prevBarValue);						// riscrivo immediatamentei bargraphs
}


void Write_BarGraphs(uint8 *data)
{
    uint8 i;
    //static uint16 cnt = 0;

	uint8 flagWrite = 1; // 1: MANDA IN CONTINUO
    // if (alternateTextCounter) return;  // non scrive niente

	// DBG_PRINTF("riscrivo barre... \n");
    for (i=0;i<16;i++)
    {
		if(prev_bargraph[i] != data[i]) {
			prev_bargraph[i] = data[i];
			// LCD_Position(0,i);
			// LCD_PrintNumber(str_bargraph[ROW_0][i]);
        	// LCD_DrawVerticalBG(0, i, 8,str_bargraph[ROW_0][i]);
			// DBG_PRINTF("riscrivo barre %d\n",i);
			flagWrite = 1;
		}
    }
    /*
	if (GetLockBargraphs()) {
		flagWrite = 0;
	}
	*/
	
	if(flagWrite) {
		if(lockBargraphs == 0) {
			M2M_Write_LCD(ROW_0,LCD_BARGRAPHS,data);	// bargraphs
			// DBG_PRINTF("M2M_Write_LCD riscrivo barre \n");
		}
	}
    // sposta il cursore sotto
    // LCD_Position(1,0);
    // LCD_PrintString(&str_bargraph[ROW_1][0]);
}

// EOF