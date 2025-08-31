/******************************************************************************
 * Manage the Display
 * 
 * Configure the arrangement of the display
 * Declare methods to change the display
 * Add a poll() method to refresh the display
 * 
 * 
 * 
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

// Display intiialization, call first.
int disp_init(void);

// show display 
int disp_startscrn(void);           // display the start screen (ver info etc.) needs update!
int disp_opscrn(void);              // display the normal operation screen
int disp_setscrn(void);             // display the settings screen

// Future:
int disp_preset_show(char P);       // update the active preset (A,B,C,D)
int disp_tip_temp(int T);           // update tip temperature (LED disp)
int disp_pset_temp(int T);          // update preset set-temp
int disp_heat_on(void);           // indicate heating
int disp_cool_on(void);           // indicate cooling
int disp_pwr_bar(int percent);      // update power bar (%)
int disp_pwr_txt(int P);            // update power numerical text (*** W)
int disp_refresh(void);             // refresh display

#endif /* _DISPLAY_H_ */
