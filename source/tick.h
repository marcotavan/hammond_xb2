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

#ifndef _TICK_H_
#define _TICK_H_
        
    #include <project.h>

    #define FLAG_TICK_TIME_SET 0xFFFFFFFF

    #define MAX_TICK_TIMERS 64 /* aureo */
    #define FLAG_TICK_TIME_SET 0xFFFFFFFF

    enum { // MAX 50!
        TICK_TEST = 1,                      // 1
        TICK_MENU,                          // 2
        TICK_LIFE_CLOCK,                    // 3
        TICK_DALCNET,                       // 4
        TICK_POWERCONSUMPTION,              // 5
        TICK_HEARTBEAT,                     // 6
        TICK_SCRITTURA_RITARDATA,           // 7
        TICK_CODA_PWM,                      // 8
        TICK_GESTIONE_RELE,                 // 9
        TICK_EEPROM_BUSY,                   // 10
        TICK_LED_SCENE_TIMEOUT,             // 11
        TICK_CLIMA,                         // 12
        TICK_APPLICATION,                   // 13
        TICK_BOOTLOADER,                    // 14
        TICK_WIDOM,                         // 15
        TICK_ZW_SEND_RF_FRAME,              // 16
        TICK_TEMPERATURA,                   // 17
        TICK_TEST_EMI,                      // 18
        TICK_PROTEZIONE_TERMICA,            // 19
        TICK_MASTER_RESET,                  // 20
        TICK_TRIPLEPRESS,                   // 21
        TICK_PROTOCOL_INIT,                 // 22
        TICK_INTERNAL_EEPROM,               // 23
        TICK_ASSOCIATION,                   // 24
        TICK_SENSOR_AUTORESET,              // 25
        TICK_MAX31865                       // 26
    };

    uint8 tick_10ms(uint8 from);
    uint8 tick_100ms(uint8 from);
    uint8 tick_1s(uint8 from);    
        
    // void SysTick_Start(void);
    void TimerTick_Start(void);
#endif

/* [] END OF FILE */
