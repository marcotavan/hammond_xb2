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
void RefreshAllButtonElements(uint8 from);

uint8 SHIFT_Button_on_Hold(void);
uint8 GetVolumeSolo(void);
uint8 GetPresetStatus(void);

// potrebbe andare in eeprom
struct switchType_t {
    uint8 marker;						// 1
    uint8 rotarySpeaker_HalfMoon;		// 2
    uint8 rotarySpeaker_bypass;			// 3
    uint8 Tube_Overdrive_Switch;		// 4
    uint8 Vibrato_Upper_Switch;			// 5
    uint8 Vibrato_Lower_Switch;			// 6
    uint8 chorus_Knob;					// 7
    uint8 percussion_Switch;			// 8
    uint8 percussionLevel_Switch;		// 9
    uint8 percussionDecay_Switch;		// 10
    uint8 percussionHarmonics_Switch;	// 11
    uint8 upperManualPreset_Switch;		// 12
    uint8 lowerManualPreset_Switch;		// 13
	uint8 free1;
	uint8 free2;
	uint8 free3;
};

extern struct switchType_t switchType;
extern const char *noteNamearray[];
uint8 SOLO_Button_on_Hold(void);

enum Volume_level_t {
	VOLUME_NORMAL,
	VOLUME_MAX
};

enum Edit_Mode_t {
	EDIT_MODE_OFF, 	// 0
	EDIT_MODE_ON	// 1
};

enum tasti_t {
	TASTO_FRECCIA = 1,
	TASTO_ON_PIU,
	TASTO_OFF_MENO,
	TASTO_VIBRATO_TUNE,
	TASTO_PERCUSSION_FOOTSW,
	TASTO_DRAWBAR_SPLIT,
	TASTO_MIDI_EFFECT,
	TASTO_PRESET_RESET
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
    BUTTON_15_KEY_5,
	BUTTON_NONE = 0xFF
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
    PERC_NORM = 0x00,
    PERC_SOFT = 0x7F
};

enum perc_decay_t {
    PERC_SLOW = 0x00,
    PERC_FAST = 0x7F
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

enum functionList_t {
	FUNC_vibrato = 1,
	FUNC_tune,
	FUNC_percussion,
	FUNC_footSwitch,
	FUNC_Drawbar,
	FUNC_Split,
	FUNC_Midi,
	FUNC_Effect,
	FUNC_Preset,
	FUNC_Reset
};

enum menuLevel_t {
	MENU_LEVEL_0,
	MENU_LEVEL_1,
	MENU_LEVEL_2,
};

enum parameters_t {
	PARAMETER_0,
	PARAMETER_1,
	PARAMETER_2,
	PARAMETER_3
};

uint8 GetButtonStatus(uint8 numTasto);
uint8 GetEditMode(void);
uint8 GetEditFunction(void);
void SplitCallbackFunction(uint8 splitPoint);
void SetSplitPoint(uint8 sp);
uint8 GetSplitPoint(void);
#endif    
/* [] END OF FILE */
