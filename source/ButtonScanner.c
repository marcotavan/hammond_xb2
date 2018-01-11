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

#define MIN_DEBOUNCE 5      // * 8ms
#define MAX_DEBOUNCE 100     // * 8ms  
#define MAX_PULSANTI 16

#define BUTTON_UART_DEBUG (0)
static struct onHold_s {
    uint8 shift;
    uint8 solo;
}OnHold;

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
static uint8 EditMode = 0;

void FootSwitchManager(void);
void ResetButtonCycle(void);

void RefreshAllButtonElements(void) {
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
/*****************************************************************************\
*  
\*****************************************************************************/
void InitSwitchButtons(void)
{
    // ricarica da eeprom se c'è il marker else prendi da default i pulsanti.
    internal_eeprom.RegPointer = (reg8 *)(CYDEV_EE_BASE + (CYDEV_EEPROM_ROW_SIZE * EEPROM_ROW_BUTTONS));  
        
    // punto all-inizio della struttura
    memcpy((uint8 *) &switchType,(uint8 *) internal_eeprom.RegPointer,sizeof(switchType));    

    if (switchType.marker != MARKER_EEPROM_DEFAULT)
    {
        // prendi da default else carica da eeprom
        switchType.marker = MARKER_EEPROM_DEFAULT;

        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;        // ok
        switchType.rotarySpeaker_bypass = SWITCH_ON;            // ok
        switchType.Tube_Overdrive_Switch = SWITCH_OFF;
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
    
	RefreshAllButtonElements();
	
	StartMidiMerge();
	// aggiungere lo SPLIT ON
    // Display_Alternate_Text(ROW_1,ALT_InitSwitchButtons_Done);
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
*  gestisce la funzione Percussion Type
\*****************************************************************************/
void ManageButton_PercussionType(uint8 status)
{
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
*  gestisce la funzione Shift Button Pressed
\*****************************************************************************/
void ManageButton_Shift(uint8 status)
{
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
            Display_Alternate_Text(ROW_1,ALT_Cancel_on_Press);
			ResetButtonCycle();
			SendProgramChange(0, MIDI_CHANNEL_1);
			
			OnHold.shift = FALSE;
            // DBG_PRINTF("Shift Released, i led del pannello tornano normali\n");
			RefreshAllAnalogElements();
			presetStatus = PRESET_FREE;
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio
        case BUTTON_ON_HOLD:        
        {
            OnHold.shift = TRUE;
            // servirà un timeout?
            DBG_PRINTF("Shift on Hold, far lampeggiare i led del pannello -> alternate Function\n");
            // si accende il led dello shift se c'è^?
			Display_Alternate_Text(ROW_1,ALT_Shift_on_Hold);
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
				Display_Alternate_Text(ROW_1,ALT_Volume_Max);
			} else {
				sendControlChange(CC_Overall_Volume,GetOverallVolumeLevel(),MIDI_CHANNEL_1);
				soloVolume = VOLUME_NORMAL;
				Display_Alternate_Text(ROW_1,ALT_Volume_Normal);
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
			RefreshAllButtonElements();
			RefreshAllAnalogElements();
        }
        break;
        
        case BUTTON_RELEASED:       // tasto rilasciato
        {
            
        }
        break;
    }       
}


void ManageButton_Edit(uint8 status)
{
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {
			DBG_PRINTF("ManageButton_Edit BUTTON_SHORT_PRESS\n");
			EditMode = 0;
			Display_Alternate_Text(ROW_1,ALT_Edit);
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        // valido al mantenimento
        {
			switch(EditMode) {
				case 0:
					EditMode = 1;
					break;
				case 1:
					EditMode = 0;
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

static uint8 buttonCycle[9] = {0,0,0,0,0,0,0,0,0};
	
void ResetButtonCycle(void) {
	memset(buttonCycle,0,sizeof(buttonCycle));
}
/*****************************************************************************\
*  TEMPLATE: gestisce la funzione tasto  Generico
\*****************************************************************************/
void ManageButton_Preset(uint8 status,uint8 numTasto)
{

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
    WriteDataToEeprom(EEPROM_BUTTON);
    
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
		debounce[0] = FOOTSWITCH_DEBOUNCE;
	}
	
	if(FootSwitch_1_Read() == 0){
		if(debounce[1]) {
			debounce[1]--;
			if(debounce[1] == 0) {
				DBG_PRINTF("FootSwitch_1 pressed %d\n",counter[1]++);
			}
		}
	} else {
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
	return EditMode;
}

uint8 GetEditFunction(void) {
	// TODO
	return BUTTON_01_LESLIE;
}
/* [] END OF FILE */
