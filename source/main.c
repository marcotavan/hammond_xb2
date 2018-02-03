/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-01-14
 * All Rights Reserve
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <project.h>
#include "tick.h"
#include "debug.h"
#include "midiEvents.h"
#include "midiLibrary.h"
#include "keyboardScanner.h"
#include "VB3_midi_map.h"
#include "analog.h"
#include "customLcd.h"
#include "ButtonScanner.h"
#include "EepromManager.h"
#include "pca9685_driver.h"
#include "M2M_SPI_Master.h"
#include "midiMerge.h"

#define BUTT1	                (0x01u)
#define BUTT2	                (0x02u)

// #define USB_SUSPEND_TIMEOUT     (2u)
#define USB_ACTIVITY_TIMEOUT	2

// volatile uint8 usbActivityCounter = 0u;

uint8 csButtStates = 0u;
uint8 csButtStatesOld = 0u;
uint8 csButtChange = 0u;

        
/* Identity Reply message */
uint8 CYCODE MIDI_IDENTITY_REPLY[] = {
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

uint16 play_note = 0;
int8 note_direction = 1;

void Check_if_host_requested_USB_Suspend(void);
// void  TestPlayNote(void);
// void TestPlayButtons(void);
// void  TestVB3Drawbars(void);
void isVSTReady();
/*******************************************************************************
* Function Name: SleepIsr
********************************************************************************
* Summary:
*  The sleep interrupt-service-routine used to determine a sleep condition.
*  The device goes into the Suspend state when there is a constant Idle 
*  state on its upstream-facing bus-lines for more than 3.0 ms. 
*  The device must be suspended drawing only suspend current from the 
*  bus after no more than 10 ms of the bus inactivity on all its ports.
*  This ISR is run each 4 ms, so after a second turn without the USB activity,  
*  the device should be suspended.
*
*******************************************************************************/
CY_ISR(SleepIsr)
{
    /* Check USB activity */
	#if defined USB_MIDI_INTERFACE
    if(USB_CheckActivity()) {
        usbActivityCounter = USB_ACTIVITY_TIMEOUT;
    } else {
        if(usbActivityCounter) {
			usbActivityCounter--;
		}
    }
	#endif
    /* Clear Pending Interrupt */
    SleepTimer_GetStatus();
}

void USB_Poll(void){
	static uint8 isPresent = 0;
	if(0u != USB_IsConfigurationChanged()) {
        // DBG_PRINTF("Initialize IN endpoints when device configured\n");
        if(0u != USB_GetConfiguration())   {
			DBG_PRINTF("\nUSB_MIDI_Init\n\n");
			/* Power ON CY8CKIT-044 board */
            
            /* Start ISR to determine sleep condition */		
           //  Sleep_isr_StartEx(SleepIsr);
            
            /* Start SleepTimer's operation */
            // SleepTimer_Start();
            
        	/* Enable output endpoint */
            USB_MIDI_Init();
			isPresent = 1;
        } 
	}   

    if(0u != USB_GetConfiguration()) {
		// Service USB MIDI when device is configured 
		
      	/* Call this API from UART RX ISR for Auto DMA mode */
        #if(!USB_EP_MANAGEMENT_DMA_AUTO) 
            USB_MIDI_IN_Service();
        #endif
        /* In Manual EP Memory Management mode OUT_EP_Service() 
        *  may have to be called from main foreground or from OUT EP ISR
        */
   		 /*
        #if(!USB_EP_MANAGEMENT_DMA_AUTO) 
            USB_MIDI_OUT_Service();
        #endif
		*/
        /* Sending Identity Reply Universal System Exclusive message 
         * back to computer */
		/*
        if(0u != (USB_MIDI1_InqFlags & USB_INQ_IDENTITY_REQ_FLAG))
        {
            USB_PutUsbMidiIn(sizeof(MIDI_IDENTITY_REPLY), \
                        (uint8 *)MIDI_IDENTITY_REPLY, USB_MIDI_CABLE_00);
            USB_MIDI1_InqFlags &= ~USB_INQ_IDENTITY_REQ_FLAG;
        }
		*/
        #if (USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF)
            if(0u != (USB_MIDI2_InqFlags & USB_INQ_IDENTITY_REQ_FLAG))
            {
                USB_PutUsbMidiIn(sizeof(MIDI_IDENTITY_REPLY), \
                        (uint8 *)MIDI_IDENTITY_REPLY, USB_MIDI_CABLE_01);
                USB_MIDI2_InqFlags &= ~USB_INQ_IDENTITY_REQ_FLAG;
            }
        #endif /* End USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF */

        // Check_if_host_requested_USB_Suspend();
    } 
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*       1. Starts the USBFS device and waits for enumaration.
*
*******************************************************************************/
int main()
{
    play_note = MIDI_FIRST_NOTE_61;
    /* Enable Global Interrupts */
    CyGlobalIntEnable;
                
    // SysTick_Start();  
    TimerTick_Start();
	
	MOD_SysLog_Init(); 
	
	M2M_SPI_Init();
	
	PCA9685_init(0,Mode_register_1,Mode_register_2);
    PCA9685_setPWMFrequency(1600);
    
	// #if defined USB_MIDI_INTERFACE
    /* Start USBFS device 0 with VDDD operation */
    USB_Start(DEVICE, USB_DWR_VDDD_OPERATION); 
    // #endif
	
    eeprom_init();
    
    KeyScanInit(); 
    // fill_icsi_log_table(8, lookup_table);

	UART_MIDI_Init();
		
    CyDelay(1000);  // non si sa mai
    
    while(1u)
    {
		USB_Poll();
		
        LCD_Poll();
        
        AnalogPoll();
      
        if(flag_500us_ISR)
        { // probabilmente questo deve essere inserito nell'ISR
            KeyScan_Poll();
            flag_500us_ISR = 0;
        }
        
        ButtonScannerPoll();
        
        EepromPoll();
        
        UART_DEBUG_PARSER_Task();
        
        // isVSTReady();
		
		LedPoll();	
		
		M2M_SPI_Master_ApplicationPoll();
		
		MidiMergePoll();
    }
}

void isVSTReady(void)
{
	static uint8 isVSTReadyToPlay = 0;
	// #define len 6
	//uint8 data[len] = {0xf0,0x7F,0x7F,0x06,0x02,0xF7};
	
	if(tick_100ms(TICK_IS_VST_READY))
	{
		// Pin_MOSI_M2M_Write(~Pin_MOSI_M2M_Read());
		// Pin_SCLK_M2M_Write(~Pin_SCLK_M2M_Read());
		// Pin_SS_M2M_Write(~Pin_SS_M2M_Read());
		
		DBG_PRINTF("A");
		if(isVSTReadyToPlay == 0)
		{
			// sendSysEx(len,data,1);
		}
	}
	
}


#if(0)
void TestPlayButtons(void)
{
    static uint8 midiMsg[MIDI_MSG_SIZE];
/* Service Keys */
	if (0u == SW1_Read()) {
        csButtStates |= BUTT1;
    } else {
        csButtStates &= ~BUTT1;
    }
    
	if (0u == SW2_Read()) {
        csButtStates |= BUTT2;
    }    else    {
        csButtStates &= ~BUTT2;
    }
    
    
   
	if (0u != (csButtChange = csButtStates ^ csButtStatesOld)) 
    { /* Process any button change */
		csButtStatesOld = csButtStates;

		/* All buttons are mapped to Note-On/Off messages */
		midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_ON;
		
		/* Button 1 */
		if (0u != (csButtChange & BUTT1)) 
        {
			/* Button determines note number */			
			midiMsg[MIDI_NOTE_NUMBER] = NOTE_69;
			if (0u != (csButtStates & BUTT1))
            {
                /* Note On */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_ON;		
            }
			else
            {
                /* Note Off */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_OFF;			
            }    
			/* Put MIDI Note-On/Off message into input endpoint */
            USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
            
            /* Button determines note number */			
			midiMsg[MIDI_NOTE_NUMBER] = NOTE_72;
			if (0u != (csButtStates & BUTT1))
            {
                /* Note On */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_ON;		
            }
			else
            {
                /* Note Off */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_OFF;			
            }    
			/* Put MIDI Note-On/Off message into input endpoint */
            USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
		}

		/* Button 2 */
		if (0u != (csButtChange & BUTT2))
        {
			/* Button determines note number */			
			midiMsg[MIDI_NOTE_NUMBER] = NOTE_76;
			if (0u != (csButtStates & BUTT2))
            {
                /* Note On */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_ON;  
            }
			else
            {
                /* Note Off */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_OFF; 
            }    
			/* Put MIDI Note-On/Off message into input endpoint */
			USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
            /* Second Note message */
            midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_ON;
        	midiMsg[MIDI_NOTE_NUMBER] = NOTE_72;
            if (0u != (csButtStates & BUTT2))
            {
                /* Note On */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_ON;		    
            }
			else
            {
                /* Note Off */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_OFF;			
            }    
			/* Put MIDI Note-On/Off message into input endpoint */
            USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
            
            midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_ON;
        	midiMsg[MIDI_NOTE_NUMBER] = NOTE_79;
            if (0u != (csButtStates & BUTT2))
            {
                /* Note On */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_ON;		    
            }
			else
            {
                /* Note Off */
                midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_OFF;			
            }    
			/* Put MIDI Note-On/Off message into input endpoint */
            USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
		}
        
        #if(USB_EP_MANAGEMENT_DMA_AUTO) 
           #if (USB_MIDI_EXT_MODE >= USB_ONE_EXT_INTRF)
                MIDI1_UART_DisableRxInt();
                #if (USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF)
                    MIDI2_UART_DisableRxInt();
                #endif /* End USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF */
            #endif /* End USB_MIDI_EXT_MODE >= USB_ONE_EXT_INTRF */            
            USB_MIDI_IN_Service();
            #if (USB_MIDI_EXT_MODE >= USB_ONE_EXT_INTRF)
                MIDI1_UART_EnableRxInt();
                #if (USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF)
                    MIDI2_UART_EnableRxInt();
                #endif /* End USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF */
            #endif /* End USB_MIDI_EXT_MODE >= USB_ONE_EXT_INTRF */                
        #endif
	} /* Process any button change */
}          

void  TestPlayNote(void)
{   // usato per mandare fuori una sequenza di note

    // static uint8 midiMsg[MIDI_MSG_SIZE];
    static uint8 divisore = 0;
    // static uint8 var1 = 0;
    // static uint8 var2 = 0;
    
    if(tick_10ms(TICK_TEST_NOTES))
    {
        divisore++;
        if (divisore == 10)
        {
            divisore = 0;
            //  midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_OFF;
        	//  midiMsg[MIDI_NOTE_NUMBER] = play_note;
            //  err=USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
            //  DBG_PRINTF("var1: %d\n",var1++);
            if(sendNoteOff(play_note,VOLUME_OFF,MIDI_CHANNEL_1)) {
                DBG_PRINTF("1.Host is not ready to receive this message\n");
            }
            
            switch(play_note) 
            {
                case MIDI_LAST_NOTE: // last note available
                {
                    // midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_OFF;
            	    // midiMsg[MIDI_NOTE_NUMBER] = play_note;
                    // err=USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
                    // if(USB_TRUE == err) DBG_PRINTF("3.Host is not ready to receive this message\n");
                
                    note_direction = -1;
                }
                break;
                
                case MIDI_FIRST_NOTE_61: // first note available
                {
                    // midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_OFF;
            	    // midiMsg[MIDI_NOTE_NUMBER] = play_note;
                    // err=USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
                    // if(USB_TRUE == err) DBG_PRINTF("4.Host is not ready to receive this message\n");
                    note_direction = 1;
                }
                break;
                
                default:
                break;
            }
            
            play_note += note_direction;
            // midiMsg[MIDI_MSG_TYPE] = USB_MIDI_NOTE_ON;
        	// idiMsg[MIDI_NOTE_NUMBER] = play_note;
            // midiMsg[MIDI_NOTE_VELOCITY] = VOLUME_ON;		
            
            // DBG_PRINTF("var2: %d\n",var2++);
            // err=USB_PutUsbMidiIn(USB_3BYTE_COMMON, midiMsg, USB_MIDI_CABLE_00);
            if(sendNoteOn(play_note,VOLUME_ON,MIDI_CHANNEL_1)) {
                DBG_PRINTF("2.Host is not ready to receive this message\n");
            }
        }// divisore
    } // tick 10ms
}

void  TestVB3Drawbars(void)
{
    static uint8 ControlNumber = UM_SET_A_DRAWBAR_16;
    static uint8 ControlValue = 0;
    static int8 direction = 1;
    static uint8 init = 1;
    // VB3_midi_map.h
    
    static uint8 i=0;
    
    if(init)
    {
        if(tick_100ms(TICK_TEST_DRAWBARS))
        {
            if(i<9)
            {
                sendControlChange(UM_SET_A_DRAWBAR_16+i,0,MIDI_CHANNEL_1);
                // CyDelayUs(100);
                sendControlChange(UM_SET_B_DRAWBAR_16+i,0,MIDI_CHANNEL_1);
                // CyDelayUs(100);
                sendControlChange(LM_SET_A_DRAWBAR_16+i,0,MIDI_CHANNEL_1);
                // CyDelayUs(100);
                sendControlChange(LM_SET_B_DRAWBAR_16+i,0,MIDI_CHANNEL_1);
                // CyDelayUs(100);
                i++;
                return;
            }
            else
            {
                init = 0;
            }
        }
        
        
        
    }
   
    
    if(tick_10ms(TICK_TEST_DRAWBARS))
    {
        sendControlChange(ControlNumber,ControlValue,MIDI_CHANNEL_1);
        // sendControlChange(ControlNumber+1,ControlValue,MIDI_CHANNEL_1);
        ControlValue+=direction;
        
        if(ControlValue == 127)
        {
            direction = -1;
        }
        else if(ControlValue == 0)
        {
            direction = 1;
            ControlNumber++;
            if(ControlNumber>UM_SET_A_DRAWBAR_1) ControlNumber = UM_SET_A_DRAWBAR_16;
        }            
    }
    
    
   /*
    if(tick_10ms(TICK_TEST_DRAWBARS))
    {
        if (ControlValue == 127)
        {   
            ControlValue = 0;
            ControlNumber++;
            if (ControlNumber == UM_SET_A_DRAWBAR_1) 
            {
                ControlNumber = UM_SET_A_DRAWBAR_16;
                direction = ~direction;
            }
            
        }
        sendControlChange(ControlNumber,ControlValue,MIDI_CHANNEL_1);
   
        ControlValue+=direction;
        
    }
    */
}
#endif

void Check_if_host_requested_USB_Suspend(void)
{
	
    //if( usbActivityCounter == 0 ) 
    {
        DBG_PRINTF("Prepares system for sleep mode\n");
//        DBG_PRINTF("MIDI_UART_Sleep\n");
//        MIDI1_UART_Sleep();
//        MIDI2_UART_Sleep();
        
        /* Power OFF CY8CKIT-044 board */
        // DBG_PRINTF("Power OFF MIDI board... ");
        // DBG_PRINTF("MIDI_PWR_Write(1u);   \n");  
        
        /***************************************************************
        * Disable USBFS block and set DP Interrupt for wake-up 
        * from sleep mode. 
        ***************************************************************/
        // DBG_PRINTF("USB_Suspend\n");
        // USB_Suspend(); 
        // DBG_PRINTF("system sleep mode\n");
        // CyDelay(1000);
        /* Prepares system clocks for sleep mode */
        // CyPmSaveClocks();
        /***************************************************************
        * Switch to the Sleep Mode for the PSoC 3 or PSoC 5LP devices:
        *  - PM_SLEEP_TIME_NONE: wakeup time is defined by PICU source
        *  - PM_SLEEP_SRC_PICU: PICU wakeup source 
        ***************************************************************/
        // CyPmSleep(PM_SLEEP_TIME_NONE, PM_SLEEP_SRC_PICU);
        /* Restore clock configuration */
        // CyPmRestoreClocks();
        
        // DBG_PRINTF("\nUSB_Resume\n");
        /* Enable USBFS block after power-down mode */
        // USB_Resume();
        
        /* Enable output endpoint */
        // USB_MIDI_Init();
        
        // DBG_PRINTF("Power ON MIDI board... \n");
        /* Power ON CY8CKIT-044 board */
        // DBG_PRINTF("MIDI_PWR_Write(0u); \n");
        
//        MIDI1_UART_Wakeup();
//        MIDI2_UART_Wakeup();
      //   usbActivityCounter = 0u; /* Re-init USB Activity Counter*/
		// usbActivityCounter++;
    }
}      
/*
da controllare cosa abbassa il volume
[sendControlChange]: b0 1 0 
[sendControlChange]: b0 85 127 
[sendControlChange]: b0 67 127 
[sendControlChange]: b0 30 0 
[sendControlChange]: b0 31 0 
[sendControlChange]: b0 73 127 
[sendControlChange]: b0 66 127 
[sendControlChange]: b0 70 127 
[sendControlChange]: b0 71 0 
[sendControlChange]: b0 72 127 
[sendControlChange]: b0 49 127 
[MidiMergePoll] ClearRxBuffer
[FiltroMediano]:chn 0 velocizzo il filtro
[sendControlChange]: b0 12 127 
[FiltroMediano]:chn 1 velocizzo il filtro
[sendControlChange]: b0 13 127 
[sendControlChange]: b0 14 0 
[sendControlChange]: b0 15 0 
[sendControlChange]: b0 16 0 
[sendControlChange]: b0 17 0 
[sendControlChange]: b0 18 0 
[sendControlChange]: b0 19 0 
[sendControlChange]: b0 20 0 
[sendControlChange]: b0 67 127 
[sendControlChange]: b0 76 0 
[sendControlChange]: b0 11 0 
[sendControlChange]: b0 84 0 
[sendControlChange]: b0 7 0 
[sendControlChange]: b0 14 127 
[sendControlChange]: b0 15 95 
[sendControlChange]: b0 19 113 
[sendControlChange]: b0 20 127 
[sendControlChange]: b0 11 127 
[sendControlChange]: b0 84 7 
[sendControlChange]: b0 7 122 
*/
/* [] END OF FILE */
