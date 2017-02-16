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
#include "common.h"
#include "debug.h"
#include "ButtonScanner.h"

void ButtonScannerInit(void)
{
    buttonInputScan_S4_Write(1); // attiva PullUp
    buttonInputScan_S5_Write(1); // attiva PullUp
    buttonInputScan_S6_Write(1); // attiva PullUp
    buttonInputScan_S7_Write(1); // attiva PullUp
}

void ButtonScannerPoll(void)
{
    static uint8 isButtonScannerInitialized = FALSE;
    
    
}
/* [] END OF FILE */
