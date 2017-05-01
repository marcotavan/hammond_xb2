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

#ifndef _MIDI_EVENTS_H_
#define _MIDI_EVENTS_H_
 
    #include <project.h>
    
    #define DEVICE                  (0u)
    #define MIDI_MSG_SIZE           (4u)

    /*MIDI Message Fields */
    #define MIDI_MSG_TYPE           (0u)
    #define MIDI_NOTE_NUMBER        (1u)
    #define MIDI_NOTE_VELOCITY      (2u)

    /* MIDI Notes*/
    #define NOTE_69                 (69u)
    #define NOTE_72                 (72u)
    #define NOTE_76                 (76u)
    #define NOTE_79                 (79u)
    #define MIDI_FIRST_NOTE_88      (21u)
    #define MIDI_FIRST_NOTE_61      (36u)
    #define MIDI_LAST_NOTE          (96u)

    /* MIDI Notes Velocity*/
    #define VOLUME_OFF              (0u)
    #define VOLUME_ON               (127u)
        
    /* Need for Identity Reply message */
    extern volatile uint8 USB_MIDI1_InqFlags;
    extern volatile uint8 USB_MIDI2_InqFlags;
 
#endif
/* [] END OF FILE */
