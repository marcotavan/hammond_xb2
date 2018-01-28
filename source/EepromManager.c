/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-02-18
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
#include "common.h"
#include "EepromManager.h"
#include "tick.h"
#include "ButtonScanner.h"

// da rifare , meglio prendere aria
uint16 eeprom_timeout[5] = {0,0,0,0,0};

uint8 WriteDataToEeprom(uint8 type)
{
    // carica un timeout che scrive in eeprom a tempo perso dopo un po
    eeprom_timeout[type] = 20+(type*2);
    return TRUE;
}

uint8 scriviInEeprom(uint8 type)
{
    // la memoria [ disponibile?
    uint8 *ps;
    uint8 row;
    
    if (EEPROM_Query() == CYRET_STARTED) {
        // Checks the status of an earlier call to EEPROM_StartWrite()
        DBG_PRINTF("riprova la eeprom non e' disponibile\n");
        return 5; 
    }
    
    if (EEPROM_Query() == CYRET_SUCCESS) {
        DBG_PRINTF("scrivi in eeprom\n");
        
        // EEPROM_UpdateTemperature();

        switch(type)
        {
            case EEPROM_BUTTON: {
                row = EEPROM_ROW_BUTTONS;
                ps = (uint8 *) &switchType;
                EEPROM_StartWrite(ps, row);
            } break;
            
            case EEPROM_MIDI: {
                row = EEPROM_ROW_MIDI;
                ps = (uint8 *) &split;
                EEPROM_StartWrite(ps, row);
            } break;
            
            case EEPROM_PRESET: {
                row = 3;
                ps = (uint8 *) &switchType;
                EEPROM_StartWrite(ps, row);
            } break;
            
            case EEPROM_DRAWBARS: {
                row = 4;
                ps = (uint8 *) &switchType;
                EEPROM_StartWrite(ps, row);
            } break;
        }
    }

    return 0;
}

void eeprom_init(void)
{
    EEPROM_Enable();
    
    // fai partire un timeout di 10 minuti che fa una UpdateTemperature
    /*
        // punto alla riga della eeprom da caricare
        internal_eeprom.RegPointer = (reg8 *)(CYDEV_EE_BASE + (CYDEV_EEPROM_ROW_SIZE * EEP_ROW_CLOCK_DATA));  
            
        // punto all-inizio della struttura
        ps = (uint8 *) &clock; 
            
        // questo è la stessa cosa: faccio fare a lui
        memcpy(ps,(uint8 *) internal_eeprom.RegPointer,sizeof(clock));
    */
    
    if (EEPROM_ReadByte(0) != MARKER_EEPROM_DEFAULT)
    {
        EEPROM_EraseSector(0);
        EEPROM_EraseSector(1);
        
        EEPROM_WriteByte(MARKER_EEPROM_DEFAULT,0);
    }
}    


void EepromPoll(void)
{
    if(tick_100ms(TICK_EEPROM))
    {
        // fai qualcosa qua
        if(eeprom_timeout[EEPROM_BUTTON])
        {
            eeprom_timeout[EEPROM_BUTTON]--;
            if (eeprom_timeout[EEPROM_BUTTON] == 0)
            {
                DBG_PRINTF("scrivi EEPROM_BUTTON\n");
                eeprom_timeout[EEPROM_BUTTON] = scriviInEeprom(EEPROM_BUTTON);
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
				eeprom_timeout[EEPROM_MIDI] = scriviInEeprom(EEPROM_MIDI);
            }
        }
    }
}


void LoadMidiData(uint8 *pdata) {
	    // ricarica da eeprom se c'è il marker else prendi da default i pulsanti.
    internal_eeprom.RegPointer = (reg8 *)(CYDEV_EE_BASE + (CYDEV_EEPROM_ROW_SIZE * EEPROM_ROW_MIDI));  
        
    // punto all-inizio della struttura
    memcpy(pdata,(uint8 *) internal_eeprom.RegPointer,CYDEV_EEPROM_ROW_SIZE); 
}


void LoadSwitchData(uint8 *pdata) {
	    // ricarica da eeprom se c'è il marker else prendi da default i pulsanti.
    internal_eeprom.RegPointer = (reg8 *)(CYDEV_EE_BASE + (CYDEV_EEPROM_ROW_SIZE * EEPROM_ROW_BUTTONS));  
        
    // punto all-inizio della struttura
    memcpy(pdata,(uint8 *) internal_eeprom.RegPointer,CYDEV_EEPROM_ROW_SIZE); 
}

#if (1)
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
 #if (1)   
        switch (row)
        {
            case EEPROM_ROW_BUTTONS:
//            case EEP_ROW_BUTTON_DATA:
//            case EEP_ROW_CLOCK_DATA:
//            case EEP_ROW_BUTTON_DATA+MAX_CANALI:
//            case EEP_ROW_DIMMER_DATA+MAX_CANALI:
//            case EEP_ROW_DIMMER_DALCNET:
//            case EEP_ROW_SCENE:
//            case EEP_ROW_SCENE_SATELLITE:
//            case EEP_ROW_FREE:
            {
                DBG_PRINTF("-------------------------------------------------------\n");
            }
            break;
            
            default:
            break;
        }
 #endif // (0)          
        // testo in TEsta alle righe
        DBG_PRINTF("ROW>%02X| ", row);

        
        for(col = 0; col <= 0x0F; col++)
        {
           
            // DBG_PRINTF("%2X ", internal_eeprom.RegPointer[address]);
            DBG_PRINTF("%02X ", internal_eeprom.RegPointer[col]);

        }
        // testo in coda alle righe
        if (row == EEPROM_ROW_BUTTONS)                   {DBG_PRINTF("< switchType");} 
 #if (0)           
        
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
    #endif // (0)            
                                                        DBG_PRINTF("\n");

    }    

    DBG_PRINTF("*******************************************************\n");    

}
#endif
