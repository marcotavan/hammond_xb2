/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-01-16
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <project.h>
#include "debug.h"
#include "midiLibrary.h"
// #include "midiEvents.h"
#include "common.h"
#include "midiMerge.h"

#define DIM_QUEUE 1000
uint8 buf[DIM_QUEUE];
uint16 ptrIn = 0;
uint16 ptrOut = 0;

uint16 getElement(void) {
	ptrOut++;	// primo elemento da prelevare
	ptrOut %= DIM_QUEUE; 
	return ptrOut;
}

uint8 hasElements(void) {
	// ci sono elementi in coda se i due puntatori sono diversi
	if (ptrIn != ptrOut) {
		return (1);
	} else {
		return (0);
	}
}

uint8 isQueueNotFull(uint16 pIn) {
	uint16 val = (pIn + 1) % DIM_QUEUE; 
	// posso accodare se il successivo elemento e' libero
	if(val != ptrOut) {
		ptrIn = val;
		return (1);
	} else {
		return (0);
	}
}

/*****************************************************/
#define VERBOSE_SEND_MIDI (1)
#define VERBOSE_SEND_MIDI_MESSAGE (0)
/*****************************************************/

struct midimsg	mMessageMerge;

enum midi_parser_t {
	MIDI_PARSER_IDLE,
	MIDI_PARSER_WAITING_SOF,
	MIDI_PARSER_COLLECTING_DATA,
	MIDI_PARSER_SEND_MIDI
};

void MidiMergePoll(void) {
	uint8 i;
	uint8 rxDataLen = MIDI1_UART_GetRxBufferSize();
	static uint8 MIDI_Parser_Status = MIDI_PARSER_WAITING_SOF;
	static uint8 messageSize = 0;
	uint16 outIndex;
	
	if(rxDataLen > 0) {
		// ci sono caratteri nel buffer da prelevare.
		for(i=0;i<rxDataLen;i++) {
			/* testare (ptrIn % sizeOf(buf)) -> if(ptrIn++ < sizeOf(buf)) else error */
			if(isQueueNotFull(ptrIn)) {
				// accoda il dato ricevuto alla struttura di indice -> ptrIn 
				buf[ptrIn] = MIDI1_UART_GetByte(); // metto nel buffer
				DBG_PRINTF("\t\t\t\t\t\t%2X(%d)\n ",buf[ptrIn],ptrIn);
			} else {
				DBG_PRINTF("[%s] coda piena\n",__func__);
			} 
		}
	}
	
	// nidi parser
	if(hasElements()) {
		outIndex = getElement();
		// ci sono caratteri da	"parsare"
		switch(MIDI_Parser_Status) {
			case MIDI_PARSER_WAITING_SOF:	// cerco il SOF
				switch(buf[outIndex] & 0xF0) {
					case NoteOff:			// NoteOff				+2: key 	- Velocity
					case NoteOn:			// NoteOn				+2: key 	- Velocity
					case AfterTouchPoly:	// AfterTouch	poly	+2: key 	- Touch
					case ControlChange:		// ContinousController	+2: ctrl# 	- Val.
					case ProgramChange:  	// patchChange			+2: intrument#
					case PitchBend: 		// PitchBend			+2: lsb(7b)	-	msb(7b)
						mMessageMerge.type = buf[outIndex] & 0xF0;
						mMessageMerge.channel = buf[outIndex] & 0x0F;
						MIDI_Parser_Status = MIDI_PARSER_COLLECTING_DATA;
						messageSize = 2; // se non c'è runnung status. Se c'è running status va gestito.
					break;
					
					case AfterTouchChannel:	// Channel Pressure		+1: pressure
						DBG_PRINTF("AfterTouchChannel\n");
						mMessageMerge.type = buf[outIndex] & 0xF0;
						mMessageMerge.channel = buf[outIndex] & 0x0F;
						MIDI_Parser_Status = MIDI_PARSER_COLLECTING_DATA;
						messageSize = 1;
					break;
					
					case SystemExclusive:	// SystemMessages		+n
					break;
					
					case InvalidType:
					break;
					
					default: // garbage
					// runningStatus? non tocco questi ma salto subito a data1
					// mMessageMerge.type = buf[ptrOut] & 0xF0;
					// mMessageMerge.channel = buf[ptrOut] & 0x0F;
						MIDI_Parser_Status = MIDI_PARSER_COLLECTING_DATA;
						messageSize = 2; // se non c'è runnung status. Se c'è running status va gestito.
					break;
				}
				// ptrOut++; // prossima volta che entro prelevo il prossimo byte
				break;		
			
			case MIDI_PARSER_COLLECTING_DATA: // ora vedo se riesco a costruire il pacchetto sa spedire
					mMessageMerge.data1 =  buf[outIndex];
					MIDI_Parser_Status = 3;
					// ptrOut++; // prossima volta che entro prelevo il prossimo byte
				break;
		
			case MIDI_PARSER_SEND_MIDI: // ok ne ho abbastanza
					mMessageMerge.data2 =  buf[outIndex];
					DBG_PRINTF("send: %2X %2X %2X %2X\n",mMessageMerge.type,mMessageMerge.data1,mMessageMerge.data2,mMessageMerge.channel);
					sendMidiMessage(mMessageMerge.type,mMessageMerge.data1,mMessageMerge.data2,mMessageMerge.channel+1);
					
					MIDI_Parser_Status = MIDI_PARSER_WAITING_SOF;
					// ptrOut++; // prossima volta che entro prelevo il prossimo byte
				break;
				
				/*						
				// se ne ho a sufficienza posso spedire.
				sendNoteOff(buf[ptrOut+1],buf[ptrOut+2],channel);
				sendNoteOn(buf[ptrOut+1],buf[ptrOut+2],channel);
				sendPolyPressure(buf[ptrOut+1],buf[ptrOut+2],channel);
				sendControlChange(buf[ptrOut+1],buf[ptrOut+2],channel);
				
				*/
		}
	}
}


/* [] END OF FILE */
