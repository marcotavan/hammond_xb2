/* Creates the ICSILog lookup table. Must be called
    once before any call to icsi_log().
    n is the number of bits to be taken from the mantissa
    (0<=n<=23)
    lookup_table is a pointer to a floating point array
    (memory has to be allocated by the user) of 2^n positions.
*/

#include "project.h"
#include "math.h"
#include "icsi_log.h"

float lookup_table[256];

void fill_icsi_log_table(const int n, float *lookup_table)
{
    float numlog;
    int *const exp_ptr = ((int*)&numlog);
    int x = *exp_ptr; //x is the float treated as an integer
    x = 0x3F800000; //set the exponent to 0 so numlog=1.0
    *exp_ptr = x;
    int incr = 1 << (23-n); //amount to increase the mantissa
    int p=pow(2,n);
    for(int i=0;i<p;++i)
    {
        lookup_table[i] = log2(numlog); //save the log value
        x += incr;
        *exp_ptr = x; //update the float value
    }
}

/* Computes an approximation of log(val) quickly.
    val is a IEEE 754 float value, must be >0.
    lookup_table and n must be the same values as
    provided to fill_icsi_table.
    returns: log(val). No input checking performed.
*/

inline float icsi_log(register float val,
register const float *lookup_table, register const int n)
{
    register int *const  exp_ptr = ((int*)&val);
    register int         x = *exp_ptr; //x is treated as integer
    register const int   log_2 = ((x >> 23) & 255) - 127;//exponent
    x &= 0x7FFFFF; //mantissa
    x = x >> (23-n); //quantize mantissa
    val = lookup_table[x]; //lookup precomputed value
    return ((val + log_2)* 0.69314718); //natural logarithm
}