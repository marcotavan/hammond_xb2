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
 
#ifndef __PCF8575_H__
#define __PCF8575_H__

 /* Includes */
#include "project.h"
 
/* I2C Address*/
#define PCF8575_I2C_ADDRESS 	0x20
#define PCF8575_INPUT_PORT_CFG	0x0000
// #define MAX_TASTI				4
#define STATUS_LED_MASK 		0x0F00

/*******************************************************************************
* @brief  	Initializes PCF8575 write and read address
********************************************************************************
* @param  	readAddress: PCF8575 read address
* @param	writeAddress: PCF8575 write address
* @retval	Nothing
*******************************************************************************/
void PCF8575_Init(uint8 address );
 
/*******************************************************************************
* @brief  	Reads pins
********************************************************************************
* @param  	Nothing
* @retval	16bit data
*******************************************************************************/
uint16 PCF8575_Read(void );
 
/*******************************************************************************
* @brief  	Setting pins as input or output
********************************************************************************
* @param  	Nothing
* @param  	data: value of pins 
* @retval	Nothing
*******************************************************************************/
uint8 PCF8575_Write(uint16 data);

/*******************************************************************************
* @brief  	Application Poll
********************************************************************************
* @param  	Nothing
* @retval	Nothing
*******************************************************************************/
void PCF8575_ApplicationPoll(void);

/*******************************************************************************
* @brief  	HAL_I2C_Master_Receive
********************************************************************************
* @param  	list
* @retval	status
*******************************************************************************/
uint8 HAL_I2C_Master_Receive(uint8 devAddress, 
						 uint8 *pData, 
						 uint16 Size, 
						 uint32 Timeout);

/*******************************************************************************
* @brief  	HAL_I2C_Master_Transmit
********************************************************************************
* @param  	list
* @retval	status
*******************************************************************************/
uint8 HAL_I2C_Master_Transmit (uint8 DevAddress, 
						 uint8 *pData, 
						 uint16 Size, 
						 uint32 Timeout);


// addres finder
uint8 HAL_I2C_PCF8575_Address_Finder (void);

#endif // __PCF8575_H__