/******************************************************************************
 * Non-specific utilities for the JBC 200W Controller
 * 
 * Time:
 *  sleep_si()                  sleep durations of integer seconds
 *  sleep_sf()                  sleep duration of [float32] seconds to the 
 *                                nearest msec
 * 
 * 
 * 
 */

#include <jbc_util.h>
#include <time.h>
#include <string.h>
#include <math.h>


void sleep_si(uint32_t t) {
    sleep_ms(t * 1000ull);
}

void sleep_sf(float t) {
    sleep_ms((uint32_t)(t * 1000.0));
}

const char * i_to_strflen(uint32_t i, char * strbuf, size_t strbuflen, size_t strclen) {
    const char * ret = NULL;
    if (strbuf && strclen && strbuflen > strclen) {
        uint32_t div  = (uint32_t)pow(10,strclen); /* highest divider power based on character window, eg. strclen = 4 => div = 10000 */
        uint8_t  lblank = 1; /* flag for initial left-J blanking */
        memset(strbuf, 0x20, strclen);
        ret = (const char *)strbuf;
        strbuf[strclen] = '\0';
        while (i >= div) {
            i -= div; /* subtract over-value until value can actually be shown in the string window */
        }
        while (strclen) {
            uint8_t idig;
            div = div / 10;
            strclen --;
            idig = (uint8_t)(i / div);
            *strbuf = '0' + idig;
            i = i - ((uint32_t)idig * div);
            if (lblank && (idig == 0)) {
                *strbuf = ' '; // make leading zeros invisible
            } else if (lblank) {
                lblank = 0; // on the first non-zero, allow further zeros to be displayed
            }
            strbuf ++;
        }
    }
    return ret;
}


