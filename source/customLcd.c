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
#include "debug.h"

// char str_bargraph[MAX_ROWS][MAX_CHARS]; // contiene le barre
uint8 alternateTextCounter = 0;

static uint8 lcdMessageStates = 0;

void LCD_Position(uint8 a, uint8 b) {
	
}

void LCD_PrintString(char data[]){

}

char8 *lcdTextMessage[10] =
{
    "Xb2 Retrofit 1.0",   // 0
    "Waiting for USB ",   // 1
    "Marco Tavan 2017",   // 2
    "Xb2 Marco Tavan ",   // 3
};


char8 *lcdAlternateTextMessage[100] =
{
    "ALT_ROTARY_FAST ",	// 0
    "ALT_ROTARY_SLOW ", // 1
    "ALT_RESTART_RSLW", // 2
    "ALT_ROTARY_STOP ", // 3
    "ALT_Perc_On     ", // 4
    "ALT_Perc_Off    ", // 5
    "ALT_Perc_NORM   ", // 6
    "ALT_Perc_SOFT   ", // 7
    "ALT_Perc_3RD    ", // 8
    "ALT_Perc_2ND    ", // 9
    "ALT_Perc_SLOW   ", // 10
    "ALT_Perc_FAST   ", // 11 
    "ALT_VCS_UPPER_ON", // 12
    "ALT_VCS_UPPR_OFF", // 13
    "ALT_VCS_LOWER_ON", // 14
    "ALT_VCS_LOWER_ON", // 15
    "ALT_VCS_0       ", // 16
    "ALT_VCS_1       ", // 17
    "ALT_VCS_2       ", // 18
    "ALT_VCS_3       ", // 19
    "ALT_VCS_4       ", // 20
    "ALT_VCS_5       ", // 21
	"ALT_Overall_Tone", // 22
	"ALT_RefreshAll  ", // 23
	"ALT_Volume_Norm ", // 24
	"ALT_Volume_Max  ", // 25
	"ALT_Shift_Hold  ", // 26
	"ALT_CANCEL_Press", // 27
	"ALT_Edit        ", // 28
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
}; // Max Array Size 50

void LCD_splashScreen(uint8 mex)
{
    /* Start LCD and set position */
    // alternateTextCounter = time;    // in 100ms
    static uint8 isLcdInit = 0;
    alternateTextCounter = 20;
    
	if(isLcdInit == 0)
    {
        DBG_PRINTF("wait for LCD\n");

        LCD_PrintString("PSoC 5LP");
	    LCD_Position(0,1);
	    LCD_PrintString("Prima fila");
	
        DBG_PRINTF("LCD READY\n");
        isLcdInit = 1;
    }
    
    switch(mex)
    {
        case 0:
            // LCD_ClearDisplay(); // spacca il display
            LCD_Position(0,0);  // write
            LCD_PrintString(lcdTextMessage[0]);  // write
            LCD_Position(1,0);  // write
            LCD_PrintString(lcdTextMessage[2]);  // write
		
        break;
        
        case 1:
		
            LCD_Position(1,0);  // write
            LCD_PrintString(lcdTextMessage[1]);  // write
		
        break;
    }
}

void LCD_bootlogo (uint8 time)
{
    /* Start LCD and set position */
    alternateTextCounter = time;    // in 100ms

    LCD_Position(0,0);
    LCD_PrintString(lcdTextMessage[0]);
    
    LCD_Position(1,0);
    LCD_PrintString(lcdTextMessage[2]);
    // LCD_ClearDisplay();

}


void LCD_Poll(uint8 status)
{
    // chiamata nel main
    static uint8 isModuleNotInitialized = 1;
    
    if(isModuleNotInitialized)
    {
       // memset(str_bargraph,0,sizeof(str_bargraph[0][0])*MAX_CHARS*MAX_ROWS);
        // LCD_bootlogo(50);
        
        isModuleNotInitialized = 0;
    }
    
    if(status == 0)
    {
        // ricarica lo splashscreen in caso di disconnseesione
        if(lcdMessageStates == 2)
        {
            lcdMessageStates = 0;
            alternateTextCounter = 10;
        }
    }
    
    if (tick_100ms(TICK_LCD))
    {
        if(alternateTextCounter)
        {
            alternateTextCounter--;
            if(alternateTextCounter == 0)
            {
                if(status == 0) // not initialized
                {
                    switch(lcdMessageStates)
                    {
                        case 0: // -> vai in 1
                        lcdMessageStates = 1;
                        LCD_splashScreen(1);
                        break;
                        
                        case 1: // -> vai in 2
                        lcdMessageStates = 0;
                        LCD_splashScreen(0);
                        break;
                    }
                }
                else
                {
                    lcdMessageStates = 2;
                    // swappa LCD
                    // LCD_ClearDisplay();
                    // Write_BarGraphs();
                }
                
            }
        }
		
		/*Write_BarGraphs(); origimale */
    }
	
	if (tick_10ms(TICK_LCD)) {
		// aa
	}
}

void Display_Alternate_Text(uint8 where, uint8 what)
{
    // where = where;
    // what = what;
    LCD_Position(where,0);
    LCD_PrintString(lcdAlternateTextMessage[what]);
    DBG_PRINTF("frase da scrivere sul display: riga %d, %s\n",where,lcdAlternateTextMessage[what]);
    // nop
}

/* [] END OF FILE */

