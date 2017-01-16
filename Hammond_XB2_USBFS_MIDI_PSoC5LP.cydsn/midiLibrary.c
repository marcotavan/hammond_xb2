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

#include <project.h>
#include "debug.h"
#include "midiLibrary.h"
#include "common.h"

struct midimsg	mMessage;

byte genstatus(const enum kMIDIType inType,const byte inChannel);
void sendRealTime(enum kMIDIType Type);

// Private method for generating a status byte from channel and type
byte genstatus(const enum kMIDIType inType,
								 const byte inChannel) {
	return ((byte)inType | ((inChannel-1) & 0x0F));
}

                               

/*! \brief Generate and send a MIDI message from the values given.
 \param type	The message type (see type defines for reference)
 \param data1	The first data byte.
 \param data2	The second data byte (if the message contains only 1 data byte, set this one to 0).
 \param channel	The output channel on which the message will be sent (values from 1 to 16). Note: you cannot send to OMNI.
 
 This is an internal method, use it only if you need to send raw data from your code, at your own risks.
 */
void send(enum kMIDIType type,
					  byte data1,
					  byte data2,
					  byte channel)
{
	
	// Then test if channel is valid
	if (channel >= MIDI_CHANNEL_OFF || channel == MIDI_CHANNEL_OMNI || type < NoteOff) {
		
#if USE_RUNNING_STATUS	
		mRunningStatus_TX = InvalidType;
#endif 
		
		return; // Don't send anything
	}
	
	if (type <= PitchBend) {
		// Channel messages
		
		// Protection: remove MSBs on data
		data1 &= 0x7F;
		data2 &= 0x7F;
		
		byte statusbyte = genstatus(type,channel);
		
#if USE_RUNNING_STATUS
		// Check Running Status
		if (mRunningStatus_TX != statusbyte) {
			// New message, memorise and send header
			mRunningStatus_TX = statusbyte;
			// TODO USE_SERIAL_PORT.write(mRunningStatus_TX);
		}
#else
		// Don't care about running status, send the Control byte.
		USE_SERIAL_PORT.write(statusbyte);
#endif
		
		// Then send data
		//TODO USE_SERIAL_PORT.write(data1);
		if (type != ProgramChange && type != AfterTouchChannel) {
			//TODO USE_SERIAL_PORT.write(data2);
		}
		return;
	}
	if (type >= TuneRequest && type <= SystemReset) {
		// System Real-time and 1 byte.
		sendRealTime(type);
	}
	
}                                
           


/*! \brief Send a Note On message 
 \param NoteNumber	Pitch value in the MIDI format (0 to 127). Take a look at the values, names and frequencies of notes here: http://www.phys.unsw.edu.au/jw/notes.html\n
 \param Velocity	Note attack velocity (0 to 127). A NoteOn with 0 velocity is considered as a NoteOff.
 \param Channel		The channel on which the message will be sent (1 to 16). 
 */
void sendNoteOn(byte NoteNumber,
							byte Velocity,
							byte Channel)
{ 
	DBG_PRINTF("[%s]\n",__func__);
	send(NoteOn,NoteNumber,Velocity,Channel);

}


/*! \brief Send a Note Off message (a real Note Off, not a Note On with null velocity)
 \param NoteNumber	Pitch value in the MIDI format (0 to 127). Take a look at the values, names and frequencies of notes here: http://www.phys.unsw.edu.au/jw/notes.html\n
 \param Velocity	Release velocity (0 to 127).
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
void sendNoteOff(byte NoteNumber,
							 byte Velocity,
							 byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	send(NoteOff,NoteNumber,Velocity,Channel);

}


/*! \brief Send a Program Change message 
 \param ProgramNumber	The Program to select (0 to 127).
 \param Channel			The channel on which the message will be sent (1 to 16).
 */
void sendProgramChange(byte ProgramNumber,
								   byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	send(ProgramChange,ProgramNumber,0,Channel);

}


/*! \brief Send a Control Change message 
 \param ControlNumber	The controller number (0 to 127). See the detailed description here: http://www.somascape.org/midi/tech/spec.html#ctrlnums
 \param ControlValue	The value for the specified controller (0 to 127).
 \param Channel			The channel on which the message will be sent (1 to 16). 
 */
void sendControlChange(byte ControlNumber,
								   byte ControlValue,
								   byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	send(ControlChange,ControlNumber,ControlValue,Channel);

}


/*! \brief Send a Polyphonic AfterTouch message (applies to only one specified note)
 \param NoteNumber		The note to apply AfterTouch to (0 to 127).
 \param Pressure		The amount of AfterTouch to apply (0 to 127).
 \param Channel			The channel on which the message will be sent (1 to 16). 
 */
void sendPolyPressure(byte NoteNumber,
								  byte Pressure,
								  byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	send(AfterTouchPoly,NoteNumber,Pressure,Channel);

}


/*! \brief Send a MonoPhonic AfterTouch message (applies to all notes)
 \param Pressure		The amount of AfterTouch to apply to all notes.
 \param Channel			The channel on which the message will be sent (1 to 16). 
 */
void sendAfterTouch(byte Pressure,
								byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	send(AfterTouchChannel,Pressure,0,Channel);

}


/*! \brief Send a Pitch Bend message using an unsigned integer value.
 \param PitchValue	The amount of bend to send (in a signed integer format), between 0 (maximum downwards bend) and 16383 (max upwards bend), center value is 8192.
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
void sendPitchBend(unsigned int PitchValue,
							   byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	send(PitchBend,(PitchValue & 0x7F),(PitchValue >> 7) & 0x7F,Channel);
	
}

/*! \brief Send a Pitch Bend message using a signed integer value.
 \param PitchValue	The amount of bend to send (in a signed integer format), between -8192 (maximum downwards bend) and 8191 (max upwards bend), center value is 0.
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
void sendPitchBendInt(int PitchValue,
							   byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	unsigned int bend = PitchValue + 8192;
	sendPitchBend(bend,Channel);
	
}

/*! \brief Send a Pitch Bend message using a floating point value.
 \param PitchValue	The amount of bend to send (in a floating point format), between -1.0f (maximum downwards bend) and +1.0f (max upwards bend), center value is 0.0f.
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
void sendPitchBendFloat(double PitchValue,
							   byte Channel)
{
	DBG_PRINTF("[%s]\n",__func__);
	unsigned int pitchval = (PitchValue+1.f)*8192;
	if (pitchval > 16383) pitchval = 16383;		// overflow protection
	sendPitchBend(pitchval,Channel);
	
}


/*! \brief Generate and send a System Exclusive frame.
 \param length	The size of the array to send
 \param array	The byte array containing the data to send
 \param ArrayContainsBoundaries  When set to 'true', 0xF0 & 0xF7 bytes (start & stop SysEx) will NOT be sent (and therefore must be included in the array).
 default value is set to 'false' for compatibility with previous versions of the library.
 */
void sendSysEx(int length,
						   const byte *const array,
						   bool ArrayContainsBoundaries)
{
    DBG_PRINTF("[%s]\n",__func__);
	int i;
	if (ArrayContainsBoundaries == FALSE) {
		
		// TODO USE_SERIAL_PORT.write(0xF0);
		
		for (i=0;i<length;++i) {
			
			// TODO USE_SERIAL_PORT.write(array[i]);
			
		}
		
		// TODO USE_SERIAL_PORT.write(0xF7);
		
	}
	else {
		
		for (i=0;i<length;++i) {
			
			// TODO USE_SERIAL_PORT.write(array[i]);
			
		}
		
	}
	
#if USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
	
}


/*! \brief Send a Tune Request message. 
 
 When a MIDI unit receives this message, it should tune its oscillators (if equipped with any) 
 */
void sendTuneRequest()
{
	DBG_PRINTF("[%s]\n",__func__);
	sendRealTime(TuneRequest);

}



/*! \brief Send a MIDI Time Code Quarter Frame. 
 
 See MIDI Specification for more information.
 \param data	 if you want to encode directly the nibbles in your program, you can send the byte here.
 */
void sendToSerialTimeCodeQuarterFrame(byte data)
{
	DBG_PRINTF("[%s]\n",__func__);
	// TODO USE_SERIAL_PORT.write((byte)TimeCodeQuarterFrame);
	// TODO USE_SERIAL_PORT.write(data);

#if USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
	
}

/*! \brief Send a MIDI Time Code Quarter Frame. 
 
 See MIDI Specification for more information.
 \param TypeNibble	MTC type
 \param ValuesNibble	MTC data
 */
void sendTimeCodeQuarterFrame(byte TypeNibble, byte ValuesNibble) 
{
	DBG_PRINTF("[%s]\n",__func__);
	byte data = ( ((TypeNibble & 0x07) << 4) | (ValuesNibble & 0x0F) );
	sendToSerialTimeCodeQuarterFrame(data);
	
}



/*! \brief Send a Song Position Pointer message.
 \param Beats	The number of beats since the start of the song.
 */
void sendSongPosition(unsigned int Beats)
{
	DBG_PRINTF("[%s]\n",__func__);
	// TODO USE_SERIAL_PORT.write((byte)SongPosition);
	// TODO USE_SERIAL_PORT.write(Beats & 0x7F);
	// TODO USE_SERIAL_PORT.write((Beats >> 7) & 0x7F);

#if USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
	
}


/*! \brief Send a Song Select message */
void sendSongSelect(byte SongNumber)
{
	DBG_PRINTF("[%s]\n",__func__);
	// TODO USE_SERIAL_PORT.write((byte)SongSelect);
	// TODO USE_SERIAL_PORT.write(SongNumber & 0x7F);

#if USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
	
}


/*! \brief Send a Real Time (one byte) message. 
 
 \param Type The available Real Time types are: Start, Stop, Continue, Clock, ActiveSensing and SystemReset.
 You can also send a Tune Request with this method.
 @see kMIDIType
 */
void sendRealTime(enum kMIDIType Type)
{
    DBG_PRINTF("[%s]\n",__func__);
	switch (Type) {
		case TuneRequest: // Not really real-time, but one byte anyway.
		case Clock:
		case Start:
		case Stop:	
		case Continue:
		case ActiveSensing:
		case SystemReset:
			// TODO USE_SERIAL_PORT.write((byte)Type);
			break;
		default:
			// Invalid Real Time marker
			break;
	}
	
	// Do not cancel Running Status for real-time messages as they can be interleaved within any message.
	// Though, TuneRequest can be sent here, and as it is a System Common message, it must reset Running Status.
#if USE_RUNNING_STATUS
	if (Type == TuneRequest) mRunningStatus_TX = InvalidType;
#endif
	
}


/* [] END OF FILE */
