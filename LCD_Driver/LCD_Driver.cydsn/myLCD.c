/* ========================================
 *
 * Copyright Marco Tavan, 2017
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "CyLib.h"
#include "myLCD.h"
#include "project.h"
#include "debug.h"
#include "tick.h"
#include "defs.h"


static void myLCD_WrDatNib(uint8 nibble) ;
static void myLCD_WrCntrlNib(uint8 nibble) ;
static void myLCD_WrCntrl8bit(uint8 data); 
static void myLCD_WrDat8bit(uint8 data) ;

#define Interface_8bit 1

/* Stores state of component. Indicates whether component is or not in enable state. */
uint8 myLCD_enableState = 0u;
uint8 myLCD_initVar = 0u;

#define LCD_READY_BIT	BIT7

uint8 const CYCODE LCD_InvertedVerticalBar[] = \
{
    /* Character LCD_CUSTOM_0   */
        0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_1   */
        0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_2   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_3   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_4   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_5   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u,    0x00u, \
    /* Character LCD_CUSTOM_6   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x00u, \
    /* Character LCD_CUSTOM_7   */
        0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu \
 };


uint8 const CYCODE LCD_VerticalBar[] = \
{
    /* Character LCD_CUSTOM_0   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu, \
    /* Character LCD_CUSTOM_1   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_2   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_3   */
    0x00u,    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_4   */
    0x00u,    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_5   */
    0x00u,    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_6   */
    0x00u,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu, \
    /* Character LCD_CUSTOM_7   */
    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu,    0x1Fu \
};


/*******************************************************************************
* Function Name: myLCD_LoadCustomFonts
********************************************************************************
*
* Summary:
*  Loads 8 custom font characters into the LCD Module for use.  Cannot use
*  characters from two different font sets at once, but font sets can be
*  switched out during runtime.
*
* Parameters:
*  customData:  pointer to a constant array of 64 bytes representing 8 custom
*               font characters.
* Return:
*  None.
*
* Theory:
*  Prior to using this function user need to import the pointer to custom
*  font array to your project by writting the following in the source code file
*  where custom font will be used:
*       extern uint8 const CYCODE LCD_Char_customFonts[];
*  This function is not automatically called by the Start() routine and must be
*  called manually by the user.
*******************************************************************************/
void myLCD_LoadCustomFonts(uint8 const customData[]) 
{
    uint8 indexU8;

    myLCD_IsReady();
    /* Set starting address in the LCD Module */
    /* Optionally: Read the current address to restore at a later time */
    myLCD_WriteControl(myLCD_CGRAM_0);

    /* Load in the 64 bytes of CustomChar Data */
    for(indexU8 = 0u; indexU8 < myLCD_CUSTOM_CHAR_SET_LEN; indexU8++)
    {
        /* Delay between each write */
        myLCD_WriteData(customData[indexU8]);
    }

    myLCD_IsReady();
    myLCD_WriteControl(myLCD_DDRAM_0);
}


/*******************************************************************************
* Function Name: myLCD_Init
********************************************************************************
*
* Summary:
*  Performs initialization required for the components normal work.
*  This function initializes the LCD hardware module as follows:
*        Enables a 4-bit interface
*        Clears the display
*        Enables the auto cursor increment
*        Resets the cursor to start position
*  Also, it loads a custom character set to the LCD if it was defined in the customizer.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myLCD_Init(void) 
{
    DBG_PRINTF("[%s]: INIT CODE...\n",__func__);
    CyDelay(40u);                                                        /* Delay 40 ms */
    myLCD_WriteControl(myLCD_DISPLAY_8_BIT_INIT);    /* Selects 8-bit mode */
    CyDelay(5u);                                                         /* Delay 5 ms */
    myLCD_WriteControl(myLCD_DISPLAY_8_BIT_INIT);    /* Selects 8-bit mode */
    CyDelay(15u);                                                        /* Delay 15 ms */
    myLCD_WriteControl(myLCD_DISPLAY_8_BIT_INIT);    /* Selects 8-bit mode */
    CyDelay(1u);                                                         /* Delay 1 ms */
	#if Interface_8bit
		myLCD_WriteControl(myLCD_DISPLAY_8B_2_LINES_5x10);		/* Selects 8-bit mode */
		// myLCD_WriteControl(0x38);		/* Selects 8-bit mode */
	#else
		myLCD_WriteControl(myLCD_DISPLAY_4B_2_LINES_5x10);    	/* Selects 4-bit mode */
	#endif
    CyDelay(5u);                                                         /* Delay 5 ms */

 	myLCD_WriteControl(myLCD_CLEAR_DISPLAY);          /* Clear LCD Screen */   
	myLCD_WriteControl(myLCD_DISPLAY_ON_CURSOR_OFF);  /* Turn Display ON, Cursor OFF */	
	myLCD_WriteControl(myLCD_CURSOR_AUTO_INCR_ON);    /* Incr Cursor After Writes */
	
	#if 0
	myLCD_WriteControl(myLCD_DISPLAY_CURSOR_ON);      /* Turn Display, Cursor ON */
	myLCD_WriteControl(myLCD_RESET_CURSOR_POSITION);  /* Set Cursor to 0,0 */
	#endif    

	CyDelay(5u);
    #if(myLCD_CUSTOM_CHAR_SET != myLCD_NONE)
        myLCD_LoadCustomFonts(myLCD_customFonts);
    #endif /* myLCD_CUSTOM_CHAR_SET != myLCD_NONE */
	
	myLCD_LoadCustomFonts(LCD_InvertedVerticalBar);	
}


void Pulse_E(void)
{
    /* Write control data and set enable signal */
	LCD_Pin_E_Write(1);	
	// LCD_Port_5_Write(nibble<<myLCD_NIBBLE_SHIFT);
	
    /* Minimum of 230 ns delay */
    CyDelayUs(1u);

    LCD_Pin_E_Write(0);
}

/*******************************************************************************
* Function Name: myLCD_Enable
********************************************************************************
*
* Summary:
*  Turns on the display.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
* Theory:
*  This function has no effect when it is called the first time as
*  myLCD_Init() turns on the LCD.
*
*******************************************************************************/
void myLCD_Enable(void) 
{
    myLCD_DisplayOn();
    myLCD_enableState = 1u;
}


/*******************************************************************************
* Function Name: myLCD_Start
********************************************************************************
*
* Summary:
*  Performs initialization required for the components normal work.
*  This function initializes the LCD hardware module as follows:
*        Enables 4-bit interface
*        Clears the display
*        Enables auto cursor increment
*        Resets the cursor to start position
*  Also, it loads a custom character set to the LCD if it was defined in the customizer.
*  If it was not the first call in this project, then it just turns on the
*  display
*
*
* Parameters:
*  myLCD_initVar - global variable.
*
* Return:
*  myLCD_initVar - global variable.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myLCD_Start(void) 
{
    /* If not initialized, perform initialization */
    if(myLCD_initVar == 0u)
    {
        myLCD_Init();
        myLCD_initVar = 1u;
    }

    /* Turn on the LCD */
    myLCD_Enable();
}


/*******************************************************************************
* Function Name: myLCD_Stop
********************************************************************************
*
* Summary:
*  Turns off the display of the LCD screen.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void myLCD_Stop(void) 
{
    /* Calls LCD Off Macro */
    myLCD_DisplayOff();
    myLCD_enableState = 0u;
}


/*******************************************************************************
*  Function Name: myLCD_Position
********************************************************************************
*
* Summary:
*  Moves the active cursor location to a point specified by the input arguments
*
* Parameters:
*  row:    Specific row of LCD module to be written
*  column: Column of LCD module to be written
*
* Return:
*  None.
*
* Note:
*  This only applies for LCD displays that use the 2X40 address mode.
*  In this case Row 2 starts with a 0x28 offset from Row 1.
*  When there are more than 2 rows, each row must be fewer than 20 characters.
*
*******************************************************************************/
void myLCD_Position(uint8 row, uint8 column) 
{
	// 0x80 = BIT7 = Set DDRAM Address in address counter
 	switch (row)
    {
        case (uint8)0:
            myLCD_WriteControl(myLCD_DDRAM_0|(myLCD_ROW_0_START + column));
            break;
        case (uint8) 1:
            myLCD_WriteControl(myLCD_DDRAM_0|(myLCD_ROW_1_START + column));
            break;
        case (uint8) 2:
            myLCD_WriteControl(myLCD_DDRAM_0|(myLCD_ROW_2_START + column));
            break;
        case (uint8) 3:
            myLCD_WriteControl(myLCD_DDRAM_0|(myLCD_ROW_3_START + column));
            break;
        default:
            /* if default case is hit, invalid row argument was passed.*/
            break;
    }
}


/*******************************************************************************
* Function Name: myLCD_PrintString
********************************************************************************
*
* Summary:
*  Writes a zero terminated string to the LCD.
*
* Parameters:
*  string: Pointer to head of char8 array to be written to the LCD module
*
* Return:
*  None.
*
*******************************************************************************/
void myLCD_PrintString(char8 const string[]) 
{
    uint8 indexU8 = 1u;
    char8 current = *string;

    /* Until null is reached, print next character */
    while((char8) '\0' != current)
    {
        myLCD_WriteData((uint8)current);
        current = string[indexU8];
        indexU8++;
    }
}




/*******************************************************************************
* Function Name: myLCD_WriteDisplayLcd
********************************************************************************
*
* Summary:
*  Writes to the LCD.
*
* Parameters:
*  string: Pointer to head of char8 array to be written to the LCD module
*
* Return:
*  None.
*
*******************************************************************************/
void myLCD_WriteDisplayLcd(uint8 *data, uint8 len) 
{
    uint8 i;

    for(i=0; i<len; i++) {
        myLCD_WriteData(data[i]);
    }
}
/*******************************************************************************
*  Function Name: myLCD_PutChar
********************************************************************************
*
* Summary:
*  Writes a single character to the current cursor position of the LCD module.
*  Custom character names (_CUSTOM_0 through
*  _CUSTOM_7) are acceptable as inputs.
*
* Parameters:
*  character: Character to be written to LCD
*
* Return:
*  None.
*
*******************************************************************************/
void myLCD_PutChar(char8 character) 
{
    myLCD_WriteData((uint8)character);
}


/*******************************************************************************
*  Function Name: myLCD_WriteData
********************************************************************************
*
* Summary:
*  Writes a data byte to the LCD module's Data Display RAM.
*
* Parameters:
*  dByte: Byte to be written to the LCD module
*
* Return:
*  None.
*
*******************************************************************************/
void myLCD_WriteData(uint8 dByte) 
{
	myLCD_IsReady();	
	
	#if Interface_8bit
	myLCD_WrDat8bit(dByte);
	#else
    uint8 nibble;

    nibble = dByte >> myLCD_NIBBLE_SHIFT;

    /* Write high nibble */
    myLCD_WrDatNib(nibble);

    nibble = dByte & myLCD_NIBBLE_MASK;
    /* Write low nibble */
    myLCD_WrDatNib(nibble);
	#endif
}


/*******************************************************************************
*  Function Name: myLCD_WriteControl
********************************************************************************
*
* Summary:
*  Writes a command byte to the LCD module.
*
* Parameters:
*  cByte:  The byte to be written to theLCD module
* 
* Return:
*  None.
*
*******************************************************************************/
void myLCD_WriteControl(uint8 cByte) 
{
    myLCD_IsReady();

	#if(Interface_8bit) 
		myLCD_WrCntrl8bit(cByte);
	#else	
		uint8 nibble;
	    nibble = cByte >> myLCD_NIBBLE_SHIFT;
	    
	    /* WrCntrlNib(High Nibble) */
	    myLCD_WrCntrlNib(nibble);
	    nibble = cByte & myLCD_NIBBLE_MASK;

	    /* WrCntrlNib(Low Nibble) */
	    myLCD_WrCntrlNib(nibble);
	#endif
}

/*******************************************************************************
* Function Name: myLCD_IsReady
********************************************************************************
*
* Summary:
*  Polls the LCD until the ready bit is set or a timeout occurs.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Note:
*  Changes the pins to High-Z.
*
*******************************************************************************/
void myLCD_IsReady(void) 
{
	#if (1)
    uint8 value;
    uint32 timeout;
    timeout = myLCD_READY_DELAY;
	
	/* Clear LCD port*/
    // LCD_Port_5_Write(0);
	Port_LCD_Write(0);
	
    /* Change Port to High-Z Status on data pins */
	Port_LCD_SetDriveMode(PIN_DM_DIG_HIZ);
    
    /* Make sure RS is low */
    LCD_Pin_RS_Write(0);

    /* Set R/W high to read */
    LCD_Pin_RW_Write(1);

    do
    {
        LCD_Pin_E_Write(0);
		
        /* 40 ns delay required before rising Enable and 500ns between neighbour Enables */
        CyDelayUs(0u);

        /* Set E high */
        LCD_Pin_E_Write(1);

        /* 360 ns delay setup time for data pins */
        CyDelayUs(1u);

        /* Get port state */
        // value = LCD_Port_5_Read();
		value = Port_LCD_Read();
		
        /* This gives true delay between disabling Enable bit and polling Ready bit */
        CyDelayUs(0u);
		// DBG_PRINTF("value %2X\n",value);
		
        /* Extract ready bit */
        value &= LCD_READY_BIT;
		
        /* If LCD is not ready make a delay */
        if (value != 0u) {
			// DBG_PRINTF("LCD Not READY value %2X\n",value);
            CyDelayUs(100u);
        }
		
		#if !Interface_8bit
		LCD_Pin_E_Write(0);
		
        /* 40 ns delay required before rising Enable and 500ns between neighbour Enables */
        CyDelayUs(0u);

        /* Set E high */
        LCD_Pin_E_Write(1);

        /* 360 ns delay setup time for data pins */
        CyDelayUs(1u);
		#endif
		
        /* Repeat until bit 4 is not zero or until timeout. */
        timeout--;
		
    } while ((value != 0u) /*&& (timeout > 0u)*/);

    /* Set R/W low to write */
    LCD_Pin_RW_Write(0);

    /* Clear LCD port*/
    // LCD_Port_5_Write(0);
	Port_LCD_Write(0);
	
    /* Change Port to Output (Strong) on data pins */
	Port_LCD_SetDriveMode(PIN_DM_STRONG);
	
	// DBG_PRINTF("[%s]\n",__func__);
	#else
		CyDelayUs(100);
	#endif
	
}


/*******************************************************************************
*  Function Name: myLCD_WrDatNib
********************************************************************************
*
* Summary:
*  Writes a data nibble to the LCD module.
*
* Parameters:
*  nibble:  Byte containing nibble in the least significant nibble to be
*           written to the LCD module.
*
* Return:
*  None.
*
*******************************************************************************/
static void myLCD_WrDatNib(uint8 nibble) 
{
    /* RS should be high to select data register */
    LCD_Pin_RS_Write(1);
	
    /* Reset RW for write operation */
    LCD_Pin_RW_Write(0);

    /* Guaranteed delay between Setting RS and RW and setting E bits */
    CyDelayUs(0u);
    
    /* Clear data pins */
    // LCD_Port_5_Write(0);
	// Control_Reg_LCD_Write(0);
	
	Port_LCD_Write(nibble<<myLCD_NIBBLE_SHIFT);
    /* Write data, bring E high */
	//DBG_PRINTF("%s: %02X - %02X - %02X\n",__func__,nibble,Control_Reg_LCD_Read(),Status_Reg_LCD_Read());
	Pulse_E();
}

static void myLCD_WrDat8bit(uint8 data) 
{
    /* RS should be high to select data register */
    LCD_Pin_RS_Write(1);
	
    /* Reset RW for write operation */
    LCD_Pin_RW_Write(0);

    /* Guaranteed delay between Setting RS and RW and setting E bits */
    CyDelayUs(0u);
    
    /* Clear data pins */
    // LCD_Port_5_Write(0);
	// Control_Reg_LCD_Write(0);
	
	Port_LCD_Write(data);
	//DBG_PRINTF("%s: %02X - %02X - %02X\n",__func__,data,Control_Reg_LCD_Read(),Status_Reg_LCD_Read());
    /* Write data, bring E high */
	Pulse_E();
}


/*******************************************************************************
*  Function Name: myLCD_WrCntrlNib
********************************************************************************
*
* Summary:
*  Writes a control nibble to the LCD module.
*
* Parameters:
*  nibble: The byte containing a nibble in the four least significant bits.????
*
* Return:
*  None.
*
*******************************************************************************/
static void myLCD_WrCntrlNib(uint8 nibble) 
{
    /* RS and RW should be low to select instruction register and  write operation respectively */
    LCD_Pin_RS_Write(0);
	
    /* Reset RW for write operation */
    LCD_Pin_RW_Write(0);

	Port_LCD_Write(nibble << myLCD_NIBBLE_SHIFT);
	
	//DBG_PRINTF("%s: %02X - %02X - %02X\n",__func__,nibble,Control_Reg_LCD_Read(),Status_Reg_LCD_Read());
	
	Pulse_E();
}

static void myLCD_WrCntrl8bit(uint8 data) 
{
    /* RS and RW should be low to select instruction register and  write operation respectively */
    LCD_Pin_RS_Write(0);
	
    /* Reset RW for write operation */
    LCD_Pin_RW_Write(0);

	Port_LCD_Write(data);
	
	//DBG_PRINTF("%s: %02X - %02X - %02X\n",__func__,data,Control_Reg_LCD_Read(),Status_Reg_LCD_Read());

	Pulse_E();
}


#if(myLCD_CONVERSION_ROUTINES == 1u)

    /*******************************************************************************
    *  Function Name: myLCD_PrintInt8
    ********************************************************************************
    *
    * Summary:
    *  Print a byte as two ASCII characters.
    *
    * Parameters:
    *  value: The byte to be printed out as ASCII characters.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void myLCD_PrintInt8(uint8 value) 
    {
        static char8 const CYCODE myLCD_hex[16u] = "0123456789ABCDEF";
        
        myLCD_PutChar((char8) myLCD_hex[value >> myLCD_BYTE_UPPER_NIBBLE_SHIFT]);
        myLCD_PutChar((char8) myLCD_hex[value & myLCD_BYTE_LOWER_NIBBLE_MASK]);
    }


    /*******************************************************************************
    *  Function Name: myLCD_PrintInt16
    ********************************************************************************
    *
    * Summary:
    *  Print a uint16 as four ASCII characters.
    *
    * Parameters:
    *  value: The uint16 to be printed out as ASCII characters.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void myLCD_PrintInt16(uint16 value) 
    {
        myLCD_PrintInt8((uint8)(value >> myLCD_U16_UPPER_BYTE_SHIFT));
        myLCD_PrintInt8((uint8)(value & myLCD_U16_LOWER_BYTE_MASK));
    }

    
    /*******************************************************************************
    *  Function Name: myLCD_PrintInt32
    ********************************************************************************
    *
    * Summary:
    *  Print a uint32 hexadecimal number as eight ASCII characters.
    *
    * Parameters:
    *  value: The uint32 to be printed out as ASCII characters.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void myLCD_PrintInt32(uint32 value) 
    {
        uint8 shift = myLCD_32_BIT_SHIFT;
        
        while (shift != 0u)
        {
            /* "shift" var is to be subtracted by 8 prior shifting. This implements 
            * shifting by 24, 16, 8 and 0u. 
            */
            shift -= myLCD_8_BIT_SHIFT;
            
            /* Print 8 bits of uint32 hex number */
            myLCD_PrintInt8((uint8) ((uint32) (value >> shift)));
        }
    }
    
    
    /*******************************************************************************
    *  Function Name: myLCD_PrintNumber
    ********************************************************************************
    *
    * Summary:
    *  Print an uint16 value as a left-justified decimal value.
    *
    * Parameters:
    *  value: A 16-bit value to be printed in ASCII characters as a decimal number
    *
    * Return:
    *  None.
    *
    * Note:
    *  This function is implemented as a macro.
    *
    *******************************************************************************/

    
    /*******************************************************************************
    *  Function Name: myLCD_PrintU32Number
    ********************************************************************************
    *
    * Summary:
    *  Print an uint32 value as a left-justified decimal value.
    *
    * Parameters:
    *  value: A 32-bit value to be printed in ASCII characters as a decimal number
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void myLCD_PrintU32Number(uint32 value) 
    {
        uint8 tmpDigit;
        char8 number[myLCD_NUMBER_OF_REMAINDERS_U32 + 1u];
        uint8 digIndex = myLCD_NUMBER_OF_REMAINDERS_U32;
        
        /* This API will output a decimal number as a string and that string will be 
        * filled from end to start. Set Null termination character first.
        */
        number[digIndex] = (char8) '\0';
        digIndex--;
        
        /* Load these in reverse order */
        while(value >= myLCD_TEN)
        {
            /* Extract decimal digit, indexed by 'digIndex', from 'value' and
            * convert it to ASCII character.
            */
            tmpDigit = (uint8) (((uint8) (value % myLCD_TEN)) + (uint8) myLCD_ZERO_CHAR_ASCII);
            
            /* Temporary variable 'tmpDigit' is used to avoid Violation of MISRA rule 
            * #10.3.
            */
            number[digIndex] = (char8) tmpDigit;
            value /= myLCD_TEN;
            digIndex--;
        }
        
        /* Extract last decimal digit 'digIndex', from the 'value' and convert it
        * to ASCII character.
        */
        tmpDigit = (uint8) (((uint8)(value % myLCD_TEN)) + (uint8) myLCD_ZERO_CHAR_ASCII);
        number[digIndex] = (char8) tmpDigit;

        /* Print out number */
        myLCD_PrintString(&number[digIndex]);
    }
    
#endif /* myLCD_CONVERSION_ROUTINES == 1u */







#if (0)

    /*******************************************************************************
    *  Function Name: LCD_DrawVerticalBG
    ********************************************************************************
    *
    * Summary:
    *  Draws the vertical bargraph.
    *
    * Parameters:
    *  row:            The row in which the bar graph starts.
    *  column:         The column in which the bar graph starts.
    *  maxCharacters:  The max height of the graph in whole characters.
    *  value:          The current length or height of the graph in pixels.
    *
    * Return:
    *  void.
    *
    *******************************************************************************/
    void myLCD_DrawVerticalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value) \
                                                            
    {
        /* 8-bit Counter */
        uint8 count8 = 0u;
        /* Current Row Tracker */
        int8 currentRow;
        uint8 fullChars;
        uint8 remainingPixels;
		
		DBG_PRINTF("%d,%d,%d,%d ",row,column,maxCharacters,value);

        /* Number of full characters to draw */
        fullChars = value / myLCD_CHARACTER_HEIGHT;

        /* Number of remaining pixels to draw */
        remainingPixels = value % myLCD_CHARACTER_HEIGHT;

        /* Put Cursor at start position */
        myLCD_Position(row, column);

        /* Make sure the bar graph fits inside the space allotted */
        if(fullChars >= maxCharacters)
        {
            fullChars = maxCharacters;
        }

        /*  Write full characters */
        while(count8 < fullChars)
        {
            myLCD_WriteData(myLCD_CUSTOM_0);

            count8++;

            /* Each pass through, move one row higher */
            if((((int8) row) - ((int8) count8)) >= 0)
            {
                myLCD_Position(row - count8, column);
            }
            else
            {
                break;
            }
        }

        if(((((int8) row) - ((int8) count8)) >= 0) && (fullChars < maxCharacters))
        {
            /* Write remaining pixels */
            if(remainingPixels == 0u)
            {
                myLCD_WriteData((uint8) ' ');
            }
            else
            {
                myLCD_WriteData(remainingPixels - 1u);
            }

            currentRow = ((int8) row) - ((int8) count8) - 1;

            if(currentRow >= 0)
            {
                /* Move up one row and fill with whitespace till top of bar graph */
                for(count8 = 0u; count8 < (maxCharacters - fullChars - 1u); count8++)
                {
                    myLCD_Position((uint8)currentRow, column);
                    myLCD_WriteData((uint8) ' ');
                    currentRow --;
                }
            }
        }
    }

#endif /* LCD_CUSTOM_CHAR_SET == LCD_VERTICAL_BG */



// char str_bargraph[MAX_CHARS]; // contiene le barre

void Write_BarGraphs(uint8 row, uint8 *str_bargraph) {
    uint8 i = 0;
    static uint16 cnt = 0;
	static char prev_bargraph[MAX_CHARS] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    // if (alternateTextCounter) return;  // non scrive niente
	
    // myLCD_LoadCustomFonts(LCD_InvertedVerticalBar);	
    for (i=0;i<8;i++) 
	{
		if(prev_bargraph[i] != str_bargraph[i]) {
			prev_bargraph[i] = str_bargraph[i];
			str_bargraph[i]%=8;
			
			
			// myLCD_Position(0,i);
			// myLCD_PrintNumber(str_bargraph[i]);
			myLCD_Position(1,i);
			myLCD_WriteData(myLCD_CUSTOM_0+str_bargraph[i]);
			
        	// myLCD_DrawVerticalBG(row, i, 8,str_bargraph[i]);
			// DBG_PRINTF("riscrivo barre %d\n",i);
		}
    }
	
	// myLCD_LoadCustomFonts(LCD_VerticalBar);	
	for (i=8;i<16;i++) 
	{
		if(prev_bargraph[i] != str_bargraph[i]) {
			prev_bargraph[i] = str_bargraph[i];
			str_bargraph[i]%=8;
			
			
			// myLCD_Position(0,i);
			// myLCD_PrintNumber(str_bargraph[i]);
			myLCD_Position(1,i);
			myLCD_WriteData(myLCD_CUSTOM_0+str_bargraph[i]);
			
        	// myLCD_DrawVerticalBG(row, i, 8,str_bargraph[i]);
			// DBG_PRINTF("riscrivo barre %d\n",i);
		}
    }

}












/***************************************************************************************************
               void LCD_ScrollMessage(uint8_t var_lineNumber_u8, char *ptr_msgPointer_u8)
 ***************************************************************************************************
 * I/P Arguments: 
                  uint8_t  : Line number on which the message has to be scrolled
                  char *: pointer to the string to be scrolled					  
 * Return value	: none
 * description  :This function scrolls the given message on the specified line.
                 If the specified line number is out of range then the message
				 will be scrolled on first line
 ***************************************************************************************************/
#if ( Enable_LCD_ScrollMessage  == 1 )
void LCD_ScrollMessage(uint8_t var_lineNumber_u8, char *ptr_msgPointer_u8)
{
	unsigned char i,j;


	if(var_lineNumber_u8 > STK_LCDConfig.mvar_MaxSupportedLines_U8)
		var_lineNumber_u8 = mENUM_LcdLineOne; // Select first line if the var_lineNumber_u8 is out of range

	LCD_CmdWrite(CMD_DISPLAY_ON_CURSOR_OFF);			 //Disable the Cursor

	for(i=0;ptr_msgPointer_u8[i];i++)
	{      
		/* Loop to display the complete string,	each time 16 chars are displayed and
		pointer is incremented to point to next char */


		LCD_GoToLine(var_lineNumber_u8);     //Move the Cursor to first line

		for(j=0;j<STK_LCDConfig.mvar_MaxSupportedChars_U8 && ptr_msgPointer_u8[i+j];j++)
		{
			//Display first 16 Chars or till Null char is reached
			LCD_DisplayChar(ptr_msgPointer_u8[i+j]);
		}


		while( j<STK_LCDConfig.mvar_MaxSupportedChars_U8)
		{
			/*If the chars to be scrolled are less than MaxLcdChars,
			  then display remaining chars with blank spaces*/
			LCD_DisplayChar(' ');
			j++;
		}

		DELAY_ms(125);
	}
	LCD_CmdWrite(CMD_DISPLAY_ON_CURSOR_ON);			  // Finally enable the Cursor
}
#endif


void LCD_Application_Poll(void) {
	static uint8 isInitialized = 0;
	static uint8 lcdCounter = 0;
	static char str[20];
	
	if(isInitialized == 0) {
		myLCD_Start();
		
		myLCD_Position(0,0);  
		myLCD_PrintString("Old Display TEST");
		
		sprintf(str,"%s",__TIME__);
	    myLCD_Position(1,0);  
		myLCD_PrintString(str);
/*	    
		myLCD_Position(2,0);  
		sprintf(str,"Date: %02d-%02d-%04d", BUILD_DAY,BUILD_MONTH, BUILD_YEAR );
		myLCD_PrintString(str);
		
		myLCD_Position(3,0);  
		sprintf(str,"Time: %02d:%02d:%02d", BUILD_HOUR, BUILD_MIN, BUILD_SEC);
		myLCD_PrintString(str);
*/
		isInitialized = 1;
	}
	
	if(tick_100ms(TICK_LCD)) {
		// myLCD_Position(1,13);  
		// sprintf(str,"%d",lcdCounter++);
		// myLCD_PrintString(str);
	}
}


/* [] END OF FILE */
