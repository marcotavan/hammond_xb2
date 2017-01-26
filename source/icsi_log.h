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
#ifndef _ICSI_LOG_H_
#define _ICSI_LOG_H_
    
void fill_icsi_log_table(const int n, float *lookup_table);

inline float icsi_log(register float val,
register const float *lookup_table, register const int n);

extern float lookup_table[];
#endif    
/* [] END OF FILE */
