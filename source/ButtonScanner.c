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


// attenzione che ho girato le linee per mantenere uniformit' con la tastiera

#include "project.h"
#include "ButtonScanner.h"
#include "common.h"
#include "debug.h"
#include "tick.h"
#include "midiLibrary.h"
#include "VB3_midi_map.h"

#define MIN_DEBOUNCE 9      // * 8ms
#define MAX_DEBOUNCE 110     // * 8ms  
#define MAX_PULSANTI 16

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
    ROTARY_SLOW,
    ROTARY_FAST,
    ROTARY_STOP
};

enum _switch_
{
    SWITCH_OFF,
    SWITCH_ON,
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
    PERC_SOFT,
    PERC_NORM
};

enum _perc_decay_
{
    PERC_FAST,
    PERC_SLOW
};

enum _perc_type_
{
    PERC_2ND,
    PERC_3RD
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
    uint8 rotarySpeaker_Switch;
    uint8 overdrive_Switch;
    uint8 chorusUp_Switch;
    uint8 chorusDown_Switch;
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

        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
        switchType.rotarySpeaker_Switch = SWITCH_ON;
        switchType.overdrive_Switch = SWITCH_OFF;
        switchType.chorusUp_Switch = SWITCH_OFF;
        switchType.chorusDown_Switch = SWITCH_OFF;
        switchType.chorus_Knob = CHORUS_C1;
        switchType.percussion_Switch = SWITCH_ON;
        switchType.percussionLevel_Switch = PERC_SOFT;
        switchType.percussionDecay_Switch = PERC_FAST;
        switchType.percussionHarmonics_Switch = PERC_2ND;
        switchType.upperManualPreset_Switch = PRESET_B;
        switchType.lowerManualPreset_Switch = PRESET_B;
    }
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
        case 8:
        {
            switch (status) 
            {
                // case BUTTON_PRESSED:
                case BUTTON_SHORT_PRESS:
                {
                    if(switchType.rotarySpeaker_HalfMoon == ROTARY_SLOW) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_FAST;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x7F,MIDI_CHANNEL_1);
                    }
                    else if(switchType.rotarySpeaker_HalfMoon == ROTARY_FAST) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0,MIDI_CHANNEL_1);
                    }
                    else if(switchType.rotarySpeaker_HalfMoon == ROTARY_STOP) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_FAST;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x7F,MIDI_CHANNEL_1);
                    }
                }
                break;
                
                case BUTTON_ON_HOLD:
                {
                    if(switchType.rotarySpeaker_HalfMoon != ROTARY_STOP) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_STOP;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0x3F,MIDI_CHANNEL_1);
                    }
                    else if(switchType.rotarySpeaker_HalfMoon == ROTARY_STOP) {
                        switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                        sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,0,MIDI_CHANNEL_1);
                    }
                }
                break;
                
            }
        }
        break;

        case 9:
        
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
