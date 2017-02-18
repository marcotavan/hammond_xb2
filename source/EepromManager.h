
#ifndef _EEPROM_H_
#define _EEPROM_H_
    
#include "project.h"

#define MARKER_EEPROM_DEFAULT_BUTTON 0xA5

enum {
    EEPROM_BUTTON,
    EEPROM_DRAWBARS,
    EEPROM_PRESET,
    EEPROM_MIDI
};


uint8 WriteDataToEeprom(uint8 type);
void EepromPoll(void);

#endif
// EOF