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
#include "tick.h"


/*************************************************************************************/
volatile uint32 now1ms[/*MAX_TICK_TIMERS/32*/ 2] = {0,0};
volatile uint32 now10ms[/*MAX_TICK_TIMERS/32*/ 2] = {0,0};
volatile uint32 now100ms[/*MAX_TICK_TIMERS/32*/ 2] = {0,0};
volatile uint32 now1000ms[/*MAX_TICK_TIMERS/32*/ 2] = {0,0};
volatile uint16 upCounter = 0;

/*****************************************************************************\
*  Funzione:                                                                  
*  Argomenti:    Nessuno                 				      				  
*  Restituisce:  Nessuno                                                      
*  Scopo:                                                                     
\*****************************************************************************/
// #warning 29/12/2015 TICK ms Interamente da RIVEDERE
// #info 14/01/2016 tick rivisto e corretto com commit dd81ceb
uint8 tick_1ms(uint8 from) 
{ // usato per i tick periodici

    if (from<32)
    { // provengo da un tid inferiore al massimo numero di flag disponibili su 32bit
        if(now1ms[0] & (BIT0<<from))
        { 
            // sono passati i 10ms per il task di provenienza
            now1ms[0] &= ~(BIT0<<from);    // usato.. cancello
            // DBG_PRINTF("tick 10ms from:%d\n",from);
            return TRUE;
        }
    }
    else if (from<64)
    { // sono su un tid superiore ai primi 32bit disponibili, passo all'array successivo
        if(now1ms[1] & (BIT0<<(from-32)))
        {
            now1ms[1] &= ~(BIT0<<(from-32)); // usato, cancello
            // DBG_PRINTF("tick 10ms from:%d\n",from);
            return TRUE;
        }
    }
    else
    {
        DBG_PRINTF("timerTick %d oltre i 64 disponibili. Verificare la provenienza\n",from);
    }
    return FALSE;
}

/*****************************************************************************\
*  Funzione:                                                                  
*  Argomenti:    Nessuno                 				      				  
*  Restituisce:  Nessuno                                                      
*  Scopo:                                                                     
\*****************************************************************************/
// #warning 29/12/2015 TICK ms Interamente da RIVEDERE
// #info 14/01/2016 tick rivisto e corretto com commit dd81ceb
uint8 tick_10ms(uint8 from)
{ // usato per i tick periodici

    if (from<32)
    { // provengo da un tid inferiore al massimo numero di flag disponibili su 32bit
        if(now10ms[0] & (BIT0<<from))
        { 
            // sono passati i 10ms per il task di provenienza
            now10ms[0] &= ~(BIT0<<from);    // usato.. cancello
            // DBG_PRINTF("tick 10ms from:%d\n",from);
            return TRUE;
        }
    }
    else if (from<64)
    { // sono su un tid superiore ai primi 32bit disponibili, passo all'array successivo
        if(now10ms[1] & (BIT0<<(from-32)))
        {
            now10ms[1] &= ~(BIT0<<(from-32)); // usato, cancello
            // DBG_PRINTF("tick 10ms from:%d\n",from);
            return TRUE;
        }
    }
    else
    {
        DBG_PRINTF("timerTick %d oltre i 64 disponibili. Verificare la provenienza\n",from);
    }
    return FALSE;
}

/*****************************************************************************\
*  Funzione:                                                                  
*  Argomenti:    Nessuno                 				      				  
*  Restituisce:  Nessuno                                                      
*  Scopo:                                                                     
\*****************************************************************************/
uint8 tick_100ms(uint8 from)
{
    // uint8 val = 0;
//    if
    
    if (from<32)
    { // provengo da un tid inferiore al massimo numero di flag disponibili su 32bit
        if(now100ms[0] & (BIT0<<from))
        {
            // sono passati i 10ms per il task di provenienza
            now100ms[0] &= ~(BIT0<<from); // usato, cancello
            // DBG_PRINTF("tick 100ms from:%d\n",from);
            return TRUE;
        }
    }
    else if (from<64)
    { // sono su un tid superiore ai primi 32bit disponibili, passo all'array successivo
        if(now100ms[1] & (BIT0<<(from-32)))
        {
            now100ms[1] &= ~(BIT0<<(from-32)); // usato, cancello
            // DBG_PRINTF("tick 100ms from:%d\n",from);
            return TRUE;
        }
    }
    else
    {
        DBG_PRINTF("timerTick %d oltre i 64 disponibili. Verificare la provenienza\n",from);
    }
    
    return FALSE;
}

/*****************************************************************************\
*  Funzione:                                                                  
*  Argomenti:    Nessuno                 				      				  
*  Restituisce:  Nessuno                                                      
*  Scopo:                                                                     
\*****************************************************************************/

uint8 tick_1s(uint8 from)
{
    // uint8 val = 0;
    
    if (from<32)
    { // provengo da un tid inferiore al massimo numero di flag disponibili su 32bit
        if(now1000ms[0] & (BIT0<<from))
        {
            // sono passati i 10ms per il task di provenienza
            now1000ms[0] &= ~(BIT0<<from); // usato, cancello
            // DBG_PRINTF("tick 1000ms from:%d\n",from);
            return TRUE;
        }
    }
    else if (from<64)
    { // sono su un tid superiore ai primi 32bit disponibili, passo all'array successivo
        if(now1000ms[1] & (BIT0<<(from-32)))
        {
            now1000ms[1] &= ~(BIT0<<(from-32)); // usato, cancello
            // DBG_PRINTF("tick 1000ms from:%d\n",from);
            return TRUE;
        }
    }
    else
    {
        DBG_PRINTF("timerTick %d oltre i 64 disponibili. Verificare la provenienza\n",from);
    }
    
    return FALSE;
}


/*****************************************************************************\
*  Funzione:     SysTick_ISR                                                  *
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:                                                                     *
\*****************************************************************************/
// CY_ISR(SysTick_ISR)
void System1ms(void) { // CY_ISR(SysTick_ISR)   
    // a 1 ms 
	Pin_SPIF_Write(1);
    upCounter++;

    now1ms[0] = FLAG_TICK_TIME_SET;
    now1ms[1] = FLAG_TICK_TIME_SET;
		
    if((upCounter%10) == 0)
    { 
        // passati 10ms    dall'origine del mondo
        // spif_on();       

        // qui si potrebeb spostare anche la parte dei 10ms del ISR
        now10ms[0] = FLAG_TICK_TIME_SET;
        now10ms[1] = FLAG_TICK_TIME_SET;

        if(fifo[UART0].timeout)
        {
            fifo[UART0].timeout--;
            if(fifo[UART0].timeout == 0)
            {
                // DBG_PRINTF("UART0 error\n");
                fifo[UART0].in = 0;
            	fifo[UART0].out = 0;
            	fifo[UART0].rxStatus = PARSE_SOF;
            }
        }
    }
        
    if((upCounter%100) == 0)
    {
        // passati 100ms    dall'origine del mondo
        now100ms[0] = FLAG_TICK_TIME_SET;
        now100ms[1] = FLAG_TICK_TIME_SET;
	}

    if((upCounter%1000) == 0) 
    {
        // passato 1s       dall'origine del mondo
        now1000ms[0] = FLAG_TICK_TIME_SET;
        now1000ms[1] = FLAG_TICK_TIME_SET;
                
        upCounter = 0;  // resetta qui dentro dato che non serve piu di un secondo anche se sinceramente questo lo toglierei
    }
	Pin_SPIF_Write(0);
} // CY_ISR(SysTick_ISR)


/*****************************************************************************\
*  Funzione:     SysTick_Start()                                              *
*  Argomenti:    Nessuno                 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:         Abilita il systick interrupt a 1ms                          *
\*****************************************************************************/
void SysTick_Start(void)
{
    DBG_PRINTF("Start the SysTick timer\n");
    /* start the systick timer */
    CySysTickStart();
    CySysTickSetCallback(0, System1ms);
}



