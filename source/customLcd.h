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

#define MAX_CHARS 16
extern uint8 str_bargraph[];
#endif
/* [] END OF FILE */
