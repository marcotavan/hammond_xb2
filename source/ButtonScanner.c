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

Implementazione:
Leslie: toggle slow verde/Fast rosso. tenedo premuto va a stop giallo lampeggiante, ripremendo parte in fast. Parametro per ripartire in fast o slow
-> scrivere nel display torna utile

vibrato: toggle on/off upper, long press toggle on off lower. default c3, edit per cambiare


*/


// attenzione che ho girato le linee per mantenere uniformit' con la tastiera

#include "project.h"
#include "ButtonScanner.h"
#include "common.h"
#include "debug.h"
#include "tick.h"
#include "midiLibrary.h"
#include "VB3_midi_map.h"

#define MIN_DEBOUNCE 9      // * 8ms
#define MAX_DEBOUNCE 120     // * 8ms  
#define MAX_PULSANTI 16

enum _num_button_
{
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_7,
    BUTTON_8,
    BUTTON_9,
    BUTTON_10,
    BUTTON_11,
    BUTTON_12,
    BUTTON_13,
    BUTTON_14,
    BUTTON_15    
};

enum _button_states_
{
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_ON_HOLD,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS
};

enum _rotary_switch_
{
    ROTARY_SLOW = 0x00,
    ROTARY_STOP = 0x3F,
    ROTARY_FAST = 0x7F
};

enum _switch_
{
    SWITCH_OFF = 0x00,
    SWITCH_ON = 0x7F
};

enum _chorus_type_
{
    CHORUS_C1,
    CHORUS_V2,
    CHORUS_C2,
    CHORUS_V3,
    CHORUS_C3,
    CHORUS_V1
};

enum _perc_level_
{
    PERC_SOFT = 0x00,
    PERC_NORM = 0x7F
};

enum _perc_decay_
{
    PERC_FAST = 0x00,
    PERC_SLOW = 0x7F
};

enum _perc_type_
{
    PERC_2ND = 0x00,
    PERC_3RD = 0x7F
};

enum _preset_
{
    PRESET_A,
    PRESET_B  
};

struct {
    uint8 debounce;
    uint8 status;
    uint8 oneShot;
} button[MAX_PULSANTI];

// potrebbe andare in eeprom
struct {
    uint8 marker;
    uint8 rotarySpeaker_HalfMoon;
    uint8 rotarySpeaker_bypass;
    uint8 Tube_Overdrive_Switch;
    uint8 Vibrato_Lower_Switch;
    uint8 Vibrato_Upper_Switch;
    uint8 chorus_Knob;
    uint8 percussion_Switch;
    uint8 percussionLevel_Switch;
    uint8 percussionDecay_Switch;
    uint8 percussionHarmonics_Switch;
    uint8 upperManualPreset_Switch;
    uint8 lowerManualPreset_Switch;
} switchType;


void InitSwitchButtons(void)
{
    // ricarica da eerpom se c'è il marker else prendi da default i pulsanti.
    if (switchType.marker != MARKER_EEPROM_DEFAULT_BUTTON)
    {
        // prendi da default else carica da eeprom
        switchType.marker = MARKER_EEPROM_DEFAULT_BUTTON;

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
    }
    else
    {
        // carica da eeprom   
    }
    
    // invia via midi la configurazione di default
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,    switchType.rotarySpeaker_HalfMoon,      MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Rotary_Speaker_Bypass,             switchType.rotarySpeaker_bypass,        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Tube_Overdrive_Switch,             switchType.Tube_Overdrive_Switch,       MIDI_CHANNEL_1);
    CyDelay(10);

    sendControlChange(CC_Vibrato_Lower,                     switchType.Vibrato_Lower_Switch,        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Vibrato_Upper,                     switchType.Vibrato_Upper_Switch,        MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_On_Off,                 switchType.percussion_Switch,           MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_Volume,                 switchType.percussionLevel_Switch,      MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_Decay,                  switchType.percussionDecay_Switch,      MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Percussion_Harmonic,               switchType.percussionHarmonics_Switch,  MIDI_CHANNEL_1);
    CyDelay(10);
    
    sendControlChange(CC_Vibrato_Type,                      switchType.chorus_Knob,                 MIDI_CHANNEL_1);
    CyDelay(10);
    
    /*
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x7F,MIDI_CHANNEL_1);
    CyDelay(10);
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x7F,MIDI_CHANNEL_1);
    CyDelay(10);
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x7F,MIDI_CHANNEL_1);
    CyDelay(10);
    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x7F,MIDI_CHANNEL_1);
    CyDelay(10);
    */
    
 }

void ButtonCommand(uint8 numTasto,uint8 status)
{
    DBG_PRINTF("[%s] tasto %d status %d\n",__func__,numTasto,status);
    // status torna: 1 pressione immediata, 
    // 3 rilascio veloce, 
    // 2 pressione prlungata, 
    // 4 riolascio da pressione prolungata
    
    switch(numTasto)
    {
        case BUTTON_0:
        {
            switch (status) 
            {
                // case BUTTON_PRESSED:
                case BUTTON_SHORT_PRESS:
                {
                    if(switchType.rotarySpeaker_HalfMoon == ROTARY_SLOW) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_FAST;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,ROTARY_FAST,MIDI_CHANNEL_1);
                    }
                    else if(switchType.rotarySpeaker_HalfMoon == ROTARY_FAST) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,ROTARY_SLOW,MIDI_CHANNEL_1);
                    }
                    else if(switchType.rotarySpeaker_HalfMoon == ROTARY_STOP) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_FAST;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,ROTARY_FAST,MIDI_CHANNEL_1);
                    }
                }
                break;
                
                case BUTTON_ON_HOLD:
                {
                    if(switchType.rotarySpeaker_HalfMoon != ROTARY_STOP) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_STOP;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,ROTARY_STOP,MIDI_CHANNEL_1);
                    }
                    else if(switchType.rotarySpeaker_HalfMoon == ROTARY_STOP) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,ROTARY_SLOW,MIDI_CHANNEL_1);
                    }
                }
                break;
                
            }
        }
        break;

        case BUTTON_8:
        {
            switch (status) 
            {
                // case BUTTON_PRESSED:
                case BUTTON_SHORT_PRESS:
                {
                    
                }
                break;
                
                // case BUTTON_LONG_PRESS
                case BUTTON_ON_HOLD:
                {
                    
                }
                break;
                
            }   
        }
        break;
        
        case 9:
        {
            switch (status) 
            {
                // case BUTTON_PRESSED:
                case BUTTON_SHORT_PRESS:
                {

                }
                break;
                
                // case BUTTON_LONG_PRESS
                case BUTTON_ON_HOLD:
                {

                }
                break;
                
            }   
        }
        break;
        
        case 13:
        
        break;

        default:
        break;
    }
    
    
}


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
            
            case BUTTON_LONG_PRESS:
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

enum {
    ButtonScanner_SELECT,
    ButtonScanner_READ
};

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
                            button[numTasto].status = BUTTON_LONG_PRESS;
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

void ButtonScannerPoll(void)
{
    // chiamata ogni 1 ms
    static uint8 isButtonScannerInitialized = FALSE;
    
    if(isButtonScannerInitialized == FALSE) {
        ButtonScannerInit();
        isButtonScannerInitialized = TRUE;
    }
    
    ButtonScanner();
    ButtonManager();
}
/* [] END OF FILE */
