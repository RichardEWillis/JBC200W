/******************************************************************************
 * Decode Keypad based Operations
 * 
 * Decode keypad based operations
 * Internal operation calls are done within the codebase.
 * Implement this as a set of state tables, where the starting char is passed 
 * into each table until one table claims ownership and sets up the first
 * state in a set of state changes. State changes step for each new character
 * until the state chain is complete or its cancelled or an invalid key is
 * entered. Regardless the chain completes and the step is reset back to 
 * the initial examination phase again.
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

#include <operations.h>
#include <board.h>
#include <display.h>
#include <stdio.h>
#include <math.h>


/* State Tree
 *
 * ? +--> '#' +--> [A,B,C,D] +--> dig[1..3],'#' --> value --> change SET [A,B,C,D],<value>
 *   |        |              |
 *   |        |              +--> '*' (CANCEL or SETTING CLEARED)
 *   |        |
 *   |        +--> 1 --> [C,D] --> Set Scale [C,D] C:=Celcius, D:=Farenheit
 *   |        |
 *   |        +--> 2 --> +--> dig[1..3],'#' --> value --> change Sleep Delay <value> [sec]
 *   |                   |
 *   |                   +--> '*' (CANCEL or RESET TO DEFAULT)
 *   |
 *   +--> [A,B,C,D] --> Select preset [A,B,C,D]. Ignore if unset
 *   |
 *   +--> '*' --> Toggle manual sleep/wake
 * 
 * (future)
 * '1' dec +1  temp
 * '4' dec +10 temp
 * '7' dec +50 temp
 * '2' inc +1  temp
 * '5' inc +10 temp
 * '8' inc +50 temp
 */

// System Operation Settings
static uint32_t setTempPoint  = IRON_START_TEMP;        // target temp. change manually or use a preset
static char     tempUnits     = IRON_START_SCALE;       // temperature range, 'C' := Celcius, 'F' := Farenheit
static bool     sw_isWoken    = true;                   // wake ~ Heating, sleeping ~ Cooling
static uint32_t setSleepDelay = SLEEP_DELAY_DEFAULT;

// The state function protype (parent type)
typedef void * (*stateFunction)(char); // returns the next state, cast to (stateFunction). If NULL then abort.

static uint32_t digs_to_val(char * digs, uint8_t digCount) {
    // digs[] is an array of numerical characters {'0' .. '9'} not numbers!
    uint32_t val = 0;
    while (digCount) {
        digCount --;
        val += (uint32_t)pow(10,digCount) * ((*digs) - '0');
        digs ++;
    }
    return val;
}


// ****** States for Temp Set/Clr *********************************************

#define TEMP_PRESET_COUNT 4
typedef struct s_tempPreset_type {
    char     presetChar;
    uint8_t  isValid;
    uint32_t setTemp;
} s_tempPreset_t;
static s_tempPreset_t tempPresets[TEMP_PRESET_COUNT] = {0};

static void init_temp_presets(void) {
    char presetLetter = 'A';
    size_t i;
    for (i = 0 ; i < TEMP_PRESET_COUNT ; i++) {
        tempPresets[i].presetChar = presetLetter;
        tempPresets[i].isValid = 0;
        tempPresets[i].setTemp = 0;
        presetLetter ++;
    }
}

// private stateful context data for sf_ts
#define TEMPSET_DIG_COUNT 3
typedef struct sf_tempSetData_type {
    char setCode;       // A,B,C or D
    char digits[TEMPSET_DIG_COUNT];     // the 3 entered numerical digits
    uint32_t digidx;    // next location in 'digits'
    uint32_t temp;      // decode temp
} sf_tempSetData_t;
static sf_tempSetData_t sf_tempData = {0};

static void * sf_ts_invoke(char k) {
    // ignore 'k', process the context data
    sf_tempData.temp = digs_to_val(sf_tempData.digits, sf_tempData.digidx);
    // TODO - Call method to change temp setting
    size_t idx = (size_t)(sf_tempData.setCode - 'A'); // convert code to index where 'A' := 0, 'B' := 1 etc.
    printf("*** [sf_ts_invoke] * Set Temp [%c] = %u, idx[%u]\n", sf_tempData.setCode, sf_tempData.temp, idx);
    tempPresets[idx].isValid = 1;
    tempPresets[idx].setTemp = sf_tempData.temp;
    return NULL; // end of the state chain
}

static void * sf_ts_wt_vals(char k) {
    if (sf_tempData.digidx < TEMPSET_DIG_COUNT) {
        if (k >= '0' && k <= '9') {
            sf_tempData.digits[sf_tempData.digidx++] = k;
        } else if (k == '#') {
            return sf_ts_invoke('#'); // call now, return the result of the call.
        }
    }
    if (k == '#') {
        return sf_ts_invoke('#');
    }
    if (k == '*') {
        if (sf_tempData.digidx == 0) {
            size_t idx = (size_t)(sf_tempData.setCode - 'A'); // convert code to index where 'A' := 0, 'B' := 1 etc.
            printf("*** [sf_ts_wt_vals] * Setting [%c] CLEARED/UNSET, idx[%u]\n", sf_tempData.setCode, idx);
            tempPresets[idx].isValid = 0;
        } else {
            printf("*** [sf_ts_wt_vals] * Operation Cancelled\n");
        }
        return NULL; // cancelled.
    }
    return sf_ts_wt_vals;
}


// ****** States for Scale Set ************************************************

static void * sf_sset_chk_scale(char k) {
    if (k == 'C') {
        printf("*** [sf_sset_chk_scale] * Set Scale :: Celcius\n");
        tempUnits = 'C';
    } else if (k == 'D') {
        printf("*** [sf_sset_chk_scale] * Set Scale :: Farenheit\n");
        tempUnits = 'F';
    } else {
        printf("*** [sf_sset_chk_scale] * Set Scale :: INVALID KEY (ignored)\n");
    }
    disp_settemp_scale(tempUnits);
    disp_refresh();
    return NULL;
}


// ****** States for Sleep Delay **********************************************

#define SLPDLY_DIG_COUNT 3
typedef struct sf_slpDlyData_type {
    char digs[SLPDLY_DIG_COUNT];
    uint8_t digidx;
    uint32_t sleepDelaySecs;
} sf_slpDlyData_t;
static sf_slpDlyData_t sf_slpdlyData;

void * sf_slpdly_invoke(char k) {
    // ignore k, just process data
    sf_slpdlyData.sleepDelaySecs = digs_to_val(sf_slpdlyData.digs, sf_slpdlyData.digidx);
    // TODO - Call method to change temp setting
    printf("*** [sf_slpdly_invoke] * Sleep delay = %u\n", sf_slpdlyData.sleepDelaySecs);
    setSleepDelay = sf_slpdlyData.sleepDelaySecs;
    return NULL; // end of the state chain
}

void * sf_slpdly_wt_vals(char k) {
    if (sf_slpdlyData.digidx < SLPDLY_DIG_COUNT) {
        if (k >= '0' && k <= '9') {
            sf_slpdlyData.digs[sf_slpdlyData.digidx++] = k;
        }
    }
    if (k == '#') {
        return sf_slpdly_invoke('#');
    }
    if (k == '*') {
        if (sf_slpdlyData.digidx == 0) {
            printf("*** [sf_slpdly_wt_vals] * Sleep Delay RESET TO DEFAULT\n");
            setSleepDelay = SLEEP_DELAY_DEFAULT;
        } else {
            printf("*** [sf_slpdly_wt_vals] * Operation Cancelled\n");
        }
        return NULL; // cancelled.
    }
    return sf_slpdly_wt_vals;
}


// ****** States for Manual Sleep/Wake ****************************************


void * sf_slpWake(void) {
    if (sw_isWoken) {
        sw_isWoken = false;
        printf("*** [sw_isWoken] * going to sleep\n");
        disp_cool_on();
    } else {
        sw_isWoken = true;
        printf("*** [sw_isWoken] * waking up\n");
        disp_heat_on();
    }
    disp_refresh();
    return NULL;
}

// ****** States for Select Preset ********************************************

void * sf_selectPreset(char k) {
    size_t idx = (size_t)(k - 'A'); // convert code to index where 'A' := 0, 'B' := 1 etc.
    printf("*** [sf_selectPreset] * Checking preset index[%u]...\n", idx);
    if (idx < TEMP_PRESET_COUNT && tempPresets[idx].isValid) {
        printf("*** [sf_selectPreset] * Changing temp preset to Setting [%c] T=[%u]\n", k, tempPresets[idx].setTemp);
        setTempPoint = tempPresets[idx].setTemp; // cache it, as this can be manually changed.
        disp_preset_show(k);
        disp_pset_temp(setTempPoint);
        disp_refresh();
    } else {
        printf("*** [sf_selectPreset] * Preset not SET or selection invalid.\n");
    }
    return NULL;
}

// ****** States for manual Temp Change ***************************************

void * sf_dec_temp(int val) {
    setTempPoint -= val;
    disp_preset_show(' '); // temp now under manual control
    disp_pset_temp(setTempPoint);
    disp_refresh();
    printf("*** [sf_dec_temp] * manual temp change to [%u]\n", setTempPoint);
    return NULL;
}

void * sf_inc_temp(int val) {
    setTempPoint += val;
    disp_preset_show(' '); // temp now under manual control
    disp_pset_temp(setTempPoint);
    disp_refresh();
    printf("*** [sf_inc_temp] * manual temp change to [%u]\n", setTempPoint);
    return NULL;
}

// ****** States for Menu Selection *******************************************

static void * sf_menu_chk(char k) {
    void * rc = NULL;
    switch (k) {
    case '1':
        // Change Temp Scale
        rc = sf_sset_chk_scale;
        break;
    case '2':
        // Change Sleep Delay
        sf_slpdlyData.digidx = 0;
        rc = sf_slpdly_wt_vals;
        break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
        // Change Temp Setting
        sf_tempData.setCode = k;
        sf_tempData.digidx = 0;
        rc = sf_ts_wt_vals;
        break;
    case '*':
    default:
        // ignore
        printf("*** [sf_menu_chk] * Invalid key, ignored: %c\n", k);
        rc = NULL;
    }
    return rc;
}

static void * sf_menu_init(char k) {
    void * rc = NULL;
    switch (k) {
    case '#':
        rc = sf_menu_chk;
        break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
        rc = sf_selectPreset(k);
        break;
    case '*':
        rc = sf_slpWake();
        break;
    case '1':
        rc = sf_dec_temp(1);
        break;
    case '4':
        rc = sf_dec_temp(10);
        break;
    case '7':
        rc = sf_dec_temp(50);
        break;
    case '2':
        rc = sf_inc_temp(1);
        break;
    case '5':
        rc = sf_inc_temp(10);
        break;
    case '8':
        rc = sf_inc_temp(50);
        break;
    default:
        rc = NULL;
    }
    return rc;
}


// ***************************************************************************
// Stateful parameters
// ***************************************************************************

// The initial steps of every state space
static stateFunction sf_init[] = {
    sf_menu_init,
    // TO-DO : add as they are created
    NULL /* end of list */
};

// track the next state to call
static stateFunction next_State = NULL;


// ***************************************************************************
// public methods
// ***************************************************************************

// Setup Operations
int ops_init(void) {
    next_State = NULL;
    init_temp_presets();
    disp_pset_temp(setTempPoint);
    disp_settemp_scale(tempUnits);
    if (sw_isWoken)
        disp_heat_on();
    else
        disp_cool_on();
    disp_refresh();
    return 0;
}

// Reset internal Operations
// Call after a poll returns a non-zero result
int ops_reset(void) {
    next_State = NULL;
    return 0;
}

// Poll Operations - push in a new key
// Returns: 
//  0 OK/Normal
//  1 Error Occured
int ops_poll(char k) {
    int rc = 1;
    if (next_State) {
        next_State = (stateFunction)next_State(k);
        rc = 0; // stepping ok
    } else {
        size_t i = 0;
        stateFunction chk = sf_init[i];
        while (chk) {
            next_State = chk(k);
            if (next_State) {
                rc = 0; // started stepping ok
                break;
            }
            chk = sf_init[++i];
        }
    }
    return rc;
}

// current temp setting for iron
uint32_t get_tipTempSetting(void) {
    return setTempPoint;
}

// current temp scale ('C' | 'F')
uint32_t get_tempScale(void) {
    return tempUnits;
}

// get wake status, true := running and heating
bool get_wakeStatus(void) {
    return sw_isWoken;
}

// get delay before sleeping
uint32_t get_sleepDelay(void) {
    return setSleepDelay;
}
