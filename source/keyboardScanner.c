/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-01-17
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
#include "math.h"  
#include "ButtonScanner.h"
#include "EepromManager.h"

#define VERBOSE_MODE 1

typedef enum key_states {
  KEY_IS_UP,
  KEY_IS_GOING_DOWN,  // increment the timer 
  KEY_IS_DOWN,
  KEY_IS_GOING_UP,    // increment the timer 
} state_t;

typedef enum events {
  KEY_PRESSED,    // Key is pressed
  KEY_RELEASED,   // Key was released
  KEY_ERROR_1,    //
  KEY_ERROR_2,
} event_t;

struct key_t {
  uint8     midi_note;
  state_t   state; 
  uint16    counter; 
};

struct split_point {
	uint8 marker;
	uint8 point;
	uint8 lowerMidiChannel;
	uint8 upperMidiChannel;
	uint8 mode;
	uint8 getNote;
};

struct split_point split;

void SplitGetNote (void) {
	split.getNote = 1;
}

void SetSplitPoint(uint8 sp) {
	split.point = sp;	
}

uint8 GetSplitPoint(void) {
	return split.point;	
	// 0: no split point
}

void SetSplitMode(uint8 mode) {
	split.mode = mode;	
}

uint8 GetSplitMode(void) {
	return split.mode;	
}

struct key_t key[MAX_KEYS];

double LOG_2 = 0;

void MatrixScanner(void);

uint8 LogVelocity(uint16 counter)
{
    // The interval between the first and second switch while hitting keys with the
    // highest velocity.
    static const float shortestInterval = MAX_SLOW_VELOCITY_COUNTER;

    // Every time the interval between the first and second switch doubles, lower
    // the midi velocity by this much:
    static const float velocityAttenuation = 15.8; // 13-20, 18 schiaccaito

    float velocity;
    
    //  excel velocity.xls     => 127 - (127 + ((LOG10(A2/$F$1)/LOG10(2))*20))
    
    velocity = 127 - (127 + ((log((float)counter/shortestInterval)/LOG_2) * velocityAttenuation));
    
    // velocity = 127 - (127 + ((icsi_log((counter/shortestInterval),lookup_table, 8)/LOG_2) * velocityAttenuation));
    
    
    
    if (velocity < 1) {
        velocity = 1;
    } 
    else if (velocity > 127) {
        velocity = 127;
    }
    
    return velocity;
}

uint8 LinearVelocity(uint16 counter)
{
    uint16 velocity;
        
    velocity = (((MAX_SLOW_VELOCITY_COUNTER+1) - (counter)) / 10);
    
    if (velocity < 1) {
        velocity = 1;
    } 
    else if (velocity > 127) {
        velocity = 127;
    }
    
    return velocity; // troncato
}

void EventTrigger(uint8 event, uint8 numTasto, uint16 counter)
{
    uint8 Velocity = 0;
    uint8 linearVelocity = 0; 
    
    uint8 play_note = MIDI_FIRST_NOTE_61 + numTasto;
	uint8 midiChannel = split.upperMidiChannel;
	
	if (split.getNote) { // flag per prendere la nota
		split.point = play_note; // devo prendere lo splitPoint
		split.mode = 1;
		split.getNote = 0; // cancello il flag
		SplitCallbackFunction(split.point); // ritorno
	}
	
	if(split.mode) { // se esiste uno split point
		if (play_note < split.point ) { // se la nota e inferiore 
			midiChannel = split.lowerMidiChannel; // si tratta di un basso
		}
	}
		
    switch (event)
    {
        case KEY_PRESSED:
        {    
            // Pin_SPIF_Write(1);
            Velocity = LogVelocity(counter);       // 100uS
            // Velocity = LinearVelocity(counter);
			// Pin_SPIF_Write(0);
            
            sendNoteOn(play_note,Velocity,midiChannel);
        }
        break;
        
        case KEY_RELEASED:
        case KEY_ERROR_1:
        {
            // Pin_SPIF_Write(1);
            linearVelocity = LinearVelocity(counter);  // 1uS
            // Pin_SPIF_Write(0);
            sendNoteOff(play_note,linearVelocity,midiChannel);
        }
        break;
    }
    
    #if VERBOSE_MODE 
	if (event==KEY_PRESSED){
    	DBG_PRINTF("[%s] %3d %3d %4d %3d %3d\n",__func__,event,numTasto,key[numTasto].counter,linearVelocity,Velocity);
	}
    #endif
}

void KeyScanInit(void) 
{
    // Init array
    // memset(banks, 0xff, sizeof(prev_banks));
    uint8 keyNumber;

	// init split point
	split.marker = MARKER_MIDI;
	split.point = 0; // 24 = C5
	split.mode = 0;
	split.upperMidiChannel = MIDI_CHANNEL_1;
	split.lowerMidiChannel = MIDI_CHANNEL_2;
	split.getNote = 0;
	
	#define splitParametersLen 5
	
	uint8 pdata[CYDEV_EEPROM_ROW_SIZE];
	LoadMidiData(pdata);
	if (pdata[0] == MARKER_MIDI) {
		DBG_PRINTF("carico in struttura ");
		DBG_PRINT_ARRAY(pdata,splitParametersLen);
		DBG_PRINTF("\n");
		memcpy((uint8 *) &split,pdata,splitParametersLen); 
	}
	
    // Init keys
    for (keyNumber = 0; keyNumber < 88; keyNumber++) 
    {
        // key[keyNumber].midi_note = 21 + keyNumber;
        key[keyNumber].counter = 0;
        key[keyNumber].state = KEY_IS_UP; // COSI SI RESETTA
    }

    // MyPin_SetDriveMode(MyPin_DM_RES_DWN);
    keyInputScan_0_SetDriveMode(keyInputScan_0_DM_RES_DWN);
    keyInputScan_1_SetDriveMode(keyInputScan_1_DM_RES_DWN);
    keyInputScan_2_SetDriveMode(keyInputScan_2_DM_RES_DWN);
    keyInputScan_3_SetDriveMode(keyInputScan_3_DM_RES_DWN);
    keyInputScan_4_SetDriveMode(keyInputScan_4_DM_RES_DWN);
    keyInputScan_5_SetDriveMode(keyInputScan_5_DM_RES_DWN);
    keyInputScan_6_SetDriveMode(keyInputScan_6_DM_RES_DWN);
    keyInputScan_7_SetDriveMode(keyInputScan_7_DM_RES_DWN);
    
    keyInputScan_0_Write(0);    // 0: attiva il pulldown
    keyInputScan_1_Write(0);
    keyInputScan_2_Write(0);
    keyInputScan_3_Write(0);
    keyInputScan_4_Write(0);
    keyInputScan_5_Write(0);
    keyInputScan_6_Write(0);
    keyInputScan_7_Write(0);
    
    LOG_2 = log(2);
}


void KeyScan_Poll(void) 
{
   // sul main con flag 500us
   MatrixScanner();
}

void MatrixScanner(void) 
{
    // va portato nell'interrupt
    
    uint8 line = 0;
    uint8 var = 0;
    uint8 bank = 0;
    uint8 numTasto = 0;

    for(line = 0;line < 16;line++)
    { // seleziona le linee dei banchi una alla volta col demultiplexer
        Control_Reg_Keyboard_Line_Select_Write(line); // Selects row 
        // qui ci vorrebbe un minimo delay
        CyDelayUs(DELAY_US_DEBOUNCE_KEYSCANNER); // 16*10us sta qui dentro. con tiziano abbiamo visto che va bene 1uS
        // DBG_PRINTF("sel line %02d ",line);
        var = KeyInputPortReg_Read(); // leggi lo stato dei tasti 8 per volta
        
        // if (line == 0) DBG_PRINTF("var : %02x\n",var);
        for(bank=0;bank<4;bank++)
        { // permette di decidere dove mettere il dato letto dalle 8 linee
            numTasto = bank + (4*line);   
  //          DBG_PRINTF("tasto %d campionato, %d %d %02x\n",numTasto,bank*2,bank*2+1,var);

            if (bitRead(var, (bank*2))) // TOP test sui bit pari
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
                    if (key[numTasto].counter != MAX_SLOW_VELOCITY_COUNTER) {
						key[numTasto].counter++;
					}
                }
            }
            else
            {
                if (key[numTasto].state == KEY_IS_GOING_UP)
                {
                    // sto risalendo e ho rilasciato tutto
                    // DBG_PRINTF(" key[%d].state = KEY_IS_UP; ",numTasto);                    
                    // DBG_PRINTF(" dynamics value RELEASE = %d\n",key[numTasto].counter);
                    EventTrigger(KEY_RELEASED,numTasto,key[numTasto].counter);
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
                    EventTrigger(KEY_ERROR_1,numTasto,key[numTasto].counter);
                    key[numTasto].state = KEY_IS_UP;
                    key[numTasto].counter = 0; // resetta
                    // DBG_PRINTF(" key[%d].state = KEY_IS_UP;\n",numTasto);
                }
            }

            if (bitRead(var, ((bank*2) + 1))) // BOTTOM test sui bit dispari
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
                    EventTrigger(KEY_PRESSED,numTasto,key[numTasto].counter);
                    key[numTasto].counter = 0; // resetta
                }
                
                // questo caso non so se va gestito
                if (key[numTasto].state == KEY_IS_GOING_UP)
                {
                    // stavo cercando di risalire ma non ho fatto in tempo e mi hanno ripremuto
                    key[numTasto].state = KEY_IS_DOWN;
                    // DBG_PRINTF(" key[%d].state = KEY_IS_DOWN; ",numTasto);
                    // DBG_PRINTF(" dynamics value PRESS = %d\n",key[numTasto].counter);
                    EventTrigger(KEY_ERROR_2,numTasto,key[numTasto].counter);
                    key[numTasto].counter = 0; // resetta
                }
                
            } // 
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
                    if (key[numTasto].counter != MAX_SLOW_VELOCITY_COUNTER) {
						key[numTasto].counter++;
					}
                }
            }
        } // for(bank=0;bank<4;bank++)
    } // for(line = 0;line < 16;line++)
} // MatrixScanner

/* [] END OF FILE */
