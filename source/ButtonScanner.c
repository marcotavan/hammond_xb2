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

#define MIN_DEBOUNCE 9      // * 8ms
#define MAX_DEBOUNCE 120     // * 8ms  
#define MAX_PULSANTI 16

static uint8 shiftOnHold = FALSE;

static char str[20];
const uint8 vibratoScannerPosition[] = {1,1,2,2,3,3};
const uint8 vibratoScannerMidiValue[] = {11,33,55,77,99,127};

enum _num_button_ {
    BUTTON_0_VIBRATO,   
    BUTTON_1_LESLIE,
    BUTTON_2_KEY_4,
    BUTTON_3_KEY_8,
    BUTTON_4_PERC_3RD,
    BUTTON_5_EDIT,
    BUTTON_6_KEY_3,
    BUTTON_7_KEY_7,
    BUTTON_8_PERC_2ND,
    BUTTON_9_REC,
    BUTTON_10_KEY_2,
    BUTTON_11_KEY_6,
    BUTTON_12_SOLO,
    BUTTON_13_CANC,
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

struct {
    uint8 debounce;
    uint8 status;
    uint8 oneShot;
} button[MAX_PULSANTI];

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
    
    Display_Write_Text(ROW_1,"InitSwitchButtons Done");
 }

/*****************************************************************************\
*  restituisce se il tasto shift e' premuto
\*****************************************************************************/
uint8 SHIFT_Button_on_Hold(void)
{
    return shiftOnHold; 
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
                switchType.rotarySpeaker_HalfMoon = ROTARY_FAST;
                Display_Write_Text(ROW_1,"ROTARY_FAST");
                break;
                
                case ROTARY_FAST: 
                switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                Display_Write_Text(ROW_1,"ROTARY_SLOW");
                break;
                
                default:
                switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                Display_Write_Text(ROW_1,"ROTARY_SLOW");
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
                switchType.rotarySpeaker_HalfMoon = ROTARY_SLOW;
                Display_Write_Text(ROW_1,"RESTART ROTARY_SLOW");
                break;
                
                default:
                switchType.rotarySpeaker_HalfMoon = ROTARY_STOP;
                Display_Write_Text(ROW_1,"ROTARY_STOP");
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
void ManageButton_Vibrato(status)
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
                    break;
                
                    case SWITCH_ON: 
                    switchType.Vibrato_Upper_Switch = SWITCH_OFF;
                    break;
                    
                    default:
                    switchType.Vibrato_Upper_Switch = SWITCH_OFF;
                    break;
                }
                
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
                    break;
                    
                    case SWITCH_ON:
                    switchType.Vibrato_Lower_Switch = SWITCH_OFF;
                    break;
                    
                    default:
                    switchType.Vibrato_Lower_Switch = SWITCH_OFF;
                    break;
                }
                
                sendControlChange(CC_Vibrato_Lower,       
                    switchType.Vibrato_Lower_Switch,     
                    MIDI_CHANNEL_1);
            }
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
                sprintf(str,"VIBRATO SCANNER %s%d",switchType.chorus_Knob%2?"C":"V", vibratoScannerPosition[switchType.chorus_Knob]);
                Display_Write_Text(ROW_1,str);    
            }
            break;
        }   
    }   
}

/*****************************************************************************\
*  gestisce la funzione Percussion Level
\*****************************************************************************/
void ManageButton_PercussionLevel(status) 
{
    switch (status) 
    {
        // case BUTTON_PRESSED:
        case BUTTON_SHORT_PRESS:
        {
            switch(switchType.percussion_Switch) 
            {
                case SWITCH_OFF:
                switchType.percussion_Switch = SWITCH_ON;
                Display_Write_Text(ROW_1,"Percussion_On");
                break;
            
                case SWITCH_ON: 
                switchType.percussion_Switch = SWITCH_OFF;
                Display_Write_Text(ROW_1,"Percussion_Off");
                break;
                
                default:
                switchType.percussion_Switch = SWITCH_OFF;
                Display_Write_Text(ROW_1,"Percussion_Off");
                break;
            }
            
            sendControlChange(CC_Percussion_On_Off,   
                switchType.percussion_Switch,        
                MIDI_CHANNEL_1);
        }
        break;
        
        // case BUTTON_LONG_PRESS
        case BUTTON_ON_HOLD:
        {
            switch(switchType.percussionLevel_Switch) 
            {
                case PERC_SOFT:
                switchType.percussionLevel_Switch = PERC_NORM;
                Display_Write_Text(ROW_1,"Percussion_NORM");
                break;
            
                case PERC_NORM: 
                switchType.percussionLevel_Switch = PERC_SOFT;
                Display_Write_Text(ROW_1,"Percussion_SOFT");
                break;
                
                default:
                switchType.percussionLevel_Switch = PERC_SOFT;
                Display_Write_Text(ROW_1,"Percussion_SOFT");
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
            switch(switchType.percussionHarmonics_Switch) 
            {
                case PERC_2ND:
                switchType.percussionHarmonics_Switch = PERC_3RD;
                Display_Write_Text(ROW_1,"Percussion_3RD");
                break;
            
                case PERC_3RD: 
                switchType.percussionHarmonics_Switch = PERC_2ND;
                Display_Write_Text(ROW_1,"Percussion_2ND");
                break;
                
                default:
                switchType.percussionHarmonics_Switch = PERC_2ND;
                Display_Write_Text(ROW_1,"Percussion_2ND");
                break;
            }
            
            sendControlChange(CC_Percussion_Harmonic,       
                switchType.percussionHarmonics_Switch,      
                MIDI_CHANNEL_1);
        }
        break;
        
        // case BUTTON_LONG_PRESS
        case BUTTON_ON_HOLD:
        {
            switch(switchType.percussionDecay_Switch) 
            {
                case PERC_FAST:
                switchType.percussionDecay_Switch = PERC_SLOW;
                Display_Write_Text(ROW_1,"Percussion_SLOW");
                break;
            
                case PERC_SLOW: 
                switchType.percussionDecay_Switch = PERC_FAST;
                Display_Write_Text(ROW_1,"Percussion_FAST");
                break;
                
                default:
                switchType.percussionDecay_Switch = PERC_FAST;
                Display_Write_Text(ROW_1,"Percussion_FAST");
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
            
        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio
        case BUTTON_ON_HOLD:        
        {
            shiftOnHold = TRUE;
            // servirà un timeout?
            DBG_PRINTF("Shift on Hold, far lampeggiare i led del pannello -> alternate Function\n");
            // si accende il led dello shift se c'è^?
        }
        break;
        
        case BUTTON_RELEASED:
        {
            shiftOnHold = FALSE;
            DBG_PRINTF("Shift Released, i led del pannello tornano normali\n");
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

        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        // valido al mantenimento
        {

        }
        break;
        
        case BUTTON_RELEASED:       // tasto rilasciato
        {
            
        }
        break;
    }   
}

/*****************************************************************************\
*  TEMPLATE: gestisce la funzione tasto  Generico
\*****************************************************************************/
void ManageButton_Generic(uint8 status)
{
    switch (status) 
    {
        // case BUTTON_PRESSED:     // valido immediatamente
        case BUTTON_SHORT_PRESS:    // valido al rilascio breve
        {

        }
        break;
        
        // case BUTTON_LONG_PRESS   // valido al rilascio lungo
        case BUTTON_ON_HOLD:        // valido al mantenimento
        {

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
    DBG_PRINTF("[%s] tasto %d status %d\n",__func__,numTasto,status);
    // status torna: 1 pressione immediata, 
    // 3 rilascio veloce, 
    // 2 pressione prlungata, 
    // 4 riolascio da pressione prolungata
    
    switch(numTasto)
    {
        case BUTTON_0_VIBRATO:
        { // vibrato
            ManageButton_Vibrato(status);
        }
        break;

        case BUTTON_1_LESLIE:
        { // Leslie 
            ManageButton_Leslie(status);
        }
        break;
        
        
        case BUTTON_14_KEY_1:
        case BUTTON_10_KEY_2:
        case BUTTON_6_KEY_3:
        case BUTTON_2_KEY_4:
        case BUTTON_15_KEY_5:
        case BUTTON_11_KEY_6:
        case BUTTON_7_KEY_7:
        case BUTTON_3_KEY_8:
        break;
        
        case BUTTON_8_PERC_2ND:
        { // percussion on/off, level soft normal
            ManageButton_PercussionLevel(status);
        }
        break;

        case BUTTON_4_PERC_3RD:
        { // percussion 3rd 2nd / fast slow
            ManageButton_PercussionType(status);
        }
        break;

        case BUTTON_9_REC:
        { // shift Button
            ManageButton_Shift(status);
        }
        break;
        
        case BUTTON_12_SOLO:
        {
            ManageButton_Solo(status);
        }
        break;
        
        // 5,13
        case BUTTON_5_EDIT:
        case BUTTON_13_CANC:
        {
            ManageButton_Generic(status);
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
}
/* [] END OF FILE */
