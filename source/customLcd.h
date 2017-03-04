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
void LCD_Poll(void);
void Write_BarGraphs(void);
void LCD_splashScreen(void);
void Display_Write_Text(uint8 where, char *what);

#define MAX_CHARS   16
#define MAX_ROWS    2
#define ROW_0       0
#define ROW_1       1    

extern char str_bargraph[MAX_ROWS][MAX_CHARS];
#endif
/* [] END OF FILE */
