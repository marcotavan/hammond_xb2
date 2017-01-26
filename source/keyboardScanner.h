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

#ifndef _KEYBOARD_SCANNER_
#define _KEYBOARD_SCANNER_
    
#include "project.h"
  
#define MAX_SLOW_VELOCITY_COUNTER 1260
#define MAX_KEYS 88
    
void KeyScan_Poll(void);
void KeyScanInit(void);
#endif 

/* [] END OF FILE */
