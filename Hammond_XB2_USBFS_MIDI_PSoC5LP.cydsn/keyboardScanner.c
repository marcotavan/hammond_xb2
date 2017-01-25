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


#include "project.h"
#include "keyboardScanner.h"
#include "midiLibrary.h"
#include "common.h"
#include "debug.h"
#include "midiEvents.h"
  
// Possible key states
typedef enum {
  KEY_IS_UP,
  KEY_IS_GOING_DOWN,  // increment the timer in this state    
  KEY_IS_DOWN,
  KEY_IS_GOING_UP,    // increment the timer in this state
} state_t;

// Possible events
typedef enum {
  KEY_PRESSED,    // Key is pressed
  KEY_RELEASED,   // Key was released
  KEY_ERROR_1,         //
  KEY_ERROR_2,
} event_t;

struct key_t {
  uint8     midi_note;
  state_t   state; 
  uint16    counter; 
};

/*
struct bank_t {
  uint8 top;
  uint8 bottom;
};
*/

struct key_t key[88];

// #define NUM_BANKS 8 // 11 per 88 tasti, 8 per 61 tasti

// void increment(void);
// void Damper_Pedal(void);
void MatrixScanner(void);

// For scanning banks
/*
struct bank_t banks[NUM_BANKS];
struct bank_t prev_banks[NUM_BANKS];
*/
/*
void KeyScanTrigger(struct key_t *key, event_t event) 
{
    if(event == KEY_PRESSED) 
    {
        key->state = KEY_IS_GOING_DOWN;
    } 
    else if (event == KEY_DOWN) 
    {
        key->state = KEY_IS_DOWN;
        DBG_PRINTF("%s KEY_IS_DOWN, sendNoteOn: %d\n",__func__,127- key->t);
        sendNoteOn(key->midi_note, 127- key->t, 1);
        key->t = 0;
    } 
    else if (event == KEY_RELEASED) 
    {
        key->state = KEY_IS_GOING_UP;
    } 
    else if (event == KEY_UP) 
    {
        key->state = KEY_IS_UP;
        DBG_PRINTF("%s KEY_IS_UP, sendNoteOff: %d\n",__func__,127- key->t);
        sendNoteOff(key->midi_note,  127 - key->t, 1);
        key->t = 0;
    } 
}
*/

void EventTrigger(uint8 event, uint8 numTasto)
{
    uint16 velocity;
    uint8 play_note = MIDI_FIRST_NOTE_61 + numTasto;
    
    velocity = (((MAX_SLOW_VELOCITY_COUNTER+1) - (key[numTasto].counter)) / 10) &0x7F;
    if (velocity == 0)velocity = 1;

    switch (event)
    {
        case KEY_PRESSED:
        sendNoteOn(play_note,velocity,MIDI_CHANNEL_1);
        break;
        
        case KEY_RELEASED:
        case KEY_ERROR_1:
        sendNoteOff(play_note,velocity,MIDI_CHANNEL_1);
        break;
    }
    
    DBG_PRINTF("[%s] %3d %3d %4d %3d\n",__func__,event,numTasto,key[numTasto].counter,velocity);
}

void KeyScanInit(void) 
{
    // Init array
    // memset(banks, 0xff, sizeof(prev_banks));
    uint8 keyNumber;

    // Init keys

    for (keyNumber = 0; keyNumber < 88; keyNumber++) 
    {
        key[keyNumber].midi_note = 21 + keyNumber;
        key[keyNumber].counter = 0;
        key[keyNumber].state = KEY_IS_UP; // COSI SI RESETTA
    }

    keyInputScan_0_Write(0);    // 0: attiva il pulldown
    keyInputScan_1_Write(0);
    keyInputScan_2_Write(0);
    keyInputScan_3_Write(0);
    keyInputScan_4_Write(0);
    keyInputScan_5_Write(0);
    keyInputScan_6_Write(0);
    keyInputScan_7_Write(0);
}


void KeyScan_Poll(void) 
{
   // sul main con flag 500us
   MatrixScanner();
   // increment();
   // Damper_Pedal();
}

/*
void increment() 
{
     // Advance timers
    uint8 key;
    
    for(key = 0; key < 88; key++) 
    {
        state_t state = keys[key].state;
        if(state == KEY_IS_GOING_UP || state == KEY_IS_GOING_DOWN) 
        {
            keys[key].timer++;
        }
    }
}
*/

void Damper_Pedal(void) {
  #define INPUT_PIN_READ 0 // PINF & 0b00000001;
  static uint8 prev_val = 0;
  uint8 val = INPUT_PIN_READ; 
  
  if(val != prev_val) {
    sendControlChange(64, (val ? 1 : 64), MIDI_CHANNEL_1);
    prev_val = val;
  }
  
}

void MatrixScanner(void) 
{
    // va portato nell'interrupt
    
    uint8 line = 0;
    uint8 var = 0;
    uint8 bank = 0;
    uint8 numTasto = 0;

    for(line = 0;line < 16;line++)
    { // seleziona le linee col demultiplexer
        Control_Reg_Line_Select_Write(line); // Selects row 
        // DBG_PRINTF("sel line %02d ",line);
        var = KeyInputPort_Read();
        
        // if (line == 0) DBG_PRINTF("var : %02x\n",var);
        for(bank=0;bank<4;bank++)
        { // permette di decidere dove mettere il dato letto dalle 8 linee
            numTasto = bank + (4*line);   
  //          DBG_PRINTF("tasto %d campionato, %d %d %02x\n",numTasto,bank*2,bank*2+1,var);

            if (bitRead(var, (bank*2))) // TOP
            {
                // tasto SxA premuto C1 0:0 1:4 2:8
                // if (line == 0) {
                // DBG_PRINTF("tasto %d A premuto \n",numTasto);
                // }
                if (key[numTasto].state == KEY_IS_UP)
                {
                    // il tasto era fermo ora sta scendendo, inizio a contare
                    key[numTasto].state = KEY_IS_GOING_DOWN;
                    // DBG_PRINTF(" key[%d].state = KEY_IS_GOING_DOWN;\n",numTasto);
                    key[numTasto].counter = 0; // parti a contare
                }
                
                if (key[numTasto].state == KEY_IS_GOING_DOWN)
                {
                    // conto poich[ non sono ancora arrivato giu del tutto
                    if (key[numTasto].counter != MAX_SLOW_VELOCITY_COUNTER) key[numTasto].counter++;
                }
                
                
            }
            else
            {
                if (key[numTasto].state == KEY_IS_GOING_UP)
                {
                    // sto risalendo e ho rilasciato tutto
                    // DBG_PRINTF(" key[%d].state = KEY_IS_UP; ",numTasto);                    
                    // DBG_PRINTF(" dynamics value RELEASE = %d\n",key[numTasto].counter);
                    EventTrigger(KEY_RELEASED,numTasto);
                    key[numTasto].state = KEY_IS_UP;
                    key[numTasto].counter = 0; // resetta

                }
                
                if (key[numTasto].state == KEY_IS_GOING_DOWN)
                { // non sono ancora arrivato giu e rilascio -> errore non fa niente
                    // non sono su quindi resetto
                    key[numTasto].state = KEY_IS_UP;
                    key[numTasto].counter = 0; // resetta
                    // DBG_PRINTF(" key[%d].state = KEY_IS_UP;\n",numTasto);
                }
                
                if (key[numTasto].state == KEY_IS_DOWN)
                { // ero giu ma rilasco prima il primo del secondo
                    EventTrigger(KEY_ERROR_1,numTasto);
                    key[numTasto].state = KEY_IS_UP;
                    key[numTasto].counter = 0; // resetta
                    // DBG_PRINTF(" key[%d].state = KEY_IS_UP;\n",numTasto);
                }
            }

            if (bitRead(var, ((bank*2) + 1))) // BOTTOM
            {
                // tasto SxB premuto C1 0 4 8
                // if (line == 0) {
                // DBG_PRINTF("tasto %d B premuto \n",numTasto);
                // }
                // sono arrivato giu, 
                if (key[numTasto].state == KEY_IS_GOING_DOWN)
                {
                    key[numTasto].state = KEY_IS_DOWN;
                    // DBG_PRINTF(" key[%d].state = KEY_IS_DOWN; ",numTasto);
                    // DBG_PRINTF(" dynamics value PRESS = %d\n",key[numTasto].counter);
                    EventTrigger(KEY_PRESSED,numTasto);
                    key[numTasto].counter = 0; // resetta
                }
                
                // questo caso non so se va gestito
                if (key[numTasto].state == KEY_IS_GOING_UP)
                {
                    // stavo cercando di risalire ma non ho fatto in tempo e mi hanno ripremuto
                    key[numTasto].state = KEY_IS_DOWN;
                    // DBG_PRINTF(" key[%d].state = KEY_IS_DOWN; ",numTasto);
                    // DBG_PRINTF(" dynamics value PRESS = %d\n",key[numTasto].counter);
                    EventTrigger(KEY_ERROR_2,numTasto);
                    key[numTasto].counter = 0; // resetta
                }
                
            }
            else
            {
                if (key[numTasto].state == KEY_IS_DOWN)
                {
                    key[numTasto].state = KEY_IS_GOING_UP;
                    // DBG_PRINTF(" key[%d].state = KEY_IS_GOING_UP;\n ",numTasto);
                    key[numTasto].counter = 0; // parti a contare
                    // DBG_PRINTF("resetta\n");
                }
                
                if (key[numTasto].state == KEY_IS_GOING_UP)
                {
                    if (key[numTasto].counter != MAX_SLOW_VELOCITY_COUNTER) key[numTasto].counter++;
                }
                
                
                
            }

        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    #if 0
    uint8 bank,key;
    event_t event;
    // Scan and store
    for(bank = 0; bank < NUM_BANKS; bank++) 
    {
        // prev_banks[bank] = banks[bank]; // Store previous state so we can look for changes
        
        Control_Reg_Line_Select_Write(bank * 2); // Selects bottom row 
        DBG_PRINTF("sel bank %02d ",bank * 2);
        // CyDelayUs(10); // Debounce
        banks[bank].bottom = KeyInputPort_Read();
        DBG_PRINTF("\tread Port 0x%02X\n",banks[bank].bottom);
        
        DBG_PRINTF("sel bank %02d ",(bank * 2) + 1);
        Control_Reg_Line_Select_Write((bank * 2) + 1); // Selects top row
        // CyDelayUs(10); // Debounce
        banks[bank].top = KeyInputPort_Read();
        DBG_PRINTF("\tread Port 0x%02X\n",banks[bank].top);
    }
  
    // Process
    for(bank = 0; bank < NUM_BANKS; bank++) 
    {
        byte diff;
        
        // Check top switches and fire events
        diff = banks[bank].top ^ prev_banks[bank].top;
        if(diff) 
        {
            for(key = 0; key < 8; key++) 
            {
                if(bitRead(diff, key)) 
                { 
                    event = bitRead(banks[bank].top, key) ? KEY_UP : KEY_PRESSED;
                    KeyScanTrigger(&keys[bank * 8 + key], event);
                }
            }
        }
    
        // Check bottom switches and fire events
        diff = banks[bank].bottom ^ prev_banks[bank].bottom;
        if(diff) 
        {
            for(key = 0; key < 8; key++) 
            {
                if(bitRead(diff, key)) 
                { 
                    event = bitRead(banks[bank].bottom, key) ? KEY_RELEASED : KEY_DOWN;           
                    KeyScanTrigger(&keys[bank * 8 + key], event);
                }
            }
        }
    }
    
    #endif
}


/* [] END OF FILE */
