/* ========================================
 *
 * Copyright MARCO TAVAN, 2017-05-09
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================

Selecting 4-bit or 8-bit mode requires careful selection of commands. There are two primary considerations. First, with D3-D0 unconnected, these lines will always appear low (0b0000) to the HD44780 when it is in 8-bit mode. Second, the LCD may initially be in one of three states:

    (State1) 8-bit mode
    (State2) 4-bit mode, waiting for the first set of 4 bits
    (State3) 4-bit mode, waiting for the second set of 4 bits

State3 may occur, for example, if a prior control was aborted after sending only the first 4 bits of a command while the LCD was in 4-bit mode.

The following algorithm ensures that the LCD is in the desired mode:

    Set D7-D4 to 0b0011, and toggle the enable bit.
        If in State1, the LCD will see the command as 0b0011_0000, and thus remain in 8-bit mode (State1).
        If in State2, the LCD will simply latch the value 0b0011 into bits 7-4 and then move to State3.
        If in State3, the LCD will latch the value 0b0011 into bits 3-0, and then execute a random command based on the (unknown to us) values in bits 7-4, after which it will either be in State1 (if the unknown bits happened to be 0b0011), or State2 (if the unknown bits were anything else).
    Repeat the above, setting D7-D4 to 0b0011 and toggling the enable bit again.
        If in State1, the LCD will remain in 8-bit mode (State1) just as above.
        If in State2, it will latch the value into bits 7-4 and move to State3, just as above.
        If in State3, the LCD will latch the value into bits 3-0 just as above and execute a command. However, the command will no longer be random, but will be the 0b0011 that was latched from State2 in the previous iteration. Thus, the LCD will switch to 8-bit mode and change to State1.
    The LCD is now in either State1 or State 3. Repeat the previous step one more time.
        If in State1, the LCD will remain in 8-bit mode (and thus State1).
        The LCD can no longer be in State2 at this point.
        If in State3, the LCD will latch the value into bits 3-0 and execute a command, which will be the 0b0011 that was latched from State2 in the previous iteration, thus switching the LCD to 8-bit mode and State1.
    Now that the LCD is definitely in 8-bit mode, it can be switched to 4-bit mode if desired. To do so, set D7-D4 to 0b0010 and toggle the enable bit. This will leave the LCD in 4-bit mode, configured for a single line and 5x8 fonts.
    Issue any desired additional Function Set commands to specify the number of lines and the font to use, being sure to use the appropriate value for bit 4 so as to remain in the desired mode (0 for 4-bit and 1 for 8-bit).

Once in 4-bit mode, character and control data are transferred as pairs of 4-bit "nibbles" on the upper data pins, D7-D4. The four most significant bits (7-4) must be written first, followed by the four least significant bits (3-0).
*/

#include "project.h"
#include "LiquidCrystal_I2C.h"

// PCF8575: 400-kHz Fast I2C Bus
// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 


uint8 g_addr;
uint8 g_displayfunction;
uint8 g_displaycontrol;
uint8 g_displaymode;
uint8 g_cols;
uint8 g_rows;
uint8 g_charsize;
uint8 g_backlightval;

void LcdModeInit(void);
uint8 ExpanderWrite(uint8 _data);
void Write4bits(uint8 value);
void LCD_command(uint8 value);
void LcdDisplay_On(void);
void LcdClearDisplay(void);
void LCDReturnHome(void);
void LCD_send(uint8 value, uint8 mode);
void PulseEnable(uint8 _data);
uint8 I2C_M_write_pcf8574(uint8 addr,uint8 data);
uint8 I2C_M_write_pcf8575(uint8 addr,uint16 data);
uint8 LCD_IsReady(void); // bloccante

void LiquidCrystal_I2C_init(uint8 lcd_addr, uint8 lcd_cols, uint8 lcd_rows, uint8 charsize)
{
	g_addr = lcd_addr;
	g_cols = lcd_cols;
	g_rows = lcd_rows;
	g_charsize = charsize;
	g_backlightval = LCD_BACKLIGHT;
    
	LcdModeInit();
    return;
}

void LcdModeInit(void){
	//Set the LCD display in the correct begin state, must be called before anything else is done.

	g_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (g_rows > 1) {
		g_displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
	if ((g_charsize != 0) && (g_rows == 1)) {
		g_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	CyDelay(50); 

	// Now we pull both RS and R/W low to begin commands
	ExpanderWrite(g_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	CyDelay(1000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	Write4bits(0x03 << 4);
	CyDelayUs(4500); // wait min 4.1ms

	// second try
	Write4bits(0x03 << 4);
	CyDelayUs(4500); // wait min 4.1ms

	// third go!
	Write4bits(0x03 << 4); 
	CyDelayUs(150);

	// finally, set to 4-bit interface
	Write4bits(0x02 << 4); 

	// set # lines, font size, etc.
	LCD_command(LCD_FUNCTIONSET | g_displayfunction);  
	
	// turn the display on with no cursor or blinking default
	g_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	LcdDisplay_On();
	
	// clear it off
	LcdClearDisplay();
	
	// Initialize to default text direction (for roman languages)
	g_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	LCD_command(LCD_ENTRYMODESET | g_displaymode);
	
	LCDReturnHome();
    
    return;
	/*	8 bit data setup.
        send_command(0x38); // 8 bit mode
        send_command(0x0E); // clear the screen
        send_command(0x01); // display on cursor on
        send_command(0x06);// increment cursor
        send_command(0x80);// cursor position
	*/
}

/********** high level commands, for the user! */
void LcdClearDisplay(void){
    // Remove all the characters currently shown. Next print/write operation will start
	// from the first position on LCD display.
	
	LCD_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	CyDelay(2);  // this command takes a long time!
    
    return;
}

void LCDReturnHome(void){
	// Next print/write operation will will start from the first position on the LCD display.
    LCD_command(LCD_RETURNHOME);  // set cursor position to zero
	CyDelay(2);  // this command takes a long time!
    
    return;
}

void LCD_Position(uint8 col, uint8 row){
    
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > g_rows) {
		row = g_rows-1;    // we count rows starting w/0
	}
	LCD_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
    
    return;
}


void LcdDisplay_Off(void){
	// Do not show any characters on the LCD display. Backlight state will remain unchanged.
	// Also all characters written on the display will return, when the display in enabled again.
	
    g_displaycontrol &= ~LCD_DISPLAYON;
	LCD_command(LCD_DISPLAYCONTROL | g_displaycontrol);
    
    return;
}


void LcdDisplay_On(void){
    // Show the characters on the LCD display, this is the normal behaviour. This method should
	// only be used after LcdDisplay_Off() has been used.
	
	g_displaycontrol |= LCD_DISPLAYON;
	LCD_command(LCD_DISPLAYCONTROL | g_displaycontrol);
    
    return;
}

// Turns the underline cursor on/off
void LcdCursorOff(void){
	// Do not show a cursor indicator.
    g_displaycontrol &= ~LCD_CURSORON;
	LCD_command(LCD_DISPLAYCONTROL | g_displaycontrol);
    
    return;
}
void LcdCursorOn(void){
	// Show a cursor indicator, cursor can blink on not blink. Use the
	// methods LCD_blink() and LCD_noBlink() for changing cursor blink.
	
    g_displaycontrol |= LCD_CURSORON;
	LCD_command(LCD_DISPLAYCONTROL | g_displaycontrol);
    
    return;
}

// Turn on and off the blinking cursor
void LCD_noBlink(void){
	// Do not blink the cursor indicator.
    g_displaycontrol &= ~LCD_BLINKON;
	LCD_command(LCD_DISPLAYCONTROL | g_displaycontrol);
    
    return;
}
void LCD_blink(void){
    // Start blinking the cursor indicator.
	g_displaycontrol |= LCD_BLINKON;
	LCD_command(LCD_DISPLAYCONTROL | g_displaycontrol);
    
    return;
}

// These commands scroll the display without changing the RAM
void ScrollDisplayLeft(void){
    
	LCD_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
    
    return;
}
void ScrollDisplayRight(void){
	
    LCD_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
    
    return;
}

// This is for text that flows Left to Right
void LeftToRight(void){
    
	g_displaymode |= LCD_ENTRYLEFT;
	LCD_command(LCD_ENTRYMODESET | g_displaymode);
    
    return;
}

// This is for text that flows Right to Left
void RightToLeft(void){
    
	g_displaymode &= ~LCD_ENTRYLEFT;
	LCD_command(LCD_ENTRYMODESET | g_displaymode);
    
    return;
}

// This will 'right justify' text from the cursor
void Autoscroll(void){
	
    g_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	LCD_command(LCD_ENTRYMODESET | g_displaymode);
    
    return;
}

// This will 'left justify' text from the cursor
void NoAutoscroll(void){
	
    g_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	LCD_command(LCD_ENTRYMODESET | g_displaymode);
    
    return;
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void CreateChar(uint8 location, uint8 const charmap[]){ 
	uint32 i = 0;
	
    location &= 0x7; // we only have 8 locations 0-7
	LCD_command(LCD_SETCGRAMADDR | (location << 3));
	for (i=0; i<8; i++) {
		LCD_Write(charmap[i]);
	}
    
    return;
}

// Turn the (optional) backlight off/on
void LCD_NoBacklight(void) {
    g_backlightval=LCD_NOBACKLIGHT;
	ExpanderWrite(0);
    return;
}

void LCD_Backlight(void){
	g_backlightval=LCD_BACKLIGHT;
	ExpanderWrite(0);
    return;
}

/*********** mid level commands, for sending data/cmds */

void LCD_command(uint8 value){
    LCD_send(value, 0);
    return;
}

 void LCD_Write(uint8 value){
	LCD_send(value, RegisterSelect_bit);
    return;
}


/************ low level data pushing commands **********/

// write either command or data
void LCD_send(uint8 value, uint8 mode){
    
	uint8 highnib=value&0xf0;
	uint8 lownib=(value<<4)&0xf0;
	
	// LCD_IsReady(); // qua va assolutamente fatta.
	
	Write4bits((highnib)|mode);
	Write4bits((lownib)|mode); 
    
    return;
}

void Write4bits(uint8 value) {
    
	ExpanderWrite(value);
	PulseEnable(value);
    
    return;
}

uint8 ExpanderWrite(uint8 _data){     
    uint8 status = 0;
	// status = I2C_M_write_byte(g_addr,_data | g_backlightval);
	// status = I2C_M_write_pcf8575(g_addr,_data | g_backlightval);
    return status;
}

void PulseEnable(uint8 _data){
    
	ExpanderWrite(_data | Enable_bit);	// En high
	CyDelayUs(1);		// enable pulse must be >450ns
	ExpanderWrite(_data & ~Enable_bit);	// En low
	CyDelayUs(50);		// commands need > 37us to settle
    
    return;
}

void Load_Custom_Char(uint8 char_num, uint8 const *rows){
    CreateChar(char_num, rows);
    return;
}

void SetBacklight(uint8 new_val){
    
	if (new_val) {
		LCD_Backlight();		// turn backlight on
	} else {
		LCD_NoBacklight();		// turn backlight off
	}
    
    return;
}

void LCD_PrintString(char uint16[]){ 
	uint32 i = 0;
    
    uint8 size = strlen(uint16);
    
    for (i = 0; i < size; i++){
        LCD_Write(uint16[i]);
    }

    return;
}

uint8 I2C_M_write_pcf8574(uint8 addr,uint8 data){ 
	// funzione che scrive nell'I2C
	uint8 status = 0;
	// provo...
	status = I2C_LCD_MasterSendStart(addr, 0);
	if (status != 0) return status;
	
	// continuo... 1 byte
	status = I2C_LCD_MasterWriteByte(data);
	if (status != 0) return status;
 
	// termino
    status = I2C_LCD_MasterSendStop();
    return status;
}

uint8 I2C_M_write_pcf8575(uint8 addr,uint16 data){ 
	// funzione che scrive nell'I2C
/*	
	The first data byte in every pair refers to Port 0
	(P07 to P00), whereas the second data byte in every pair
	refers to Port 1 (P17 to P10), see Fig.11
*/
	uint8 status = 0;
	// provo...
	status = I2C_LCD_MasterSendStart(addr, 0);
	if (status != 0) return status;
	
	// continuo... 1 byte LSB
	status = I2C_LCD_MasterWriteByte(data);
	if (status != 0) return status;
	
	// continuo... 2 byte MSB
	status = I2C_LCD_MasterWriteByte(data>>8);
	if (status != 0) return status;
	
	// termino
    status = I2C_LCD_MasterSendStop();
	
    return status;
}

uint16 I2C_M_Read_pcf8575(uint8 addr){
	/*
	Reading from a port (input mode):
	All ports programmed as input should be set to logic 1.
	To read, the master (microcontroller) first addresses the slave device after it receives the interrupt. 
	By setting the last bit of the byte containing the slave address to logic 1 the read mode is entered. T
	he data bytes that follow on the SDA are the values on the ports. 
	If the data on the input port changes faster than the master can read, this data may be lost.
	*/
	
	
	return 0;
}

uint8 LCD_IsReady(void) {
	uint8 value = 0;
	uint32 timeout;
	
	#define LCD_LONGEST_CMD_US           (0x651u)
	#define LCD_WAIT_CYCLE               (0x10u)
	#define LCD_READY_DELAY              ((LCD_LONGEST_CMD_US * 4u)/(LCD_WAIT_CYCLE))
	
	timeout = LCD_READY_DELAY;
	
	/*
	Busy Flag (BF)
	When the busy flag is set at a logical "1", the LCD
	unit is executing an internal operation, and no in-
	struction will be accepted. The state of the busy flag
	is output on data line DB 7 in response to the register
	selection signals RS = 0, R/W = 1 as shown in Table
	3. The next instruction may be entered after the
	busy flag is reset to logical "0". */
	
	/* Make sure RS is low */
	//    LCD_PORT_DR_REG &= ((uint8)(~LCD_RS));

	    /* Set R/W high to read */
	//    LCD_PORT_DR_REG |= LCD_RW;

	    do
	    {
	        /* 40 ns delay required before rising Enable and 500ns between neighbour Enables */
	        CyDelayUs(0u);

	        /* Set E high */
//	        LCD_PORT_DR_REG |= LCD_E;

	        /* 360 ns delay setup time for data pins */
	        CyDelayUs(1u);

	        /* Get port state */
//	        value = LCD_PORT_PS_REG;

	        /* Set enable low */
//	        LCD_PORT_DR_REG &= ((uint8)(~LCD_E));

	        /* This gives true delay between disabling Enable bit and polling Ready bit */
	        CyDelayUs(0u);

	        /* Extract ready bit */
//	        value &= LCD_READY_BIT;

	        /* Set E high as we in 4-bit interface we need extra operation */
//	        LCD_PORT_DR_REG |= LCD_E;

	        /* 360 ns delay setup time for data pins */
	        CyDelayUs(1u);

	        /* Set enable low */
//	        LCD_PORT_DR_REG &= ((uint8)(~LCD_E));

	        /* If LCD is not ready make a delay */
	        if (value == 0u)
	        {
	            CyDelayUs(10u);
	        }

	        /* Repeat until bit 4 is not zero or until timeout. */
	        timeout--;

	    } while ((value != 0u) && (timeout > 0u));

	    /* Set R/W low to write */
//	    LCD_PORT_DR_REG &= ((uint8)(~LCD_RW));

	    /* Clear LCD port*/
//	    LCD_PORT_DR_REG &= ((uint8)(~LCD_PORT_MASK));

        /* Change Port to Output (Strong) on data pins */
        /* Mask off data pins to clear high z values out. Configure data pins 
        * to Strong Drive, others unchanged.
        */
//	        LCD_PORT_DM0_REG &= ((uint8)(~LCD_DATA_MASK));
        /* Mask off data pins to clear high z values out */
//	        value = LCD_PORT_DM1_REG & ((uint8)(~LCD_DATA_MASK));
        /* Configure data pins to Strong Drive, others unchanged */
//	        LCD_PORT_DM1_REG = value | (LCD_STRONG_DM1 & LCD_DATA_MASK);

        /* Mask off data pins to clear high z values out */
//	        value = LCD_PORT_DM2_REG & ((uint8)(~LCD_DATA_MASK));
        /* Configure data pins to Strong Drive, others unchanged */
//	        LCD_PORT_DM2_REG = value | (LCD_STRONG_DM2 & LCD_DATA_MASK);
		
	return 0;
}

/* EOP */ 
