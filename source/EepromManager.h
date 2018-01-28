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
#define MARKER_MIDI 0xA1

enum {
    EEPROM_BUTTON,
    EEPROM_MIDI,
    EEPROM_PRESET,
	EEPROM_DRAWBARS,
};

// internal_eeprom.RegPointer = (reg8 *)(CYDEV_EE_BASE + (CYDEV_EEPROM_ROW_SIZE * EEPROM_ROW_BUTTONS));  
#define EEPROM_ROW_BUTTONS 	1
#define EEPROM_ROW_MIDI 	2

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
void LoadSwitchData(uint8 *pdata);
void LoadMidiData(uint8 *pdata);

extern struct split_point split;

#endif
// EOF