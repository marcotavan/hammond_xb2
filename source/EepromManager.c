/*******************************************************************************
* File Name: eeprom.c
*******************************************************************************/

#include <project.h>
#include "debug.h"
#include "common.h"
#include "EepromManager.h"
#include "tick.h"

// da rifare , meglio prendere aria
uint16 eeprom_timeout[5] = {0,0,0,0,0};

uint8 WriteDataToEeprom(uint8 type)
{
    // carica un timeout che scrive in eeprom a tempo perso dopo un po
    eeprom_timeout[type] = 20+(type*2);
    return TRUE;
}

/*
cystatus EEPROM_StartWrite(const uint8 *rowData, uint8 rowNumber)
Description: Starts the write of a row (16 bytes) of data to the EEPROM. This function does not block. The
function returns once the SPC has begun writing the data. This function must be used in
combination with EEPROM_Query(). EEPROM_Query() must be called until it returns a
status other than CYRET_STARTED. That indicates the write has completed. Until
EEPROM_Query() detects that the write is complete the SPC is marked as locked to prevent
another SPC operation from being performed. For reliable write procedure to occur you
should call EEPROM_UpdateTemperature() API if the temperature of the silicon has
changed for more than 10°C since component was started.
Parameters: rowData: Address of the data to write to the EEPROM
rowNumber: Row number to write
Return Value: Value Description
CYRET_SUCCESS Successful completion.
CYRET_BAD_PARAM Row number or byte number out.
CYRET_LOCKED SPC locked by another operation.
CYRET_UNKNOWN Other error from the SPC.
Side Effects: After calling this API, the device should not be powered down, reset, or switched to low
power mode until the EEPROM operation is complete. Not following this recommendation
may lead to data corruption or silicon unexpected behavior.

cystatus EEPROM_Query(void)
Description: Checks the status of an earlier call to EEPROM_StartWrite() or EEPROM_StartErase(). This
function must be called until it returns a value other than CYRET_STARTED. Once that
occurs the write has been completed and the SPC is unlocked.
Parameters: None
Return Value: Value Description
CYRET_SUCCESS The operation completed successfully.
CYRET_STARTED The SPC command is still being executed.
CYRET_UNKNOWN Other error from the SPC.

*/

void EepromPoll(void)
{
    static uint8 isEepromInitialized = FALSE;
    
    if(isEepromInitialized == FALSE)
    {
        EEPROM_Enable();
        
        // if marker not present
        // EEPROM_EraseSector
        isEepromInitialized = TRUE;
    }
    
    if(tick_100ms(TICK_EEPROM))
    {
        // fai qualcosa qua
        if(eeprom_timeout[EEPROM_BUTTON])
        {
            eeprom_timeout[EEPROM_BUTTON]--;
            if (eeprom_timeout[EEPROM_BUTTON] == 0)
            {
                DBG_PRINTF("scrivi EEPROM_BUTTON\n");
            }
        }
        else if(eeprom_timeout[EEPROM_DRAWBARS])
        {
            eeprom_timeout[EEPROM_DRAWBARS]--;
            if (eeprom_timeout[EEPROM_DRAWBARS] == 0)
            {
                DBG_PRINTF("scrivi EEPROM_DRAWBARS\n");
            }
        }
        if(eeprom_timeout[EEPROM_PRESET])
        {
            eeprom_timeout[EEPROM_PRESET]--;
            if (eeprom_timeout[EEPROM_PRESET] == 0)
            {
                DBG_PRINTF("scrivi EEPROM_PRESET\n");
            }
        }
        if(eeprom_timeout[EEPROM_MIDI])
        {
            eeprom_timeout[EEPROM_MIDI]--;
            if (eeprom_timeout[EEPROM_MIDI] == 0)
            {
                DBG_PRINTF("scrivi EEPROM_MIDI\n");
            }
        }
    }
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