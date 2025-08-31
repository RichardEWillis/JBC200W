/******************************************************************************
 * Non-specific utilities for the JBC 200W Controller
 * 
 * Time:
 *  sleep_si()                  sleep durations of integer seconds
 *  sleep_sf()                  sleep duration of [float32] seconds to the 
 *                                nearest msec
 * Strings
 *  i_to_strflen()              integer-to-string_with_fixed_length
 *                                returns fixed character length string with
 *                                leading blank spaces
 *                                eg. 13 -> str[4] := "  13"
 * 
 */

#ifndef _JBC_UTIL_H_
#define _JBC_UTIL_H_

#include "pico/stdlib.h"

void sleep_si(uint32_t t);
void sleep_sf(float t);

// Convert integer value to a string with fixed character window.
// Inputs
//  i           value to convert from (unsigned!)
//  strbuf      user's buffer to place string into, is cleaned first to 'strclen' chars
//  strbuflen   true length of 'strbuf' should be at least one more than 'strclen'
//  strclen     fixed size char "window" in 'strbuf', pre cleaned with whitespace.
// Returns: 'strbuf' as a const char * w/ terminating null '\0'
// ----------------------------------------------------------------------------
const char * i_to_strflen(uint32_t i, char * strbuf, size_t strbuflen, size_t strclen);


#endif /* _JBC_UTIL_H_ */
