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

#include "project.h"
#include "crc16_ccitt.h"
#include "debug.h"

typedef uint16_t bit_order_16(uint16_t value);
typedef uint8_t bit_order_8(uint8_t value);

uint16_t straight_16(uint16_t value) 
{
    return value;
}

uint16_t reverse_16(uint16_t value) 
{
    uint16_t reversed = 0;
    uint8_t i;
    for (i = 0; i < 16; ++i) 
    {
        reversed <<= 1;
        reversed |= value & 0x1;
        value >>= 1;
    }
    return reversed;
}

uint8_t straight_8(uint8_t value) 
{
    return value;
}

uint8_t reverse_8(uint8_t value) 
{
    uint8_t reversed = 0;
    uint8_t i;
    for (i = 0; i < 8; ++i) 
    {
        reversed <<= 1;
        reversed |= value & 0x1;
        value >>= 1;
    }
    return reversed;
}

uint16_t crc16(uint8_t const *message, int nBytes,
        bit_order_8 data_order, bit_order_16 remainder_order,
        uint16_t remainder, uint16_t polynomial) 
{
    uint8_t byte;
    uint8_t bit;
    
    for (byte = 0; byte < nBytes; ++byte) 
    {
        remainder ^= (data_order(message[byte]) << 8);
        for (bit = 8; bit > 0; --bit) 
        {
            if (remainder & 0x8000) 
            {
                remainder = (remainder << 1) ^ polynomial;
            } 
            else 
            {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder_order(remainder);
}


uint16_t crc16ccitt(uint8_t  *message, uint16_t nBytes) 
{
    return crc16(message, nBytes, straight_8, straight_16, 0xffff, 0x1021);
}

uint16_t crc16ccitt_xmodem(uint8_t  *message, uint16_t nBytes) 
{
    return crc16(message, nBytes, straight_8, straight_16, 0x0000, 0x1021);
}

uint16_t crc16ccitt_kermit(uint8_t  *message, uint16_t nBytes) 
{
    uint16_t swap = crc16(message, nBytes, reverse_8, reverse_16, 0x0000, 0x1021);
    return swap << 8 | swap >> 8;
}

uint16_t crc16ccitt_1d0f(uint8_t  *message, uint16_t nBytes) 
{
    return crc16(message, nBytes, straight_8, straight_16, 0x1d0f, 0x1021);
}

uint16_t crc16ibm(uint8_t  *message, uint16_t nBytes) 
{
    return crc16(message, nBytes, reverse_8, reverse_16, 0x0000, 0x8005);
}
