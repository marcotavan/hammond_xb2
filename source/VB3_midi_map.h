/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-01-18
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


#define VAL_UPPER_DRAWBARS_A                    	0
#define VAL_UPPER_DRAWBARS_B                    	127
/**********************************************************************/
#define CC_Rotary_Speaker_Speed_Fast_Slow           1  // switch
	// free 										2-6
#define CC_Overall_Volume                           7   // level
#define CC_Overall_Tone                             8   // level
#define CC_Rotary_Speaker_Spread                    9   // level
#define CC_Rotary_Speaker_Balance                   10  // level
#define CC_Expression_Pedal                         11  // level

	// free											12-20	
enum lowerManualDrawbarSetB {
    LM_SET_B_DRAWBAR_16     					  = 21,
    LM_SET_B_DRAWBAR_5_13,						 // 22
    LM_SET_B_DRAWBAR_8,							 // 23
    LM_SET_B_DRAWBAR_4,							 // 24
    LM_SET_B_DRAWBAR_2_23,						 // 25
    LM_SET_B_DRAWBAR_2,							 // 26
    LM_SET_B_DRAWBAR_1_35,						 // 27
    LM_SET_B_DRAWBAR_1_13,						 // 28
    LM_SET_B_DRAWBAR_1							 // 29
};

#define CC_Vibrato_Lower                            30 // switch
#define CC_Vibrato_Upper                            31 // switch
	// free											32	

enum pedalManualDrawbar { // Pedal Drawbars      16'=33, 8'=35
    PM_DRAWBAR_16     							  = 33,
	// free											34
    PM_DRAWBAR_8      							  = 35,
};
	// free											36-39
enum upperManualDrawbarSetA {
    UM_SET_A_DRAWBAR_16     					  = 40,
    UM_SET_A_DRAWBAR_5_13,						 // 41
    UM_SET_A_DRAWBAR_8,							 // 42
    UM_SET_A_DRAWBAR_4,							 // 43
    UM_SET_A_DRAWBAR_2_23,						 // 44
    UM_SET_A_DRAWBAR_2,							 // 45
    UM_SET_A_DRAWBAR_1_35,						 // 46
    UM_SET_A_DRAWBAR_1_13,						 // 47
    UM_SET_A_DRAWBAR_1							 // 48
};
 
#define CC_Upper_Manual_Drawbars_AB_Switch          49    // switch
enum lowerManualDrawbarSetA {
    LM_SET_A_DRAWBAR_16      					  = 50,
    LM_SET_A_DRAWBAR_5_13,						 // 51
    LM_SET_A_DRAWBAR_8,							 // 52
    LM_SET_A_DRAWBAR_4,							 // 53
    LM_SET_A_DRAWBAR_2_23,						 // 54
    LM_SET_A_DRAWBAR_2,							 // 55
    LM_SET_A_DRAWBAR_1_35,						 // 56
    LM_SET_A_DRAWBAR_1_13,						 // 57
    LM_SET_A_DRAWBAR_1							 // 58	
};

#define CC_Lower_Manual_Drawbars_AB_Switch        	59  // switch
	// free											60-63
#define CC_DamperPedal								64   
	// free											65
#define CC_Percussion_On_Off                        66 // switch
#define CC_Tube_Overdrive_Switch                    67 // switch
#define CC_Rotary_Speaker_Brake                    	68  // level
	// free											69
#define CC_Percussion_Volume                        70 // switch
#define CC_Percussion_Decay                         71 // switch
#define CC_Percussion_Harmonic                      72 // switch
#define CC_Vibrato_Type                             73 // level
	// free											74
#define CC_Keyclick_Level                           75 // level
#define CC_Tube_Overdrive_Drive                     76 // level
	// free											77
#define CC_Tube_Overdrive_Edge                      78 // level    
    // free											79-80
#define CC_Rotary_Speaker_Treble_Horn_Slow_Speed   	81  // level
#define CC_Rotary_Speaker_Treble_Horn_Fast_Speed   	82  // level
#define CC_Rotary_Speaker_Treble_Horn_Acceleration 	83  // level
#define CC_Reverb	                                84 // level
#define CC_Rotary_Speaker_Bypass                    85 // switch 
	// free											86-90
#define CC_Rotary_Speaker_Bass_Horn_Slow_Speed     	91  // level
#define CC_Rotary_Speaker_Bass_Horn_Fast_Speed     	92  // level
#define CC_Rotary_Speaker_Bass_Horn_Acceleration   	93  // level
	// free											94+...
#endif

/* [] END OF FILE */
