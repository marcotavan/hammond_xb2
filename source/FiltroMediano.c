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

/*
come si usa:
    temperaturaLetta = FiltroMediano(iTemp);    //
    temperaturaLetta = movingAverageFilter(temperaturaLetta); 
*/

#include <project.h>
#include "debug.h"
#include "common.h"
#include "analog.h"

#define ORDINE_FILTRO		7 //solo numeri dispari a partire da 3

static uint32 filt_temp;
static uint8 indice_campioni,i,j;
static int32 Array_FILTERED[MAX_DRAWBAR_CHANNELS][ORDINE_FILTRO];
static int32 Array_RAW[MAX_DRAWBAR_CHANNELS][ORDINE_FILTRO];
static uint8 isMedianFilterInitialized = FALSE;

/* inizializza il filtro la prima volta */
static void FiltroMedianoInit(void)
{
    indice_campioni = 0;
    
    // memset(array, 0, sizeof(array[0][0]) * m * n);
    memset(Array_FILTERED,0,sizeof(Array_FILTERED[0][0])*ORDINE_FILTRO*MAX_DRAWBAR_CHANNELS);
    memset(Array_RAW,0,sizeof(Array_RAW[0][0])*ORDINE_FILTRO*MAX_DRAWBAR_CHANNELS);
}

/* gestisce il FiltroMediano */
int32 FiltroMediano(uint8 channel, int32 data)
{
    uint32 retVal = 0;
    
    if (isMedianFilterInitialized == FALSE)
    {
        // inizializza la prima volta che viene chiamato
        FiltroMedianoInit();
        isMedianFilterInitialized = TRUE;        
    }

	if(indice_campioni < ORDINE_FILTRO)
	{
        DBG_PRINTF("[%s]:chn %d velocizzo il filtro\n",__func__,channel);
		// questo va velocizzato un pochino...

		Array_RAW[channel][indice_campioni] = data;
		indice_campioni++;

		// all-inizio velocizzo un po'...
		if(indice_campioni < ORDINE_FILTRO)
		{
			Array_RAW[channel][indice_campioni] = data;
			indice_campioni++;
		}
		if(indice_campioni < ORDINE_FILTRO)
		{
			Array_RAW[channel][indice_campioni] = data;
			indice_campioni++;
		}
		if(indice_campioni < ORDINE_FILTRO)
		{
			Array_RAW[channel][indice_campioni] = data;
			indice_campioni++;
		}
        
        retVal = data;
	}
	else if(indice_campioni == ORDINE_FILTRO)
	{
        // DBG_PRINTF("[%s] ho raccolto tutti i dati, filtro\n",__func__);
        //coda FILO

		for(i=0; i<(ORDINE_FILTRO - 1); i++)
		{
			filt_temp = Array_RAW[channel][i+1];
			Array_RAW[channel][i] = filt_temp;
		}

		Array_RAW[channel][(ORDINE_FILTRO - 1)] = data;

		//copia array
		for(i=0;i<ORDINE_FILTRO;i++)
		{
			Array_FILTERED[channel][i] = Array_RAW[channel][i];
		}

		//ordinamento con bubble-sort
		//ordina un array dal dato più piccolo al più grande

		for(i=0; i<ORDINE_FILTRO; i++)
		{
			for(j=0; j<(ORDINE_FILTRO-1); j++)
			{
				if(Array_FILTERED[channel][j] > Array_FILTERED[channel][j+1])
				{
					filt_temp = Array_FILTERED[channel][j];
					Array_FILTERED[channel][j] = Array_FILTERED[channel][j+1];
					Array_FILTERED[channel][j+1] = filt_temp;
				}
			}
		}
        
        #if (0)
        for(i=0; i<ORDINE_FILTRO; i++)
        {
            DBG_PRINTF("%d ",Array_FILTERED[i]);
        }
        
        DBG_PRINTF("\n");
        #endif
            
		retVal =  (Array_FILTERED[channel][(ORDINE_FILTRO>>1)]);
	} //coda FILO
    
    return retVal;
}


/* movingAverageFilter */
int32 movingAverageFilter(int32 data) 
{
    #define NUM_CAMPIONI 16
    static int32 samples[NUM_CAMPIONI];    
    static int32 sum = 0;
    static uint8 index = 0;
    static uint8 isInitialized = FALSE;
    static uint8 validData = FALSE;
    uint32 retVal = data;
    
    if (isInitialized == FALSE) {
        index = 0;                      // parto dall-inizio
        sum = 0;                        // con nulla 
        memset(samples,0,NUM_CAMPIONI); 
        isInitialized = TRUE;
    }
                                    // prima di essere a regime ci vogliono NUM_CAMPIONI.
    sum -= samples[index];          // sottraggo dalla somma totale il dato presente all'indice attuale 
    sum += data;                    // sommo il nuovo dato al totale (al posto del precedente)
    samples[index] = data;          // inserisco nella posizione corrente il nuovo dato
    
    index++;                        // incremento al posizione corrente
    if (index == NUM_CAMPIONI) {    // se sono rrivato alla fine...
        index = 0;                  // ricomincio
        
        if (validData != TRUE){
            DBG_PRINTF("[%s]: Start Filtro media mobile\n",__func__);
            validData = TRUE;           // ma da adesso ho i dati validi 
        }
    }

    if (validData) {
        retVal = (sum >> 4); // quindi posso fare la media sui NUM_CAMPIONI 
    }
    
    return retVal;                  // restituisco
}


/* [] END OF FILE */
