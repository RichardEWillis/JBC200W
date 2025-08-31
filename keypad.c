/******************************************************************************
 * Manage the Keypad
 * 
 * Background task operations and key buffer.
 * This firmware uses a non-blocking poll read, not block-waiting for keys.
 * 
 * 
 */

#include "pico/critical_section.h"
#include <keyboard-gpio.h>
#include <board.h>

static void * kybd_hndl = NULL; /* keyboard object handle */
static char   keybuf[KEYBUFFER_LEN+1];
static int    keycount = 0;
static int    repeat_timer = 0;
static char   lastkey = 0;
static bool   keytask_running = false;
static repeating_timer_t kbtmr;
critical_section_t keybrd_queue;


// ***************************************************************************
// bottom-end functions for supporting keypad scanning and key buffering
// including a higher level key repeat limiter.
// ***************************************************************************

// call from a timer routine to increment timer if running.
static void keybrd_queue_tick(void) {
    critical_section_enter_blocking(&keybrd_queue);
    if (repeat_timer) {
        repeat_timer += KEYBUFFER_TICK_PD_MS;
    }
    critical_section_exit(&keybrd_queue);
}

static void keybrd_queue_push(char c) {
    critical_section_enter_blocking(&keybrd_queue);
    if (c == lastkey) {
        // manage key-repeat timing
        if (repeat_timer) {
            if (repeat_timer >= KEYBUFFER_DLY_TMOUT_MS) {
                repeat_timer = 0;
            }
        } else {
            repeat_timer = KEYBUFFER_TICK_PD_MS;
        }
    } else {
        repeat_timer = 0; // different key, kill the repeat delay if running
    }
    if (repeat_timer == 0) {
        // timeout or unique keypress, can add the key
        if (keycount < KEYBUFFER_LEN) {
            keybuf[keycount] = c;
            keycount ++;
            keybuf[keycount] = '\0';
            //putchar((int)c);
        }
    }
    lastkey = c;
    critical_section_exit(&keybrd_queue);
}

// ** TASK **
static bool chk_keyboard(repeating_timer_t * rptdata) {
    // Put your timeout handler code in here
    keybrd_queue_tick();
    if (kybd_hndl) {
        int rc = keyboard_poll(kybd_hndl);
        if (rc > 0) {
            char ck;
            while (rc) {
                rc = keyboard_getkey(kybd_hndl, &ck);
                keybrd_queue_push(ck);
            }
        }
    }
    return 1; // set to 0/false to stop the r-timer
}


// ***************************************************************************
// top-end functions for supporting keypad scanning and key buffering
// this block waits on an simple character queue for one or more chars
// until a timeout occurs.
// ***************************************************************************

// get buffered keycount, safely
// value returned is a guaranteed buffered key count
// returning 0 means key buffer is empty
static int keybrd_get_keycount(void) {
    int rc = 0;
    critical_section_enter_blocking(&keybrd_queue);
    rc = keycount;
    critical_section_exit(&keybrd_queue);
    return rc;
}

// pull single character from keybuf queue
// returns keycount remaining.
static int keybrd_queue_pop_c(char * c) {
    int rc = 0;
    critical_section_enter_blocking(&keybrd_queue);
    if (c) {
        int i;
        *c = keybuf[0];
        for ( i = 1 ; i < keycount ; i++ ) {
            keybuf[i-1] = keybuf[i]; /* copy down, FIFO style */
        }
        keybuf[--keycount] = 0; /* not required but keeps the buffer clean for debugging */
        rc = keycount;
    }
    critical_section_exit(&keybrd_queue);
    return rc;
}


// ***************************************************************************
// public methods
// ***************************************************************************

// Call to initialize the resources needed for keypad management.
int keypad_init(void) {
    if (!kybd_hndl) {
        kybd_hndl = keyboard_map_create(KYBD_ROW_COUNT, KYBD_COL_COUNT, KYBD_DBTIME, KYBD_BUFLEN);
        if (kybd_hndl) {
            keyboard_assign_row_gpio(kybd_hndl, 0, KYBD_ROW0);
            keyboard_assign_row_gpio(kybd_hndl, 1, KYBD_ROW1);
            keyboard_assign_row_gpio(kybd_hndl, 2, KYBD_ROW2);
            keyboard_assign_row_gpio(kybd_hndl, 3, KYBD_ROW3);
            keyboard_assign_col_gpio(kybd_hndl, 0, KYBD_COL0);
            keyboard_assign_col_gpio(kybd_hndl, 1, KYBD_COL1);
            keyboard_assign_col_gpio(kybd_hndl, 2, KYBD_COL2);
            keyboard_assign_col_gpio(kybd_hndl, 3, KYBD_COL3);
            keyboard_key_assign(kybd_hndl, 0, 0, '1');
            keyboard_key_assign(kybd_hndl, 0, 1, '2');
            keyboard_key_assign(kybd_hndl, 0, 2, '3');
            keyboard_key_assign(kybd_hndl, 0, 3, 'A');
            keyboard_key_assign(kybd_hndl, 1, 0, '4');
            keyboard_key_assign(kybd_hndl, 1, 1, '5');
            keyboard_key_assign(kybd_hndl, 1, 2, '6');
            keyboard_key_assign(kybd_hndl, 1, 3, 'B');
            keyboard_key_assign(kybd_hndl, 2, 0, '7');
            keyboard_key_assign(kybd_hndl, 2, 1, '8');
            keyboard_key_assign(kybd_hndl, 2, 2, '9');
            keyboard_key_assign(kybd_hndl, 2, 3, 'C');
            keyboard_key_assign(kybd_hndl, 3, 0, '*');
            keyboard_key_assign(kybd_hndl, 3, 1, '0');
            keyboard_key_assign(kybd_hndl, 3, 2, '#');
            keyboard_key_assign(kybd_hndl, 3, 3, 'D');
        }
    }
    return (kybd_hndl == NULL); // 0 := SUCCESS
}

// Call to start keypad polling  task operations.
int keypad_start(void) {
    int rc = 1;
    if (kybd_hndl && !keytask_running) {
        keytask_running = add_repeating_timer_ms(KYBD_SCAN_PD_MS, chk_keyboard, NULL, &kbtmr);
        rc = (keytask_running == false); // 0 := SUCCESS
    }
    return rc;
} 

// Stop the keypad task and cleanup. Call _start() to restart it.
int keypad_stop(void) {
    int rc = 1;
    if (kybd_hndl && keytask_running) {
        keytask_running = ! cancel_repeating_timer(&kbtmr);
        rc = (keytask_running == true); // 0 := SUCCESS
    }
    return rc;
}

// get a key. Returns # buffered keys including the one 
// being returned. Places the returning key in 'c'.
// if returning 0 then the buffer is empty and there was
// no value placed into 'c'.
int keypad_get(char * c) {
    int rc = 0;
    if (keytask_running && c) {
        rc = keybrd_get_keycount();
        if (rc) {
            keybrd_queue_pop_c(c);
        }
    }
    return rc; // # buffered key incl. returning key.
}

