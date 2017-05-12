/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-05-01
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "project.h"
#include "userPreset.h"
#include "tick.h"
#include "common.h"
#include "debug.h"
#include "customLcd.h"

void userPresetManager(uint8 presetNumber) {
	DBG_PRINTF("[%s]:%d\n",__func__,presetNumber);
	Display_Alternate_Text(ROW_1,ALT_USR_Preset_1+(presetNumber-1));

	/*
	treble horn: | 0,765 | 6.600 | 1.2s | -25% | 0%   |
	bass Rotor : | 0,720 | 6.200 | 8s   |  20% | 100% |
	key click 60%
	drive 10% edge 50% reverb 10%
	*/
}

#if 0
{
	// invia via midi la configurazione di default
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,   
        switchType.rotarySpeaker_HalfMoon,
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Rotary_Speaker_Bypass,             
        switchType.rotarySpeaker_bypass,
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Tube_Overdrive_Switch,             
        switchType.Tube_Overdrive_Switch,       
        MIDI_CHANNEL_1);
    CyDelay(10);

    sendControlChange(CC_Vibrato_Lower,                     
        switchType.Vibrato_Lower_Switch,        
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Vibrato_Upper,
        switchType.Vibrato_Upper_Switch,
        MIDI_CHANNEL_1);
    CyDelay(10);

    sendControlChange(CC_Vibrato_Type,          
        vibratoScannerMidiValue[switchType.chorus_Knob],
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_On_Off,          
        switchType.percussion_Switch,        
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_Volume,         
        switchType.percussionLevel_Switch,    
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_Decay,        
        switchType.percussionDecay_Switch,   
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_Harmonic,       
        switchType.percussionHarmonics_Switch,
        MIDI_CHANNEL_1);
    CyDelay(10);
	    // preset
    sendControlChange(CC_Upper_Manual_Drawbars_AB_Switch,
        VAL_UPPER_DRAWBARS_B,
        MIDI_CHANNEL_1);  // invia un cambio forzato
	CyDelay(10);
}

#endif
// EOF