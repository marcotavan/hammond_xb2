/*******************************************************************************
* File Name: eeprom.c
*******************************************************************************/

#include <project.h>
#include "debug.h"
#include "common.h"
#include "EepromManager.h"

uint8 WriteDataToEeprom(uint8 type)
{
    // carica un timeout che scrive in eeprom a tempo perso dopo un po
    type = type;
    
    return TRUE;
}


#if (0)
/*****************************************************************************\
*  Funzione:     internal_eeprom_inspector(uint16 start_row, uint16 last_row) *
*  Argomenti:    start_row,       last_row 				      				  *
*  Restituisce:  Nessuno                                                      *
*  Scopo:   visualizza su UART                                                                   *
\*****************************************************************************/
void internal_eeprom_inspector(uint16 start_row, uint16 last_row)
{
    // scrive fuori la struttura della memoria   
    
    uint16 row;
    uint16 col;
    
    DBG_PRINTF("internal eeprom inspector. START FROM:%02X TO:%02X\n",  start_row,last_row);
    DBG_PRINTF("*******************************************************\n"); 
    DBG_PRINTF("  COL | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
    DBG_PRINTF("-------------------------------------------------------\n");    
    for(row = start_row; row<last_row; row++)
    {
        internal_eeprom.RegPointer = (reg8 *)(CYDEV_EE_BASE + (CYDEV_EEPROM_ROW_SIZE * row));     //punta all'inizio della eeprom
    
        switch (row)
        {
            case EEPROM_POSITION_0:
            case EEP_ROW_BUTTON_DATA:
            case EEP_ROW_CLOCK_DATA:
            case EEP_ROW_BUTTON_DATA+MAX_CANALI:
            case EEP_ROW_DIMMER_DATA+MAX_CANALI:
            case EEP_ROW_DIMMER_DALCNET:
            case EEP_ROW_SCENE:
            case EEP_ROW_SCENE_SATELLITE:
            case EEP_ROW_FREE:
            {
                DBG_PRINTF("-------------------------------------------------------\n");
            }
            break;
            
            default:
            break;
        }
        
        // testo in TEsta alle righe
        DBG_PRINTF("ROW>%02X| ", row);

        
        for(col = 0; col <= 0x0F; col++)
        {
           
            // DBG_PRINTF("%2X ", internal_eeprom.RegPointer[address]);
            DBG_PRINTF("%02X ", internal_eeprom.RegPointer[col]);

        }
        
        // testo in coda alle righe
        if (row == EEPROM_POSITION_0)                   {DBG_PRINTF("< MARKER");} 
        if (row == EEP_ROW_STATUS_DATA)                 {DBG_PRINTF("< STATUS");} 
        if (row == EEP_ROW_COUNTER_RESTART)             {DBG_PRINTF("< RESTART CNT");} 
        
        if (row == EEP_NODENAMING_LOCATION_STATUS)      {DBG_PRINTF("< EEP_NODENAMING_LOCATION_STATUS");}
        if (row == EEP_NODENAMING_ROWDATA)              {DBG_PRINTF("< EEP_NODENAMING_ROWDATA");} 
        if (row == EEP_NODELOCATION_ROWDATA)            {DBG_PRINTF("< EEP_NODELOCATION_ROWDATA");} 
        
        if (row == EEP_ROW_DEBOUNCE_BUTTON_DATA)        {DBG_PRINTF("< DEBOUNCE GLOBAL");} 
        if (row == EEP_ROW_BUTTON_DATA)                 {DBG_PRINTF("< BUTTON 0 CFG");}      
        if (row == EEP_ROW_BUTTON_DATA+1)               {DBG_PRINTF("< BUTTON 1 CFG");}      
        if (row == EEP_ROW_BUTTON_DATA+2)               {DBG_PRINTF("< BUTTON 2 CFG");}      
        if (row == EEP_ROW_BUTTON_DATA+3)               {DBG_PRINTF("< BUTTON 3 CFG");}      
        if (row == EEP_ROW_BUTTON_DATA+4)               {DBG_PRINTF("< BUTTON 4 CFG");}      
        if (row == EEP_ROW_BUTTON_DATA+5)               {DBG_PRINTF("< BUTTON 5 CFG");}      
        
        if (row == EEP_ROW_CLOCK_DATA)                  {DBG_PRINTF("< CLOCK");}  
        if (row == (EEP_ROW_BUTTON_DATA+MAX_CANALI))    {DBG_PRINTF("< DIMMER 0 CFG");}     
        if (row == (EEP_ROW_BUTTON_DATA+MAX_CANALI+1))  {DBG_PRINTF("< DIMMER 1 CFG");}     
        if (row == (EEP_ROW_BUTTON_DATA+MAX_CANALI+2))  {DBG_PRINTF("< DIMMER 2 CFG");}     
        if (row == (EEP_ROW_BUTTON_DATA+MAX_CANALI+3))  {DBG_PRINTF("< DIMMER 3 CFG");}     
        if (row == (EEP_ROW_BUTTON_DATA+MAX_CANALI+4))  {DBG_PRINTF("< DIMMER 4 CFG");}     
        if (row == (EEP_ROW_BUTTON_DATA+MAX_CANALI+5))  {DBG_PRINTF("< DIMMER 5 CFG");}     
        
        if (row == EEP_ROW_MOTOR_TIMER_DATA)            {DBG_PRINTF("< MOTOR CONTROL TIMER 2B 2B 2B");} 
        if (row == EEP_ROW_DIMMER_DALCNET)              {DBG_PRINTF("< DIMMER_DALCNET 0 CFG");} 
        if (row == (EEP_ROW_DIMMER_DALCNET+1))          {DBG_PRINTF("< DIMMER_DALCNET 1 CFG");} 
        if (row == (EEP_ROW_DIMMER_DALCNET+2))          {DBG_PRINTF("< DIMMER_DALCNET 2 CFG");} 
        if (row == (EEP_ROW_DIMMER_DALCNET+3))          {DBG_PRINTF("< DIMMER_DALCNET 3 CFG");} 
        
        for(col = 0; col < MAX_SCENE; col++)  // cicla per 10 scene
        {
            if (row == (EEP_ROW_SCENE+col))             {DBG_PRINTF("< SCENE %d",col+1);}    
        }
        
        for(col = 0; col < MAX_CANALI; col++)  // cicla per 10 scene
        {
            if (row == (EEP_ROW_SCENE_SATELLITE+col))   {DBG_PRINTF("< SCENE SAT %d",col);}    
        }
        
        if (row == (EEP_ROW_FREE))                      {DBG_PRINTF("< free");}    

        // DBG_PRINTF("%2X ", internal_eeprom.RegPointer[address]);
        
                                                        DBG_PRINTF("\n");
    }    
    
    DBG_PRINTF("*******************************************************\n");    

}
#endif