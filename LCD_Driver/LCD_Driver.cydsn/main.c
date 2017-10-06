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
#include "debug.h"
#include "myLCD.h"
#include "tick.h"
#include "M2M_SPI_Slave.h"

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	SysTick_Start();
	PWM_LED_Start();
	Debug_Init();
	
	CyWdtStart(CYWDT_1024_TICKS, CYWDT_LPMODE_NOCHANGE); // 2.048 - 3.072 s
	
    for(;;)
    {
        /* Place your application code here. */
		CyWdtClear(); 
		LCD_Application_Poll();
		M2M_SPI_Slave_ApplicationPoll();
    }
}

/* [] END OF FILE */
