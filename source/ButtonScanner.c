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


// attenzione che ho girato le linee per mantenere uniformit' con la tastiera

#include "project.h"
#include "ButtonScanner.h"
#include "common.h"
#include "debug.h"

#include "tick.h"

#define MIN_DEBOUNCE 5
#define MAX_DEBOUNCE 100
#define MAX_PULSANTI 16

enum {
    BUTTON_RELEASED,
    BUTTON_PRESSED,
    BUTTON_ON_HOLD,
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS
};

struct {
    uint8 debounce;
    uint8 status;
    uint8 oneShot;
} button[MAX_PULSANTI];

void ButtonManager(void)
{
    uint8 numTasto;
    for(numTasto=0;numTasto<MAX_PULSANTI;numTasto++)
    {
        switch(button[numTasto].status)
        {
            case BUTTON_PRESSED:
            {
                if (bitRead(button[numTasto].oneShot,0) == 0) {   
                    // esegui una sola volta
                    DBG_PRINTF("tasto %d appena premuto valido, con dito ancora sopra il pulsante\n",numTasto);
                    bitSet(button[numTasto].oneShot,0);
                }
            } break;
            
            case BUTTON_ON_HOLD:
            {
                if (bitRead(button[numTasto].oneShot,1) == 0) {
                    // esegui una sola volta
                    DBG_PRINTF("tasto %d tenuto premuto, con dito ancora sopra il pulsante\n",numTasto);
                    bitSet(button[numTasto].oneShot,1);
                }
            } break;
            
            case BUTTON_SHORT_PRESS:
            {
                DBG_PRINTF("tasto %d rilasciato da pressione breve\n",numTasto);
                button[numTasto].status = BUTTON_RELEASED;
                button[numTasto].oneShot = 0; // resetta il singolo sparo
            } break;
            
            case BUTTON_LONG_PRESS:
            {
                DBG_PRINTF("tasto %d rilasciato da pressione prolungata\n",numTasto);
                button[numTasto].status = BUTTON_RELEASED;
                button[numTasto].oneShot = 0; // resetta il singolo sparo
            } break;
                
            default:
                break;
        }
    }
}



void ButtonScanner(void)
{
    uint8 writeLine = 0;
    uint8 data = 0;
    uint8 readLine = 0;
    uint8 numTasto = 0;

    for(writeLine = 0;writeLine < 4;writeLine++)
    { // seleziona le linee dei banchi una alla volta col demultiplexer
        Control_Reg_Button_Line_Select_Write(writeLine); // Selects row 
        // DBG_PRINTF("sel line %02d ",line);
        data = ButtonInputPort_Read(); // leggi lo stato dei tasti 8 per volta
        
               // if (line == 0) DBG_PRINTF("var : %02x\n",var);
        for(readLine=0;readLine<4;readLine++)
        { // permette di decidere dove mettere il dato letto dalle 8 linee
  //          DBG_PRINTF("tasto %d campionato, %d %d %02x\n",numTasto,bank*2,bank*2+1,var);
            numTasto = readLine + (4*writeLine); // (0 1 2 3) + (4*0) | (4*1) | (4*2) | (4*3) 
            if (bitRead(data, readLine)) // tasto premuto
            {
                if (button[numTasto].debounce < MAX_DEBOUNCE) 
                {   // incremento ildebounce del tasto
                    button[numTasto].debounce++;
                    
                    if (button[numTasto].debounce > MIN_DEBOUNCE)
                    {
                        button[numTasto].status = BUTTON_PRESSED;
                    }
                }
                else
                {
                    // sono in pulsante premuto
                    button[numTasto].status = BUTTON_ON_HOLD;
                }
            }
            else // tasto rilasciato
            {
                switch(button[numTasto].status) 
                {
                    case BUTTON_PRESSED: // ho già passato il minimo debounce utile
                        button[numTasto].status = BUTTON_SHORT_PRESS;
                    break;
                    
                    case BUTTON_ON_HOLD:
                        button[numTasto].status = BUTTON_LONG_PRESS;
                    break;
                }
                    
                button[numTasto].debounce = 0;
                // button[numTasto].status = BUTTON_RELEASED;
            }
        }
    }
}

void ButtonScannerInit(void)
{
    uint8 i;
    
    buttonInputScan_S0_Write(0); // attiva PullUp
    buttonInputScan_S1_Write(0); // attiva PullUp
    buttonInputScan_S2_Write(0); // attiva PullUp
    buttonInputScan_S3_Write(0); // attiva PullUp

    for(i=0;i<MAX_PULSANTI;i++)
    {
        button[i].debounce = 0;
        button[i].status = BUTTON_RELEASED;
        button[i].oneShot = 0;
    }
}

void ButtonScannerPoll(void)
{
    // chiamata ogni 10ms
    static uint8 isButtonScannerInitialized = FALSE;
    
    if(isButtonScannerInitialized == FALSE) {
        ButtonScannerInit();
        isButtonScannerInitialized = TRUE;
    }
    
    ButtonScanner();
    ButtonManager();
}
/* [] END OF FILE */
