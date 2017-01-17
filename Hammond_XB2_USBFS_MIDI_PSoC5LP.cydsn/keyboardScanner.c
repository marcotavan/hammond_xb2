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

  
// Possible key states
typedef enum {
  KEY_IS_UP,
  KEY_IS_DOWN,
  KEY_IS_GOING_UP,    // We increment the timer in this state
  KEY_IS_GOING_DOWN,  // We increment the timer in this state
} state_t;

// Possible events
typedef enum {
  KEY_PRESSED,    // Key is pressed
  KEY_DOWN,       // Key reached bottom
  KEY_RELEASED,   // Key was released
  KEY_UP,         // Key reached top
} event_t;

struct key_t {
  char midi_note:8;
  state_t       state:4; // Bit fields
  unsigned int  t    :7; // Lines up nicely to 16bits, t overflows at 4096
};

struct bank_t {
  char top;
  char bottom;
};


struct key_t keys[88];

#define NUM_BANKS 11

void increment(void);
void footpedal(void);
void scan(void);

// For scanning banks
struct bank_t banks[NUM_BANKS];
struct bank_t prev_banks[NUM_BANKS];

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

void KeyScanInit(void) 
{
    // Init array
    memset(banks, 0xff, sizeof(prev_banks));
    uint8 key;

    // Init keys
    for (key = 0; key < 88; key++) 
    {
        keys[key].midi_note = 21 + key;
        keys[key].t = 0;
    }
}


void KeyScan(void) 
{
   scan();
   increment();
   footpedal();
}

void increment() 
{
     // Advance timers
    uint8 key;
    
    for(key = 0; key < 88; key++) 
    {
        state_t state = keys[key].state;
        if(state == KEY_IS_GOING_UP || state == KEY_IS_GOING_DOWN) 
        {
            keys[key].t++;
        }
    }
}

void footpedal() {
  
  static int prev_val = 1;
  int val = 0; // PINF & 0b00000001;
  
  if(val != prev_val) {
    sendControlChange(64, (val ? 1 : 64), MIDI_CHANNEL_1);
    prev_val = val;
  }
  
}

void scan(void) 
{
    uint8 bank,key;
    event_t event;
    // Scan and store
    for(bank = 0; bank < NUM_BANKS; bank++) 
    {
        prev_banks[bank] = banks[bank]; // Store previous state so we can look for changes
        
        // PORTD = bank * 2; // Selects bottom row
        CyDelayUs(10); // Debounce
        // banks[bank].bottom = PINC;
        
        // PORTD = bank * 2 + 1; // Selects top row
        CyDelayUs(10); // Debounce
        // banks[bank].top = PINC;
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
}


/* [] END OF FILE */
