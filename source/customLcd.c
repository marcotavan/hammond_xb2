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
	"ALT_Preset_3    ", // 
	"ALT_Preset_4    ", // 
	"ALT_Preset_5    ", // 
	"ALT_Preset_6    ", // 
	"ALT_Preset_7    ", // 
	"ALT_Preset_8    ", // 
	"ALT_Preset_9    ", // 
	"ALT_Preset_10   ", // 
	"ALT_Preset_11   ", // 
	"ALT_Preset_12   ", // 
	"ALT_Preset_13   ", // 
	"ALT_Preset_14   ", // 
	"ALT_Preset_15   ", // 
	"ALT_Preset_16   ", // 
	"ALT_Preset_17   ", // 
	"ALT_Preset_18   ", // 
	"ALT_Preset_19   ", // 
	"ALT_Preset_20   ", // 
	"ALT_Preset_21   ", // 
	"ALT_Preset_22   ", // 
	"ALT_Preset_23   ", // 
	"ALT_Preset_24   ", // 
	"ALT_Preset_25   ", // 
	"ALT_Preset_26   ", // 
	"ALT_Preset_27   ", // 
	"ALT_Preset_28   ", // 
	"ALT_Preset_29   ", // 
	"ALT_Preset_30   ", // 
	"ALT_Preset_31   ", // 
	"ALT_Preset_32   ", // 
	"ALT_USR_Preset_1", // 
	"ALT_USR_Preset_2", // 
	"ALT_USR_Preset_3", // 
	"ALT_USR_Preset_4", // 
	"ALT_USR_Preset_5", // 
	"ALT_USR_Preset_6", // 
	"ALT_USR_Preset_7", // 
	"ALT_USR_Preset_8", // 
	"Overdrive ON    ",
	"Overdrive OFF   ",
}; // Max Array Size 50

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
	
	switch(switchType.chorus_Knob) {
		case CHORUS_C1:
		lcdMainText[0] = 'C';
		lcdMainText[1] = '1';
		break;
		
		case CHORUS_C2:
		lcdMainText[0] = 'C';
		lcdMainText[1] = '2';
		break;

		case CHORUS_C3:
		lcdMainText[0] = 'C';
		lcdMainText[1] = '3';
		break;

		case CHORUS_V1:
		lcdMainText[0] = 'V';
		lcdMainText[1] = '1';
		break;

		case CHORUS_V2:
		lcdMainText[0] = 'V';
		lcdMainText[1] = '2';
		break;
	
		case CHORUS_V3:
		lcdMainText[0] = 'V';
		lcdMainText[1] = '3';
		break;
	}

	if(switchType.Vibrato_Upper_Switch == SWITCH_OFF) {
		lcdMainText[2] = '-';
	} else {
	    lcdMainText[2] = 'U';
	}
	
	if(switchType.Vibrato_Lower_Switch == SWITCH_OFF) {
		lcdMainText[3] = '-';
	} else {
	    lcdMainText[3] = 'L';
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
			lcdMainText[5] = 'S';
			lcdMainText[6] = 'f';
			lcdMainText[7] = 't';
		break;
		case PERC_NORM:
			lcdMainText[5] = 'N';
			lcdMainText[6] = 'm';
			lcdMainText[7] = 'l';
		break;
	}

	switch(switchType.percussionDecay_Switch) {
		case PERC_FAST:
			lcdMainText[9]  = 'F';
			lcdMainText[10] = 's';
			lcdMainText[11] = 't';
		break;
		case PERC_SLOW:
			lcdMainText[9]  = 'S';
			lcdMainText[10] = 'l';
			lcdMainText[11] = 'w';
		break;
	}
	
	switch(switchType.Tube_Overdrive_Switch) {
		case 0:
		case 3:
			lcdMainText[13] = 'O';
			lcdMainText[14] = 'f';
			lcdMainText[15] = 'f';
		break;
		case 1:
		case 2:	
			lcdMainText[13] = 'D';
			lcdMainText[14] = 'r';
			lcdMainText[15] = 'v';
		break;
	}
	
	M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) lcdMainText);		// testo scritto nella riga bassa
	// M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) lcdAlternateTextMessage[50]);		// testo scritto nella riga bassa
	if(lockBargraphs) {
		lockBargraphs = 0;
		uint8 data[16];
		memset(data,0xFF,sizeof(data));
		Write_BarGraphs(data);
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

void DisplayEditFunction(char * text, char * textblink, uint8 BlinkLcdSwitch) {
	// 
	strncpy(EditTextMessage,text,sizeof(EditBlnkMessage));
	strncpy(EditBlnkMessage,textblink,sizeof(EditBlnkMessage));
	memset(EditTempMessage,' ',sizeof(EditTempMessage));
	if(BlinkLcdSwitch == 0) {
		M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) EditTextMessage);		// testo scritto nella riga bassa
	}
	BlinkSwitch = BlinkLcdSwitch;
	
	// M2M_Write_LCD(ROW_1,LCD_STANDARD,(uint8 *) EditTextMessage);		// testo scritto nella riga bassa
}
	
void Display_Alternate_Text(uint8 where, uint8 what)
{
	// scrive un testo alternativo
	if(where == ROW_0) {
		lockBargraphs = 1;
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

/*
uint8 GetLockBargraphs (void ) {
	return lockBargraphs;	
}
*/
/* [] END OF FILE */

