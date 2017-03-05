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

#include <project.h>
#include "debug.h"
#include "midiEvents.h"

uint8 inqFlagsOld = 0u;

/*******************************************************************************
* Function Name: USB_callbackLocalMidiEvent
********************************************************************************
* Summary: Local processing of the USB MIDI out-events.
*
*******************************************************************************/
void USB_callbackLocalMidiEvent(uint8 cable, uint8 *midiMsg) CYREENTRANT
{
    /* Support General System On/Off Message. */
    if((0u == (USB_MIDI1_InqFlags & USB_INQ_SYSEX_FLAG)) \
            && (0u != (inqFlagsOld & USB_INQ_SYSEX_FLAG)))
    {
        if(midiMsg[USB_EVENT_BYTE0] == USB_MIDI_SYSEX_GEN_MESSAGE)
        {
            if(midiMsg[USB_EVENT_BYTE1] == USB_MIDI_SYSEX_SYSTEM_ON)
            {
                DBG_PRINTF("[%s] MIDI_PWR_Write(0u); /* Power ON */\n",__func__);
            }
            else if(midiMsg[USB_EVENT_BYTE1] == USB_MIDI_SYSEX_SYSTEM_OFF)
            {
                DBG_PRINTF("[%s] MIDI_PWR_Write(1u); /* Power OFF */\n",__func__);
            }
        }
    }
    inqFlagsOld = USB_MIDI1_InqFlags;
    cable = cable;
}    


/*******************************************************************************
* Function Name: USB_MIDI1_ProcessUsbOut_EntryCallback
********************************************************************************
* Summary:  Turn the LED_OutA on at the beginning of the function
* USB_MIDI1_ProcessUsbOut() when data comes to be put in the UART1 out
* buffer.
* 
*******************************************************************************/
void USB_MIDI1_ProcessUsbOut_EntryCallback(void)
{
    DBG_PRINTF("[%s]\n",__func__);
    // LED_OutA_Write(1);
}


/*******************************************************************************
* Function Name: USB_MIDI1_ProcessUsbOut_ExitCallback
********************************************************************************
* Summary:  Turn the LED_OutA off at the end of the function  
* USB_MIDI1_ProcessUsbOut() when data is put in the UART1 out-buffer.
* 
*******************************************************************************/
void USB_MIDI1_ProcessUsbOut_ExitCallback(void)
{
    DBG_PRINTF("[%s]\n",__func__);
    // LED_OutA_Write(0);
}


/*******************************************************************************
* Function Name: USB_MIDI2_ProcessUsbOut_EntryCallback
********************************************************************************
* Summary:  Turn the LED_OutB on at the beginning of the function  
* USB_MIDI2_ProcessUsbOut() when data comes to be put in the UART2 out- 
* buffer  
* 
*******************************************************************************/
void USB_MIDI2_ProcessUsbOut_EntryCallback(void)
{
    DBG_PRINTF("[%s]\n",__func__);
    // LED_OutB_Write(1);
}


/*******************************************************************************
* Function Name: USB_MIDI2_ProcessUsbOut_ExitCallback
********************************************************************************
* Summary:  Turn the LED_OutB off at the end of the function  
* USB_MIDI2_ProcessUsbOut() when data is put in the UART2 out-buffer.
* 
*******************************************************************************/
void USB_MIDI2_ProcessUsbOut_ExitCallback(void)
{
    DBG_PRINTF("[%s]\n",__func__);
    // LED_OutB_Write(0);
}


/*******************************************************************************
* Function Name: MIDI1_UART_RXISR_EntryCallback
********************************************************************************
* Summary:  Turn the LED_InA on at the beginning of the MIDI1_UART_RXISR ISR  
* when data comes to UART1 to be put in the USBFS_MIDI IN endpoint
* buffer.
*
*******************************************************************************/
void MIDI1_UART_RXISR_EntryCallback(void)
{
    /* These LEDs indicate MIDI input activity */
    DBG_PRINTF("[%s]: MIDI input activity\n",__func__);
    // LED_InA_Write(1);
}


/*******************************************************************************
* Function Name: MIDI1_UART_RXISR_ExitCallback
********************************************************************************
* Summary:  Turn the LED_InA off at the end of the MIDI1_UART_RXISR ISR  
* when data is put in the USBFS_MIDI IN endpoint buffer.
*
*******************************************************************************/
void MIDI1_UART_RXISR_ExitCallback(void)
{
    #if (USB_EP_MANAGEMENT_DMA_AUTO) 
        USB_MIDI_IN_Service();
    #endif /* (USB_EP_MANAGEMENT_DMA_AUTO) */
    DBG_PRINTF("[%s]\n",__func__);
    // LED_InA_Write(0);
}


/*******************************************************************************
* Function Name: MIDI2_UART_RXISR_EntryCallback
********************************************************************************
* Summary:  Turn the LED_InB on at the beginning of the MIDI2_UART_RXISR ISR  
* when data comes to UART2 to be put in the USBFS_MIDI IN endpoint  
* buffer.
*
*******************************************************************************/
void MIDI2_UART_RXISR_EntryCallback(void)
{
    /* These LEDs indicate MIDI input activity */
    DBG_PRINTF("[%s]: MIDI input activity\n",__func__);
    // LED_InB_Write(1);
}


/*******************************************************************************
* Function Name: MIDI2_UART_RXISR_ExitCallback
********************************************************************************
* Summary:  Turn the LED_InB off at the end of the MIDI2_UART_RXISR ISR  
* when data is put in the USBFS_MIDI IN endpoint buffer
*
*******************************************************************************/
void MIDI2_UART_RXISR_ExitCallback(void)
{
    #if (USB_EP_MANAGEMENT_DMA_AUTO) 
        USB_MIDI_IN_Service();
    #endif /* (USB_EP_MANAGEMENT_DMA_AUTO) */
    DBG_PRINTF("[%s]\n",__func__);
    // LED_InB_Write(0);
}

void MidiPanicButton(void)
{
    
}

/* [] END OF FILE */
