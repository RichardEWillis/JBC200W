
#include <analog_psu_ctrl.h>
#include <board.h>
#include "hardware/gpio.h"
#include <pico/time.h>

// set '1' if -16v/-12v/-5v side of the PSU is populated on the PCB
#define USING_N16V_PSU 0

void gpio_callback(uint gpio, uint32_t event_mask);

// Setup for managing the +/- 16 V Analog switching PSU
// when intialized, the PSU will remain disabled.
// Call apc_enable to start voltage control.
int apc_init(void) {
    // +16v power control
    gpio_init(APSU_P16V_ON_L);
    gpio_put(APSU_P16V_ON_L, APSU_X16V_DISABLE);
    gpio_set_dir(APSU_P16V_ON_L, GPIO_OUT);
    // +16v threshold detect
    gpio_init(APSU_P16V_CHARGE_STATE);
    gpio_set_dir(APSU_P16V_CHARGE_STATE, GPIO_IN);
#if (USING_N16V_PSU==1)
    // -16v power control
    gpio_init(APSU_N16V_ON_L);
    gpio_put(APSU_N16V_ON_L, APSU_X16V_DISABLE);
    gpio_set_dir(APSU_N16V_ON_L, GPIO_OUT);
    // -16v threshold detect
    gpio_init(APSU_N16V_CHARGE_STATE);
    gpio_set_dir(APSU_N16V_CHARGE_STATE, GPIO_IN);
#endif
    // enable/start GPIO ISR - leave running all the time!
    gpio_set_irq_enabled_with_callback(APSU_P16V_CHARGE_STATE, 
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, 
        &gpio_callback);
    return 0;
}


static bool              timer_running = 0;             // 1ms timer
static repeating_timer_t psutmr;                        // 1ms timer handle
static uint32_t          P16v_discharge_counter = 0;    // count the discharge period for +16V
static bool              P16V_count_enable = false;     // when true, counter is to increment
static bool              P16V_dischg_wt_enable = false; // when true, the discharge fault is to be monitored
static bool              P16V_FAULT = false;
#if (USING_N16V_PSU==1)
static uint32_t          N16v_discharge_counter = 0;    // count the discharge period for -16V
static bool              N16V_count_enable = false;     // when true, counter is to increment
static bool              N16V_dischg_wt_enable = false; // when true, the discharge fault is to be monitored
static bool              N16V_FAULT = false;
#endif
#define P16V_CHG_EN_THRESH       1  /* (10ms) msec delay before re-enabling the +16V charge MOSFET */
#define P16V_DISCHG_WT_ALARM    20  /* (200ms) msec delay before signalling a fault (did not see the over-voltage signal de-assert in this time) */
#define APSU_SCAN_PD_MS         10  /* msec periodic timer interval [msec] */

/* ISR Routine - GPIO Edge Interrupts */
void gpio_callback(uint gpio, uint32_t event_mask) {
    if (gpio == APSU_P16V_CHARGE_STATE) {
        if (event_mask & GPIO_IRQ_EDGE_FALL) {
            // (falling edge into: APSU_X16V_CHG_OVER)
            // +16V threshold reached, turn off MOSFET
            gpio_put(APSU_P16V_ON_L, APSU_X16V_DISABLE);
            P16v_discharge_counter = 0;
            P16V_count_enable = true;
            P16V_dischg_wt_enable = true;
        } else if (event_mask & GPIO_IRQ_EDGE_RISE) {
            // (rising edge into: APSU_X16V_CHG_OVER)
            // +16V level fallen below the threshold, voltage 
            // is falling, disable the discharge wait fault enable flag.
            P16V_dischg_wt_enable = false;
        }
    }
#if (USING_N16V_PSU==1)
    if (gpio == APSU_N16V_CHARGE_STATE) {
        if (event_mask & GPIO_IRQ_EDGE_FALL) {
            // (falling edge into: APSU_X16V_CHG_OVER)
            // -16V threshold reached, turn off MOSFET
            gpio_put(APSU_N16V_ON_L, APSU_X16V_DISABLE);
            N16v_discharge_counter = 0;
            N16V_count_enable = true;
            N16V_dischg_wt_enable = true;
        } else if (event_mask & GPIO_IRQ_EDGE_RISE) {
            // (rising edge into: APSU_X16V_CHG_OVER)
            // -16V level fallen below the threshold, voltage 
            // is falling, disable the discharge wait fault enable flag.
            N16V_dischg_wt_enable = false;
        }
    }
#endif
}

/* msec polling task, timing the 16v discharge period and faults */
static bool chk_thresholds(repeating_timer_t * rptdata) {
    if (P16V_count_enable) {
        if (P16V_dischg_wt_enable && (P16v_discharge_counter > P16V_DISCHG_WT_ALARM)) {
            // throw a fault on the +16v charge system.
            // next PCB version, add a crowbar/disable on the +50v voltage rail
            // and add a fuse to it (blow the fuse!)
            P16V_FAULT = true;
            P16V_dischg_wt_enable = false;
        }
        if (P16v_discharge_counter > P16V_CHG_EN_THRESH) {
            // +16v fallen enough, set it to charge again
            P16V_count_enable = false;
            gpio_put(APSU_P16V_ON_L, APSU_X16V_ENABLE);
        }
        P16v_discharge_counter ++;
    }
    return timer_running; // set to 0/false to stop the repeating-timer
}


// Enable 16v regulation
int apc_enable(void) {
    int rc = 1;
    if (!timer_running) {
        timer_running = (bool)add_repeating_timer_ms(APSU_SCAN_PD_MS, chk_thresholds, NULL, &psutmr);
        if (timer_running) {
            // enable 16v charging
            gpio_put(APSU_P16V_ON_L, APSU_X16V_ENABLE);
            rc = 0; // ok
        }
    }
    return rc;
}

// is psu manager running ?
bool apc_is_running(void) {
    return timer_running;
}

// Disable 16v regulation
int apc_disable(void) {
    int rc = 1;
    if (timer_running) {
        timer_running = false;
        // disable 16v charging
        gpio_put(APSU_P16V_ON_L, APSU_X16V_DISABLE);
        rc = 0;
    }
    return rc;
}
