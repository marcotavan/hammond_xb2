/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This example project demonstrates the MIDI interface device operation.  The 
*  project enumerates as a USB Audio Device with the MIDI feature and does not 
*  require additional drivers. The main goal of the USB MIDI interface is to 
*  transfer and convert MIDI data between external MIDI devices that use the 
*  UART interface, and a PC through the USB bus.			
*   
* Related Document:
*  Universal Serial Bus Specification Revision 2.0 
*  Universal Serial Bus Device Class Definition for MIDI Devices Release 1.0
*  MIDI 1.0 Detailed Specification Document Version 4.2
*
********************************************************************************
* Copyright 2012-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

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

#define BUTT1	                (0x01u)
#define BUTT2	                (0x02u)

#define USB_SUSPEND_TIMEOUT     (2u)

volatile uint8 usbActivityCounter = 0u;

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
void  TestPlayNote(void);
void TestPlayButtons(void);
void  TestVB3Drawbars(void);
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
    if(0u != USB_CheckActivity()) 
    {
        usbActivityCounter = 0u;
    } 
    else 
    {
        usbActivityCounter++;
    }
    /* Clear Pending Interrupt */
    SleepTimer_GetStatus();
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
    /* Start USBFS device 0 with VDDD operation */
    USB_Start(DEVICE, USB_DWR_VDDD_OPERATION); 
    
    eeprom_init();
    
    KeyScanInit(); 
    // fill_icsi_log_table(8, lookup_table);
    
    CyDelay(1000);  // non si sa mai
    
    while(1u)
    {
        /* Host can send double SET_INTERFACE request */
        if(0u != USB_IsConfigurationChanged())
        {
            /* Initialize IN endpoints when device configured */
            if(0u != USB_GetConfiguration())   
            {
                /* Power ON CY8CKIT-044 board */
                // MIDI_PWR_Write(0u); 
                
                /* Start ISR to determine sleep condition */		
                Sleep_isr_StartEx(SleepIsr);
                
                /* Start SleepTimer's operation */
                SleepTimer_Start();
                
            	/* Enable output endpoint */
                USB_MIDI_Init();
            }
            else
            {
                SleepTimer_Stop();
            }    
        }        
  
        /* Service USB MIDI when device is configured */
        if(0u != USB_GetConfiguration())    
        {
  
          /* Call this API from UART RX ISR for Auto DMA mode */
            #if(!USB_EP_MANAGEMENT_DMA_AUTO) 
                USB_MIDI_IN_Service();
            #endif
            /* In Manual EP Memory Management mode OUT_EP_Service() 
            *  may have to be called from main foreground or from OUT EP ISR
            */
          
            #if(!USB_EP_MANAGEMENT_DMA_AUTO) 
                USB_MIDI_OUT_Service();
            #endif
    
            /* Sending Identity Reply Universal System Exclusive message 
             * back to computer */
            if(0u != (USB_MIDI1_InqFlags & USB_INQ_IDENTITY_REQ_FLAG))
            {
                USB_PutUsbMidiIn(sizeof(MIDI_IDENTITY_REPLY), \
                            (uint8 *)MIDI_IDENTITY_REPLY, USB_MIDI_CABLE_00);
                USB_MIDI1_InqFlags &= ~USB_INQ_IDENTITY_REQ_FLAG;
            }
            #if (USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF)
                if(0u != (USB_MIDI2_InqFlags & USB_INQ_IDENTITY_REQ_FLAG))
                {
                    USB_PutUsbMidiIn(sizeof(MIDI_IDENTITY_REPLY), \
                            (uint8 *)MIDI_IDENTITY_REPLY, USB_MIDI_CABLE_01);
                    USB_MIDI2_InqFlags &= ~USB_INQ_IDENTITY_REQ_FLAG;
                }
            #endif /* End USB_MIDI_EXT_MODE >= USB_TWO_EXT_INTRF */

            // TestPlayButtons();
            
            // TestPlayNote();
            
            // TestVB3Drawbars();
            LCD_Poll();
            
            AnalogPoll();
            
            Check_if_host_requested_USB_Suspend();
            
            if(flag_500us_ISR)
            { // probabilmente questo deve essere inserito nell'ISR
                KeyScan_Poll();
                flag_500us_ISR = 0;
            }
            
            ButtonScannerPoll();
            
            EepromPoll();
            
            UART_DEBUG_PARSER_Task();
        }
        
       
    }
}


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

void Check_if_host_requested_USB_Suspend(void)
{
    if( usbActivityCounter >= USB_SUSPEND_TIMEOUT ) 
    {
        DBG_PRINTF("Prepares system for sleep mode\n");
        DBG_PRINTF("MIDI_UART_Sleep\n");
        MIDI1_UART_Sleep();
        MIDI2_UART_Sleep();
        
        /* Power OFF CY8CKIT-044 board */
        DBG_PRINTF("Power OFF MIDI board\n");
        MIDI_PWR_Write(1u);     
        
        /***************************************************************
        * Disable USBFS block and set DP Interrupt for wake-up 
        * from sleep mode. 
        ***************************************************************/
        DBG_PRINTF("USB_Suspend\n");
        USB_Suspend(); 
        DBG_PRINTF("system sleep mode\n");
        CyDelay(1000);
        /* Prepares system clocks for sleep mode */
        CyPmSaveClocks();
        /***************************************************************
        * Switch to the Sleep Mode for the PSoC 3 or PSoC 5LP devices:
        *  - PM_SLEEP_TIME_NONE: wakeup time is defined by PICU source
        *  - PM_SLEEP_SRC_PICU: PICU wakeup source 
        ***************************************************************/
        CyPmSleep(PM_SLEEP_TIME_NONE, PM_SLEEP_SRC_PICU);
        /* Restore clock configuration */
        CyPmRestoreClocks();
        
        DBG_PRINTF("\nUSB_Resume\n");
        /* Enable USBFS block after power-down mode */
        USB_Resume();
        
        /* Enable output endpoint */
        USB_MIDI_Init();
        
        DBG_PRINTF("Power ON MIDI board\n");
        /* Power ON CY8CKIT-044 board */
        MIDI_PWR_Write(0u);
        
        MIDI1_UART_Wakeup();
        MIDI2_UART_Wakeup();
        usbActivityCounter = 0u; /* Re-init USB Activity Counter*/
    }
}      

/* [] END OF FILE */
