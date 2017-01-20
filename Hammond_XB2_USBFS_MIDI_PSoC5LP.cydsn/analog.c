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
// #include "string.h"
#include "tick.h"
#include "debug.h"
#include "midiLibrary.h"
#include "VB3_midi_map.h"

#define MAX_SAMPLE  1

volatile uint8 adcConversionDone = 0;

#if ADC_ISR_ENABLE
CY_ISR( ADC_ISR )
{
    /* Place your code here */
    adcConversionDone = 1;
}
#endif

uint8 isValidDifference(uint8 a, uint8 b, uint8 diff)
{
    // se la differenza tra i valori è sufficientemente alta allora invia.
    if(a>b) {
        if((a-b) > diff) {
            return 1;
        }
    } 
    else if (b>a) {
        if((b-a) > diff) {
            return 1;
        }
    }
    
    return 0;
}


void AnalogPoll(void)
{
    /* Variable to count number of samples collected from ADC */
    static uint8 sampleCount = 0;
	
    /* Variable to hold cumulative samples */
    static uint16 adcSamples = 0;
	
    /* Variable to hold the average volts for 8 samples */
    static uint16 averageSamples = 0;
	
    /* Character array to hold the micro volts*/
    static char displayStr[15] = {'\0'};
    
    static uint8 init = 1;
    
    static uint8 drawbarChannel = 0;
    
    static uint8 drawbarVal[9];
    
    if (init)
    {
        /* Start ADC and start conversion */
        memset(drawbarVal,0xff,sizeof(drawbarVal));
        adcConversionDone = 0;
        AMux_Start();
        ADC_Start();
        
        #if ADC_ISR_ENABLE
        ADC_IRQ_Enable(); /* Enable ADC interrupts */
        ADC_IRQ_StartEx(ADC_ISR);
        #endif
        
        /* Start LCD and set position */
        LCD_Start();
        LCD_Position(0,1);
        LCD_PrintString("ADC DRAWBAR ");
        drawbarChannel = 8;
        adcSamples = 0;
        sampleCount = 0;
        init = 0;
        AMux_FastSelect(drawbarChannel);
        
        ADC_StartConvert();
    }
    
    if(tick_10ms(TICK_ANALOG))
    {
        /* Read ADC count and convert to milli volts */
        
#if ADC_ISR_ENABLE
        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT)
        // DBG_PRINTF("%d \n",ADC_GetResult16());
        if(adcConversionDone)
#else
        if(ADC_IsEndConversion(ADC_RETURN_STATUS))
#endif
        {
            adcSamples = adcSamples + ADC_GetResult16();
            sampleCount++;
            
            if(sampleCount == MAX_SAMPLE)
            {
                averageSamples = adcSamples >> 1; // diviso 4 + 1 per fare il 127
                
                // DBG_PRINTF("averageSamples %d\n ",averageSamples);
                
                adcSamples = 0;
                sampleCount = 0;

                if(isValidDifference(drawbarVal[drawbarChannel],averageSamples,2))
                {
                    sprintf(displayStr,"%4d - DWB %2d",averageSamples,drawbarChannel);
                    LCD_Position(1,0);
                    LCD_PrintString(displayStr);
                    
                    LCD_DrawVerticalBG(0, drawbarChannel+7, 8,averageSamples>>4);
                    
                    sendControlChange(UM_SET_A_DRAWBAR_16+drawbarChannel,averageSamples,MIDI_CHANNEL_1);
                    drawbarVal[drawbarChannel] = averageSamples;
                }
                
                drawbarChannel++;
                if (drawbarChannel == 9) {
                    drawbarChannel = 0;
                }
                AMux_FastSelect(drawbarChannel);
            }
            adcConversionDone = 0;   
        }
        #if 0
        voltCount = ADC_GetResult16();
        mVolts = ADC_CountsTo_mVolts(voltCount);
        
        DBG_PRINTF("voltCount:%4ld, mVolts:%4ld\n",voltCount,mVolts);
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
            sprintf(displayStr,"%4ld mV - DWB %d",averageVolts,drawbarChannel);
            LCD_Position(1,0);
            LCD_PrintString(displayStr);
            drawbarChannel++;
            if (drawbarChannel == 9) {
                drawbarChannel = 0;
            }
        }
        #endif
    }
}
/* [] END OF FILE */
