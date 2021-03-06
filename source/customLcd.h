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

#ifndef _CUSTOMLCD_H_
#define _CUSTOMLCD_H_
    
#include "project.h"

void LCD_DrawVerticalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value);
void LCD_Poll(void);

void Display_Alternate_Text(uint8 where, uint8 what);
void Display_Analog_Value(uint8 CC, uint8 data);
void DisplayEditFunction(char * text, char * textblink, uint8 BlinkLcdSwitch, uint8 row);
void LockBargraphs (uint8 lock);

#define MAX_CHARS   16
#define MAX_ROWS    2
#define ROW_0       0
#define ROW_1       1    

/* LCD Characteristics */
// #define LCD_CHARACTER_WIDTH          (0x05u)
#define LCD_CHARACTER_HEIGHT         (0x08u)

/* Custom Character References */
#define LCD_CUSTOM_0                 (0x00u)
#define LCD_CUSTOM_1                 (0x01u)
#define LCD_CUSTOM_2                 (0x02u)
#define LCD_CUSTOM_3                 (0x03u)
#define LCD_CUSTOM_4                 (0x04u)
#define LCD_CUSTOM_5                 (0x05u)
#define LCD_CUSTOM_6                 (0x06u)
#define LCD_CUSTOM_7                 (0x07u)

enum alternative_text {
    ALT_ROTARY_FAST		= 0,		// 0	
    ALT_ROTARY_SLOW,				// 1
    ALT_RESTART_ROTARY_SLOW,		// 2
    ALT_ROTARY_STOP,				// 3
    ALT_Percussion_On,				// 4
    ALT_Percussion_Off,				// 5
    ALT_Percussion_NORM,			// 6
    ALT_Percussion_SOFT,			// 7
    ALT_Percussion_3RD,				// 8
    ALT_Percussion_2ND,				// 9
    ALT_Percussion_SLOW,			// 10
    ALT_Percussion_FAST,			// 11
    ALT_VIBRATO_SCANNER_UPPER_ON,	// 12
    ALT_VIBRATO_SCANNER_UPPER_OFF,	// 13
    ALT_VIBRATO_SCANNER_LOWER_ON,	// 14
    ALT_VIBRATO_SCANNER_LOWER_OFF,	// 15
    ALT_VIBRATO_SCANNER_0,			// 16
    ALT_VIBRATO_SCANNER_1,			// 17
    ALT_VIBRATO_SCANNER_2,			// 18		
    ALT_VIBRATO_SCANNER_3,			// 19
    ALT_VIBRATO_SCANNER_4,			// 20
    ALT_VIBRATO_SCANNER_5,			// 21
	ALT_Overall_Tone,				// 22	
	ALT_RefreshAll_Elements,		// 23
	ALT_Volume_Normal,				// 24
	ALT_Volume_Max,					// 25
	ALT_Shift_on_Hold,				// 26
	ALT_Cancel_on_Press,			// 27
	ALT_Edit,						// 28	
	ALT_Preset_1, 					// 29
	ALT_Preset_2, 					// 
	ALT_Preset_3, 					// 
	ALT_Preset_4, 					// 
	ALT_Preset_5, 					// 
	ALT_Preset_6, 					// 
	ALT_Preset_7, 					// 
	ALT_Preset_8, 					// 
	ALT_Preset_9, // 
	ALT_Preset_10, // 
	ALT_Preset_11, // 
	ALT_Preset_12, // 
	ALT_Preset_13, // 
	ALT_Preset_14, // 
	ALT_Preset_15, // 
	ALT_Preset_16, // 
	ALT_Preset_17, // 
	ALT_Preset_18, // 
	ALT_Preset_19, // 
	ALT_Preset_20, // 
	ALT_Preset_21, // 
	ALT_Preset_22, // 
	ALT_Preset_23, // 
	ALT_Preset_24, // 
	ALT_Preset_25, // 
	ALT_Preset_26, // 
	ALT_Preset_27, // 
	ALT_Preset_28, // 
	ALT_Preset_29, // 
	ALT_Preset_30, // 
	ALT_Preset_31, // 
	ALT_Preset_32, // 
	ALT_USR_Preset_1, // 
	ALT_USR_Preset_2, // 
	ALT_USR_Preset_3, // 
	ALT_USR_Preset_4, // 
	ALT_USR_Preset_5, // 
	ALT_USR_Preset_6, // 
	ALT_USR_Preset_7, // 
	ALT_USR_Preset_8, // 
	ALT_overdriveOn,
	ALT_overdriveOff,
	ALT_Edit_Exit
};

#endif
/* [] END OF FILE */
