/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-02-18
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#ifndef _EEPROM_H_
#define _EEPROM_H_
    
#include "project.h"

#define MARKER_EEPROM_DEFAULT 0xA5

enum {
    EEPROM_BUTTON,
    EEPROM_DRAWBARS,
    EEPROM_PRESET,
    EEPROM_MIDI
};

#define EEPROM_ROW_BUTTONS 1

//variabili eeprom
struct _internal_eeprom_
{
    reg8 * RegPointer;
    cystatus eraseStatus;
    cystatus writeStatus;
    uint8 FL_EE_ERROR;
} internal_eeprom;

uint8 WriteDataToEeprom(uint8 type);
void EepromPoll(void);
void eeprom_init(void);

void internal_eeprom_inspector(uint16 start_row, uint16 last_row);
#endif
// EOF