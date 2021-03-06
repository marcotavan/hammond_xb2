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
#include "PCF8575.h"

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


uint8 g_displayfunction;
uint8 g_displaycontrol;
uint8 g_displaymode;
uint8 g_cols;
uint8 g_rows;
uint8 g_charsize;

void LcdModeInit(void);

void LCD_command(uint8 value);
void LcdDisplay_On(void);
void LcdClearDisplay(void);
void LCDReturnHome(void);

void PulseEnable(uint8 _data);


//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
// Allows us to fill the first 8 CGRAM locations with custom characters
void CreateChar(uint8 location, uint8 const charmap[]){ 
	uint32 i = 0;
	
    location &= 0x7; // we only have 8 locations 0-7
	LCD_command(LCD_SETCGRAMADDR | (location << 3));
	for (i=0; i<8; i++) {
		if(LCD_Write(charmap[i])) {
			i--; // display busy
		}
	}
}

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void LiquidCrystal_I2C_init(uint8 lcd_cols, uint8 lcd_rows, uint8 charsize)
{
	g_cols = lcd_cols;
	g_rows = lcd_rows;
	g_charsize = charsize;
    
	LcdModeInit();
	LCD_Position(0,0);
    LCD_PrintString("ciao ciao");
	LCD_Position(1,0);
    LCD_PrintString("zoe tavan");
}

void LcdModeInit(void){
	//Set the LCD display in the correct begin state, must be called before anything else is done.

	g_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

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

	PCF8575_Write(0x0000);	// reset expander
	// this is according to the hitachi HD44780 datasheet
	// figure 23, pg 45

	// we start in 8bit mode, try to set 4 bit mode
	PCF8575_Write(0x0030);
	CyDelayUs(4500); // wait min 4.1ms

	// second try
	PCF8575_Write(0x0030);
	CyDelayUs(4500); // wait min 4.1ms

	// third go!
	PCF8575_Write(0x0030);
	CyDelayUs(150);

	// finally, set to 8-bit interface
	PCF8575_Write(0x0030);

	// set # lines, font size, etc.
	LCD_command(LCD_FUNCTIONSET | g_displayfunction);  // 0038
	
	// turn the display on with no cursor or blinking default
	g_displaycontrol = /*LCD_DISPLAYCONTROL |*/ LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	LcdDisplay_On();
	
	// clear it off
	LcdClearDisplay();
	
	// Initialize to default text direction (for roman languages)
	g_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	LCD_command(LCD_ENTRYMODESET | g_displaymode);
	
	LCDReturnHome();

	/*	8 bit data setup.
        send_command(0x38); // 8 bit mode
        send_command(0x0E); // clear the screen
        send_command(0x01); // display on cursor on
        send_command(0x06);// increment cursor
        send_command(0x80);// cursor position
	*/
}


 uint8 LCD_Write(uint8 value){
	// va fatta macchina a stati nel poll
	// prima bisogna leggere BusyFlag
	PCF8575_Write(ReadWrite_bit | BusyFlag_bit); // abilito la lettura del busy flag su DB7
	if(PCF8575_Read() & BusyFlag_bit) {
		// leggo il dato. display è busy, ritorna
		// questo potrebbe essere un while(PCF8575_Read() & BusyFlag_bit)
		return 1;
	}
	
	// display not busy
	uint16 data = value|RegisterSelect_bit;
	PCF8575_Write(data);
	PulseEnable(data);
	return 0;
}

void PulseEnable(uint8 data){
    
	PCF8575_Write(data | Enable_bit);	// En high
	CyDelayUs(1);		// enable pulse must be >450ns
	PCF8575_Write(data & ~Enable_bit);	// En low
	CyDelayUs(50);		// commands need > 37us to settle
}

void LCD_command(uint8 value){
    uint16 data = value;
	PCF8575_Write(data);
	PulseEnable(data);
}


void Load_Custom_Char(uint8 char_num, uint8 const *rows){
    CreateChar(char_num, rows);
}


void LCD_PrintString(char uint16[]){ 
	uint32 i = 0;
    uint8 size = strlen(uint16);
    
    for (i = 0; i < size; i++){
        if(LCD_Write(uint16[i])) {
			// display busy, aspetta
			i--;
		}
    }
}



/* EOP */ 
