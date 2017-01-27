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
#include "customLcd.h"
#include "FiltroMediano.h"

#define MAX_SAMPLE  1

volatile uint8 adcConversionDone = 0;
static  void AnalogEventTrigger(uint8 event, uint8 channel, uint16 data);

#if ADC_ISR_ENABLE
CY_ISR( ADC_ISR )
{
    /* Place your code here */
    adcConversionDone = 1;
}
#endif

enum {
    EVENT_NONE,
    EVENT_DRAWBAR,
    EVENT_MODULATION_WHEEL,
    EVENT_PITCH_WHEEL
};

void AnalogEventTrigger(uint8 event, uint8 channel, uint16 data)
{
    static char displayStr[15] = {'\0'};
    uint8 barGraph = 0;
    
    switch(event)
    {
        case EVENT_DRAWBAR:
        {
            sendControlChange(UM_SET_A_DRAWBAR_16+channel,data,MIDI_CHANNEL_1);
            
            sprintf(displayStr,"%4d - DWB %2d",data,channel);
            LCD_Position(1,0);
            LCD_PrintString(displayStr);
            
            barGraph = ((data>>4) + 1) & 0x7F;
            
            LCD_DrawVerticalBG(0, channel+7, 8,barGraph);
        } 
        break;
        
        default:
        break;
    }
}

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
    // chiamata nel main 
    /* Variable to count number of samples collected from ADC */
    static uint8 sampleCount = 0;
	
    /* Variable to hold cumulative samples */
    static uint16 adcSamples = 0;
	
    /* Variable to hold the average volts for 8 samples */
    static uint16 averageSamples = 0;
	
    static uint8 isAnalogPollNotInitialized = 1;
    
    static uint8 drawbarChannel = 0;
    
    static uint8 drawbarVal[MAX_DRAWBAR_CHANNELS];
    
    static uint16 validData=0;
    
    if (isAnalogPollNotInitialized)
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
        
        drawbarChannel = 0; // start from 1st
        adcSamples = 0;
        sampleCount = 0;
        isAnalogPollNotInitialized = 0;
        AMux_FastSelect(drawbarChannel);
        
        ADC_StartConvert();
    }
    
    if(tick_1ms(TICK_ANALOG))
    {
        
#if ADC_ISR_ENABLE
        ADC_IsEndConversion(ADC_WAIT_FOR_RESULT)
        // DBG_PRINTF("%d \n",ADC_GetResult16());
        if(adcConversionDone)
#else
        if(ADC_IsEndConversion(ADC_RETURN_STATUS)) // continua solo se la precedente conversione e' andata a buon fine
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
                
                validData = FiltroMediano(drawbarChannel,averageSamples);    //
                if(isValidDifference(drawbarVal[drawbarChannel],validData,2))
                {
                    // c'è una valida differenza con il campione precedente?
                    AnalogEventTrigger(EVENT_DRAWBAR,drawbarChannel, validData);
                    drawbarVal[drawbarChannel] = validData;
                }
                
                drawbarChannel++;
                if (drawbarChannel == MAX_DRAWBAR_CHANNELS) {
                    drawbarChannel = 0;
                }
                
                AMux_FastSelect(drawbarChannel);
            }
            adcConversionDone = 0;   
        }
   }
}
/* [] END OF FILE */
