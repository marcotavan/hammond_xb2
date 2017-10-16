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

uint8 buf[1000];
uint8 ptrIn = 0;
uint8 ptrOut = 0;


/*****************************************************/
#define VERBOSE_SEND_MIDI (1)
#define VERBOSE_SEND_MIDI_MESSAGE (0)
/*****************************************************/

struct midimsg	mMessageMerge;

void MidiMergePoll(void) {
	uint8 i;
	uint8 rxDataLen = MIDI1_UART_GetRxBufferSize();
	static uint8 MIDI_Parser_Status = 0;
	static uint8 messageSize = 0;

	
	if(rxDataLen > 0) {
		// ci sono caratteri nel buffer da prelevare.
		for(i=0;i<rxDataLen;i++) {
			/* testare (ptrIn % sizeOf(buf)) -> if(ptrIn++ < sizeOf(buf)) else error */
			buf[ptrIn++] = MIDI1_UART_GetByte(); // metto nel buffer
		}
	}
	
	// nidi parser
	if(ptrIn!=ptrOut) {
		// ci sono caratteri da	"parsare"
		switch(MIDI_Parser_Status) {
			case 1:	// cerco il SOF
				switch(buf[ptrOut] & 0xF0) {
					case NoteOff:			// NoteOff				+2: key 	- Velocity
					case NoteOn:			// NoteOn				+2: key 	- Velocity
					case AfterTouchPoly:	// AfterTouch	poly	+2: key 	- Touch
					case ControlChange:		// ContinousController	+2: ctrl# 	- Val.
					case ProgramChange:  	// patchChange			+2: intrument#
					case PitchBend: 		// PitchBend			+2: lsb(7b)	-	msb(7b)
						mMessageMerge.type = buf[ptrOut] & 0xF0;
						mMessageMerge.channel = buf[ptrOut] & 0x0F;
						MIDI_Parser_Status = 2;
						messageSize = 2; // se non c'è runnung status. Se c'è running status va gestito.
					break;
					
					case AfterTouchChannel:	// Channel Pressure		+1: pressure
						mMessageMerge.type = buf[ptrOut] & 0xF0;
						mMessageMerge.channel = buf[ptrOut] & 0x0F;
						MIDI_Parser_Status = 2;
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
						MIDI_Parser_Status = 2;
						messageSize = 2; // se non c'è runnung status. Se c'è running status va gestito.
					break;
				}
				ptrOut++; // prossima volta che entro prelevo il prossimo byte
				break;		
			
			case 2: // ora vedo se riesco a costruire il pacchetto sa spedire
					mMessageMerge.data1 =  buf[ptrOut];
					MIDI_Parser_Status = 3;
					ptrOut++; // prossima volta che entro prelevo il prossimo byte
				break;
		
			case 3: // ok ne ho abbastanza
					mMessageMerge.data2 =  buf[ptrOut];
					sendMidiMessage(mMessageMerge.type,mMessageMerge.data1,mMessageMerge.data2,mMessageMerge.channel);
					
					// MIDI_Parser_Status = 3;
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
