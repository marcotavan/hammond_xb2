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

#include "project.h"
    
void ButtonScannerPoll(void);
void ButtonCommand(uint8 numTasto,uint8 status);
void InitSwitchButtons(void);

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


#endif    
/* [] END OF FILE */
