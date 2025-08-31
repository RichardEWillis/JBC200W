/******************************************************************************
 * Manage the Display
 * 
 * Configure the arrangement of the display
 * Declare methods to change the display
 * Add a poll() method to refresh the display
 * 
 */

#include <display.h>
#include <gfxDriverLowPriv.h>
#include <linegfx.h>
#include <textgfx.h>
#include <led_overlay.h>
#include <jbc_util.h>
#include <board.h>  /* system limits */

/* Screen Setup - START */
/* for now this is a simple 2 lines of text, can improve later.
 * see board.h for text lines to display.
 */

/* Screen Setup - Operations */
    /* large 7-segment display */
#define TEMP_LED_DIGCOUNT   3       /* 3 digit display: 000 .. 999  */
#define TEMP_LED_TL_POS_X   4       /* top-left position (X)        */
#define TEMP_LED_TL_POS_Y   8       /* top-left position (Y)        */
#define TEMP_LED_UPDT_ON_CHG 0      /* do not update if changed, use a "global" refresh */
    /* Setting (A,B,C,D) text position */
#define PRESET_TEXT_LINE    1       /* 2nd line down */
#define PRESET_TEXT_XPOS    16      /* 17th char position, from 21  */
    /* temp preset text */
#define PRESET_TXT_TMP_LN   2       /* 3rd line down */
#define PRESET_TXT_TMP_XP   PRESET_TEXT_XPOS
    /* heat indicator (*) */
#define HEAT_IND_LINE       3       /* 4th line down (out of 8 )    */
#define HEAT_IND_XPOS       PRESET_TEXT_XPOS
    /* cool indicator (*) */
#define COOL_IND_LINE       4       /* 5th line down (out of 8 )    */
#define COOL_IND_XPOS       PRESET_TEXT_XPOS
    /* wattage value (xxx) */
#define WATT_TEXT_LINE      6
#define WATT_TEXT_XPOS      11
    /* Power BAR indicator */
#define PWR_BAR_TL_X        4       /* try to line up with Wattage text*/
#define PWR_BAR_TL_Y        48
#define PWR_BAR_H           8
#define PWR_BAR_W           50
#define PWR_BAR_BOX_X1      3
#define PWR_BAR_BOX_Y1      47
#define PWR_BAR_BOX_X2      55
#define PWR_BAR_BOX_Y2      56
    /* Line art (linegfx )*/
#define BRDR_X1             0
#define BRDR_Y1             4
#define BRDR_X2             127
#define BRDR_Y2             43
#define LIN1_X1             62
#define LIN1_Y1             4
#define LIN1_X2             62
#define LIN1_Y2             43

// Layers (4 total)
#define LAYER_GFX           3
#define LAYER_TXT           1
#define LAYER_LED           2

/* Screen Setup - Settings */
/* To - do
 *
 */

 /* Board info */
#include <version.h>
// Building Board Info String      :: HW_BOARD_STRN + " Ver. " + HW_VER_STRN + '\n'
// Building f/w release info string:: "F/W Ver " + P_VER_STRN + '\n' + P_VER_REL_STRN + '\n'

#define REPORT_BRD_INFO \
    textgfx_puts(HW_BOARD_STRN); \
    textgfx_puts(" Ver. "); \
    textgfx_puts(HW_VER_STRN); \
    textgfx_putc('\n'); 

#define REPORT_FW_VERSION \
    textgfx_puts("F/W Ver "); \
    textgfx_puts(P_VER_STRN); \
    textgfx_putc('\n'); \
    textgfx_puts(P_VER_REL_STRN); \
    textgfx_putc('\n');

/* |         
 * | *******  PRESET
 * | **LED**  TEMP
 * | *******  HEAT
 * | *******  COOL
 * |
 * | (bar)       W
 * |
 * 
 * "PRESET" Text X-pos start-char = 11
 */
static const char op_txt_overlay[] = "\n           PSET A\n           TEMP 300\n           HEAT *\n           COOL\n\n              W\n";


static bool is_initialized = false;
static void * led_hndl = NULL;

int disp_init(void) {
    if (!is_initialized) {
        bsp_ConfigureGfxDriver();   // as defined by GFX_DRIVER_LL_STACK
        bsp_StartGfxDriver();
        gfx_displayOn();
        gfx_clearDisplay();
        lgfx_init(LAYER_GFX);
        text_init(LAYER_TXT);
        textgfx_init(REFRESH_ON_DEMAND, SET_TEXTWRAP_ON);
        led0_init(LAYER_LED);
        lgfx_visibility(0);
        ledo_visible(0); // initially set invisible
        led_hndl = ledo_open(TEMP_LED_TL_POS_X, TEMP_LED_TL_POS_Y, TEMP_LED_DIGCOUNT, 298, TEMP_LED_UPDT_ON_CHG);
        is_initialized = true;
    }
}

int disp_startscrn(void) {
    int rc = 1;
    if (is_initialized) {
        textgfx_clear();
        REPORT_BRD_INFO;
        REPORT_FW_VERSION;
        textgfx_puts("FRAXSYS ENG.\n");
        rc = disp_refresh();
    }
    return rc;
}

static void add_border_gfx(void) {
    lgfx_box(BRDR_X1, BRDR_Y1, BRDR_X2, BRDR_Y2, COLOUR_BLK);
    lgfx_line(LIN1_X1, LIN1_Y1, LIN1_X2, LIN1_Y2, COLOUR_BLK);
    lgfx_visibility(1);
}

int disp_opscrn(void) {
    int rc = 1;
    if (is_initialized) {
        textgfx_clear();
        textgfx_puts(op_txt_overlay);
        textgfx_refresh();
        ledo_visible(1); // make visible
        ledo_refresh(led_hndl); // currently also calls the compositor which needs to be straightened out.
        // border graphics (line art)
        add_border_gfx();
        // Add Powerbar
        disp_pwr_bar(50);
        // Test wattage text update
        disp_pwr_txt(103);
        rc = disp_refresh();
    }
    return rc;
}

int disp_setscrn(void) {
    int rc = 1;
    if (is_initialized) {
    }
    return rc;
}

// update the active preset (A,B,C,D)
int disp_preset_show(char P) {
    textgfx_cursor(PRESET_TEXT_XPOS, PRESET_TEXT_LINE);
    textgfx_putc(P);
    return 0;
}

// update tip temperature (LED disp)
int disp_tip_temp(int T) {
    int rc = 1;
    if (T >= 0 && T <= IRON_MAX_TEMP) {
        rc = ledo_update(led_hndl, (uint32_t)T);
        ledo_refresh(led_hndl); // currently also calls the compositor which needs to be straightened out.
    }
    return rc;
}

// update preset set-temp
#define TEMP_PSET_CHAR_LEN  3
static char temp_pset[TEMP_PSET_CHAR_LEN+1];
int disp_pset_temp(int T) {
    int rc = 1;
    if (T >= 0 && T <= IRON_MAX_TEMP) {
        if (i_to_strflen((uint32_t)T, temp_pset, TEMP_PSET_CHAR_LEN+1, TEMP_PSET_CHAR_LEN) != NULL) {
            textgfx_cursor(PRESET_TXT_TMP_XP, PRESET_TXT_TMP_LN);
            textgfx_puts(temp_pset);
            rc = 0;
        }
    }
    return rc;
}   

static bool is_heating = false;
static int update_heat_cool(void) {
    textgfx_cursor(HEAT_IND_XPOS, HEAT_IND_LINE);
    textgfx_putc( (is_heating) ? '*' : ' ' );
    textgfx_cursor(COOL_IND_XPOS, COOL_IND_LINE);
    textgfx_putc( (is_heating) ? ' ' : '*' );
    return 0;
}

// indicate heating
int disp_heat_on(void) {
    is_heating = true;
    return update_heat_cool();
}

// indicate cooling
int disp_cool_on(void) {
    is_heating = false;
    return update_heat_cool();
}

// update power bar (%)
int disp_pwr_bar(int percent) {
    int rc = 1;
    if (percent >= 0 && percent <= 100) {
        float pdiv = 100.0 / percent;
        uint8_t plen = (uint8_t)((float)PWR_BAR_W / pdiv);
        // power bar-graph and surrounding box/border
        lgfx_box(PWR_BAR_BOX_X1, PWR_BAR_BOX_Y1, PWR_BAR_BOX_X2, PWR_BAR_BOX_Y2, COLOUR_BLK);
        rc = lgfx_bgraph(PWR_BAR_TL_X, PWR_BAR_TL_Y, PWR_BAR_H, PWR_BAR_W, plen, COLOUR_BLK);
    }
    return rc;
}

// update power numerical text (*** W)
#define PWR_WATTAGE_CHAR_LEN  3
static char pwr_wattage[PWR_WATTAGE_CHAR_LEN+1];
int disp_pwr_txt(int P) {
    int rc = 1;
    if (P >= 0) {
        if (i_to_strflen((uint32_t)P, pwr_wattage, PWR_WATTAGE_CHAR_LEN+1, PWR_WATTAGE_CHAR_LEN) != NULL) {
            textgfx_cursor(WATT_TEXT_XPOS, WATT_TEXT_LINE);
            textgfx_puts(pwr_wattage);
            rc = 0;
        }
    }
    return rc;
}

int disp_refresh(void) {
    // for now, call the text graphic refresh function 
    // as it is calling the compositor after updating it's
    // own text framebuffer. This needs to get
    // straightened out as it's still working in the 
    // old non-layered way...
    //return gfx_displayRefresh();
    return textgfx_refresh();
}


