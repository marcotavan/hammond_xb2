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


/*****************************************************/
#define VERBOSE_SEND_MIDI (1)
#define VERBOSE_SEND_MIDI_MESSAGE (0)
/*****************************************************/

struct midimsg	mMessageMerge;

void MidiMergePoll(void) {
	uint8 rxDataLen = MIDI1_UART_GetRxBufferSize();
	if(rxDataLen != 0) {
	}
}


/* [] END OF FILE */
