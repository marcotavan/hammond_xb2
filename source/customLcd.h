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
#ifndef _CUSTOMLCD_H_
#define _CUSTOMLCD_H_
    
#include "project.h"

void LCD_DrawVerticalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value);
void LCD_Poll(uint8 status);
void Write_BarGraphs(void);
void LCD_splashScreen(uint8 mex);
void Display_Alternate_Text(uint8 where, uint8 what);

#define MAX_CHARS   16
#define MAX_ROWS    2
#define ROW_0       0
#define ROW_1       1    

extern char str_bargraph[MAX_ROWS][MAX_CHARS];

enum alternative_text {
    ALT_ROTARY_FAST,
    ALT_ROTARY_SLOW,
    ALT_RESTART_ROTARY_SLOW,
    ALT_ROTARY_STOP,
    ALT_Percussion_On,
    ALT_Percussion_Off,
    ALT_Percussion_NORM,
    ALT_Percussion_SOFT,
    ALT_Percussion_3RD,
    ALT_Percussion_2ND,
    ALT_Percussion_SLOW,
    ALT_Percussion_FAST,
    ALT_VIBRATO_SCANNER_UPPER_ON,
    ALT_VIBRATO_SCANNER_UPPER_OFF,
    ALT_VIBRATO_SCANNER_LOWER_ON,
    ALT_VIBRATO_SCANNER_LOWER_OFF,
    ALT_VIBRATO_SCANNER_0,
    ALT_VIBRATO_SCANNER_1,
    ALT_VIBRATO_SCANNER_2,
    ALT_VIBRATO_SCANNER_3,
    ALT_VIBRATO_SCANNER_4,
    ALT_VIBRATO_SCANNER_5,
};

#endif
/* [] END OF FILE */
