#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/rand.h"
#include <jbc_util.h>
#include <board.h>
#include <operations.h>
#include <display.h>
#include <keypad.h>

#define PWR_TOTAL   IRON_MAX_WATT
#define PSET_COUNT  MAX_TEMP_PRESETS
#define TEMP_TOTAL  IRON_MAX_TEMP

// spin here for 1 second while scanning for keypad inputs
// and sending to the menu operations. Return after
// 1 second.
#define PCHK_MS_SLP_INTVAL 100
static void poll_chk_operations(void) {
    char key = 0;
    uint32_t ms_intval = 0;
    while (ms_intval < 1000) {
        if (keypad_get(&key)) {
            if ( ops_poll(key) ) {
                printf("[ops_poll()] resetting operations\n");
                ops_reset(); // silently reset state machine, ok if operation completed as well.
            }
        }
        sleep_ms(PCHK_MS_SLP_INTVAL);
        ms_intval += PCHK_MS_SLP_INTVAL;
    }
}

int main()
{
    stdio_init_all();

    // Setup Display handler and show the operating screen
    disp_init();
    disp_startscrn();
    //sleep_si(10);
    sleep_si(1);
    disp_opscrn();
    // Setup/Init Menu Operations
    ops_init();
    // Startup keypad scanning
    keypad_init();
    keypad_start();

    // test loop stage
    //char  key = 0;
    uint32_t R;                 // random value
    int pwr = 0;                // random power setting
    int pwr_percent = 0;        // computed % of total power
    bool isHeating = false;     // random heating/cooling cycle
    char preset = '*';          // keyed preset ['A', 'B', 'C', 'D']
    int temp = 0;               // random tip PRESET temp
    int tip_temp = 0;           // LED tip temperatur value displayed (max:TEMP_TOTAL)
    while (true) {
        // look for any relevent keys (A,B,C,D)
        // key = 0;
        // do {
        //     if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
        //         preset = key;
        //         break;
        //     }
        // } while (keypad_get(&key));
        R = get_rand_32();
        isHeating = (bool)(R & 0x1);
        pwr = R % PWR_TOTAL;
        pwr_percent = (int)( (float)pwr * 100.0 / (float)PWR_TOTAL );
        temp = pwr + 100;
        tip_temp = R % TEMP_TOTAL;
        // ---
        // if (isHeating)
        //     disp_heat_on();
        // else
        //     disp_cool_on();
        disp_pwr_txt(pwr);
        disp_pwr_bar(pwr_percent);
        // preset ABCD to-do
        //disp_pset_temp(temp);
        //disp_preset_show(preset);
        disp_tip_temp(tip_temp);
        disp_refresh();
        // ---
        //sleep_si(2);
        poll_chk_operations(); // 1 sec delay while keypad scanning & menu operations
    }
}
