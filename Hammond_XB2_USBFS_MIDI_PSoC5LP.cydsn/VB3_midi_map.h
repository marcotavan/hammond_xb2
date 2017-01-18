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

#ifndef _VB3_MIDI_MAP_H_
#define _VB3_MIDI_MAP_H_
    
    enum upperManualDrawbarSetA
    {
        UM_SET_A_DRAWBAR_16     = 40,
        UM_SET_A_DRAWBAR_5_13,
        UM_SET_A_DRAWBAR_8,
        UM_SET_A_DRAWBAR_4,
        UM_SET_A_DRAWBAR_2_23,
        UM_SET_A_DRAWBAR_2,
        UM_SET_A_DRAWBAR_1_35,
        UM_SET_A_DRAWBAR_1_13,
        UM_SET_A_DRAWBAR_1
    };
    
    enum upperManualDrawbarSetB
    {
        UM_SET_B_DRAWBAR_16     = 12,
        UM_SET_B_DRAWBAR_5_13,
        UM_SET_B_DRAWBAR_8,
        UM_SET_B_DRAWBAR_4,
        UM_SET_B_DRAWBAR_2_23,
        UM_SET_B_DRAWBAR_2,
        UM_SET_B_DRAWBAR_1_35,
        UM_SET_B_DRAWBAR_1_13,
        UM_SET_B_DRAWBAR_1
    };
    
    enum lowerManualDrawbarSetA
    {
        LM_SET_A_DRAWBAR_16     = 50,
        LM_SET_A_DRAWBAR_5_13,
        LM_SET_A_DRAWBAR_8,
        LM_SET_A_DRAWBAR_4,
        LM_SET_A_DRAWBAR_2_23,
        LM_SET_A_DRAWBAR_2,
        LM_SET_A_DRAWBAR_1_35,
        LM_SET_A_DRAWBAR_1_13,
        LM_SET_A_DRAWBAR_1
    };
    
    enum lowerManualDrawbarSetB
    {
        LM_SET_B_DRAWBAR_16     = 21,
        LM_SET_B_DRAWBAR_5_13,
        LM_SET_B_DRAWBAR_8,
        LM_SET_B_DRAWBAR_4,
        LM_SET_B_DRAWBAR_2_23,
        LM_SET_B_DRAWBAR_2,
        LM_SET_B_DRAWBAR_1_35,
        LM_SET_B_DRAWBAR_1_13,
        LM_SET_B_DRAWBAR_1
    };
/*    
VB3 Default MIDI Map
CONTROL TYPE                            Control Change Number

Upper Manual Drawbars A/B Switch        49 (can be changed manually from the .dat file) 
Upper Manual Drawbars - SET A           40-48 
Upper Manual Drawbars - SET B           12-20 

Lower Manual Drawbars A/B Switch        59 (can be changed manually from the .dat file) 
Lower Manual Drawbars - SET A           50-58 
Lower Manual Drawbars - SET B           21-29 

Pedal Drawbars                          16'=33, 8'=35

Vibrato Type                            73 
Vibrato Lower                           30 
Vibrato Upper                           31 

Percussion On/Off                       66 
Percussion Volume                       70 
Percussion Decay                        71 
Percussion Harmonic                     72 

Keyclick Level                          75 rotary

Tube Overdrive Switch                   67 
Tube Overdrive Drive                    76 
Tube Overdrive Edge                     78 
Reverb                                  84 

Rotary Speaker Bypass                   85 
Rotary Speaker Speed (Fast/Slow)        1 

Rotary Speaker Treble Horn Slow Speed   81 
Rotary Speaker Treble Horn Fast Speed   82 
Rotary Speaker Treble Horn Acceleration 83 

Rotary Speaker Bass Horn Slow Speed     91 
Rotary Speaker Bass Horn Fast Speed     92 
Rotary Speaker Bass Horn Acceleration   93 

Rotary Speaker Brake                    68 
Rotary Speaker Spread                   9 
Rotary Speaker Balance                  10 
Overall Tone                            8 

Expression Pedal                        11 

Overall Volume                          7 

    */
#endif

/* [] END OF FILE */
