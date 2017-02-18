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
#ifndef _BUTTON_SCANNER_
#define _BUTTON_SCANNER_

#define MARKER_EEPROM_DEFAULT_BUTTON 0xA5

enum {    
    EEPROM_BUTTON,
    EEPROM_DRAWBARS,
    EEPROM_PRESET,
    EEPROM_MIDI
};

void ButtonScannerPoll(void);

#endif    
/* [] END OF FILE */
