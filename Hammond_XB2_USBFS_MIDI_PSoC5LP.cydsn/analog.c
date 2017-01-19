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

#include "project.h"
#include "analog.h"
#include "stdio.h"
#include "tick.h"

#define MAX_SAMPLE                  8

void AnalogPoll(void)
{
    
 /* Variable to hold ADC count */
    static int32 voltCount = 0;
    
    /* Variable to hold the result in millivolts converted from ADC counts */
    static int32 mVolts = 0;

    /* Variable to count number of samples collected from ADC */
    static uint8 sampleCount = 0;
	
    /* Variable to hold cumulative samples */
    static int32 voltSamples = 0;
	
    /* Variable to hold the average volts for 8 samples */
    static uint32 averageVolts = 0;
	
    /* Character array to hold the micro volts*/
    static char displayStr[15] = {'\0'};
    
    static char init = 0;
    
    
    if (init == 0)
    {
        /* Start ADC and start conversion */
        ADC_Start();
        ADC_StartConvert();

        /* Start LCD and set position */
        LCD_Start();
        LCD_Position(0,1);
        LCD_PrintString("ADC input volt");
        init = 1;
    }
    
    if(tick_10ms(TICK_ANALOG))
    {
        /* Read ADC count and convert to milli volts */
        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
        voltCount = ADC_GetResult16();
        mVolts = ADC_CountsTo_mVolts(voltCount);
        
        /* Add the current ADC reading to the cumulated samples*/
        voltSamples = voltSamples + mVolts;

        sampleCount++;

        /* If 8 samples have been collected then average the samples and update the display*/
        if(sampleCount == MAX_SAMPLE)
        {
            averageVolts = voltSamples >> 3;
            voltSamples = 0;
            sampleCount = 0;

            /* Convert milli volts to string and display on the LCD. sprintf()
            *  function is standard library function defined in the stdio.h 
            *  header file */
            sprintf(displayStr,"    %4ld mV",averageVolts);
            LCD_Position(1,0);
            LCD_PrintString(displayStr);
        }	
    }
}
/* [] END OF FILE */
