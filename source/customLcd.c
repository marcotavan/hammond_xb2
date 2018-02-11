/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-01-20
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/*******************************************************************************
* File Name: LCD_BarGraph.c
* Version 2.20
*
* Description:
*  This file provides the custom character data and API for creating and using
*  bar graphs with the Character LCD component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "customLcd.h"
#include "tick.h"
#include "common.h"
#include "debug.h"
#include "M2M_SPI_Master.h"
#include "ButtonScanner.h"
#include "VB3_midi_map.h"
#include "pca9685_driver.h"

uint8 alternateTextCounter = 0;
uint8 lockBargraphs = 0;
uint8 BlinkSwitch = 0;

char8 EditTextMessage[16];
char8 EditBlnkMessage[16];
char8 EditTempMessage[16];
	
char8 *lcdTextMessage[10] =
{
    "Xb2 Retrofit 1.0",   // 0
    "Waiting for USB ",   // 1
    "Marco Tavan 2017",   // 2
    "Xb2 Marco Tavan ",   // 3
};


char8 *lcdAlternateTextMessage[100] =
{
    "FAST Rotary spkr",	// 0
    "SLOW Rotary spkr", // 1
    "ALT_RESTART_RSLW", // 2
    "STOP Rotary spkr", // 3
    "ON Percussion   ", // 4
    "OFF Percussion  ", // 5
    "NORMAL Perc Lev ", // 6
    "SOFT Perc Lev   ", // 7
    "3RD Percussion  ", // 8
    "2ND Percussion  ", // 9
    "SLOW Percussion ", // 10
    "FAST Percussion ", // 11 
    "ON Scanner Up   ", // 12
    "OFF Scanner Up  ", // 13
    "ON Scanner Low  ", // 14
    "OFF Scanner LOW ", // 15
    "V1 Vibrato      ", // 16
    "C1 Chorus       ", // 17
    "V2 Vibrato      ", // 18
    "C2 Chorus       ", // 19
    "V3 Vibrato      ", // 20
    "C3 Chorus       ", // 21
	"Overall Tone    ", // 22
	"Refresh All     ", // 23
	"Volume Normal   ", // 24
	"Volume Max      ", // 25
	"Shift on Hold   ", // 26
	"CANCEL Press    ", // 27
	"Edit            ", // 28
	"ALT_Preset_1    ", // 29
	"ALT_Preset_2    ", // 30
	"ALT_Preset_3    ", // 31
	"ALT_Preset_4    ", // 32
	"ALT_Preset_5    ", // 33
	"ALT_Preset_6    ", // 34
	"ALT_Preset_7    ", // 35
	"ALT_Preset_8    ", // 36
	"ALT_Preset_9    ", // 37
	"ALT_Preset_10   ", // 38
	"ALT_Preset_11   ", // 39
	"ALT_Preset_12   ", // 40
	"ALT_Preset_13   ", // 41
	"ALT_Preset_14   ", // 42
	"ALT_Preset_15   ", // 43
	"ALT_Preset_16   ", // 44
	"ALT_Preset_17   ", // 45
	"ALT_Preset_18   ", // 46
	"ALT_Preset_19   ", // 47
	"ALT_Preset_20   ", // 48
	"ALT_Preset_21   ", // 49
	"ALT_Preset_22   ", // 50
	"ALT_Preset_23   ", // 51
	"ALT_Preset_24   ", // 52
	"ALT_Preset_25   ", // 53
	"ALT_Preset_26   ", // 54
	"ALT_Preset_27   ", // 55
	"ALT_Preset_28   ", // 56
	"ALT_Preset_29   ", // 57
	"ALT_Preset_30   ", // 58
	"ALT_Preset_31   ", // 59
	"ALT_Preset_32   ", // 60
	"ALT_USR_Preset_1", // 61
	"ALT_USR_Preset_2", // 62
	"ALT_USR_Preset_3", // 63
	"ALT_USR_Preset_4", // 64
	"ALT_USR_Preset_5", // 65
	"ALT_USR_Preset_6", // 66
	"ALT_USR_Preset_7", // 67
	"ALT_USR_Preset_8", // 68
	"Overdrive ON    ", // 69
	"Overdrive OFF   ", // 70
	"Edit Mode OFF   ", // 71
}; // Max Array Size 100

void DisplayMainView(void){
	char lcdMainText[16] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',};
	/*
        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;        // ok
        switchType.rotarySpeaker_bypass = SWITCH_ON;            // ok
        switchType.Tube_Overdrive_Switch = 0;
        switchType.Vibrato_Lower_Switch = SWITCH_OFF;
        switchType.Vibrato_Upper_Switch = SWITCH_OFF;
        switchType.chorus_Knob = CHORUS_C1;
        switchType.percussion_Switch = SWITCH_ON;
        switchType.percussionLevel_Switch = PERC_SOFT;
        switchType.percussionDecay_Switch = PERC_FAST;
        switchType.percussionHarmonics_Switch = PERC_2ND;
        switchType.upperManualPreset_Switch = PRESET_B;
        switchType.lowerManualPreset_Switch = PRESET_B;
	*/
	
	switch(switchType.Tube_Overdrive_Switch) {
		case 0:
		case 3:
			lcdMainText[0] = 'O';
			lcdMainText[1] = 'f';
			lcdMainText[2] = 'f';
		break;
		case 1:
		case 2:	
			lcdMainText[0] = 'D';
			lcdMainText[1] = 'r';
			lcdMainText[2] = 'v';
		break;
	}
		
	
	/*
	if(switchType.percussion_Switch == SWITCH_OFF){
		lcdMainText[5] = '-';
		// lcdMainText[6] = 'f';
	} else {
		switch(switchType.percussionHarmonics_Switch) {
			case PERC_2ND:
				lcdMainText[5] = '2';
				// lcdMainText[6] = 'D';
			break;
			case PERC_3RD:
				lcdMainText[5] = '3';
				// lcdMainText[6] = 'D';
			break;
		}
	}
	*/
	switch(switchType.percussionLevel_Switch) {
		case PERC_SOFT:
			lcdMainText[4] = 'S';
			lcdMainText[5] = 'f';
			lcdMainText[6] = 't';
		break;
		case PERC_NORM:
			lcdMainText[4] = 'N';
			lcdMainText[5] = 'm';
			lcdMainText[6] = 'l';
		break;
	}

	switch(switchType.percussionDecay_Switch) {
		case PERC_FAST:
			lcdMainText[8]  = 'F';
			lcdMainText[9] = 's';
			lcdMainText[10] = 't';
		break;
		case PERC_SLOW:
			lcdMainText[8]  = 'S';
			lcdMainText[9] = 'l';
			lcdMainText[10] = 'w';
		break;
	}
	
	switch(switchType.chorus_Knob) {
		case CHORUS_C1:
		lcdMainText[12] = 'C';
		lcdMainText[13] = '1';
		break;
		
		case CHORUS_C2:
		lcdMainText[12] = 'C';
		lcdMainText[13] = '2';
		break;

		case CHORUS_C3:
		lcdMainText[12] = 'C';
		lcdMainText[13] = '3';
		break;

		case CHORUS_V1:
		lcdMainText[12] = 'V';
		lcdMainText[13] = '1';
		break;

		case CHORUS_V2:
		lcdMainText[12] = 'V';
		lcdMainText[13] = '2';
		break;
	
		case CHORUS_V3:
		lcdMainText[12] = 'V';
		lcdMainText[13] = '3';
		break;
	}

	if(switchType.Vibrato_Upper_Switch == SWITCH_OFF) {
		lcdMainText[14] = '-';
	} else {
	    lcdMainText[14] = 'U';
	}
	
	if(switchType.Vibrato_Lower_Switch == SWITCH_OFF) {
		lcdMainText[15] = '-';
	} else {
	    lcdMainText[15] = 'L';
	}
	
	M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) lcdMainText);		// testo scritto nella riga bassa
	// M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) lcdAlternateTextMessage[50]);		// testo scritto nella riga bassa
	if(lockBargraphs) {
		LockBargraphs(0);
	}
}

void LCD_Poll(void)
{
	uint8 retval;
    // chiamata nel main
    static uint8 isModuleNotInitialized = 1;
    
    if(isModuleNotInitialized)
    {
       // memset(str_bargraph,0,sizeof(str_bargraph[0][0])*MAX_CHARS*MAX_ROWS);
        alternateTextCounter = 10;
        isModuleNotInitialized = 0;
    }
    
    if (tick_100ms(TICK_LCD))
    {
        if(alternateTextCounter)
        {
            alternateTextCounter--;
            if(alternateTextCounter == 0) {
                DisplayMainView();
                // alternateTextCounter = refreshTime;
            }
        }
		
		/*Write_BarGraphs(); originale */
    }
	
	if (tick_10ms(TICK_LCD)) {
		if(BlinkSwitch) {
			if(BlinkTime(SLOW_BLINK)) {
				retval = strncmp(EditTextMessage, EditTempMessage, sizeof(EditTempMessage)); 
				if(retval) {
					// DBG_PRINTF("1 strncmp: ret %02x %s %s\n",retval,EditTextMessage,EditTempMessage);
					strncpy(EditTempMessage,EditTextMessage,sizeof(EditTempMessage));
					// DBG_PRINTF(" scrivo %s sul display\n",EditTextMessage);
					M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) EditTextMessage);		// testo scritto nella riga bassa
				}
			} else {
				retval = strncmp(EditBlnkMessage, EditTempMessage, sizeof(EditTempMessage));
				if(retval) {
					// DBG_PRINTF("0 strncmp: ret %02x %s %s\n",retval,EditBlnkMessage,EditTempMessage);
					strncpy(EditTempMessage,EditBlnkMessage,sizeof(EditTempMessage));
					// DBG_PRINTF(" scrivo %s sul display\n",EditBlnkMessage);
					M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) EditBlnkMessage);		// testo scritto nella riga bassa
				}
			}
		}
	}
}

void DisplayEditFunction(char * text, char * textblink, uint8 BlinkLcdSwitch, uint8 row) {
	// 
	strncpy(EditTextMessage,text,sizeof(EditBlnkMessage));
	strncpy(EditBlnkMessage,textblink,sizeof(EditBlnkMessage));
	memset(EditTempMessage,' ',sizeof(EditTempMessage));
	LockBargraphs(1);
	if(BlinkLcdSwitch == 0) {
		M2M_Write_LCD(row,LCD_STANDARD,(uint8 *) EditTextMessage);		// testo scritto nella riga bassa
	}
	BlinkSwitch = BlinkLcdSwitch;
	
	// M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) EditTextMessage);		// testo scritto nella riga bassa
}
	
void Display_Alternate_Text(uint8 where, uint8 what)
{
	// scrive un testo alternativo
	if(where == ROW_0) {
		LockBargraphs(1);
	}
	
    alternateTextCounter = 11;
	// DBG_PRINTF("frase da scrivere sul display: riga %d, %s\n",where,lcdAlternateTextMessage[what]);
	M2M_Write_LCD(where,LCD_STANDARD,(uint8 *) lcdAlternateTextMessage[what]);		// testo scritto nella riga bassa
}

void Display_Analog_Value(uint8 CC, uint8 data)
{
	// scrive il livello del dato
	char text[17];
	
    alternateTextCounter = 20;
	switch(CC) {
		case CC_Expression_Pedal:
			sprintf(text,"Pedal : %3d     ",data);
			break;
		case CC_Reverb:
			sprintf(text,"Reverb: %3d     ",data);
			break;
		case CC_Overall_Tone:
			sprintf(text,"Tone:   %3d     ",data);
			break;
		case CC_Overall_Volume:
			sprintf(text,"Volume: %3d     ",data);
			break;
		case CC_Tube_Overdrive_Drive:
			sprintf(text,"Overdrive: %3d     ",data);
			break;	
	}
	// DBG_PRINTF("frase da scrivere sul display: riga %d, %s\n",where,lcdAlternateTextMessage[what]);
	M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *)text);		// testo scritto nella riga bassa
}

void LockBargraphs (uint8 lock) {
	if(lock == 0) {
		ResetBarGraphs();
	}
	lockBargraphs = lock;	
	
}

/* [] END OF FILE */

