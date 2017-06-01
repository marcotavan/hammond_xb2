/* ========================================
 *
 * Copyright MArco Tavan, 23-05-2017
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef pca9685_Driver_H
#define pca9685_Driver_H

#include <project.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
/* BOARD SPECIFIC configuration data */
#define PCA9685_I2C_BASE_ADDRESS    0x40
#define Mode_register_1 (PCA9685_MODE_RESTART | PCA9685_MODE_AUTOINC)
#define Mode_register_2 (PCA9685_MODE2_OUTDRV_TOTEM_POLE /*| PCA9685_MODE2_INVRT*/) 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
// Register addresses from data sheet
#define PCA9685_MODE1_REG           0x00
#define PCA9685_MODE2_REG           0x01
#define PCA9685_SUBADR1_REG         0x02
#define PCA9685_SUBADR2_REG         0x03
#define PCA9685_SUBADR3_REG         0x04
#define PCA9685_ALLCALL_REG         0x05
#define PCA9685_LED0_REG            0x06 // Start of LEDx regs, 4B per reg, 2B on phase, 2B off phase, little-endian
#define PCA9685_PRESCALE_REG        0xFE
#define PCA9685_ALLLED_REG          0xFA

// Mode1 register pin layout
#define PCA9685_MODE_RESTART        0x80
#define PCA9685_MODE_EXTCLK         0x40
#define PCA9685_MODE_AUTOINC        0x20
#define PCA9685_MODE_SLEEP          0x10
#define PCA9685_MODE_SUBADR1        0x08
#define PCA9685_MODE_SUBADR2        0x04
#define PCA9685_MODE_SUBADR3        0x02
#define PCA9685_MODE_ALLCALL        0x01

#define PCA9685_SW_RESET            0x06 // Sent to address 0x00 to reset all devices on Wire line
#define PCA9685_PWM_FULL            0x01000 // Special value for full on/full off LEDx modes
#define PCA9685_FULL				0xFFFF
#define PCA9685_MID					0x0FFF
	
#define PCA9685_MODE2_OUTDRV_TOTEM_POLE 0x04
#define PCA9685_MODE2_INVRT			 	0x10
	
// To balance the load out in a weaved fashion, we use this offset table to distribute
// the load on the outputs in a more interleaving fashion than just a simple 16 offset
// per channel. We can set the off cycle value to be lower than the on cycle, which will
// put the high edge across the 0-4095 phase cycle range, which is supported by device.
static uint16_t phaseDistTable[16] = { 0, 2048, 1024, 3072, 512, 3584, 1536, 2560, 256, 3840, 1280, 2304, 3328, 768, 2816, 1792 };
    
typedef enum {
    PCA9685_PhaseBalancer_None = -1,    // Disables phase balancing, all high phase areas start at begining of cycle
    PCA9685_PhaseBalancer_Linear = 0,   // Balances all outputs linearly, 256 steps away from previous output
    PCA9685_PhaseBalancer_Weaved,       // Balances first few outputs better, steps away from previous shorten towards last output
    PCA9685_PhaseBalancer_Count
} PCA9685_PhaseBalancer;

enum i2cErrors {
	I2C_ERROR_0,
	I2C_ERROR_1,
	I2C_ERROR_2,
	I2C_ERROR_3,
	I2C_ERROR_4,
	I2C_ERROR_5,
	I2C_ERROR_6,
	I2C_ERROR_7,
	I2C_ERROR_8,
	I2C_ERROR_9,
	I2C_ERROR_10,
	I2C_ERROR_11,
	I2C_ERROR_12,
	I2C_ERROR_13,
	I2C_ERROR_14,
	I2C_ERROR_15,
	I2C_ERROR_16,
	I2C_ERROR_17,
	I2C_ERROR_18,
	I2C_ERROR_19,
	I2C_ERROR_20,
};

void PCA9685_init(uint8 i2cAddress, uint8 mode1, uint8 mode2);
void PCA9685_reset(void);
void PCA9685_setPWMFrequency(uint16 pwmFrequency);
void PCA9685_setChannelPWM(int channel, uint16_t pwmAmount);

void PWM_setPWM(uint8_t num, uint16_t on, uint16_t off);
void PWM_setPWMFreq(float freq);
void PCA9685_setAllChannelsPWM(uint16_t pwmAmount);
void test_pwm(void);
void LedPoll(void);

#define ROSSO_LESLIE_FAST 0
#define VERDE_LESLIE_FAST 1
#define ROSSO_VIBRATO_ON 2
#define VERDE_VIBRATO_ON 3
#define ROSSO_THIRD_PERCUSSION 4
#define VERDE_THIRD_PERCUSSION 5
#define ROSSO_SECOND_PERCUSSION 6
#define VERDE_SECOND_PERCUSSION 7
#define ROSSO_ORGAN_SOLO 8
#define VERDE_ORGAN_SOLO 9




#endif
/* EOF */