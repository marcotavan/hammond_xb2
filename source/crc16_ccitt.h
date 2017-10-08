
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

    
#ifndef _CRC16_CCITT_
 #define _CRC16_CCITT_
 
    #include "cytypes.h"

    /****************************************************************************************** 
    SDS12657-7Z-Wave Command Class Specification, A-M2015-06-23 page 232
    The checksum field MUST be used to ensure the consistency of the entire command, 
    including the command class and command identifiers. It is RECOMMENDED to use a 
    checksum algorithm that implements the CRC-CCITT polynomium using initialization 
    value equal to 0x1D0F     and 0x1021 (normal representation). The checksum algorithm used 
    for the Firmware Update Meta Data Command Class, version 3 MUST be the same as the algorithm 
    used for the Firmware Update Meta Data Command Class, version 2 in the actual product     
    ********************************************************************************************/
    
    #define CRC16_CCITT_SEED 0x1D0F
    #define CRC16_CCITT_POLY 0x1021
    
    uint16_t crc16ccitt_1d0f(uint8_t *message, uint16_t nBytes);
   
    /* altri crc disponibili:
    
    uint16_t crc16ccitt(uint8_t  *message, uint16_t nBytes);
    uint16_t crc16ccitt_xmodem(uint8_t  *message, uint16_t nBytes);
    uint16_t crc16ccitt_kermit(uint8_t  *message, uint16_t nBytes);
    uint16_t crc16ibm(uint8_t  *message, uint16_t nBytes);
    */
    
#endif // _CRC16_CCITT_