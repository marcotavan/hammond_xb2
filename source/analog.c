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
#include "ButtonScanner.h"

#define MAX_SAMPLE  1
#define DELTA_DRAWBARS		 3
#define DELTA_POTENZIOMETRI 2
volatile uint8 adcConversionDone = 0;
static  void AnalogEventTrigger(uint8 event, uint8 channel, uint16 data);

enum rotarySpeed
{
    ROTARY_STOP_SPEED,   
    ROTARY_SLOW_SPEED,
    ROTARY_FAST_SPEED
};

uint8 rotaryWheelStatus = ROTARY_SLOW_SPEED;
static uint8 overallVolumeLevel = 0;

#if ADC_ISR_ENABLE
CY_ISR( ADC_ISR )
{
    /* Place your code here */
    adcConversionDone = 1;
}
#endif
/***************************************************************************
* invia il dato di Control Change e prepara la stringa da scrivere nel display
* chiamta solo se isValidDifference()
****************************************************************************/

void AnalogEventTrigger(uint8 event, uint8 channel, uint16 data)
{
    // static char displayStr[15] = {'\0'};
	#define DRAWBARS_MULTIPLIER 116	// usat oper rapportare la sacla 0>110 a 127 PER I 220OHM in serie
	#define EXPRESSION_MULTIPLIER 100
    uint8 barGraph = 0;
    uint8 lcdColPosition = 0;
    uint8 offset = 1;
    static uint8 SendOverdriveSwitchOn = 0;
    static uint8 pitchWheelData = 0;
	uint16 scaledData = 0;
	
    switch(event)
    {
        case EVENT_DRAWBAR_GENERIC:
        {
			// sendControlChange(CC_Upper_Manual_Drawbars_AB_Switch,VAL_UPPER_DRAWBARS_A,MIDI_CHANNEL_1);  // invia un cambio forzato
            // mnon si puo' perchè si impappa il sistema
			
			scaledData = (data * DRAWBARS_MULTIPLIER) / 100 ;
			if(scaledData>127) scaledData = 127;

			DBG_PRINTF("EVENT_DRAWBAR_GENERIC %d->%d\n",data,scaledData);
            sendControlChange(UM_SET_B_DRAWBAR_16+channel,scaledData,MIDI_CHANNEL_1);
            
            lcdColPosition = channel+7;
            barGraph = ((scaledData>>4) + 1) & 0x7F;
            str_bargraph[ROW_0][lcdColPosition] =  barGraph;
            
            if (scaledData >= 126) offset = 0;
            str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
			// Write_BarGraphs();
            
        } 
        break;
        
        case MOD_WHEEL_ANALOG_INPUT:
        {
			DBG_PRINTF("MOD_WHEEL_ANALOG_INPUT %d\n",data);
            // 1 	00000001 	01 	Modulation Wheel or Lever 	            0-127 	MSB
            // if(SOLO_Button_on_Hold())
			if(pitchWheelData < 40)
            {
                if(SendOverdriveSwitchOn == 0)
                {
                    sendControlChange(CC_Tube_Overdrive_Switch,127,MIDI_CHANNEL_1);
                    SendOverdriveSwitchOn = 1;
                }
                
                sendControlChange(CC_Tube_Overdrive_Drive,data,MIDI_CHANNEL_1);
                
                lcdColPosition = event-MOD_WHEEL_ANALOG_INPUT;
                barGraph = ((data>>4) + 1) & 0x7F;
                str_bargraph[ROW_0][lcdColPosition] =  barGraph;
                
                if (data >= 126) offset = 0;
                str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
            } else {
                SendOverdriveSwitchOn = 0;
            }
        }
        break;
        
        
        case PITCH_WHEEL_ANALOG_INPUT:
        {
			DBG_PRINTF("PITCH_WHEEL_ANALOG_INPUT %d\n",data);
            // 1 	00000001 	01 	Modulation Wheel or Lever 	            0-127 	MSB
            // 0:31 SLOW.
            // 32:65 STOP
            // 96:127 FAST
			pitchWheelData = data;
			if(data > 88)
            {
				/*
                if (rotaryWheelStatus == ROTARY_SLOW_SPEED)
                {
                    data = 127;
                    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,data,MIDI_CHANNEL_1);
                    
                    lcdColPosition = event-MOD_WHEEL_ANALOG_INPUT;
                    barGraph = ((data>>4) + 1) & 0x7F;
                    str_bargraph[ROW_0][lcdColPosition] =  barGraph;
                    
                    if (data >= 126) offset = 0;
                    str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
                    
                    rotaryWheelStatus = ROTARY_FAST_SPEED;
                }
				*/
            }
            else if(data < 40)
            {
				/*
                if (rotaryWheelStatus == ROTARY_FAST_SPEED)
                {
                    data = 0;
                    sendControlChange(CC_Rotary_Speaker_Speed_Fast_Slow,data,MIDI_CHANNEL_1);
                    
                    lcdColPosition = event-MOD_WHEEL_ANALOG_INPUT;
                    barGraph = ((data>>4) + 1) & 0x7F;
                    str_bargraph[ROW_0][lcdColPosition] =  barGraph;
                    
                    if (data >= 126) offset = 0;
                    str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
                    
                    rotaryWheelStatus = ROTARY_SLOW_SPEED;
                }
				*/
            }
			else {
				// zona centrale
			}
        }
        break;
        
        case EXPRESSION_ANALOG_INPUT:
        {
			scaledData = (data * EXPRESSION_MULTIPLIER) / 100 ;
			if(scaledData>127) scaledData = 127;
			
			DBG_PRINTF("EXPRESSION_ANALOG_INPUT %d->%d\n",data,scaledData);
            // 1 	00000001 	01 	Modulation Wheel or Lever 	            0-127 	MSB
            sendControlChange(CC_Expression_Pedal,data,MIDI_CHANNEL_1);
            
            lcdColPosition = event-MOD_WHEEL_ANALOG_INPUT;
            barGraph = ((data>>4) + 1) & 0x7F;
            str_bargraph[ROW_0][lcdColPosition] =  barGraph;

            if (data >= 126) offset = 0;
            str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
            
        }
        break;
        
        case REVERB_ANALOG_INPUT:
        {
			DBG_PRINTF("REVERB_ANALOG_INPUT %d\n",data);
           // 1 	00000001 	01 	Modulation Wheel or Lever 	            0-127 	MSB
            sendControlChange(CC_Reverb,data,MIDI_CHANNEL_1);
            
            lcdColPosition = event-MOD_WHEEL_ANALOG_INPUT;
            barGraph = ((data>>4) + 1) & 0x7F;
            str_bargraph[ROW_0][lcdColPosition] =  barGraph;
            
            if (data >= 126) offset = 0;
            str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
        }
        break;
        
        case VOLUME_ANALOG_INPUT:
        {
			DBG_PRINTF("VOLUME_ANALOG_INPUT %d\n",data);
            // 1 	00000001 	01 	Modulation Wheel or Lever 	            0-127 	MSB
			if(SHIFT_Button_on_Hold()) {
				sendControlChange(CC_Overall_Tone,data,MIDI_CHANNEL_1);
            	Display_Alternate_Text(ROW_1,ALT_Overall_Tone);
			} else {
				if(getVolumeSolo() == VOLUME_NORMAL) {
					overallVolumeLevel = data; // store per ritorno da solo
		            sendControlChange(CC_Overall_Volume,data,MIDI_CHANNEL_1);
		            
		            lcdColPosition = event-MOD_WHEEL_ANALOG_INPUT;
		            barGraph = ((data>>4) + 1) & 0x7F;
		            str_bargraph[ROW_0][lcdColPosition] =  barGraph;

		            if (data >= 126) offset = 0;
		            str_bargraph[ROW_1][lcdColPosition] = '0'+barGraph-offset;
				}
			}
        }
        break;
        
        default:
        break;
    }
    
    // Write_BarGraphs();
}

uint8 isValidDifference(uint8 a, uint8 b, uint8 diff)
{
    // se la differenza tra i valori è sufficientemente alta allora invia.
    if(a>b) {
        if((a-b) >= diff) {
            return 1;
        }
    } 
    else if (b>a) {
        if((b-a) >= diff) {
            return 1;
        }
    }
    
    return 0;
}

static uint8 analogVal[MAX_ANALOG_CHANNELS];

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
    
    static uint8 analogChannel = 0;

	static uint16 validData=0;
    
    if (isAnalogPollNotInitialized)
    {
        /* Start ADC and start conversion */
        memset(analogVal,0xff,sizeof(analogVal));
        adcConversionDone = 0;
        AMux_Start();
        ADC_Start();
        
        #if ADC_ISR_ENABLE
        ADC_IRQ_Enable(); /* Enable ADC interrupts */
        ADC_IRQ_StartEx(ADC_ISR);
        #endif
        
        analogChannel = 0; // start from 1st
        adcSamples = 0;
        sampleCount = 0;
        isAnalogPollNotInitialized = 0;
        AMux_FastSelect(analogChannel);
        
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
                
                validData = FiltroMediano(analogChannel,averageSamples);    //
                
                switch (analogChannel)
                {
                    case MOD_WHEEL_ANALOG_INPUT:
                    { // analogInputs
                        if(isValidDifference(analogVal[analogChannel],validData,1)) // granularita del dato in uscita: 1 la differenza è 1 55 56 57 58 
                        {
                            // c'è una valida differenza con il campione precedente?
                            AnalogEventTrigger(analogChannel,analogChannel, validData);
                            analogVal[analogChannel] = validData;
                        }
                    }
                    break;
					
                    case EXPRESSION_ANALOG_INPUT:
					case REVERB_ANALOG_INPUT:
					case VOLUME_ANALOG_INPUT:					
                    case PITCH_WHEEL_ANALOG_INPUT:
                    { // analogInputs
                        if(isValidDifference(analogVal[analogChannel],validData,DELTA_POTENZIOMETRI)) // granularita del dato in uscita: 2 la differenza è 2 55 57 59 61
                        {
                            // c'è una valida differenza con il campione precedente?
                            AnalogEventTrigger(analogChannel,analogChannel, validData);
                            analogVal[analogChannel] = validData;
                        }
                    }
                    break;
                    
                    default: 
                    { // drawbars
                        if(isValidDifference(analogVal[analogChannel],validData,DELTA_DRAWBARS)) // granularita del dato in uscita: 2 la differenza è 2 55 57 59 61
                        {
                            // c'è una valida differenza con il campione precedente?
                            AnalogEventTrigger(EVENT_DRAWBAR_GENERIC,analogChannel, validData);
                            analogVal[analogChannel] = validData;
                        }
                    }
                    break;
                }
                
                analogChannel++;
                if (analogChannel == MAX_ANALOG_CHANNELS) {
                    analogChannel = 0;
                }
                
                AMux_FastSelect(analogChannel);
            }
            adcConversionDone = 0;   
        }
   }
}

uint8 GetOverallVolumeLevel(void) {
	// prende il livello attual del volume letto dal potenziometro
	return overallVolumeLevel;
}

void RefreshAllAnalogElements(void){
	DBG_PRINTF("reinvia tutti i dati delle analogiche\n");
	memset(analogVal,0xff,sizeof(analogVal));
}
/* [] END OF FILE */
