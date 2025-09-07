/******************************************************************************
 * Decode Keypad based Operations
 * 
 * Decode keypad based operations
 * Internal operation calls are done within the codebase.
 * 
 * Operation list:
 * - Temp Set: Set
 * - Temp Set: Clear
 * - Scale Set
 * - Sleep Delay
 * - Manual Sleep/Wake
 * - Calibration (FUTURE)
 * - Power Tweeks (FUTURE)
 * - Select Preset
 * 
 */

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include <stddef.h>
#include <pico/types.h>

// Setup Operations
int ops_init(void);

// Reset internal Operations
// Call after a poll returns a non-zero result
int ops_reset(void);

// Poll Operations - push in a new key
// Returns: 
//  0 OK/Normal
//  1 Error Occured
int ops_poll(char k);

// Getters

uint32_t get_tipTempSetting(void);  // current temp setting for iron
uint32_t get_tempScale(void);       // current temp scale ('C' | 'F')
bool     get_wakeStatus(void);      // get wake status, true := running and heating
uint32_t get_sleepDelay(void);      // get delay before sleeping


#endif /* _OPERATIONS_H_ */
