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

enum Volume_level_t {
	VOLUME_NORMAL,
	VOLUME_MAX
};

enum Edit_Mode_t {
	EDIT_MODE_OFF,
	EDIT_MODE_ON
};
enum num_button_t {
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

enum button_states_t {
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_ON_HOLD,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_RELEASE
};

enum rotary_switch_t {
    ROTARY_SLOW = 0x00,
    ROTARY_STOP = 0x3F,
    ROTARY_FAST = 0x7F
};

enum switch_t {
    SWITCH_OFF = 0x00,
    SWITCH_ON = 0x7F
};

enum chorus_type_t { 
    CHORUS_V1,      // 0
    CHORUS_C1,      // 1
    CHORUS_V2,      // 2
    CHORUS_C2,      // 3
    CHORUS_V3,      // 4
    CHORUS_C3       // 5    
};

enum perc_level_t {
    PERC_SOFT = 0x00,
    PERC_NORM = 0x7F
};

enum perc_decay_t {
    PERC_FAST = 0x00,
    PERC_SLOW = 0x7F
};

enum perc_type_t {
	PERC_2ND = 0x00,
	PERC_OFF = 0x01,
    PERC_3RD = 0x7F
};

enum preset_t {
    PRESET_A,
    PRESET_B  
};

enum ButtonScanner_t {
    ButtonScanner_SELECT,
    ButtonScanner_READ
};

uint8 GetButtonStatus(uint8 numTasto);
uint8 GetEditMode(void);
#endif    
/* [] END OF FILE */
