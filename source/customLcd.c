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

#include "LCD_Driver.h"
#include "customLcd.h"
#include "tick.h"
#include "debug.h"

char str_bargraph[MAX_ROWS][MAX_CHARS]; // contiene le barre
uint8 alternateTextCounter = 0;

static uint8 lcdMessageStates = 0;


char8 const *lcdTextMessage[10] =
{
    "Xb2 Retrofit 1.0",   // 0
    "Waiting for USB ",   // 1
    "Marco Tavan 2017",   // 2
    "Xb2 Marco Tavan ",   // 3
};


char8 const *lcdAlternateTextMessage[50] =
{
    "ALT_ROTARY_FAST ",
    "ALT_ROTARY_SLOW ",
    "ALT_RESTART_RSLW",
    "ALT_ROTARY_STOP ",
    "ALT_Perc_On     ",
    "ALT_Perc_Off    ",
    "ALT_Perc_NORM   ",
    "ALT_Perc_SOFT   ",
    "ALT_Perc_3RD    ",
    "ALT_Perc_2ND    ",
    "ALT_Perc_SLOW   ",
    "ALT_Perc_FAST   ",
    "ALT_VCS_UPPER_ON",
    "ALT_VCS_UPPR_OFF",
    "ALT_VCS_LOWER_ON",
    "ALT_VCS_LOWER_ON",
    "ALT_VCS_0       ",
    "ALT_VCS_1       ",
    "ALT_VCS_2       ",
    "ALT_VCS_3       ",
    "ALT_VCS_4       ",
    "ALT_VCS_5       ",
	"ALT_Overall_Tone",
	"ALT_RefreshAll  ",
	"ALT_Volume_Norm ",
	"ALT_Volume_Max  ",
	"ALT_Shift_Hold  "
};


uint8 const CYCODE LCD_InvertedVerticalBar[] = \
{
    /* Character LCD_CUSTOM_0   */
        0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_1   */
        0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_2   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_3   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_4   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_5   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_6   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u, \
    /* Character LCD_CUSTOM_7   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu \
 };


uint8 const CYCODE LCD_VerticalBar[] = \
{
    /* Character LCD_CUSTOM_0   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu, \
    /* Character LCD_CUSTOM_1   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_2   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_3   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_4   */
    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_5   */
    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_6   */
    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_7   */
    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu \
};


/*******************************************************************************
* Function Name: LCD_LoadCustomFonts
********************************************************************************
*
* Summary:
*  Loads 8 custom font characters into the LCD Module for use.  Cannot use
*  characters from two different font sets at once, but font sets can be
*  switched out during runtime.
*
* Parameters:
*  customData:  pointer to a constant array of 64 bytes representing 8 custom
*               font characters.
* Return:
*  None.
*
* Theory:
*  Prior to using this function user need to import the pointer to custom
*  font array to your project by writting the following in the source code file
*  where custom font will be used:
*       extern uint8 const CYCODE LCD_Char_customFonts[];
*  This function is not automatically called by the Start() routine and must be
*  called manually by the user.
*******************************************************************************/
void LCD_LoadCustomFonts(uint8 const customData[]) 
{
    uint8 indexU8;

    LCD_IsReady();
    /* Set starting address in the LCD Module */
    /* Optionally: Read the current address to restore at a later time */
    LCD_WriteControl(LCD_CGRAM_0);

    /* Load in the 64 bytes of CustomChar Data */
    for(indexU8 = 0u; indexU8 < LCD_CUSTOM_CHAR_SET_LEN; indexU8++)
    {
        /* Delay between each write */
        LCD_WriteData(customData[indexU8]);
    }

    LCD_IsReady();
    LCD_WriteControl(LCD_DDRAM_0);
}


#if (LCD_CUSTOM_CHAR_SET == LCD_HORIZONTAL_BG)

    /*******************************************************************************
    * Function Name: LCD_DrawHorizontalBG
    ********************************************************************************
    *
    * Summary:
    *  Draws the horizontal bargraph.
    *
    * Parameters:
    *  row:            The row in which the bar graph starts.
    *  column:         The column in which the bar graph starts.
    *  maxCharacters:  The max length of the graph in whole characters.
    *  value:          The current length or height of the graph in pixels.
    *
    * Return:
    *  void.
    *
    *******************************************************************************/
    void LCD_DrawHorizontalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value) \
                                                            
    {
        /* 8-bit Counter */
        uint8 count8;
        uint8 fullChars;
        uint8 remainingPixels;

        /* Number of full characters to draw */
        fullChars = value / LCD_CHARACTER_WIDTH;

        /* Number of remaining pixels to draw */
        remainingPixels = value % LCD_CHARACTER_WIDTH;

        /* Ensure that the maximum character limit is followed. */
        if(fullChars >= maxCharacters)
        {
            fullChars = maxCharacters;
        }

        /* Put Cursor at start position */
        LCD_Position(row, column);

        /* Write full characters */
        for(count8 = 0u; count8 < fullChars; count8++)
        {
            LCD_WriteData(LCD_CUSTOM_5);
        }

        if(fullChars < maxCharacters)
        {
            /* Write remaining pixels */
            LCD_WriteData(remainingPixels);

            if(fullChars < (maxCharacters - 1u))
            {
                /* Fill with whitespace to end of bar graph */
                for(count8 = 0u; count8 < (maxCharacters - fullChars - 1u); count8++)
                {
                    LCD_WriteData(LCD_CUSTOM_0);
                }
            }
        }
    }

#endif /* LCD_CUSTOM_CHAR_SET == LCD_HORIZONTAL_BG */


#if (1) // LCD_CUSTOM_CHAR_SET == LCD_VERTICAL_BG)

    /*******************************************************************************
    *  Function Name: LCD_DrawVerticalBG
    ********************************************************************************
    *
    * Summary:
    *  Draws the vertical bargraph.
    *
    * Parameters:
    *  row:            The row in which the bar graph starts.
    *  column:         The column in which the bar graph starts.
    *  maxCharacters:  The max height of the graph in whole characters.
    *  value:          The current length or height of the graph in pixels.
    *
    * Return:
    *  void.
    *
    *******************************************************************************/
    void LCD_DrawVerticalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value) \
                                                            
    {
        /* 8-bit Counter */
        uint8 count8 = 0u;
        /* Current Row Tracker */
        int8 currentRow;
        uint8 fullChars;
        uint8 remainingPixels;
        
        static uint8 init = 1;
        
        if(init)
        {
            LCD_LoadCustomFonts(LCD_InvertedVerticalBar);
            init = 0;
        }

        /* Number of full characters to draw */
        fullChars = value / LCD_CHARACTER_HEIGHT;

        /* Number of remaining pixels to draw */
        remainingPixels = value % LCD_CHARACTER_HEIGHT;

        // DBG_PRINTF("Put Cursor at start position r:%d, c:%d\n",row, column);
        LCD_Position(row, column);

        /* Make sure the bar graph fits inside the space allotted */
        if(fullChars >= maxCharacters)
        {
            fullChars = maxCharacters;
        }

        /*  Write full characters */
        while(count8 < fullChars)
        {
			// DBG_PRINTF("LCD_WriteData(LCD_CUSTOM_7)\n");
            LCD_WriteData(LCD_CUSTOM_7);

            count8++;

            /* Each pass through, move one row higher */
            if((((int8) row) - ((int8) count8)) >= 0)
            {
				// DBG_PRINTF("LCD_Position r:%d, c:%d\n",row - count8, column);
                LCD_Position(row - count8, column);
            }
            else
            {
                break;
            }
        }

        if(((((int8) row) - ((int8) count8)) >= 0) && (fullChars < maxCharacters))
        {
            /* Write remaining pixels */
            if(remainingPixels == 0u)
            {
				// DBG_PRINTF("LCD_WriteData(\' \')\n");
                LCD_WriteData((uint8) ' ');
            }
            else
            {
				// DBG_PRINTF("LCD_WriteData(remainingPixels-1 %d)\n",remainingPixels - 1u);
                LCD_WriteData(remainingPixels - 1u);
            }

            currentRow = ((int8) row) - ((int8) count8) - 1;

            if(currentRow >= 0)
            {
                /* Move up one row and fill with whitespace till top of bar graph */
                for(count8 = 0u; count8 < (maxCharacters - fullChars - 1u); count8++)
                {
					// DBG_PRINTF("r:%d, c:%d\n",currentRow, column);
                    LCD_Position((uint8)currentRow, column);
					// DBG_PRINTF("LCD_WriteData(\' \')\n");
                    LCD_WriteData((uint8) ' ');
                    currentRow --;
                }
            }
        }
    }

#endif /* LCD_CUSTOM_CHAR_SET == LCD_VERTICAL_BG */

void LCD_splashScreen(uint8 mex)
{
    /* Start LCD and set position */
    // alternateTextCounter = time;    // in 100ms
    static uint8 isLcdInit = 0;
    alternateTextCounter = 20;
    if(isLcdInit == 0)
    {
        DBG_PRINTF("wait for LCD\n");
        LCD_Start();    // write
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

void Write_BarGraphs(void)
{
    uint8 i;
    static uint16 cnt = 0;
	static char prev_bargraph[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    if (alternateTextCounter) return;  // non scrive niente
    
    // DBG_PRINTF("riscrivo barre %d\n",cnt++);
    for (i=0;i<MAX_CHARS;i++)
    {
		if(prev_bargraph[i] != str_bargraph[ROW_0][i]) {
			prev_bargraph[i] = str_bargraph[ROW_0][i];
			// LCD_Position(0,i);
			// LCD_PrintNumber(str_bargraph[ROW_0][i]);
        	LCD_DrawVerticalBG(0, i, 8,str_bargraph[ROW_0][i]);
			// DBG_PRINTF("riscrivo barre %d\n",i);
		}
    }
    
    // sposta il cursore sotto
    // LCD_Position(1,0);
    // LCD_PrintString(&str_bargraph[ROW_1][0]);
}

void LCD_Poll(uint8 status)
{
    // chiamata nel main
    static uint8 isModuleNotInitialized = 1;
    
    if(isModuleNotInitialized)
    {
        memset(str_bargraph,0,sizeof(str_bargraph[0][0])*MAX_CHARS*MAX_ROWS);
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
		
		Write_BarGraphs();
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

