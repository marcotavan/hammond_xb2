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

#ifndef _MIDI_LIBRARY_H_
#define _MIDI_LIBRARY_H_

#include "project.h"
#define MIDI_BAUDRATE			31250

#define MIDI_CHANNEL_OMNI		0
#define MIDI_CHANNEL_OFF		17			// and over
#define MIDI_CHANNEL_1          1
#define MIDI_CHANNEL_2          2
#define MIDI_CHANNEL_3          3
#define MIDI_CHANNEL_4          4
#define MIDI_CHANNEL_5          5
    
#define MIDI_SYSEX_ARRAY_SIZE	255			// Maximum size is 65535 bytes.

#define COMPILE_MIDI_IN         0           // Set this setting to 1 to use the MIDI input.
#define COMPILE_MIDI_OUT        1           // Set this setting to 1 to use the MIDI output. 
#define COMPILE_MIDI_THRU       0           // Set this setting to 1 to use the MIDI Soft Thru feature
                                            // Please note that the Thru will work only when both COMPILE_MIDI_IN and COMPILE_MIDI_OUT set to 1.


// #define USE_SERIAL_PORT         Serial      // Change the number (to Serial1 for example) if you want
                                            // to use a different serial port for MIDI I/O.

#define USE_RUNNING_STATUS		1			// Running status enables short messages when sending multiple values
                                            // of the same type and channel.
                                            // Set to 0 if you have troubles with controlling you hardware.


#define USE_CALLBACKS           0           // Set this to 1 if you want to use callback handlers (to bind your functions to the library).
                                            // To use the callbacks, you need to have COMPILE_MIDI_IN set to 1

#define USE_1BYTE_PARSING       0           // Each call to MIDI.read will only parse one byte (might be faster).
    
#define MIDI_2BYTE_COMMON               (0x02u)
#define MIDI_3BYTE_COMMON               (0x03u)
	
typedef uint8 byte;
typedef uint8 bool;
    
    /*! Enumeration of MIDI types */
enum kMIDIType {
	NoteOff	              = 0x80,	///< Note Off
	NoteOn                = 0x90,	///< Note On
	AfterTouchPoly        = 0xA0,	///< Polyphonic AfterTouch
	ControlChange         = 0xB0,	///< Control Change / Channel Mode
	ProgramChange         = 0xC0,	///< Program Change
	AfterTouchChannel     = 0xD0,	///< Channel (monophonic) AfterTouch
	PitchBend             = 0xE0,	///< Pitch Bend
	SystemExclusive       = 0xF0,	///< System Exclusive
	TimeCodeQuarterFrame  = 0xF1,	///< System Common - MIDI Time Code Quarter Frame
	SongPosition          = 0xF2,	///< System Common - Song Position Pointer
	SongSelect            = 0xF3,	///< System Common - Song Select
	TuneRequest           = 0xF6,	///< System Common - Tune Request
	Clock                 = 0xF8,	///< System Real Time - Timing Clock
	Start                 = 0xFA,	///< System Real Time - Start
	Continue              = 0xFB,	///< System Real Time - Continue
	Stop                  = 0xFC,	///< System Real Time - Stop
	ActiveSensing         = 0xFE,	///< System Real Time - Active Sensing
	SystemReset           = 0xFF,	///< System Real Time - System Reset
	InvalidType           = 0x00    ///< For notifying errors
};

/*! Enumeration of Thru filter modes */
enum kThruFilterMode {
	Off                   = 0,  ///< Thru disabled (nothing passes through).
	Full                  = 1,  ///< Fully enabled Thru (every incoming message is sent back).
	SameChannel           = 2,  ///< Only the messages on the Input Channel will be sent back.
	DifferentChannel      = 3   ///< All the messages but the ones on the Input Channel will be sent back.
};


/*! The midimsg structure contains decoded data of a MIDI message read from the serial port with read() or thru(). \n */
struct midimsg {
	/*! The MIDI channel on which the message was recieved. \n Value goes from 1 to 16. */
	byte channel; 
	/*! The type of the message (see the define section for types reference) */
	enum kMIDIType type;
	/*! The first data byte.\n Value goes from 0 to 127.\n */
	byte data1;
	/*! The second data byte. If the message is only 2 bytes long, this one is null.\n Value goes from 0 to 127. */
	byte data2;
	/*! System Exclusive dedicated byte array. \n Array length is stocked on 16 bits, in data1 (LSB) and data2 (MSB) */
	byte sysex_array[MIDI_SYSEX_ARRAY_SIZE];
	/*! This boolean indicates if the message is valid or not. There is no channel consideration here, validity means the message respects the MIDI norm. */
	bool valid;
};


	// Attributes
#if USE_RUNNING_STATUS
	byte			mRunningStatus_TX;
#endif // USE_RUNNING_STATUS


uint8 sendNoteOn(byte NoteNumber,
							byte Velocity,
							byte Channel);

uint8 sendNoteOff(byte NoteNumber,
							 byte Velocity,
							 byte Channel);

uint8 sendProgramChange(byte ProgramNumber,
								   byte Channel);

uint8 sendControlChange(byte ControlNumber,
								   byte ControlValue,
								   byte Channel);

void sendSysEx(int length, 
					const byte *const array,
					bool ArrayContainsBoundaries);

void UART_MIDI_Init(void);
#endif // _MIDI_LIBRARY_H_

/* [] END OF FILE */
