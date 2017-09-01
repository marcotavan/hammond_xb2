/* ========================================
 *
 * Copyright Marco Tavan, 2017
 * V1.0 31/08/2017 
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
#include "PCF8575.h"
#include "debug.h"
#include "tick.h"
#include "common.h"
#include "LiquidCrystal_I2C.h"

volatile uint8 PCF8575_Read_Input_flag = 0;

CY_ISR_PROTO(PCF8575_INPUT_INTERRUPT);


CY_ISR(PCF8575_INPUT_INTERRUPT) //ZEROCROSSING DI RETE
{ 
	pcf8575_IRQ_ClearInterrupt();
    PCF8575_Read_Input_flag = 1;
}

/*
HAL_I2C_Master_Transmit (uint16 DevAddress, 
						 uint8 *pData, 
						 uint16 Size, 
						 uint32 Timeout);
 	Transmits in master mode an amount of data in blocking mode. 
*/

const char *I2C_ERROR_MESSAGE[] = {
	"I2C_1_MSTR_NO_ERROR", //           (0x00u) /* Function complete without error */
    "I2C_1_MSTR_BUS_BUSY", //           (0x01u) /* Bus is busy, process not started */
    "I2C_1_MSTR_NOT_READY", //          (0x02u) /* Master not Master on the bus or */
    "I2C_1_MSTR_ERR_LB_NAK", //         (0x03u) /* Last Byte Naked */
    "I2C_1_MSTR_ERR_ARB_LOST", //       (0x04u) /* Master lost arbitration during communication */
    "I2C_1_MSTR_ERR_ABORT_START_GEN", //  (0x05u) /* Master did not generate Start, the Slave */
};

void PCF8575_Init(uint8 address) {
	DBG_PRINTF("[%s] Address:0x%2X \n",__func__,address);
	
	isr_pcf8575_StartEx(PCF8575_INPUT_INTERRUPT);

	isr_pcf8575_ClearPending();
    isr_pcf8575_Enable();

	PCF8575_Write(PCF8575_INPUT_PORT_CFG); // prima volta mette a 1 le porte in ingresso
}

void PCF8575_ButtonManager(uint8 *input) {
	static uint16 tastoAttivo[MAX_TASTI] = {0,0,0,0};
	
	if ((input[1] & BIT7) == 0) { 
		DBG_PRINTF("tasto 1 premuto\n");
		tastoAttivo[0]++;
	} else {
		if(tastoAttivo[0]){
			tastoAttivo[0] = 0;
			DBG_PRINTF("tasto 1 rilasciato\n");
		}
	}

	if ((input[1] & BIT6) == 0) { 
		DBG_PRINTF("tasto 2 premuto\n");
		tastoAttivo[1]++;
	} else {
		if(tastoAttivo[1]){
			tastoAttivo[1] = 0;
			DBG_PRINTF("tasto 2 rilasciato\n");
		}
	}
		
	if ((input[1] & BIT5) == 0) { 
		DBG_PRINTF("tasto 3 premuto\n");
		tastoAttivo[2]++;
	} else {
		if(tastoAttivo[2]){
			tastoAttivo[2] = 0;
			DBG_PRINTF("tasto 3 rilasciato\n");
		}
	}
			
	if ((input[1] & BIT4) == 0) { 
		DBG_PRINTF("tasto 4 premuto\n");
		tastoAttivo[3]++;
	} else {
		if(tastoAttivo[3]){
			tastoAttivo[3] = 0;
			DBG_PRINTF("tasto 4 rilasciato\n");
		}
	}
}

uint16 PCF8575_Read(void) {
	uint8 input[2] = {0,0};

	HAL_I2C_Master_Receive(PCF8575_I2C_ADDRESS, input, 2, 1000);	
	
	PCF8575_ButtonManager(input);
				
	// DBG_PRINTF("[%s]  0x%02X 0x%02X\n",__func__,input[1],input[0]);
	return ((input[1] << 8) | input[0]);
}

uint8 PCF8575_Write(uint16 data) {
	uint8 retStatus = 0;
	uint8 output[2];
	// static uint16 lastData = 0;
	
	output[0] = PCF8575_INPUT_PORT_CFG | data; // P07 to P00.
	// i 4 led devono esssere pilotati invertiti quindi basta fare (data ^ 0x0F00) Xor inverte il pilotaggio dei 4 led verdi
	output[1] = (PCF8575_INPUT_PORT_CFG | (data ^ STATUS_LED_MASK)) >> 8; // P17 to P10
	  
	retStatus = HAL_I2C_Master_Transmit(PCF8575_I2C_ADDRESS, output, 2, 1000);
	if (retStatus != I2C_LCD_MSTR_NO_ERROR) { 
		// scrive che c-[ stato un errore
		DBG_PRINTF("[%s] address: 0x%02X, \t%s\n",__func__,PCF8575_I2C_ADDRESS,I2C_ERROR_MESSAGE[retStatus]);
	}
	return retStatus;
}


uint8 HAL_I2C_Master_Receive(uint8 devAddress, 
						 uint8 *pData, 
						 uint16 Size, 
						 uint32 Timeout) {
	uint8 retStatus = 0;
	uint8 i = 0;
	uint8 acknNak = 0;
							
	Timeout = Timeout;
							
	retStatus = I2C_LCD_MasterSendStart(devAddress,1);	// 0 write, 1 read
		
	if (retStatus == I2C_LCD_MSTR_NO_ERROR) {
		for(i=0;i<Size;i++) {
			acknNak = I2C_LCD_ACK_DATA;
			if ((i+1) == Size) {
				acknNak = I2C_LCD_NAK_DATA;
			}
			
			pData[i] = I2C_LCD_MasterReadByte(acknNak);
		}
	} 
	
	retStatus = I2C_LCD_MasterSendStop();
	return retStatus;						
							
}


uint8 HAL_I2C_Master_Transmit (uint8 devAddress, 
						 uint8 *pData, 
						 uint16 Size, 
						 uint32 Timeout) {
	uint8 retStatus = 0;				
	uint8 i = 0;
							
	Timeout	= Timeout;
		
	retStatus = I2C_LCD_MasterSendStart(devAddress,0);	// 0 write, 1 read
		
	if (retStatus == I2C_LCD_MSTR_NO_ERROR) {
		for(i=0;i<Size;i++) {
			retStatus = I2C_LCD_MasterWriteByte(pData[i]);
			if (retStatus != I2C_LCD_MSTR_NO_ERROR) {
			break;
			}
		}
	} 
	
	retStatus = I2C_LCD_MasterSendStop();
	return retStatus;
}

uint8 HAL_I2C_PCF8575_Address_Finder (void) {
	
	uint8 retStatus = 0;				
    uint8 address = 0;
	// uint8 pData[2] = {0xff,0xff};
	
	for (address=0;address<255;address++) 
	{
		retStatus = I2C_LCD_MasterSendStart(address,0);
		
		if (retStatus == I2C_LCD_MSTR_NO_ERROR) {
			DBG_PRINTF("[%s] address: 0x%02X, \t%s\n",__func__,address,I2C_ERROR_MESSAGE[retStatus]);
			// I2C_1_MasterWriteByte(pData[0]);
			// I2C_1_MasterWriteByte(pData[1]);
		} else {
			// DBG_PRINTF("[%s]> address: 0x%02X, \t%d\n",__func__,address,status);
		}
		
		retStatus = I2C_LCD_MasterSendStop();
		
		CyDelay(10);
	}	
	
	DBG_PRINTF("[%s] done \t%s\n",__func__,I2C_ERROR_MESSAGE[retStatus]);
	printStarLine();
	return retStatus;
}
					
void PCF8575_ApplicationPoll(void){
	static uint8 isInitialized = 0;
	static uint8 portCounter = 0;
	static uint8 applicationDivider = 0;
	
	if(isInitialized == 0) {
		I2C_LCD_Start();
		HAL_I2C_PCF8575_Address_Finder();
			
		PCF8575_Init(PCF8575_I2C_ADDRESS);
		
		LiquidCrystal_I2C_init(16,2,0);
		isInitialized = 1;
	}
	
	if(tick_100ms(TICK_PCF8575)) {
		applicationDivider++; // rallenta
		if(applicationDivider == 10) {
			applicationDivider = 0;
			
			// PCF8575_Write(BIT0 << portCounter); // scrive un'uscita alla volta
			
			LCD_PrintString("dai che stavolta funziona");
			
			portCounter++;	// incrementa l'usicta da scrivere
			if (portCounter == 12){
				portCounter = 0;
			}
		}
	}

	// PCF8575_Write(0xFFFF);
	// CyDelay(100);
		
	// PCF8575_Write(0x0000);
	// CyDelay(100);
	
	if(PCF8575_Read_Input_flag) {
		// PCF8575_Read();
		PCF8575_Read_Input_flag = 0;
	}
}