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

#ifndef _MIDI_H_
#define _MIDI_H_
 
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
    #define MIDI_FIRST_NOTE         (35u)
    #define MIDI_LAST_NOTE          (97u)

    /* MIDI Notes Velocity*/
    #define VOLUME_OFF              (0u)
    #define VOLUME_ON               (127u)
        
        
        /* Identity Reply message */
    const uint8 CYCODE MIDI_IDENTITY_REPLY[] = {
        0xF0u,                      /* SysEx */
        0x7Eu,                      /* Non-real time */
        0x7Fu,                      /* ID of target device (7F - "All Call") */
        0x06u,                      /* Sub-ID#1 - General Information */
        0x02u,                      /* Sub-ID#2 - Identity Reply */
        0x7Du,                      /* Manufacturer's ID: 7D - Educational Use */
        0xB4u, 0x04u,               /* Family code */
        0x32u, 0xD2u,               /* Model number */
        0x01u, 0x00u, 0x00u, 0x00u, /* Version number */
        /*0xF7         End of SysEx automatically appended */
    };

    /* Need for Identity Reply message */
    extern volatile uint8 USB_MIDI1_InqFlags;
    extern volatile uint8 USB_MIDI2_InqFlags;

#endif
/* [] END OF FILE */
