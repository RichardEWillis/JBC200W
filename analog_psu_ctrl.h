/******************************************************************************
 * Manage the Analog Power Supplies
 * 
 * The Analog PSU generates +16v, +12v, +5v and optionally -16v, -12v and -5v
 * This module uses GPIOs to pulse 50v power inputs to generate intermediate
 * +/- 16v power regulation to feed +/- 12v linear regulators.
 * 
 */

#ifndef _ANALOG_PSU_H_
#define _ANALOG_PSU_H_

#include "pico/stdlib.h"

// Setup for managing the +/- 16 V Analog switching PSU
// when intialized, the PSU will remain disabled.
// Call apc_enable to start voltage control.
int apc_init(void);

// Enable 16v regulation
int apc_enable(void);

// is psu manager running ?
bool apc_is_running(void);

// Disable 16v regulation
int apc_disable(void);

#endif /* _ANALOG_PSU_H_ */
