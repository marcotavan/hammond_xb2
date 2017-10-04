/*******************************************************************************
* File Name: myLCD.h
* Version 2.20
*
* Description:
*  This header file contains registers and constants associated with the
*  Character LCD component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CHARLCD_myLCD_H)
#define CY_CHARLCD_myLCD_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*   Conditional Compilation Parameters
***************************************/

#define myLCD_CONVERSION_ROUTINES     (1u)
#define myLCD_CUSTOM_CHAR_SET         (0u)

/* Custom character set types */
#define myLCD_NONE                     (0u)    /* No Custom Fonts      */
#define myLCD_HORIZONTAL_BG            (1u)    /* Horizontal Bar Graph */
#define myLCD_VERTICAL_BG              (2u)    /* Vertical Bar Graph   */
#define myLCD_USER_DEFINED             (3u)    /* User Defined Fonts   */


/***************************************
*        Function Prototypes
***************************************/

void myLCD_Init(void) ;
void myLCD_Enable(void) ;
void myLCD_Start(void) ;
void myLCD_Stop(void) ;
void myLCD_WriteControl(uint8 cByte) ;
void myLCD_WriteData(uint8 dByte) ;
void myLCD_PrintString(char8 const string[]) ;
void myLCD_Position(uint8 row, uint8 column) ;
void myLCD_PutChar(char8 character) ;
void myLCD_IsReady(void) ;
void myLCD_SaveConfig(void) ;
void myLCD_RestoreConfig(void) ;
void myLCD_Sleep(void) ;
void myLCD_Wakeup(void) ;
void LCD_Application_Poll(void);

#if((myLCD_CUSTOM_CHAR_SET == myLCD_VERTICAL_BG) || \
                (myLCD_CUSTOM_CHAR_SET == myLCD_HORIZONTAL_BG))

    void  myLCD_LoadCustomFonts(uint8 const customData[])
                        ;

    void  myLCD_DrawHorizontalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value)
                         ;

    void myLCD_DrawVerticalBG(uint8 row, uint8 column, uint8 maxCharacters, uint8 value)
                        ;

#endif /* ((myLCD_CUSTOM_CHAR_SET == myLCD_VERTICAL_BG) */

#if(myLCD_CUSTOM_CHAR_SET == myLCD_USER_DEFINED)

    void myLCD_LoadCustomFonts(uint8 const customData[])
                            ;

#endif /* ((myLCD_CUSTOM_CHAR_SET == myLCD_USER_DEFINED) */

#if(myLCD_CONVERSION_ROUTINES == 1u)

    /* ASCII Conversion Routines */
    void myLCD_PrintInt8(uint8 value) ;
    void myLCD_PrintInt16(uint16 value) ;
    void myLCD_PrintInt32(uint32 value) ;
    void myLCD_PrintNumber(uint16 value) ; 
    void myLCD_PrintU32Number(uint32 value) ;
    
#endif /* myLCD_CONVERSION_ROUTINES == 1u */

/* Clear Macro */
#define myLCD_ClearDisplay() myLCD_WriteControl(myLCD_CLEAR_DISPLAY)

/* Off Macro */
#define myLCD_DisplayOff() myLCD_WriteControl(myLCD_DISPLAY_CURSOR_OFF)

/* On Macro */
#define myLCD_DisplayOn() myLCD_WriteControl(myLCD_DISPLAY_ON_CURSOR_OFF)

#define myLCD_PrintNumber(value) myLCD_PrintU32Number((uint16) (value))


/***************************************
*           Global Variables
***************************************/

extern uint8 myLCD_initVar;
extern uint8 myLCD_enableState;
extern uint8 const CYCODE myLCD_customFonts[64u];


/***************************************
*           API Constants
***************************************/

/* Full Byte Commands Sent as Two Nibbles */
#define myLCD_DISPLAY_8_BIT_INIT       (0x30u)
#define myLCD_DISPLAY_4_BIT_INIT       (0x02u)
#define myLCD_DISPLAY_CURSOR_OFF       (0x08u)
#define myLCD_CLEAR_DISPLAY            (0x01u)
#define myLCD_CURSOR_AUTO_INCR_ON      (0x06u)
#define myLCD_DISPLAY_CURSOR_ON        (0x0Eu)
#define myLCD_DISPLAY_4B_2_LINES_5x10  (0x2Cu)
#define myLCD_DISPLAY_8B_2_LINES_5x10  (0x3Cu)
#define myLCD_DISPLAY_ON_CURSOR_OFF    (0x0Cu)

#define myLCD_RESET_CURSOR_POSITION    (0x03u)
#define myLCD_CURSOR_WINK              (0x0Du)
#define myLCD_CURSOR_BLINK             (0x0Fu)
#define myLCD_CURSOR_SH_LEFT           (0x10u)
#define myLCD_CURSOR_SH_RIGHT          (0x14u)
#define myLCD_DISPLAY_SCRL_LEFT        (0x18u)
#define myLCD_DISPLAY_SCRL_RIGHT       (0x1Eu)
#define myLCD_CURSOR_HOME              (0x02u)
#define myLCD_CURSOR_LEFT              (0x04u)
#define myLCD_CURSOR_RIGHT             (0x06u)

/* Point to Character Generator Ram 0 */
#define myLCD_CGRAM_0                  (0x40u)

/* Point to Display Data Ram 0 */
#define myLCD_DDRAM_0                  (0x80u)

/* LCD Characteristics */
#define myLCD_CHARACTER_WIDTH          (0x05u)
#define myLCD_CHARACTER_HEIGHT         (0x08u)

#if(myLCD_CONVERSION_ROUTINES == 1u)
    #define myLCD_NUMBER_OF_REMAINDERS_U32 (0x0Au)
    #define myLCD_TEN                      (0x0Au)
    #define myLCD_8_BIT_SHIFT              (8u)
    #define myLCD_32_BIT_SHIFT             (32u)
    #define myLCD_ZERO_CHAR_ASCII          (48u)
#endif /* myLCD_CONVERSION_ROUTINES == 1u */

/* Nibble Offset and Mask */
#define myLCD_NIBBLE_SHIFT             (0x04u)
#define myLCD_NIBBLE_MASK              (0x0Fu)

/* LCD Module Address Constants */
#define myLCD_ROW_0_START              (0x00u)
#define myLCD_ROW_1_START              (0x40u)
#define myLCD_ROW_2_START              (0x10u)
#define myLCD_ROW_3_START              (0x50u)

/* Custom Character References */
#define myLCD_CUSTOM_0                 (0x00u)
#define myLCD_CUSTOM_1                 (0x01u)
#define myLCD_CUSTOM_2                 (0x02u)
#define myLCD_CUSTOM_3                 (0x03u)
#define myLCD_CUSTOM_4                 (0x04u)
#define myLCD_CUSTOM_5                 (0x05u)
#define myLCD_CUSTOM_6                 (0x06u)
#define myLCD_CUSTOM_7                 (0x07u)

/* Other constants */
#define myLCD_BYTE_UPPER_NIBBLE_SHIFT  (0x04u)
#define myLCD_BYTE_LOWER_NIBBLE_MASK   (0x0Fu)
#define myLCD_U16_UPPER_BYTE_SHIFT     (0x08u)
#define myLCD_U16_LOWER_BYTE_MASK      (0xFFu)
#define myLCD_CUSTOM_CHAR_SET_LEN      (0x40u)

#define myLCD_LONGEST_CMD_US           (0x651u)
#define myLCD_WAIT_CYCLE               (0x10u)
#define myLCD_READY_DELAY              ((myLCD_LONGEST_CMD_US * 4u)/(myLCD_WAIT_CYCLE))

#endif /* CY_CHARLCD_myLCD_H */


/* [] END OF FILE */
