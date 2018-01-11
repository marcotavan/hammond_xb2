/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-02-13
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

#include "project.h"
    
void ButtonScannerPoll(void);
void ButtonCommand(uint8 numTasto,uint8 status);
void InitSwitchButtons(void);
void RefreshAllButtonElements(void);

uint8 SHIFT_Button_on_Hold(void);
uint8 GetVolumeSolo(void);
uint8 GetPresetStatus(void);

// potrebbe andare in eeprom
struct _switchType_ {
    uint8 marker;
    uint8 rotarySpeaker_HalfMoon;
    uint8 rotarySpeaker_bypass;
    uint8 Tube_Overdrive_Switch;
    uint8 Vibrato_Upper_Switch;
    uint8 Vibrato_Lower_Switch;
    uint8 chorus_Knob;
    uint8 percussion_Switch;
    uint8 percussionLevel_Switch;
    uint8 percussionDecay_Switch;
    uint8 percussionHarmonics_Switch;
    uint8 upperManualPreset_Switch;
    uint8 lowerManualPreset_Switch;
} switchType;

uint8 SOLO_Button_on_Hold(void);

enum _Volume_level_ {
	VOLUME_NORMAL,
	VOLUME_MAX
};


enum _num_button_ {
    BUTTON_00_VIBRATO,   
    BUTTON_01_LESLIE,
    BUTTON_02_KEY_4,
    BUTTON_03_KEY_8,
    BUTTON_04_PERC_3RD,
    BUTTON_05_EDIT,
    BUTTON_06_KEY_3,
    BUTTON_07_KEY_7,
    BUTTON_08_PERC_2ND,
    BUTTON_09_RECORD,
    BUTTON_10_KEY_2,
    BUTTON_11_KEY_6,
    BUTTON_12_SOLO,
    BUTTON_13_SHIFT_CANCEL,
    BUTTON_14_KEY_1,
    BUTTON_15_KEY_5    
};

enum _button_states_ {
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_ON_HOLD,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_RELEASE
};

enum _rotary_switch_ {
    ROTARY_SLOW = 0x00,
    ROTARY_STOP = 0x3F,
    ROTARY_FAST = 0x7F
};

enum _switch_ {
    SWITCH_OFF = 0x00,
    SWITCH_ON = 0x7F
};

enum _chorus_type_ { 
    CHORUS_V1,      // 0
    CHORUS_C1,      // 1
    CHORUS_V2,      // 2
    CHORUS_C2,      // 3
    CHORUS_V3,      // 4
    CHORUS_C3       // 5    
};

enum _perc_level_ {
    PERC_SOFT = 0x00,
    PERC_NORM = 0x7F
};

enum _perc_decay_ {
    PERC_FAST = 0x00,
    PERC_SLOW = 0x7F
};

enum _perc_type_ {
	PERC_2ND = 0x00,
	PERC_OFF = 0x01,
    PERC_3RD = 0x7F
};

enum _preset_ {
    PRESET_A,
    PRESET_B  
};

enum _ButtonScanner_ {
    ButtonScanner_SELECT,
    ButtonScanner_READ
};

uint8 GetButtonStatus(uint8 numTasto);

#endif    
/* [] END OF FILE */
