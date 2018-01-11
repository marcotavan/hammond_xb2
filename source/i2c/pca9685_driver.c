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

uso:
int main()
{   
    CyGlobalIntEnable;
	Debug_Log_Init();
	PCA9685_init(0,Mode_register_1,Mode_register_2);
    PCA9685_setPWMFrequency(1600);
	for(;;) {
		test_pwm();	
	}
    return 0;
}
*/

#include <pca9685_driver.h>
#include <math.h>
#include "debug.h"
#include "tick.h"
#include "common.h" 
#include "ButtonScanner.h"
#include "stdlib.h"
// #define PCA9685_ENABLE_DEBUG_OUTPUT

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static uint8 pca9685_i2cAddress = PCA9685_I2C_BASE_ADDRESS;
static uint8 pca9685_lastI2CError = 0;         // Last i2c error
PCA9685_PhaseBalancer _phaseBalancer; // Phase balancer scheme to distribute load

#define VERBOSE_DEBUG 1

uint8 lowByte(uint16 data) {
	// verificare
	return (data & 0xFF);
}

uint8 highByte(uint16 data) {
	// verificare
	return (data >> 8) & 0xFF;
}

uint8 PCA9685_writeChannelBegin(int channel) {
	// verificare
    uint8 regAddress;
	uint8 i2cStatus = 0;
	
    if (channel != -1) {
        regAddress = PCA9685_LED0_REG + (channel * 0x04);
	} else {
        regAddress = PCA9685_ALLLED_REG;
	}
	
#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s] channel: %d, regAddress: 0x%02X\n\n",__func__,channel,regAddress);
#endif
	
    if ((i2cStatus = I2C_LED_MasterSendStart(pca9685_i2cAddress, I2C_LED_WRITE_XFER_MODE))) {
		// DBG_PRINTF("[%s] i2cStatus :%d\n",__func__,i2cStatus);
		pca9685_lastI2CError = I2C_ERROR_1;
		return i2cStatus;
	}
	
    I2C_LED_MasterWriteByte(regAddress);
	return i2cStatus;
}

void PCA9685_writeChannelPWM(uint16_t phaseBegin, uint16_t phaseEnd) {
	// verificare
#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s] phaseBegin: %d, phaseEnd: %d\n",__func__,phaseBegin,phaseEnd);
#endif

    I2C_LED_MasterWriteByte(lowByte(phaseBegin));
    I2C_LED_MasterWriteByte(highByte(phaseBegin));
    I2C_LED_MasterWriteByte(lowByte(phaseEnd));
    I2C_LED_MasterWriteByte(highByte(phaseEnd));
}

uint8 PCA9685_getLastI2CError() {
    return pca9685_lastI2CError;
}


#ifdef PCA9685_ENABLE_DEBUG_OUTPUT

static const char *textForI2CError(uint8 errorCode) {
    switch (errorCode) {
    case 0:
        return "Success";
    case 1:
        return "Data too long to fit in transmit buffer";
    case 2:
        return "Received NACK on transmit of address";
    case 3:
        return "Received NACK on transmit of data";
    default:
        return "Other error";
    }
}

void PCA9685_checkForErrors() {
    if (pca9685_lastI2CError) {
        DBG_PRINTF("[%s] lastI2CError: %d ",__func__,pca9685_lastI2CError);
        DBG_PRINT_LINE(textForI2CError(PCA9685_getLastI2CError()));
    }
}

#endif

void PCA9685_writeChannelEnd() {
	// verificare
    I2C_LED_MasterSendStop();

#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    PCA9685_checkForErrors();
#endif
}


uint8 PCA9685_writeRegister(uint8_t reg, uint8_t data) {
	// verificare
	// DBG_PRINTF("[%s]: 0x%02X 0x%02X\n",__func__,reg,data);
	uint8 i2cStatus = 0;
	
  	if ((i2cStatus = I2C_LED_MasterSendStart(pca9685_i2cAddress, I2C_LED_WRITE_XFER_MODE))) {
		// DBG_PRINTF("[%s] i2cStatus :%d\n",__func__,i2cStatus);
		pca9685_lastI2CError = I2C_ERROR_2;
		return i2cStatus;
	}
	
	if ((i2cStatus = I2C_LED_MasterWriteByte(reg))) {
		// DBG_PRINTF("2 i2cStatus :%d\n",i2cStatus);
		pca9685_lastI2CError = I2C_ERROR_3;
		return i2cStatus;
	}
	
  	if ((i2cStatus = I2C_LED_MasterWriteByte(data))) {
		// DBG_PRINTF("3 i2cStatus :%d\n",i2cStatus);
		pca9685_lastI2CError = I2C_ERROR_4;
		return i2cStatus;
	}
	
  	if ((i2cStatus = I2C_LED_MasterSendStop())) {
		// DBG_PRINTF("4 i2cStatus :%d\n",i2cStatus);
		pca9685_lastI2CError = I2C_ERROR_5;
		return i2cStatus;
	}
	
	#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    PCA9685_checkForErrors();
	#endif
	
	return i2cStatus;
}


uint8_t PCA9685_readRegister(uint8_t reg) {
  	// verificare
	#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s]: reg=0x%02X, ",__func__,reg);
	#endif
	uint8 val = 0x00;
  	if(I2C_LED_MasterSendStart(pca9685_i2cAddress, I2C_LED_WRITE_XFER_MODE)) {
		pca9685_lastI2CError = I2C_ERROR_6;
		return val;
	}
	
  	I2C_LED_MasterWriteByte(reg);
  	I2C_LED_MasterSendRestart(pca9685_i2cAddress, I2C_LED_READ_XFER_MODE);

  	val =  I2C_LED_MasterReadByte(I2C_LED_NAK_DATA);
  	I2C_LED_MasterSendStop();
	#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("val=0x%02X\n",val);
	#endif
	
  	return val;
}

/* 7.6   Software reset
	The Software Reset Call (SWRST Call)  allows all the devices in the I2C-bus to be reset to 
	the power-up state value through a specific formatted I2C-bus command. To be performed 
	correctly, it implies that the I2C-bus is functional and that there is no device hanging the bus.
*/
uint8 PCA9685_SoftwareReset(void) {
	// verificare
	uint8 i2cStatus = 0;
	
	if((i2cStatus = I2C_LED_MasterSendStart(pca9685_i2cAddress, I2C_LED_WRITE_XFER_MODE))){
		DBG_PRINTF("1 i2cStatus :%d\n",i2cStatus);
		pca9685_lastI2CError = I2C_ERROR_7;
		I2C_LED_MasterSendStop();
		return i2cStatus;
	}
	
	
  	I2C_LED_MasterWriteByte(PCA9685_SW_RESET);
	I2C_LED_MasterSendStop();
	
	return i2cStatus;
}


void PCA9685_init(uint8 i2cAddress, uint8 mode1, uint8 mode2) {
	// verificare
	
	pca9685_i2cAddress = PCA9685_I2C_BASE_ADDRESS | (i2cAddress & 0x3F);
	_phaseBalancer = PCA9685_PhaseBalancer_None;
	
	DBG_PRINTF("[%s]  pca9685_i2cAddress:0x%02X\n",__func__,pca9685_i2cAddress);
	
	I2C_LED_Start();
	PCA9685_SoftwareReset();
    PCA9685_writeRegister(PCA9685_MODE1_REG, mode1);
    PCA9685_writeRegister(PCA9685_MODE2_REG, mode2);
	PCA9685_setAllChannelsPWM(0);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PCA9685_setPWMFrequency(uint16 pwmFrequency) {
	// verificare
    // This equation comes from section 7.3.5 of the datasheet, but the rounding has been
    // removed because it isn't needed. Lowest freq is 23.84, highest is 1525.88.
    uint16 preScalerVal = (25000000 / (4096 * pwmFrequency)) - 1;
    if (preScalerVal > 255) preScalerVal = 255;
    if (preScalerVal < 3) preScalerVal = 3;

//#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s] pwmFrequency: %d, preScalerVal: 0x%02X\n",__func__,pwmFrequency,preScalerVal);
//#endif

    // The PRE_SCALE register can only be set when the SLEEP bit of MODE1 register is set to logic 1.
    uint8 mode1Reg = PCA9685_readRegister(PCA9685_MODE1_REG);
    PCA9685_writeRegister(PCA9685_MODE1_REG, (mode1Reg = (mode1Reg & ~PCA9685_MODE_RESTART) | PCA9685_MODE_SLEEP));
    PCA9685_writeRegister(PCA9685_PRESCALE_REG, (uint8)preScalerVal);

    // It takes 500us max for the oscillator to be up and running once SLEEP bit has been set to logic 0.
    PCA9685_writeRegister(PCA9685_MODE1_REG, (mode1Reg = (mode1Reg & ~PCA9685_MODE_SLEEP) | PCA9685_MODE_RESTART));
    CyDelayUs(500);
}



void PCA9685_setChannelOn(int channel) {
    if (channel < 0 || channel > 15) return;

#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s]\n",__func__);
#endif

    PCA9685_writeChannelBegin(channel);
    PCA9685_writeChannelPWM(PCA9685_PWM_FULL, 0);  // time_on = FULL; time_off = 0;
    PCA9685_writeChannelEnd();
}

void PCA9685_setChannelOff(int channel) {
    if (channel < 0 || channel > 15) return;

#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s]\n",__func__);
#endif

    PCA9685_writeChannelBegin(channel);
    PCA9685_writeChannelPWM(0, PCA9685_PWM_FULL);  // time_on = 0; time_off = FULL;
    PCA9685_writeChannelEnd();
}

void PCA9685_getPhaseCycle(int channel, uint16_t pwmAmount, uint16_t *phaseBegin, uint16_t *phaseEnd) {
    // Set delay
    if (channel < 0) {
        // All channels
        *phaseBegin = 0;
    }
    else if (_phaseBalancer == PCA9685_PhaseBalancer_Linear) {
        // Distribute high phase area over entire phase range to balance load.
        *phaseBegin = channel * (4096 / 16);
    }
    else if (_phaseBalancer == PCA9685_PhaseBalancer_Weaved) {
        // Distribute high phase area over entire phase range to balance load.
        *phaseBegin = phaseDistTable[channel];
    }
    else {
        *phaseBegin = 0;
    }
    
    // See datasheet section 7.3.3
    if (pwmAmount == 0) {
        // DBG_PRINTF(" Full OFF => time_off[12] = 1;\n");
        *phaseEnd = PCA9685_PWM_FULL;
    }
    else if (pwmAmount >= PCA9685_PWM_FULL) {
        // DBG_PRINTF(" Full ON => time_on[12] = 1; time_off = ignored;\n");
        *phaseBegin |= PCA9685_PWM_FULL;
        *phaseEnd = 0;
    }
    else {
        *phaseEnd = *phaseBegin + pwmAmount;
        if (*phaseEnd >= PCA9685_PWM_FULL)
            *phaseEnd -= PCA9685_PWM_FULL;
		
		// DBG_PRINTF(" phaseEnd %d, phaseBegin %d\n",*phaseEnd,*phaseBegin);
    }
}


void PCA9685_setChannelPWM(int channel, uint16_t pwmAmount) {
    if (channel < 0 || channel > 15) return;

#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
    DBG_PRINTF("[%s]\n",__func__);
#endif

    PCA9685_writeChannelBegin(channel);

    uint16_t phaseBegin, phaseEnd;
    PCA9685_getPhaseCycle(channel, pwmAmount, &phaseBegin, &phaseEnd);

    PCA9685_writeChannelPWM(phaseBegin, phaseEnd);

    PCA9685_writeChannelEnd();
}


void PCA9685_setAllChannelsPWM(uint16_t pwmAmount) {

    PCA9685_writeChannelBegin(-1); // Special value for ALLLED registers

    uint16_t phaseBegin, phaseEnd;
    PCA9685_getPhaseCycle(-1, pwmAmount, &phaseBegin, &phaseEnd);

    PCA9685_writeChannelPWM(phaseBegin, phaseEnd);

   	PCA9685_writeChannelEnd();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
	#if 0
	void PCA9685_printModuleInfo() {
	    DBG_PRINT_LINE(""); 
		DBG_PRINT_LINE(" ~~~ PCA9685 Module Info ~~~");

	    DBG_PRINT_LINE(""); 
		DBG_PRINTF("i2c Address: 0x%02X\n",pca9685_i2cAddress);

	    DBG_PRINT_LINE(""); DBG_PRINT_LINE("Phase Balancer:");
	    switch (_phaseBalancer) {
	        case PCA9685_PhaseBalancer_None:
	            DBG_PRINT_LINE("PCA9685_PhaseBalancer_None"); break;
	        case PCA9685_PhaseBalancer_Linear:
	            DBG_PRINT_LINE("PCA9685_PhaseBalancer_Linear"); break;
	        case PCA9685_PhaseBalancer_Weaved:
	            DBG_PRINT_LINE("PCA9685_PhaseBalancer_Weaved"); break;
	        default:
	            DBG_PRINT_LINE(""); break;
	    }

	    if (!_isProxyAddresser) {
	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("Proxy Addresser:");
	        DBG_PRINT_LINE("false");

	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("Mode1 Register:");
	        uint8 mode1Reg = PCA9685_readRegister(PCA9685_MODE1_REG);
	        DBG_PRINT_TEXT("0x");
	        DBG_PRINT_TEXT(mode1Reg, HEX);
	        DBG_PRINT_TEXT(", Bitset:");
	        if (mode1Reg & PCA9685_MODE_RESTART)
	            DBG_PRINT_TEXT(" PCA9685_MODE_RESTART");
	        if (mode1Reg & PCA9685_MODE_EXTCLK)
	            DBG_PRINT_TEXT(" PCA9685_MODE_EXTCLK");
	        if (mode1Reg & PCA9685_MODE_AUTOINC)
	            DBG_PRINT_TEXT(" PCA9685_MODE_AUTOINC");
	        if (mode1Reg & PCA9685_MODE_SLEEP)
	            DBG_PRINT_TEXT(" PCA9685_MODE_SLEEP");
	        if (mode1Reg & PCA9685_MODE_SUBADR1)
	            DBG_PRINT_TEXT(" PCA9685_MODE_SUBADR1");
	        if (mode1Reg & PCA9685_MODE_SUBADR2)
	            DBG_PRINT_TEXT(" PCA9685_MODE_SUBADR2");
	        if (mode1Reg & PCA9685_MODE_SUBADR3)
	            DBG_PRINT_TEXT(" PCA9685_MODE_SUBADR3");
	        if (mode1Reg & PCA9685_MODE_ALLCALL)
	            DBG_PRINT_TEXT(" PCA9685_MODE_ALLCALL");
	        DBG_PRINT_LINE("");

	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("Mode2 Register:");
	        uint8 mode2Reg = readRegister(PCA9685_MODE2_REG);
	        DBG_PRINT_TEXT("0x");
	        DBG_PRINT_TEXT(mode2Reg, HEX);
	        DBG_PRINT_TEXT(", Bitset:");
	        if (mode2Reg & PCA9685_MODE_INVRT)
	            DBG_PRINT_TEXT(" PCA9685_MODE_INVRT");
	        if (mode2Reg & PCA9685_MODE_OUTPUT_ONACK)
	            DBG_PRINT_TEXT(" PCA9685_MODE_OUTPUT_ONACK");
	        if (mode2Reg & PCA9685_MODE_OUTPUT_TPOLE)
	            DBG_PRINT_TEXT(" PCA9685_MODE_OUTPUT_TPOLE");
	        if (mode2Reg & PCA9685_MODE_OUTNE_HIGHZ)
	            DBG_PRINT_TEXT(" PCA9685_MODE_OUTNE_HIGHZ");
	        if (mode2Reg & PCA9685_MODE_OUTNE_LOW)
	            DBG_PRINT_TEXT(" PCA9685_MODE_OUTNE_LOW");
	        DBG_PRINT_LINE("");

	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("SubAddress1 Register:");
	        uint8 subAdr1Reg = readRegister(PCA9685_SUBADR1_REG);
	        DBG_PRINT_TEXT("0x");
	        DBG_PRINT_LINE(subAdr1Reg, HEX);

	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("SubAddress2 Register:");
	        uint8 subAdr2Reg = readRegister(PCA9685_SUBADR2_REG);
	        DBG_PRINT_TEXT("0x");
	        DBG_PRINT_LINE(subAdr2Reg, HEX);

	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("SubAddress3 Register:");
	        uint8 subAdr3Reg = readRegister(PCA9685_SUBADR3_REG);
	        DBG_PRINT_TEXT("0x");
	        DBG_PRINT_LINE(subAdr3Reg, HEX);

	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("AllCall Register:");
	        uint8 allCallReg = readRegister(PCA9685_ALLCALL_REG);
	        DBG_PRINT_TEXT("0x");
	        DBG_PRINT_LINE(allCallReg, HEX);
	    }
	    else {
	        DBG_PRINT_LINE(""); DBG_PRINT_LINE("Proxy Addresser:");
	        DBG_PRINT_LINE("true");
	    }
	}
	#endif
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// Sets pin without having to deal with on/off tick placement and properly handles
// a zero value as completely off.  Optional invert parameter supports inverting
// the pulse for sinking to ground.  Val should be a value from 0 to 4095 inclusive.
void PWM_setPin(uint8_t num, uint16_t val)
{
  // Clamp value between 0 and 4095 inclusive.
    val = MIN(val, 4095);
    if (val == 0) {
      // Special value for signal fully on.
      PWM_setPWM(num, 4096, 0);
    }
    else if (val == 4095) {
      // Special value for signal fully off.
      PWM_setPWM(num, 0, 4096);
    }
    else {
      PWM_setPWM(num, 0, 4095-val);
    }
}


uint8 menuLed(void) {
	// TODO da completare
	return 1;
}

#define MAX_REFRESH_TIMEOUT 50
#define FAST_BLINK	BIT0
#define MID_BLINK	BIT1
#define SLOW_BLINK	BIT2

uint8 BlinkTime(uint8 blink, uint8 vel) {
	if(blink & vel) return 1;
	else return 0;
}

void LeslieFastButtonLed(uint8 blink) {
	static uint8 prev = 1;
	static uint8 refresh = 0;
	static uint8 oneShot = 0;
	
	if(GetEditMode() == EDIT_MODE_ON) {
		if(GetEditFunction() == BUTTON_01_LESLIE) {
			
			if(BlinkTime(blink, MID_BLINK)) {
				if(oneShot) {
					LED_ROSSO_LESLIE;
					oneShot = 0; 
				}
			} else {
				if(!oneShot) {
					LED_LESLIE_OFF;
					oneShot = 1; 
				}
			}
			 
			refresh = 0;
			return;
		}
	}
	
	if(refresh) refresh--;		
	
	if(prev != switchType.rotarySpeaker_HalfMoon || refresh == 0) {
		switch(switchType.rotarySpeaker_HalfMoon) {
			case ROTARY_SLOW:
				LED_VERDE_LESLIE;
				break;
			
			case ROTARY_STOP:
				LED_LESLIE_OFF; // LED_ROSSO_LESLIE
				break;
			
			case ROTARY_FAST:
				LED_ROSSO_LESLIE;
				break;
		}
		
		prev = switchType.rotarySpeaker_HalfMoon;
		refresh = MAX_REFRESH_TIMEOUT + rand()%5;
		
		#if VERBOSE_DEBUG
		DBG_PRINTF("%s next refresh in %dms\n",__func__,refresh*100);
		#endif
	}
}
	
void VibratoOnButtonLed(uint8 blink) {
	static uint8 prev = 1;
	static uint8 refresh = 0;
	static uint8 oneShot = 0;

	if(GetEditMode() == EDIT_MODE_ON) {
		if(GetEditFunction() == BUTTON_00_VIBRATO) {
			if(BlinkTime(blink, MID_BLINK)) {
				if(oneShot) {
					LED_ROSSO_VIBRATO;
					oneShot = 0;
				}
			} else {
				if(!oneShot) {
					LED_VIBRATO_OFF;
					oneShot = 1;
				}
			}

			refresh = 0;
			return;
		}
	} 
	
	if(refresh) refresh--;	
	
	if(GetButtonStatus(BUTTON_00_VIBRATO) == BUTTON_ON_HOLD) {
		if(BlinkTime(blink, MID_BLINK)) {
			if(oneShot) {
				switch(switchType.Vibrato_Lower_Switch) {
					case SWITCH_ON:
						LED_GIALLO_VIBRATO;
						break;
					
					case SWITCH_OFF:
						LED_ROSSO_VIBRATO;
						break;
				}
				oneShot = 0;
			}
		} else {
			if(!oneShot) {
				LED_VIBRATO_OFF;
				oneShot = 1;
			}
		}
		refresh = 0; // al rilascio risistema il led
	} else {
		if(prev != switchType.Vibrato_Upper_Switch || refresh == 0) {
			switch(switchType.Vibrato_Upper_Switch) {
				case SWITCH_ON:
					LED_VERDE_VIBRATO;
					break;
				
				case SWITCH_OFF:
					LED_VIBRATO_OFF; // LED_ROSSO_LESLIE
					break;
			}
			prev = switchType.Vibrato_Upper_Switch;
			refresh = MAX_REFRESH_TIMEOUT + rand()%5;
			#if VERBOSE_DEBUG
			DBG_PRINTF("%s next refresh in %dms\n",__func__,refresh*100);
			#endif
		}
	}
}	

void Percussion3rdButtonLed(uint8 blink) {
	static uint8 prev = 1;
	static uint8 refresh = 0;
	static uint8 oneShot = 0;
	
	if(GetEditMode() == EDIT_MODE_ON) {
		if(GetEditFunction() == BUTTON_04_PERC_3RD) {
			if(BlinkTime(blink, MID_BLINK)) {
				if(oneShot) {
					LED_ROSSO_3RD_PERCUSSION;
					oneShot = 0;
				}
			} else {
				if(oneShot == 0) {
					LED_3RD_PERCUSSION_OFF;
					oneShot = 1;
				}	
			}
			 
			refresh = 0;
			return;
		}
	}
	
	if(refresh) refresh--;	
	
	if(GetButtonStatus(BUTTON_04_PERC_3RD) == BUTTON_ON_HOLD) {
		if(BlinkTime(blink, MID_BLINK)) {
			switch(switchType.percussionLevel_Switch) {
				case PERC_SOFT:
					LED_ROSSO_3RD_PERCUSSION;
					break;
				
				case PERC_NORM:
					LED_VERDE_3RD_PERCUSSION;
					break;
			}
		} else {
			LED_3RD_PERCUSSION_OFF;
		}
		refresh = 0; // al rilascio risistema il led
	} else {
		if(prev != switchType.percussionHarmonics_Switch || refresh == 0) {
			switch(switchType.percussionHarmonics_Switch) {
				case PERC_3RD:
					LED_GIALLO_3RD_PERCUSSION;
					break;
				
				default:
					LED_3RD_PERCUSSION_OFF; 
					break;
			}
			prev = switchType.percussionHarmonics_Switch;
			refresh = MAX_REFRESH_TIMEOUT + rand()%5;
			#if VERBOSE_DEBUG
			DBG_PRINTF("%s next refresh in %dms\n",__func__,refresh*100);
			#endif
		}
	}
}

void Percussion2ndButtonLed(uint8 blink){
	static uint8 prev = 1;
	static uint8 refresh = 0;

	if(GetEditMode() == EDIT_MODE_ON) {
		if(GetEditFunction() == BUTTON_08_PERC_2ND) {
			
			if(BlinkTime(blink, MID_BLINK)) {
				LED_ROSSO_2ND_PERCUSSION;
			} else {
				LED_2ND_PERCUSSION_OFF;
			}
			 
			refresh = 0;
			return;
		}
	}

	if(refresh) refresh--;	
	
	if(GetButtonStatus(BUTTON_08_PERC_2ND) == BUTTON_ON_HOLD) {
		if(BlinkTime(blink, MID_BLINK)) {
			switch(switchType.percussionDecay_Switch) {
				case PERC_FAST:
					LED_ROSSO_2ND_PERCUSSION;
					break;
				
				case PERC_SLOW:
					LED_VERDE_2ND_PERCUSSION;
					break;
			}
		} else {
			LED_2ND_PERCUSSION_OFF;
		}
		refresh = 0; // al rilascio risistema il led
	} else {
		if(prev != switchType.percussionHarmonics_Switch || refresh == 0) {
			switch(switchType.percussionHarmonics_Switch) {
				case PERC_2ND:
					LED_GIALLO_2ND_PERCUSSION;
					break;
				
				default:
					LED_2ND_PERCUSSION_OFF; 
					break;
			}
			prev = switchType.percussionHarmonics_Switch;
			refresh = MAX_REFRESH_TIMEOUT + rand()%5;
			#if VERBOSE_DEBUG
			DBG_PRINTF("%s next refresh in %dms\n",__func__,refresh*100);
			#endif
		}
	}
}

void OrganSoloButtonLed(uint8 blink) {
	static uint8 refresh = 0;

	if(GetEditMode() == EDIT_MODE_ON) {
		if(GetEditFunction() == BUTTON_12_SOLO) {
						
			if(BlinkTime(blink, MID_BLINK)) {
				LED_ROSSO_ORGAN;
			} else {
				LED_ORGAN_OFF;
			}
			 
			refresh = 0;
			return;
		}
	} 
	
	if(refresh) refresh--;	

	switch(GetVolumeSolo()) {
		case VOLUME_NORMAL:
			if(refresh == 0) {
				LED_ORGAN_OFF;
				refresh = MAX_REFRESH_TIMEOUT + rand()%5;
				#if VERBOSE_DEBUG
				DBG_PRINTF("%s next refresh in %dms\n",__func__,refresh*100);	
				#endif
			}
			break;
		
		default:
			if(BlinkTime(blink, MID_BLINK)) {
				LED_ROSSO_ORGAN; 
			} else {
				LED_ORGAN_OFF;
			}
			refresh = 0;
			break;
	}
}

void EditButtonLed(uint8 blink) {
	static uint8 refresh = 0;
	static uint8 oneShot = 0;
	
	if(refresh) refresh--;	
	
	switch(GetEditMode()) {
		case EDIT_MODE_OFF:
			if(refresh == 0) {
				LED_EDIT_OFF;
				refresh = MAX_REFRESH_TIMEOUT + rand()%5;
				#if VERBOSE_DEBUG
				DBG_PRINTF("%s next refresh in %dms\n",__func__,refresh*100);
				#endif
			}
			break;
			
		default:
			if(BlinkTime(blink, MID_BLINK)) {
				if(oneShot){
					LED_EDIT_ON;
					oneShot = 0; 
				}
			} else {
				if(!oneShot) {
					LED_EDIT_OFF;
					oneShot = 1; 
				}
			}
			refresh = 0;
			break;
	} 
}

void LedPoll(void)
{
	static uint16 pwm = 0;
	static uint8 toggle = 0;
	
	if(tick_100ms(TICK_PWM_LED)){
		toggle++;
		
		switch(menuLed()) {
			case 0xff:
				if(pwm==PCA9685_FULL) {
					// accende di verde
					pwm = PCA9685_MID;
					LED_VERDE_LESLIE;
					LED_VERDE_VIBRATO;
					LED_VERDE_3RD_PERCUSSION;
					LED_VERDE_2ND_PERCUSSION;
					LED_VERDE_ORGAN;
					
				} 
				else if(pwm==PCA9685_MID){
					// accende di giallo
					pwm = 0;
					LED_GIALLO_LESLIE;
					LED_GIALLO_VIBRATO;
					LED_GIALLO_3RD_PERCUSSION;
					LED_GIALLO_2ND_PERCUSSION;
					LED_GIALLO_ORGAN;
				}
				else {
					// accende di rosso
					pwm = PCA9685_FULL;
					LED_ROSSO_LESLIE;
					LED_ROSSO_VIBRATO;
					LED_ROSSO_3RD_PERCUSSION;
					LED_ROSSO_2ND_PERCUSSION;
					LED_ROSSO_ORGAN;
				}
				break;
			
			case 0:	
				LED_LESLIE_OFF;
				LED_VIBRATO_OFF;	
				LED_3RD_PERCUSSION_OFF;
				LED_2ND_PERCUSSION_OFF;
				LED_ORGAN_OFF;
				break;
				
			case 1:
				// funzionamento normale dei led
				LeslieFastButtonLed(toggle);
				VibratoOnButtonLed(toggle);
				Percussion3rdButtonLed(toggle);
				Percussion2ndButtonLed(toggle);
				OrganSoloButtonLed(toggle);
				EditButtonLed(toggle);
			break; // case 1:
			
		}
	} // 100ms
}

void test_pwm(void) {
	static uint16 i,j;
	static uint16 pwm;
	
	// fade in
	for (i = 0; i < 4096; i += 8) {
		// PCA9685_setChannelPWM(15, i); // Set PWM to 128/255, but in 4096 land
		PCA9685_setAllChannelsPWM(i);
		#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
		DBG_PRINTF("PWM_setPWM up:%d\n",i);
		#endif
	 	CyDelay(20);
	}
		
	// fade out
	for (i = 4096; i > 0; i -= 8) {
      	// PCA9685_setChannelPWM(15, i); // Set PWM to 128/255, but in 4096 land
		PCA9685_setAllChannelsPWM(i);
		#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
	  	DBG_PRINTF("PWM_setPWM dn:%d\n",i);
		#endif
      	CyDelay(20);
    }
	
	// lampeggia
	for(j=0;j<20;j++){
		// PCA9685_setChannelPWM(15, i); // Set PWM to 128/255, but in 4096 land
		if(j%2) {
			pwm = 0;
		} else {
			pwm = PCA9685_FULL;
		}
		
		PCA9685_setAllChannelsPWM(pwm);
		#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
		DBG_PRINTF("PWM_setPWM up:%d\n",i);
		#endif
	 	CyDelay(100);
	}
	
	for(j=0;j<20;j++){
		// fade in
		for (i = 0; i < 4096; i += 256) {
			// PCA9685_setChannelPWM(15, i); // Set PWM to 128/255, but in 4096 land
			PCA9685_setAllChannelsPWM(i);
			#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
			DBG_PRINTF("PWM_setPWM up:%d\n",i);
			#endif
		 	CyDelay(20);
		}
			
		// fade out
		for (i = 4096; i > 0; i -= 256) {
	      	// PCA9685_setChannelPWM(15, i); // Set PWM to 128/255, but in 4096 land
			PCA9685_setAllChannelsPWM(i);
			#ifdef PCA9685_ENABLE_DEBUG_OUTPUT
		  	DBG_PRINTF("PWM_setPWM dn:%d\n",i);
			#endif
	      	CyDelay(20);
	    }
	}
	
}
