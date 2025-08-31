/******************************************************************************
 * Manage the Keypad
 * 
 * Background task operations and key buffer.
 * This firmware uses a non-blocking poll read, not block-waiting for keys.
 * 
 * 
 */

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

// Call to initialize the resources needed for keypad management.
int keypad_init(void);

// Call to start keypad polling  task operations.
int keypad_start(void);

// Stop the keypad task and cleanup. Call _start() to restart it.
int keypad_stop(void);

// get a key. Returns # buffered keys including the one 
// being returned. Places the returning key in 'c'.
// if returning 0 then the buffer is empty and there was
// no value placed into 'c'.
int keypad_get(char * c);

#endif /* _KEYPAD_H_ */
