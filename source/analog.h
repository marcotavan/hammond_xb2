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

#ifndef _ANALOG_H_
#define _ANALOG_H_

#define EVENT_DRAWBAR_GENERIC           1
#define MOD_WHEEL_ANALOG_INPUT          9
#define PITCH_WHEEL_ANALOG_INPUT        10
#define EXPRESSION_ANALOG_INPUT         11
#define REVERB_ANALOG_INPUT             12
#define VOLUME_ANALOG_INPUT             13
#define MAX_ANALOG_CHANNELS             14
    
void AnalogPoll(void);
   
#endif
/* [] END OF FILE */