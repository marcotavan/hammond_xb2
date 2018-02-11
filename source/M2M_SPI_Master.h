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
#ifndef M2M_SPI_MASTER_H
#define M2M_SPI_MASTER_H
    
#include "cytypes.h"
	
enum fontType_e {
	LCD_STANDARD,
	LCD_BARGRAPHS
};

void M2M_SPI_Master_ApplicationPoll(void);
void M2M_SPI_Init(void);
void Write_BarGraphs(uint8 *data);
uint8 M2MSpiWriteData(uint8 address, uint8 *data, uint8 len);
void M2M_Write_LCD(uint8 position, uint8 type, uint8 *data);
void ResetBarGraphs();
extern uint8 lockBargraphs;
#endif 
    /* [] END OF FILE */
