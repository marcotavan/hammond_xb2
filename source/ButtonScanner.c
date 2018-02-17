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


Implementazione:
1. Leslie: toggle slow verde/Fast rosso. tenedo premuto va a stop giallo lampeggiante, ripremendo parte in fast. Parametro per ripartire in fast o slow
-> scrivere nel display torna utile

2. vibrato: toggle on/off upper, long press toggle on off lower. default c3, edit per cambiare
3. percussion on off / level, normal/soft
4. percussion 2nd/ 3rd, decay fast/slow

serve un tasto SHIFT tenuto premuto agisce sui singoli comandi.
2 permette di toggglare tra i vari vibrato scanner

5 tasto SOLO incrementa il volume per fare un solo? potrebbe essere! da un BOOST
*/


// attenzione che ho girato le linee per mantenere uniformit' con la tastiera

#include "project.h"
#include "ButtonScanner.h"
#include "common.h"
#include "debug.h"
#include "tick.h"
#include "midiLibrary.h"
#include "VB3_midi_map.h"
#include "customLcd.h"
#include "EepromManager.h"
#include "analog.h"
#include "midiLibrary.h"
#include "userPreset.h"
#include "midiMerge.h"
#include "keyboardScanner.h"
#include "midiEvents.h"
#define MIN_DEBOUNCE 5      // * 8ms
#define MAX_DEBOUNCE 100     // * 8ms  
#define MAX_PULSANTI 16

#define BUTTON_UART_DEBUG (0)
#define MENU_DEBUG (0)
static struct onHold_s {
    uint8 shift;
    uint8 solo;
}OnHold;

struct switchType_t switchType;
// static char str[20];
const uint8 vibratoScannerPosition[] = {1,1,2,2,3,3};
const uint8 vibratoScannerMidiValue[] = {11,33,55,77,99,127};

const uint8 keyTranslator[16] = { 
	0,0,4,8,
	0,0,3,7,
	0,0,2,6,
	0,0,1,5
};

struct {
    uint8 debounce;
    uint8 status;
    uint8 oneShot;
} button[MAX_PULSANTI];

static uint8 soloVolume = VOLUME_NORMAL;

enum {
	PRESET_FREE,
	PRESET_CUSTOM,
	PRESET_FACTORY
};

static uint8 presetStatus = PRESET_FREE;
static uint8 EditMode = EDIT_MODE_OFF;
static uint8 EditFunction = BUTTON_NONE;

static uint8 selectedFunction = 0;  // rimane questa> FUNC_Split
static uint8 menuLevel = MENU_LEVEL_0;
static uint8 subMenuParameter = PARAMETER_0; // niente da cambiare

char splitTextMessage[16];
char splitBlinkMessage[16];

void FootSwitchManager(void);
void ResetButtonCycle(void);
void WriteSplitPoint(uint8 blnk);

void RefreshAllButtonElements(uint8 from) {
	// invia via midi la configurazione di default
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,   
        switchType.rotarySpeaker_HalfMoon,
        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Rotary_Speaker_Bypass,             
        switchType.rotarySpeaker_bypass,
        MIDI_CHANNEL_1);
    CyDelay(10);
    
	switch(switchType.Tube_Overdrive_Switch) {
		case 1:
		case 2:
		    sendControlChange(CC_Tube_Overdrive_Switch,             
		        127,       
		        MIDI_CHANNEL_1);
			break;
		default:	
			sendControlChange(CC_Tube_Overdrive_Switch,             
		        0,       
		        MIDI_CHANNEL_1);
			break;
	}
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
/*****************************************************************************\
*  
\*****************************************************************************/
void InitSwitchButtons(void)
{
	uint8 pdata[CYDEV_EEPROM_ROW_SIZE];
	LoadSwitchData(pdata);   
	
	memcpy((uint8 *) &switchType,pdata,sizeof(switchType)); 

    if (switchType.marker != MARKER_EEPROM_DEFAULT)
    {
        // prendi da default else carica da eeprom
        switchType.marker = MARKER_EEPROM_DEFAULT;

        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;        // ok
        switchType.rotarySpeaker_bypass = SWITCH_ON;            // ok
        switchType.Tube_Overdrive_Switch = 0;
        switchType.Vibrato_Lower_Switch = SWITCH_OFF;
        switchType.Vibrato_Upper_Switch = SWITCH_OFF;
        switchType.chorus_Knob = CHORUS_C1;
        switchType.percussion_Switch = SWITCH_ON;
        switchType.percussionLevel_Switch = PERC_SOFT;
        switchType.percussionDecay_Switch = PERC_FAST;
        switchType.percussionHarmonics_Switch = PERC_2ND;
        switchType.upperManualPreset_Switch = PRESET_B;
        switchType.lowerManualPreset_Switch = PRESET_B;
        
        // write in eeprom
        EEPROM_Write((uint8 *) &switchType, EEPROM_ROW_BUTTONS);
    }
    
    OnHold.shift = FALSE;
    OnHold.solo = FALSE;
    
	DBG_PRINTF("%s\n",__func__);
	RefreshAllButtonElements(0);
	RefreshAllAnalogElements();
	
	/*
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,   
    sendControlChange(CC_Rotary_Speaker_Bypass,             
	switch(switchType.Tube_Overdrive_Switch) {
    sendControlChange(CC_Tube_Overdrive_Switch,             
	sendControlChange(CC_Tube_Overdrive_Switch,             
    sendControlChange(CC_Vibrato_Lower,                     
    sendControlChange(CC_Vibrato_Upper,
    sendControlChange(CC_Vibrato_Type,          
    sendControlChange(CC_Percussion_On_Off,          
    sendControlChange(CC_Percussion_Volume,         
    sendControlChange(CC_Percussion_Decay,        
    sendControlChange(CC_Percussion_Harmonic,       
    sendControlChange(CC_Upper_Manual_Drawbars_AB_Switch,
	*/
	
	StartMidiMerge();
	// aggiungere lo SPLIT ON
    // Display_Alternate_Text(ROW_1,ALT_InitSwitchButtons_Done);
	
	selectedFunction = 0;
	menuLevel = MENU_LEVEL_0;
	subMenuParameter = PARAMETER_0;
	presetStatus = PRESET_FREE;
	EditMode = EDIT_MODE_OFF;
	EditFunction = BUTTON_NONE;
 }

/*****************************************************************************\
*  restituisce se il tasto shift e' premuto
\*****************************************************************************/
uint8 SHIFT_Button_on_Hold(void) {
    return OnHold.shift; 
}

/*****************************************************************************\
*  restituisce se il tasto shift e' premuto
\*****************************************************************************/
uint8 SOLO_Button_on_Hold(void) {
    return OnHold.solo; 
}

/*****************************************************************************\
*  gestisce la funzione Rotary Speaker
\*****************************************************************************/
void ManageButton_Leslie (uint8 status)
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
    switch (status) 
    {
        // case BUTTON_PRESSED:
        case BUTTON_SHORT_PRESS:
        {
            switch(switchType.rotarySpeaker_HalfMoon) 
            {
                case ROTARY_SLOW:
                case ROTARY_STOP:
                {
                	switchType.rotarySpeaker_HalfMoon = ROTARY_FAST;
                    Display_Alternate_Text(ROW_1,ALT_ROTARY_FAST);
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("ALT_ROTARY_FAST\n");
					#endif
                }
                break;
                
                case ROTARY_FAST: 
                {
                	switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                    Display_Alternate_Text(ROW_1,ALT_ROTARY_SLOW);
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("ALT_ROTARY_SLOW\n");
					#endif
                }
                break;
                
                default:
                {
                	switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                    Display_Alternate_Text(ROW_1,ALT_ROTARY_SLOW);
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("ALT_ROTARY_SLOW\n");
					#endif
                }
                break;
            }
                           
            sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,
                switchType.rotarySpeaker_HalfMoon,  
                MIDI_CHANNEL_1);    
        }
        break;
        
        case BUTTON_ON_HOLD:
        {
            switch(switchType.rotarySpeaker_HalfMoon) 
            {
                case ROTARY_STOP:
                {
                    switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                    Display_Alternate_Text(ROW_1,ALT_RESTART_ROTARY_SLOW);
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("ALT_RESTART_ROTARY_SLOW\n");
					#endif
                }
                break;
                
                default:
                {
                    switchType.rotarySpeaker_HalfMoon = ROTARY_STOP;
                    Display_Alternate_Text(ROW_1,ALT_ROTARY_STOP);
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("ALT_ROTARY_STOP\n");
					#endif
                }
                break;
            }
            
            sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,   
                switchType.rotarySpeaker_HalfMoon,   
                MIDI_CHANNEL_1);                    
        }
        break;
        
        default:
        break;
    }
}
   
/*****************************************************************************\
*  gestisce la funzione Vibrato Scanner
\*****************************************************************************/
void ManageButton_Vibrato(uint8 status)
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
		
    if (SHIFT_Button_on_Hold() == FALSE)
    {
        switch (status) 
        {
            // case BUTTON_PRESSED:
            case BUTTON_SHORT_PRESS:
            {
                switch(switchType.Vibrato_Upper_Switch) 
                {
                    case SWITCH_OFF:
                    switchType.Vibrato_Upper_Switch = SWITCH_ON;
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("Vibrato_Upper_Switch = SWITCH_ON\n");
					#endif
                    break;
                
                    case SWITCH_ON: 
                    switchType.Vibrato_Upper_Switch = SWITCH_OFF;
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("Vibrato_Upper_Switch = SWITCH_OFF\n");
					#endif
                    break;
                    
                    default:
                    switchType.Vibrato_Upper_Switch = SWITCH_OFF;
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("Vibrato_Upper_Switch = SWITCH_OFF\n");
					#endif
                    break;
                }
                
                Display_Alternate_Text(ROW_1,switchType.Vibrato_Upper_Switch?ALT_VIBRATO_SCANNER_UPPER_ON:ALT_VIBRATO_SCANNER_UPPER_OFF); 
                
                sendControlChange(CC_Vibrato_Upper,             
                    switchType.Vibrato_Upper_Switch,    
                    MIDI_CHANNEL_1);
            }
            break;
            
            // case BUTTON_LONG_PRESS
            case BUTTON_ON_HOLD:
            {
                switch (switchType.Vibrato_Lower_Switch) 
                {
                    case SWITCH_OFF:
                    switchType.Vibrato_Lower_Switch = SWITCH_ON;
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("Vibrato_Lower_Switch = SWITCH_ON\n");
					#endif
                    break;
                    
                    case SWITCH_ON:
                    switchType.Vibrato_Lower_Switch = SWITCH_OFF;
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("Vibrato_Lower_Switch = SWITCH_OFF\n");
					#endif
                    break;
                    
                    default:
                    switchType.Vibrato_Lower_Switch = SWITCH_OFF;
					#if BUTTON_UART_DEBUG
					DBG_PRINTF("Vibrato_Lower_Switch = SWITCH_OFF\n");
					#endif
                    break;
                }
                
                Display_Alternate_Text(ROW_1,switchType.Vibrato_Lower_Switch?ALT_VIBRATO_SCANNER_LOWER_ON:ALT_VIBRATO_SCANNER_LOWER_OFF); 
                
                sendControlChange(CC_Vibrato_Lower,       
                    switchType.Vibrato_Lower_Switch,     
                    MIDI_CHANNEL_1);
            }
            break;
			
			case BUTTON_RELEASED:
			break;
			
			case BUTTON_LONG_RELEASE:
			break;
        }
    }   // shift on hold
    else
    {
        // alternate function
        switch (status) 
        {
            // case BUTTON_SHORT_PRESS:    // valido al rilascio breve
            case BUTTON_PRESSED:     // valido immediatamente
            {
                switchType.chorus_Knob++;
                switchType.chorus_Knob %= 6;    // gira su 6 elementi
                    
                sendControlChange(CC_Vibrato_Type,          
                    vibratoScannerMidiValue[switchType.chorus_Knob],              
                    MIDI_CHANNEL_1);
                
                // questo serve per scrivere C1 V1 C2,V2 C3, V3
                // sprintf(str,"VIBRATO SCANNER %s%d",switchType.chorus_Knob%2?"C":"V", vibratoScannerPosition[switchType.chorus_Knob]);
                Display_Alternate_Text(ROW_1,ALT_VIBRATO_SCANNER_0+switchType.chorus_Knob);  
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_VIBRATO_SCANNER = %d\n",ALT_VIBRATO_SCANNER_0+switchType.chorus_Knob);
				#endif
            }
            break;
        }   
    }   
}

/*****************************************************************************\
*  gestisce la funzione Percussion Level
\*****************************************************************************/
void ManageButton_PercussionLevel(uint8 status) 
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
    switch (status) 
    {
        // case BUTTON_PRESSED:
        case BUTTON_SHORT_PRESS:
        {
			switch(switchType.percussionHarmonics_Switch) {
                case PERC_3RD:
                switchType.percussion_Switch = SWITCH_OFF;
				switchType.percussionHarmonics_Switch = PERC_OFF;
                Display_Alternate_Text(ROW_1,ALT_Percussion_Off);
				
				sendControlChange(CC_Percussion_On_Off,   
                	switchType.percussion_Switch,        
                	MIDI_CHANNEL_1);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_Off\n");
				#endif
                break;
            
				case PERC_2ND:
                case PERC_OFF: 
				switchType.percussion_Switch = SWITCH_ON;
				switchType.percussionHarmonics_Switch = PERC_3RD;
                Display_Alternate_Text(ROW_1,ALT_Percussion_3RD);

				sendControlChange(CC_Percussion_Harmonic,       
	                switchType.percussionHarmonics_Switch,      
	                MIDI_CHANNEL_1);         
				
				CyDelay(10);
				
				sendControlChange(CC_Percussion_On_Off,   
                	switchType.percussion_Switch,        
                	MIDI_CHANNEL_1);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_3RD\n");
				#endif
				break;
                
                default:
                break;
            }
        }
        break;
        
        // case BUTTON_LONG_PRESS
		case BUTTON_ON_HOLD:
        {
            switch(switchType.percussionDecay_Switch) 
            {
                case PERC_FAST:
                switchType.percussionDecay_Switch = PERC_SLOW;
                Display_Alternate_Text(ROW_1,ALT_Percussion_SLOW);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_SLOW\n");
				#endif
                break;
            
                case PERC_SLOW: 
                switchType.percussionDecay_Switch = PERC_FAST;
                Display_Alternate_Text(ROW_1,ALT_Percussion_FAST);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_FAST\n");
				#endif
                break;
                
                default:
                switchType.percussionDecay_Switch = PERC_FAST;
                Display_Alternate_Text(ROW_1,ALT_Percussion_FAST);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_FAST\n");
				#endif
                break;
            }
            
            sendControlChange(CC_Percussion_Decay,            
                switchType.percussionDecay_Switch,     
                MIDI_CHANNEL_1);
        }
        break;
    }   
}

/*****************************************************************************\
*  gestisce la funzione Percussion Type
\*****************************************************************************/
void ManageButton_PercussionType(uint8 status)
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
    switch (status) 
    {
        // case BUTTON_PRESSED:
        case BUTTON_SHORT_PRESS:
        {
			switch(switchType.percussionHarmonics_Switch) {
                case PERC_2ND:
                switchType.percussion_Switch = SWITCH_OFF;
				switchType.percussionHarmonics_Switch = PERC_OFF;
                Display_Alternate_Text(ROW_1,ALT_Percussion_Off);
				
				sendControlChange(CC_Percussion_On_Off,   
                	switchType.percussion_Switch,        
                	MIDI_CHANNEL_1);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("PERC_2ND -> PERC_OFF\n");
				#endif
                break;
            
				case PERC_3RD:
                case PERC_OFF: 
				switchType.percussion_Switch = SWITCH_ON;
				switchType.percussionHarmonics_Switch = PERC_2ND;
                Display_Alternate_Text(ROW_1,ALT_Percussion_2ND);

				sendControlChange(CC_Percussion_Harmonic,       
	                switchType.percussionHarmonics_Switch,      
	                MIDI_CHANNEL_1);         
				
				CyDelay(10);
				
				sendControlChange(CC_Percussion_On_Off,   
                	switchType.percussion_Switch,        
                	MIDI_CHANNEL_1);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_2ND\n");
				#endif
				break;
                
                default:
                break;
            }
        }
        break;
        
        // case BUTTON_LONG_PRESS
		case BUTTON_ON_HOLD:
        {
            switch(switchType.percussionLevel_Switch) 
            {
                case PERC_SOFT:
                switchType.percussionLevel_Switch = PERC_NORM;
                Display_Alternate_Text(ROW_1,ALT_Percussion_NORM);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_NORM\n");
				#endif
                break;
            
                case PERC_NORM: 
                switchType.percussionLevel_Switch = PERC_SOFT;
                Display_Alternate_Text(ROW_1,ALT_Percussion_SOFT);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_SOFT\n");
				#endif
                break;
                
                default:
                switchType.percussionLevel_Switch = PERC_SOFT;
                Display_Alternate_Text(ROW_1,ALT_Percussion_SOFT);
				#if BUTTON_UART_DEBUG
				DBG_PRINTF("ALT_Percussion_SOFT\n");
				#endif
                break;
            }
            
            sendControlChange(CC_Percussion_Volume, 
                switchType.percussionLevel_Switch,        
                MIDI_CHANNEL_1);
        }
        break;
        
    }   
}

/*****************************************************************************\
*  gestisce la funzione Shift Button Pressed
\*****************************************************************************/
void ManageButton_Shift(uint8 status)
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
			if(OnHold.shift == TRUE) {
	            Display_Alternate_Text(ROW_0,ALT_Cancel_on_Press);
				ResetButtonCycle();
				SendProgramChange(0, MIDI_CHANNEL_1);
				
				OnHold.shift = FALSE;
	            // DBG_PRINTF("Shift Released, i led del pannello tornano normali\n");
				RefreshAllAnalogElements();
				presetStatus = PRESET_FREE;
			}
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio
        case BUTTON_ON_HOLD:        
        {
            OnHold.shift = TRUE;
            // servirà un timeout?
            DBG_PRINTF("Shift on Hold, far lampeggiare i led del pannello -> alternate Function\n");
            // si accende il led dello shift se c'è^?
			Display_Alternate_Text(ROW_0,ALT_Shift_on_Hold);
        }
        break;
        
        case BUTTON_RELEASED:
        case BUTTON_LONG_RELEASE:
        {
            // OnHold.shift = FALSE;
            // DBG_PRINTF("Shift Released, i led del pannello tornano normali\n");
        }
        break;
        
    }   
}

/*****************************************************************************\
*  gestisce la funzione Solo Button
\*****************************************************************************/
void ManageButton_Solo(uint8 status)
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
    // solo button
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
            DBG_PRINTF("Solo pressed\n");
			if(GetVolumeSolo() == VOLUME_NORMAL) {
				soloVolume = VOLUME_MAX;
				sendControlChange(CC_Overall_Volume,127,MIDI_CHANNEL_1);
				Display_Analog_Value(CC_Overall_Volume,127);
			} else {
				// sendControlChange(CC_Overall_Volume,GetOverallVolumeLevel(),MIDI_CHANNEL_1);
				// GetOverallVolumeLevel();
				TriggerVolumeRead();
				soloVolume = VOLUME_NORMAL;
				Display_Analog_Value(CC_Overall_Volume,GetOverallVolumeLevel());
			}
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        
        {
            OnHold.solo = TRUE;
            // servirà un timeout?
            DBG_PRINTF("Solo on Hold -> alternate Function\n");
            // si accende il led dello shift se c'è^?
        }
        break;
        
        case BUTTON_RELEASED:
        case BUTTON_LONG_RELEASE:
        {
            OnHold.solo = FALSE;
            DBG_PRINTF("solo Released\n");
        }
        break;
    }   
}

/*****************************************************************************\
*  TEMPLATE: gestisce la funzione tasto  Generico
\*****************************************************************************/
void ManageButton_Record(uint8 status)
{
	if (GetEditMode()) {
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
			DBG_PRINTF("ManageButton_Record BUTTON_SHORT_PRESS\n");
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        // valido al mantenimento
        {
			DBG_PRINTF("ManageButton_Record BUTTON_ON_HOLD -> RefreshAll Elements\n");
			Display_Alternate_Text(ROW_1,ALT_RefreshAll_Elements);
			RefreshAllButtonElements(1);
			RefreshAllAnalogElements();
        }
        break;
        
        case BUTTON_RELEASED:       // tasto rilasciato
        {
            
        }
        break;
    }       
}

static uint8 prevKey = 0; // globale...

void EditModeExit(void) {		
	// WriteDataToEeprom(EEPROM_BUTTON);
	DBG_PRINTF("%s: salvare in eeprom i parametri modificati\n",__func__);
	EditMode = EDIT_MODE_OFF;
	prevKey = 0;
	Display_Alternate_Text(ROW_1,ALT_Edit_Exit); // non so cosa fa questo
	DisplayEditFunction(NULL,NULL,0,ROW_1);
	
	selectedFunction = 0;
	menuLevel = MENU_LEVEL_0;
	subMenuParameter = PARAMETER_0;
}

void ManageButton_Edit(uint8 status)
{
	char8 *lcdEditTextMessage;
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
			DBG_PRINTF("ManageButton_Edit BUTTON_SHORT_PRESS\n");
			if(EditMode == EDIT_MODE_ON) {
				EditModeExit();
			}
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        // valido al mantenimento
        {
			switch(EditMode) {
				case EDIT_MODE_OFF:
					EditMode = EDIT_MODE_ON;
					OnHold.shift = FALSE;
					lcdEditTextMessage = "Edit Mode ON    ";
					DisplayEditFunction(lcdEditTextMessage,NULL,1,ROW_1);
					// reset prev
					prevKey = 0;
					break;
				case EDIT_MODE_ON:
					EditModeExit();
					break;
			}
			DBG_PRINTF("ManageButton_Edit BUTTON_ON_HOLD\n");
        }
        break;
        
        case BUTTON_RELEASED:       // tasto rilasciato
        {
            
        }
        break;
    }       
}

void ManageDamperPedal(uint8 status) {
	// MIDI CC 64 	Damper Pedal / Sustain Pedal 	On/Off switch that controls sustain. (See also Sostenuto CC 66) 0 to 63 = Off, 64 to 127 = On
	
	switch(status) {
		case BUTTON_PRESSED:
			sendControlChange(CC_DamperPedal,127,MIDI_CHANNEL_1);
			break;

		case BUTTON_RELEASED:
			sendControlChange(CC_DamperPedal,0,MIDI_CHANNEL_1);
			break;
	}	
}

static uint8 buttonCycle[9] = {0,0,0,0,0,0,0,0,0};
	
void ResetButtonCycle(void) {
	memset(buttonCycle,0,sizeof(buttonCycle));
}

void FunctionViewSelected(uint8 selectedFunction) {
	char8 *lcdEditTextMessage;
	char8 *lcdEditBlnkMessage;

	switch(selectedFunction) {
		case FUNC_vibrato:
			lcdEditTextMessage = "Edit Vibrato    ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_tune:
			lcdEditTextMessage = "Edit tune       ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_percussion:
			lcdEditTextMessage = "Edit percussion ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_footSwitch:
			lcdEditTextMessage = "Edit footSwitch ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_Drawbar:
			lcdEditTextMessage = "Edit Drawbar    ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
			// DBG_PRINTF("%s %s \n",__func__,lcdEditTextMessage);
		break;
		
		case FUNC_Midi:
			lcdEditTextMessage = "Edit Midi       ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_Effect:
			lcdEditTextMessage = "Edit Effect     ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_Preset:
			lcdEditTextMessage = "Edit Preset     ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_Reset:
			lcdEditTextMessage = "Edit Reset      ";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
		break;
		
		case FUNC_Split:
			lcdEditTextMessage = "Edit Split Point";
			DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0,ROW_0);
			CyDelay(100); // altrimenti non scrive sul display
			
			WriteSplitPoint(0);
			/*
			switch(menuLevel) {
				case MENU_LEVEL_0:
					DBG_PRINTF(" nel display va scritto: | SPLIT:OFF  KEY#:C2 | (menu level 0)");
					lcdEditTextMessage = "SPLIT:OFF KEY:C2";
	
					DisplayEditFunction(lcdEditTextMessage,lcdEditTextMessage,0);
					
				break; // case MENU_LEVEL_0:
				
				case MENU_LEVEL_1:
					switch(subMenuParameter) { // select
						case PARAMETER_1:
							lcdEditBlnkMessage = "SPLIT>    KY:";
							// sprintf(tm,"%s%s",lcdEditTextMessage,noteNamearray[2]);
							DBG_PRINTF(" nel display va scritto: SPLIT>{%s} KEY#:%s%d   - %s\n","off",noteNamearray[(GetSplitPoint())%12],(GetSplitPoint()),lcdEditTextMessage);
							
							DisplayEditFunction(tm,lcdEditBlnkMessage,1);
							break; // PARAMETER_1
							
						case PARAMETER_2:
							DBG_PRINTF(" nel display va scritto: | SPLIT:OFF  KEY#>{C2} | (menu level 1), func:%d, par:%d\n",selectedFunction,subMenuParameter);
							lcdEditTextMessage = "SPLIT:OFF KY>C2 ";
							lcdEditBlnkMessage = "SPLIT:OFF KY>   ";
							DisplayEditFunction(lcdEditTextMessage,lcdEditBlnkMessage,1);
						break; // case PARAMETER_2:
					}	// switch(subMenuParameter)
				break; // subMenuParameter
			} // switch(level)
			*/
		break; // case FUNC_Split:
	}
}

void SplitCallbackFunction(uint8 splitPoint) {
	// numTasto = splitPoint-MIDI_FIRST_NOTE_61;
	WriteDataToEeprom(EEPROM_MIDI);
	DBG_PRINTF("%s splitpoint %d, write eeprom	immediatly\n",__func__,splitPoint);
	// exit menu
}

void doAction(uint8 selectedFunction, uint8 subMenuParameter, uint8 tasto) {
	uint8 splitPoint = 0;
	
	switch(selectedFunction) {
		case FUNC_Split:
			switch (subMenuParameter) { // parametro da cambiare
				
				case PARAMETER_1:
				/*
					if(tasto == 0) { // on/off split point in base a tasto off
						splitPoint = 0;
					} else { // tasto on
						splitPoint = MIDI_FIRST_NOTE_61+24;
					}
					
					SetSplitPoint(splitPoint);

					// DBG_PRINTF("[%s] nel display va scritto: SPLIT>{%s}  KEY#:%d, (menu level 1), func:%d, par:%d\n",__func__,noteNamearray[(sp%12)],tasto,selectedFunction,subMenuParameter);
					DBG_PRINTF("write eeprom immediatly\n");
				*/
				break;
				
				case PARAMETER_2:
					DBG_PRINTF("[%s] press any key to split...\n",__func__);
					// SplitGetNote();
				break;
			}
		break; // case FUNC_Split:
	} // switch(selectedFunction) 

}

// permette di selezionare una funzione da editare  con i tasti numerici
uint8 FunctionSelect(uint8 numTasto){
	static uint8 editFunctionToggle = 0;
	uint8 selectedFunction = 0;
	// #if MENU_DEBUG
	DBG_PRINTF("[%s] ",__func__);
	// #endif 
	
	if(prevKey != keyTranslator[numTasto]) {
		prevKey = keyTranslator[numTasto];
		editFunctionToggle = 0;
	}

	// do something(keyTranslator[numTasto],editFunctionToggle)
	switch(numTasto) {
		case 4:
			#if MENU_DEBUG
			DBG_PRINTF("vibrato/tune: ");
			#endif
			if(editFunctionToggle == 0) {
				DBG_PRINTF("vibrato");
				selectedFunction = FUNC_vibrato;
			} else {
				DBG_PRINTF("tune");
				selectedFunction = FUNC_tune;
			}
			break;

		case 5:
			#if MENU_DEBUG
			DBG_PRINTF("percussion/footSwitch: ");
			#endif
			if(editFunctionToggle == 0) {
				DBG_PRINTF("percussion");
				selectedFunction = FUNC_percussion;
			} else {
				DBG_PRINTF("footSwitch");
				selectedFunction = FUNC_footSwitch;
			}
			break;

		case 6:
			#if MENU_DEBUG
			DBG_PRINTF("Drawbar/Split: ");
			#endif
			if(editFunctionToggle == 0) {
				DBG_PRINTF("Drawbar");
				selectedFunction = FUNC_Drawbar;
			} else {
				DBG_PRINTF("Split");
				selectedFunction = FUNC_Split;
				
				/*
					2.premo tasto > e attivo il menu
					SPLIT>OFF, KEY#:C2
					DBG_PRINTF("[%s]\t %d=%s %d\n",__func__,NoteNumber, noteNamearray[(NoteNumber%12)], Velocity);
				*/
			}
			break;
		
		case 7:
			#if MENU_DEBUG
			DBG_PRINTF("Midi/Effect: ");
			#endif
			if(editFunctionToggle == 0) {
				DBG_PRINTF("Midi");
				selectedFunction = FUNC_Midi;
			} else {
				DBG_PRINTF("Effect");
				selectedFunction = FUNC_Effect;
			}
			break;
		
		case 8:
			#if MENU_DEBUG
			DBG_PRINTF("Preset/Reset: ");
			#endif
			if(editFunctionToggle == 0) {
				DBG_PRINTF("Preset");
				selectedFunction = FUNC_Preset;
			} else {
				DBG_PRINTF("Reset");
				selectedFunction = FUNC_Reset;
			}
			break;
	}
	DBG_PRINTF("\n");
	FunctionViewSelected(selectedFunction);
	editFunctionToggle = ~editFunctionToggle;
	
	return selectedFunction;
}

/*
uint8 SubMenuPage(uint8 selectedFunction) {
	static uint8 page = 0;	
	uint8 maxPages = 0;
	
	switch(selectedFunction) {
		case FUNC_vibrato:
		case FUNC_tune:
		case FUNC_percussion:
		case FUNC_footSwitch:
		case FUNC_Drawbar:
		case FUNC_Midi:
		case FUNC_Effect:
		case FUNC_Preset:
		case FUNC_Reset:
			break;
		
		case FUNC_Split:
			maxPages = 2;
			page++; 
			break;
	}
	
	page %= maxPages;
	
	DBG_PRINTF("%s %d\n",__func__,page);
	return page;
}
*/



/*****************************************************************************\
*  TEMPLATE: gestisce la funzione tasto  Generico
\*****************************************************************************/
void ManageButton_Preset(uint8 status,uint8 numTasto)
{
	if (GetEditMode()) // true in edit
	{
		switch (status) 
		{
	        case BUTTON_PRESSED:     // valido immediatamente
	        // case BUTTON_SHORT_PRESS:    // valido al rilascio breve
	        {
				// tasti validi 4,5,6,7,8
				switch(keyTranslator[numTasto]) {
					case TASTO_FRECCIA: // tasto_>
						if(selectedFunction) {
							// DBG_PRINTF("entro in submenu e blocco tutti i tasti tranne quello che mi interessa\n");
							switch(menuLevel) {
								case MENU_LEVEL_0: // pagina principale con EDIT attivo
									menuLevel = MENU_LEVEL_1;
									subMenuParameter = PARAMETER_0; // parametro da cambiare
									// DisplayEditFunction("MENU_LEVEL_1  ","MENU_LEVEL_1  ",0,ROW_0);
									// switch(selectedFunction) {
									//	case FUNC_Split:
											// FunctionViewParameter(selectedFunction,menuLevel,subMenuParameter);
									//	break;
									// } // switch(selectedFunction) 
								// break; // case MENU_LEVEL_0: cado subito giu altrimenti dovrei premere ddue volte
									
								case MENU_LEVEL_1: 
									switch(selectedFunction) {
										case FUNC_Split: // funzione con 2 parametri
											switch(subMenuParameter) {
												case PARAMETER_2:
												case PARAMETER_0:
													splitTextMessage[5] = '>'; // sostituisco i ":" con ">"
													splitTextMessage[12] = ':'; // sostituisco ">" con ":"
													memcpy(splitBlinkMessage,splitTextMessage,sizeof(splitTextMessage)); // copia tutto
													memset(&splitBlinkMessage[6],' ',3); // cancello il testo che non mi serve
													DisplayEditFunction(splitTextMessage,splitBlinkMessage,1,ROW_1); // attivo il lampeggio
													subMenuParameter = PARAMETER_1; // pagina principale con EDIT attivo
													// DBG_PRINTF("aaa\n");
												break;
											
												case PARAMETER_1:
													splitTextMessage[5]  = ':'; // sostituisco 
													splitTextMessage[12] = '>'; // sostituisco 
													memcpy(splitBlinkMessage,splitTextMessage,sizeof(splitTextMessage)); // copia tutto
													memset(&splitBlinkMessage[13],' ',3); // cancello il testo che non mi serve
													DisplayEditFunction(splitTextMessage,splitBlinkMessage,1,ROW_1); // attivo il lampeggio
													subMenuParameter = PARAMETER_2; // pagina principale con EDIT attivo
												break;
											} // switch(subMenuPage)
											// WriteSplitPoint();
										break;	// case FUNC_Split:
									} // switch(selectedFunction)
									
									// FunctionViewParameter(selectedFunction,menuLevel,subMenuParameter);
									
								break; // case MENU_LEVEL_1:
							} // switch(menuLevel)
						} // if(selectedFunction) 
						break; // case 1: // >
					
					case TASTO_ON_PIU: // on / +
						switch(menuLevel) {
							case MENU_LEVEL_1:
								switch(selectedFunction) {
									case FUNC_Split:
										switch (subMenuParameter) { // parametro da cambiare
											case PARAMETER_1:
												// DBG_PRINTF("func:%d, nel display va scritto: SPLIT>{ON}  KEY#:C2, (menu level 1), par:%d\n",selectedFunction,subMenuParameter);
												// doAction(selectedFunction,subMenuParameter,TASTO_ON_PIU);
												// splitTextMessage[5] = '>';
												// memcpy(splitBlinkMessage,splitTextMessage,sizeof(splitTextMessage)); // copia tutto
												// memset(&splitBlinkMessage[6],' ',3);
												SetSplitMode(1);
												splitTextMessage[6]='O';
												splitTextMessage[7]='n';
												splitTextMessage[8]=' ';
												DisplayEditFunction(splitTextMessage,splitBlinkMessage,1,ROW_1);
											break;
											
											case PARAMETER_2:
												// DBG_PRINTF("press any key to split...\n");
												// doAction(selectedFunction,subMenuParameter,TASTO_ON_PIU);
												//if(GetSplitPoint()) 
												{
													SetSplitPoint(1+GetSplitPoint());
												}
												WriteSplitPoint(1);
												// DBG_PRINTF("GetSplitPoint>%d\n",GetSplitPoint());
											break;
										}
									break; // case FUNC_Split:
								} // switch(selectedFunction) 
								
								// doAction(selectedFunction,subMenuParameter,1);
							break; // case MENU_LEVEL_1:
						} // switch(menuLevel)
					break; // case 2: // on / +
					
					case TASTO_OFF_MENO: // off / -
						switch(menuLevel) {
							case MENU_LEVEL_1:
								
								switch(selectedFunction) {
									case FUNC_Split:
										switch (subMenuParameter) { // parametro da cambiare
											case PARAMETER_1:
												// DBG_PRINTF("func:%d, nel display va scritto: SPLIT>{OFF}  KEY#:C2, (menu level 1), par:%d\n",selectedFunction,subMenuParameter);
												// doAction(selectedFunction,subMenuParameter,TASTO_OFF_MENO);
												SetSplitMode(0);
												splitTextMessage[6]='O';
												splitTextMessage[7]='f';
												splitTextMessage[8]='f';
												DisplayEditFunction(splitTextMessage,splitBlinkMessage,1,ROW_1);
											break; // case 1:
												
											case PARAMETER_2:
												// DBG_PRINTF("press any key to split...\n");
												// doAction(selectedFunction,subMenuParameter,TASTO_OFF_MENO);
												if(GetSplitPoint()-1) {
													SetSplitPoint(GetSplitPoint()-1);
												}
												
												WriteSplitPoint(1);
												// DBG_PRINTF("GetSplitPoint>%d\n",GetSplitPoint());
											break;
										}
									break; // case FUNC_Split:
								} // switch(selectedFunction) 
								
								// doAction(selectedFunction,subMenuParameter,0);
							break; // case 1:
						} // switch(menuLevel)
						break; // case 3: // off / -
					break;

					case TASTO_VIBRATO_TUNE: 	
					case TASTO_PERCUSSION_FOOTSW: 
					case TASTO_DRAWBAR_SPLIT: 
					case TASTO_MIDI_EFFECT: 
					case TASTO_PRESET_RESET: 
						switch(menuLevel) {
							case MENU_LEVEL_0:
								// seleziono la funzione
								selectedFunction = FunctionSelect(keyTranslator[numTasto]); // ritorna selectedFunction
							break;
								
							case MENU_LEVEL_1:
								// subMenuPage = SubMenuPage(selectedFunction);
								// DBG_PRINTF("posso cambiare pagina della funzione %d, se disponibile\n",selectedFunction);
							break;
						} // switch(menuLevel) 
						// se torna con una funzione allora attivo il menu (!= 0)
						break; // casi 4:8
				} // switch(keyTranslator[numTasto])
				break;
	        }

			case BUTTON_ON_HOLD:        // valido al mantenimento
        	{
				// tasti validi 4,5,6,7,8
				switch(keyTranslator[numTasto]) {
					case TASTO_VIBRATO_TUNE: 	
					case TASTO_PERCUSSION_FOOTSW: 
					case TASTO_DRAWBAR_SPLIT: 
					case TASTO_MIDI_EFFECT: 
					case TASTO_PRESET_RESET: 

					case TASTO_ON_PIU:
					case TASTO_OFF_MENO:
					
					// EditFunctionSelect(keyTranslator[numTasto]);
						break;
				}
				break;
			} 
		}
		
		return; // evita di cambiare lo stato in edit mode
	} // else...
	
	DBG_PRINTF(" resetto per entrare bene la prossima volta.\n");
	selectedFunction = 0;
	menuLevel = MENU_LEVEL_0;
	subMenuParameter = PARAMETER_0;
	
	uint8 sommatore;
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
			userPresetManager(keyTranslator[numTasto]);
			presetStatus = PRESET_CUSTOM;
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        // valido al mantenimento
        {
			// DBG_PRINTF("ON_HOLD %d\n",keyTranslator[numTasto]);
			sommatore = 8*buttonCycle[keyTranslator[numTasto]];
			
			DBG_PRINTF("Preset %d->%d\n",keyTranslator[numTasto],sommatore);
			
			Display_Alternate_Text(ROW_1,ALT_Preset_1+(keyTranslator[numTasto]-1)+sommatore);
			// SendControlChange(keyTranslator[numTasto],MIDI_CHANNEL_1);
			SendProgramChange(sommatore+keyTranslator[numTasto], MIDI_CHANNEL_1);
			
			buttonCycle[keyTranslator[numTasto]]++; // inclrementa il ciclo del tasto
			if (buttonCycle[keyTranslator[numTasto]] == 4) {
				buttonCycle[keyTranslator[numTasto]] = 0;
			}
			
			presetStatus = PRESET_FACTORY;
        }
        break;
        
        case BUTTON_RELEASED:       // tasto rilasciato
        {
            
        }
        break;
    }       
}

/*****************************************************************************\
*  In base al taaso premuto richiama la corrispondente funzione
\*****************************************************************************/
void ButtonCommand(uint8 numTasto,uint8 status)
{
    // funzione chiamata ad ogni pressione di un pulsante
    // DBG_PRINTF("[%s] tasto %d status %d\n",__func__,numTasto,status);
    // status torna: 1 pressione immediata, 
    // 3 rilascio veloce, 
    // 2 pressione prlungata, 
    // 4 riolascio da pressione prolungata
    EditFunction = numTasto;
	
    switch(numTasto)
    {
        case BUTTON_00_VIBRATO:
        { // vibrato
            ManageButton_Vibrato(status);
        }
        break;

        case BUTTON_01_LESLIE:
        { // Leslie 
            ManageButton_Leslie(status);
        }
        break;
        
        case BUTTON_08_PERC_2ND:
        { // percussion on/off, level soft normal
            ManageButton_PercussionType(status);
        }
        break;

        case BUTTON_04_PERC_3RD:
        { // percussion 3rd 2nd / fast slow
            ManageButton_PercussionLevel(status);
        }
        break;

        case BUTTON_13_SHIFT_CANCEL:
        { // shift Button
            ManageButton_Shift(status);
        }
        break;
        
        case BUTTON_12_SOLO:
        {
            ManageButton_Solo(status);
        }
        break;
        
        case BUTTON_09_RECORD:
		{
            ManageButton_Record(status);
        }
		break;
		
		// 5,13
        case BUTTON_05_EDIT:
		{
            ManageButton_Edit(status);
        }
		break;
		
        case BUTTON_14_KEY_1:
        case BUTTON_10_KEY_2:
        case BUTTON_06_KEY_3:
        case BUTTON_02_KEY_4:
        case BUTTON_15_KEY_5:
        case BUTTON_11_KEY_6:
        case BUTTON_07_KEY_7:
        case BUTTON_03_KEY_8:
        {
            ManageButton_Preset(status,numTasto);
        }
        break;
        
        default:
        break;
    } // switch(numTasto)
    
    // chiamare una scrittura in eeprom
	if(GetEditMode() == EDIT_MODE_OFF) {
    	WriteDataToEeprom(EEPROM_BUTTON);
    }
} // ButtonCommand

/*****************************************************************************\
*  gestisce il tipo di pressione del tasto
\*****************************************************************************/
void ButtonManager(void)
{
    uint8 numTasto;
    for(numTasto=0;numTasto<MAX_PULSANTI;numTasto++)
    {
        switch(button[numTasto].status)
        {
            case BUTTON_PRESSED:
            {
                if (bitRead(button[numTasto].oneShot,0) == 0) {   
                    // esegui una sola volta
                    // DBG_PRINTF("tasto %d appena premuto valido, con dito ancora sopra il pulsante\n",numTasto);
                    ButtonCommand(numTasto,button[numTasto].status);
                    bitSet(button[numTasto].oneShot,0);
                }
            } break;
            
            case BUTTON_ON_HOLD:
            {
                if (bitRead(button[numTasto].oneShot,1) == 0) {
                    // esegui una sola volta
                    // DBG_PRINTF("tasto %d tenuto premuto, con dito ancora sopra il pulsante\n",numTasto);
                    ButtonCommand(numTasto,button[numTasto].status);
                    bitSet(button[numTasto].oneShot,1);
                }
            } break;
            
            case BUTTON_SHORT_PRESS:
            {
                // DBG_PRINTF("tasto %d rilasciato da pressione breve\n",numTasto);
                ButtonCommand(numTasto,button[numTasto].status);
                button[numTasto].status = BUTTON_RELEASED;
                button[numTasto].oneShot = 0; // resetta il singolo sparo
            } break;
            
            case BUTTON_LONG_RELEASE:
            {
                // DBG_PRINTF("tasto %d rilasciato da pressione prolungata\n",numTasto);
                ButtonCommand(numTasto,button[numTasto].status);
                button[numTasto].status = BUTTON_RELEASED;
                button[numTasto].oneShot = 0; // resetta il singolo sparo
            } break;
                
            default:
                break;
        }
    }
}



/*****************************************************************************\
*  scansiona le linee dei tasti
\*****************************************************************************/
void ButtonScanner(void)
{
    // tra un select e un read passa circa 1ms
    static uint8 writeLine = 0;
    uint8 data = 0;
    uint8 readLine = 0;
    uint8 numTasto = 0;
    static uint8 stateMachine = 0;
    
    switch(stateMachine)
    {
        case ButtonScanner_SELECT:
        {
            stateMachine = ButtonScanner_READ;
            Control_Reg_Button_Line_Select_Write(writeLine); // Selects row 
        }
        break;
    
        case ButtonScanner_READ:
        // for(writeLine = 0;writeLine < 4;writeLine++)
        { // seleziona le linee dei banchi una alla volta col demultiplexer
            // Control_Reg_Button_Line_Select_Write(writeLine); // Selects row 
            // DBG_PRINTF("sel line %02d ",line);
            // CyDelayUs(100);
            data = ButtonInputPort_Read(); // leggi lo stato dei tasti 8 per volta
            
                   // if (line == 0) DBG_PRINTF("var : %02x\n",var);
            for(readLine=0;readLine<4;readLine++)
            { // permette di decidere dove mettere il dato letto dalle 8 linee
      //          DBG_PRINTF("tasto %d campionato, %d %d %02x\n",numTasto,bank*2,bank*2+1,var);
                numTasto = readLine + (4*writeLine); // (0 1 2 3) + (4*0) | (4*1) | (4*2) | (4*3) 
                if (bitRead(data, readLine)) // tasto premuto
                {
                    if (button[numTasto].debounce < MAX_DEBOUNCE) 
                    {   // incremento ildebounce del tasto
                        button[numTasto].debounce++;
                        
                        if (button[numTasto].debounce > MIN_DEBOUNCE)
                        {
                            button[numTasto].status = BUTTON_PRESSED;
                        }
                    }
                    else
                    {
                        // sono in pulsante premuto
                        button[numTasto].status = BUTTON_ON_HOLD;
                    }
                }
                else // tasto rilasciato
                {
                    switch(button[numTasto].status) 
                    {
                        case BUTTON_PRESSED: // ho già passato il minimo debounce utile
                            button[numTasto].status = BUTTON_SHORT_PRESS;
                        break;
                        
                        case BUTTON_ON_HOLD:
                            button[numTasto].status = BUTTON_LONG_RELEASE;
                        break;
                    }
                        
                    button[numTasto].debounce = 0;
                    // button[numTasto].status = BUTTON_RELEASED;
                }
            }
            
            writeLine++;
            if (writeLine == 4) writeLine = 0;
            
            stateMachine = ButtonScanner_SELECT;
        }
        break;
    }
        

}

/*****************************************************************************\
*  inizializza la scansione della matrice dei tasti
\*****************************************************************************/
void ButtonScannerInit(void)
{
    uint8 i;
    
    // MyPin_SetDriveMode(MyPin_DM_RES_UP);
    buttonInputScan_S0_SetDriveMode(buttonInputScan_S0_DM_RES_DWN);
    buttonInputScan_S1_SetDriveMode(buttonInputScan_S1_DM_RES_DWN);
    buttonInputScan_S2_SetDriveMode(buttonInputScan_S2_DM_RES_DWN);
    buttonInputScan_S3_SetDriveMode(buttonInputScan_S3_DM_RES_DWN);

    buttonInputScan_S0_Write(0); // attiva PullDown
    buttonInputScan_S1_Write(0); // attiva PullDown
    buttonInputScan_S2_Write(0); // attiva PullDown
    buttonInputScan_S3_Write(0); // attiva PullDown

    
    for(i=0;i<MAX_PULSANTI;i++)
    {
        button[i].debounce = 0;
        button[i].status = BUTTON_RELEASED;
        button[i].oneShot = 0;
    }
    
    InitSwitchButtons();
}

/*****************************************************************************\
*  chiamata in continuo nel main
\*****************************************************************************/
void ButtonScannerPoll(void)
{
    static uint8 isButtonScannerInitialized = FALSE;
    
    if(isButtonScannerInitialized == FALSE) {
        ButtonScannerInit();
        isButtonScannerInitialized = TRUE;
    }
    
    if (tick_1ms(TICK_BUTTONSCANNER)) {
        ButtonScanner();
        ButtonManager();
    }
	
	if (tick_10ms(TICK_BUTTONSCANNER)) {
		FootSwitchManager();
    }
}

uint8 GetVolumeSolo(void) {
	return soloVolume;
}

void FootSwitchManager(void) {
	#define FOOTSWITCH_DEBOUNCE 5 // per 10ms
	static uint8 debounce[3] = {FOOTSWITCH_DEBOUNCE,FOOTSWITCH_DEBOUNCE,FOOTSWITCH_DEBOUNCE};
	static uint16 counter[3] = {0,0,0};
	
	if(FootSwitch_0_Read() == 0){
		if(debounce[0]) {
			debounce[0]--;
			if(debounce[0] == 0) {
				DBG_PRINTF("FootSwitch_0 pressed %d\n",counter[0]++);
				/* fsw0FunctionAssigned */
				ManageButton_Leslie (BUTTON_SHORT_PRESS);
			}
		}
	} else {
		if(debounce[0] == 0){
			DBG_PRINTF("FootSwitch_0 released %d\n",counter[0]++);
		}
		debounce[0] = FOOTSWITCH_DEBOUNCE;
	}
	
	if(FootSwitch_1_Read() == 0){ // pedalino premuto
		if(debounce[1]) {
			debounce[1]--;
			if(debounce[1] == 0) {
				DBG_PRINTF("FootSwitch_1 pressed %d\n",counter[1]++);
				ManageDamperPedal(BUTTON_PRESSED);
			}
		}
	} else {
		if(debounce[1] == 0){
			DBG_PRINTF("FootSwitch_1 released %d\n",counter[1]++);
			ManageDamperPedal(BUTTON_RELEASED);
		}
		debounce[1] = FOOTSWITCH_DEBOUNCE;
		
	}
	
	if(FootSwitch_2_Read() == 0){
		if(debounce[2]) {
			debounce[2]--;
			if(debounce[2] == 0) {
				DBG_PRINTF("FootSwitch_2 pressed %d\n",counter[2]++);
			}
		}
	} else {
		if(debounce[2] == 0){
			DBG_PRINTF("FootSwitch_2 released %d\n",counter[2]++);
		}
		debounce[2] = FOOTSWITCH_DEBOUNCE;
	}
}

uint8 GetPresetStatus(void) {
	
	return (presetStatus == PRESET_FREE);
}

uint8 GetButtonStatus(uint8 numTasto) {
	return (button[numTasto].status);
}

uint8 GetEditMode(void) {
	 // torna solo true/false
	return EditMode;
}

uint8 GetEditFunction(void) {
	// TODO
	return EditFunction;
}

void WriteSplitPoint(uint8 blnk) {
	if(GetSplitPoint()<MIDI_FIRST_NOTE_61) {
		SetSplitPoint(MIDI_FIRST_NOTE_61);
	}
	
	uint8 note = (GetSplitPoint()-MIDI_FIRST_NOTE_61)%12;
	uint8 len;
	// memset(splitTextMessage,'v',sizeof(splitTextMessage));
	
	
	len=sprintf(splitTextMessage,"SPLIT:%s KY%s%s%d",
		GetSplitMode()?"On ":"Off",\
		blnk?">":":",\
		noteNamearray[note],\
		GetSplitPoint()/12);
	
	if(len%2) { // se dispari aggiungo uno spazio
		sprintf(splitTextMessage,"%s%s",splitTextMessage," ");
	}
	
	// DBG_PRINTF("%s %d\n",__func__,a);	
	DisplayEditFunction(splitTextMessage,splitBlinkMessage,blnk,ROW_1);
	
}
/* [] END OF FILE */
