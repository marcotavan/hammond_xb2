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
#include "midiEvents.h"
#include "common.h"

/*****************************************************/
#define VERBOSE_SEND_MIDI (0)
#define VERBOSE_SEND_MIDI_MESSAGE (0)
/*****************************************************/

struct midimsg	mMessage;

byte genstatus(const enum kMIDIType inType,const byte inChannel);
void sendRealTime(enum kMIDIType Type);
void SendUartMidiOut(uint8 len, uint8 *midiMsg);

/*
volatile uint8 MIDI_TX_STS_COMPLETE = 0;

uint8 GetMIDI_TX_STS(uint8 bitField) {
	uint8 retVal = MIDI_TX_STS_COMPLETE;
	MIDI_TX_STS_COMPLETE &=~ bitField;
	return retVal;
}

CY_ISR(isr_midi_tx_status_InterruptHandler) {
	MIDI_TX_STS_COMPLETE = 0xFF;
}
*/

const char *noteNamearray[] = {
            "C",
            "C#",
            "D",
            "D#",
            "E",
            "F",
            "F#",
            "G",
            "G#",
            "A",
            "A#",
            "B"
        };
// DBG_PRINTF("[%s]\t %d=%s %d\n",__func__,NoteNumber, noteNamearray[(NoteNumber%12)], Velocity);

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
uint8 sendMidiMessage(enum kMIDIType type,
					  byte data1,
					  byte data2,
					  byte channel)
{
    // static uint16 var3 = 0;
	
	
	DBG_PRINTF("[%s] type:%02X data1:%02X data2:%02X channel:%02X\n",__func__,type,data1,data2,channel);
    static uint8 midiMsg[MIDI_MSG_SIZE];
    uint8 err = 0;
    
    // DBG_PRINTF("var3: %d\n",var3++);
    
	// Then test if channel is valid
	if (channel >= MIDI_CHANNEL_OFF || channel == MIDI_CHANNEL_OMNI || type < NoteOff) 
    {
		
#if USE_RUNNING_STATUS	
		mRunningStatus_TX = InvalidType;
#endif 
		DBG_PRINTF("Channel Error. Don't send anything!\n");
		return 0xFF; // Don't send anything
	}
	
	if (type <= PitchBend) 
    {
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
			// DBG_PRINTF("USE_SERIAL_PORT.write(mRunningStatus_TX);\n");
            midiMsg[MIDI_MSG_TYPE] = mRunningStatus_TX;
		}
#else
		// Don't care about running status, send the Control byte.
		// DBG_PRINTF("USE_SERIAL_PORT.write(statusbyte);\n2);
        midiMsg[MIDI_MSG_TYPE] = statusbyte;
#endif
		
		// Then send data
		// DBG_PRINTF("USE_SERIAL_PORT.write(data1);\n");
        midiMsg[MIDI_NOTE_NUMBER] = data1;
		if (type != ProgramChange && type != AfterTouchChannel) 
        {
			// DBG_PRINTF("USB_3BYTE_COMMON\n");
            midiMsg[MIDI_NOTE_VELOCITY] = data2;
            // DBG_PRINTF("%02x %02x %02x\n",midiMsg[0],midiMsg[1],midiMsg[2]);
			SendUartMidiOut(MIDI_3BYTE_COMMON, midiMsg);
			
			#if defined USB_MIDI_INTERFACE
			err = USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
			#endif	
		}
        else 
        {
            DBG_PRINTF("USB_2BYTE_COMMON\n");
			SendUartMidiOut(MIDI_2BYTE_COMMON, midiMsg);
			
			#if defined USB_MIDI_INTERFACE
            err = USB_PutUsbMidiIn(USB_2BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
			#endif
        }
        
        #if VERBOSE_SEND_MIDI_MESSAGE
        DBG_PRINTF("[%s] return err %d; %d\n",__func__,err,var3++);
        #endif
        
		return err;
	}
    
	if (type >= TuneRequest && type <= SystemReset) {
		// System Real-time and 1 byte.
		sendRealTime(type);
	}
	
    return err;
}                                
           
/*  data1 =NoteNumber, data2=Velocity
    USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);

    midiMsg[MIDI_MSG_TYPE == 0]         = USB_MIDI_NOTE_ON; 0x9+channel
    midiMsg[MIDI_NOTE_NUMBER == 1]      = NOTE_69;
    midiMsg[MIDI_NOTE_VELOCITY == 2]    = VOLUME_ON
*/

/*! \brief Send a Note On message 
 \param NoteNumber	Pitch value in the MIDI format (0 to 127). Take a look at the values, names and frequencies of notes here: http://www.phys.unsw.edu.au/jw/notes.html\n
 \param Velocity	Note attack velocity (0 to 127). A NoteOn with 0 velocity is considered as a NoteOff.
 \param Channel		The channel on which the message will be sent (1 to 16). 
 */
uint8 sendNoteOn(byte NoteNumber,
							byte Velocity,
							byte Channel)
{ 
    Velocity &= 0x7F; // 0-127    
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\t %d=%s %d\n",__func__,NoteNumber, noteNamearray[(NoteNumber%12)], Velocity);
    #endif

	return sendMidiMessage(NoteOn,NoteNumber,Velocity,Channel);

}


/*! \brief Send a Note Off message (a real Note Off, not a Note On with null velocity)
 \param NoteNumber	Pitch value in the MIDI format (0 to 127). Take a look at the values, names and frequencies of notes here: http://www.phys.unsw.edu.au/jw/notes.html\n
 \param Velocity	Release velocity (0 to 127).
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
uint8 sendNoteOff(byte NoteNumber,
							 byte Velocity,
							 byte Channel)
{
    Velocity &= 0x7F; // 0-127
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\t %d=%s %d\n",__func__,NoteNumber, noteNamearray[(NoteNumber%12)], Velocity);
    #endif
	return sendMidiMessage(NoteOff,NoteNumber,Velocity,Channel);

}


/*! \brief Send a Program Change message 
 \param ProgramNumber	The Program to select (0 to 127).
 \param Channel			The channel on which the message will be sent (1 to 16).
 */
uint8 SendProgramChange(uint8 ProgramNumber,   uint8 Channel)
{
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s] %d \n",__func__,ProgramNumber);
    #endif
	return sendMidiMessage(ProgramChange,ProgramNumber,0,Channel);

}


/*! \brief Send a Control Change message 
 \param ControlNumber	The controller number (0 to 127). See the detailed description here: http://www.somascape.org/midi/tech/spec.html#ctrlnums
 \param ControlValue	The value for the specified controller (0 to 127).
 \param Channel			The channel on which the message will be sent (1 to 16). 
 */
uint8 sendControlChange(byte ControlNumber,
								   byte ControlValue,
								   byte Channel)
{
    // ! dovrebbe gestire una coda di invio temporizzata.
    #if ~VERBOSE_SEND_MIDI
        // if(ControlNumber == 48)
	DBG_PRINTF("[%s]: %2x %d %d \n",__func__,ControlChange,ControlNumber,ControlValue);
    #endif
	return sendMidiMessage(ControlChange,ControlNumber,ControlValue,Channel);

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
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\n",__func__);
    #endif
	sendMidiMessage(AfterTouchPoly,NoteNumber,Pressure,Channel);

}


/*! \brief Send a MonoPhonic AfterTouch message (applies to all notes)
 \param Pressure		The amount of AfterTouch to apply to all notes.
 \param Channel			The channel on which the message will be sent (1 to 16). 
 */
void sendAfterTouch(byte Pressure, byte Channel)
{
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\n",__func__);
    #endif
	sendMidiMessage(AfterTouchChannel,Pressure,0,Channel);

}


/*! \brief Send a Pitch Bend message using an unsigned integer value.
 \param PitchValue	The amount of bend to send (in a signed integer format), between 0 (maximum downwards bend) and 16383 (max upwards bend), center value is 8192.
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
void sendPitchBend(unsigned int PitchValue,
							   byte Channel)
{
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\n",__func__);
    #endif
	sendMidiMessage(PitchBend,(PitchValue & 0x7F),(PitchValue >> 7) & 0x7F,Channel);
	
}

/*! \brief Send a Pitch Bend message using a signed integer value.
 \param PitchValue	The amount of bend to send (in a signed integer format), between -8192 (maximum downwards bend) and 8191 (max upwards bend), center value is 0.
 \param Channel		The channel on which the message will be sent (1 to 16).
 */
void sendPitchBendInt(int PitchValue,
							   byte Channel)
{
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\n",__func__);
    #endif
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
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\n",__func__);
    #endif
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
	// int i;
	// static const uint8 endByte[1] = {0xF7};
	if (ArrayContainsBoundaries == FALSE) {
		
		DBG_PRINTF("TODO USE_SERIAL_PORT.write(0xF0);\n");
		
		// for (i=0;i<length;++i) 
		// {
			DBG_PRINTF("USE_SERIAL_PORT.write(array[i]);\n");
			// USB_PutUsbMidiIn(length, array, USB_MIDI_CABLE_00);
		// }
		
		DBG_PRINTF("USE_SERIAL_PORT.write(0xF7);\n");
		// USB_PutUsbMidiIn(1, endByte, USB_MIDI_CABLE_00);
	}
	else {
		
		// for (i=0;i<length;++i) 
		// {
			DBG_PRINTF("USE_SERIAL_PORT.write(array[i]);\n");
			// USB_PutUsbMidiIn(length, array, USB_MIDI_CABLE_00);
		// }
		
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
    #if VERBOSE_SEND_MIDI
	DBG_PRINTF("[%s]\n",__func__);
    #endif
	sendRealTime(TuneRequest);

}



/*! \brief Send a MIDI Time Code Quarter Frame. 
 
 See MIDI Specification for more information.
 \param data	 if you want to encode directly the nibbles in your program, you can send the byte here.
 */
void sendToSerialTimeCodeQuarterFrame(byte data)
{
	DBG_PRINTF("[%s]\n",__func__);
	DBG_PRINTF("USE_SERIAL_PORT.write((byte)TimeCodeQuarterFrame);\n");
	DBG_PRINTF("USE_SERIAL_PORT.write(data);\n");

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
	DBG_PRINTF("USE_SERIAL_PORT.write((byte)SongPosition);\n");
	DBG_PRINTF("USE_SERIAL_PORT.write(Beats & 0x7F);\n");
	DBG_PRINTF("USE_SERIAL_PORT.write((Beats >> 7) & 0x7F);\n");

#if USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
	
}


/*! \brief Send a Song Select message */
void sendSongSelect(byte SongNumber)
{
	DBG_PRINTF("[%s]\n",__func__);
	DBG_PRINTF("USE_SERIAL_PORT.write((byte)SongSelect);\n");
	DBG_PRINTF("USE_SERIAL_PORT.write(SongNumber & 0x7F);\n");

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
			DBG_PRINTF("USE_SERIAL_PORT.write((byte)Type);\n");
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


void UART_MIDI_Init(void) {
/* Start UART block */
	MIDI1_UART_Start();
//	isr_midi_tx_StartEx(isr_midi_tx_status_InterruptHandler);
    #define MIDI_UART_RX_PRIOR_NUM   (0x02u)
    #define MIDI_UART_TX_PRIOR_NUM   (0x04u)
	
	/* Change the priority of the UART TX and RX interrupt */
    CyIntSetPriority(MIDI1_UART_TX_VECT_NUM, MIDI_UART_TX_PRIOR_NUM);
    CyIntSetPriority(MIDI1_UART_RX_VECT_NUM, MIDI_UART_RX_PRIOR_NUM);

}

void SendUartMidiOut(uint8 len, uint8 *midiMsg) {
 /* Puts data into the MIDI TX output buffer.*/
	// DBG_PRINTF("[%s]: ",__func__);
	// DBG_PRINT_ARRAY(midiMsg,len);
	// DBG_PRINTF("\n");
	/*
	uint8 i = 0;
	
	do
	{
	    MIDI1_UART_PutChar(midiMsg[i]);
	    i++;
	}
	while (i < len);
	*/
	MIDI1_UART_PutArray(midiMsg,len);
	if(0u != USB_GetConfiguration()) {
		USB_PutUsbMidiIn(len, midiMsg, USB_MIDI_CABLE_00);
	}
}


/* [] END OF FILE */
