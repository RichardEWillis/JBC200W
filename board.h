/* Define the board resources for enabled drivers and services
 *
 * This BSP is for the 200W-JBC Ver 3.1 (2025) with
 * a RP2040 PICO plugged in.
 * 
 * Display: SSD1309 connected to J3.
 * 
 */

#ifndef BOARD_H
#define BOARD_H

#include <pico_bsp.h>


/* JBC_PCB_BOARD_VER_3.1
 * Board configuration to support the JBC Iron Controller on a RP Pico 
 * mezzanine board.
 * 
 * PINOUT
 * 1  CONSOLE_TX (default UART)     40 VBUS
 * 2  CONSOLE_RX (default UART)     39 VSYS
 * 3  GND                           38 GND
 * 4  Keyboard R0                   37 3V3_EN
 * 5  Keyboard R1                   36 3V3_OUT
 * 6  Keyboard R2                   35 ADC_VREF
 * 7  Keyboard R3                   34 DISP_RESET
 * 8  GND                           33 GND
 * 9  Keyboard C0                   32 
 * 10 Keyboard C1                   31 ADC_TEMP
 * 11 Keyboard C2                   30 *RUN
 * 12 Keyboard C3                   29 IRON_ON_HOOK_DET
 * 13 GND                           28 GND
 * 14 AC_ZC_INPUT                   27 PSU_N16_CHRG_STATE
 * 15 HTR_CTRL_ON_L                 26 DISP_DC
 * 16 HTR_CTRL_OFF_L                25 DISP_SPI_MOSI
 * 17 PSU_P16_ON_L                  24 DISP_SPI_SCK
 * 18 GND                           23 GND
 * 19 PSU_N16_ON_L                  22 DISP_SPI_CS
 * 20 PSU_P16_CHRG_STATE            21 DISP_SPI_MISO_NC
 */

/* SSD1309 Graphics Driver , 128 x 64 */
#define DISP_DRVR_SPI_CHAN          spi_default
#define DISP_DRVR_SPI_CLK           PICO_DEFAULT_SPI_SCK_PIN
#define DISP_DRVR_SPI_MISO          PICO_DEFAULT_SPI_RX_PIN
#define DISP_DRVR_SPI_MOSI          PICO_DEFAULT_SPI_TX_PIN
#define DISP_DRVR_SPI_CS            PICO_DEFAULT_SPI_CSN_PIN
#define DISP_DRVR_SPI_CLK_FREQ_HZ   4000000UL   /* 4 MHz */
#define DISP_DRVR_SPI_GPIO_DC       GP20
#define DISP_DRVR_SPI_GPIO_RST      GP28

/* ** [GPIO] Keyboard ---------------------- */
#define KYBD_ROW0   GP2
#define KYBD_ROW1   GP3
#define KYBD_ROW2   GP4
#define KYBD_ROW3   GP5
#define KYBD_COL0   GP6
#define KYBD_COL1   GP7
#define KYBD_COL2   GP8
#define KYBD_COL3   GP9
//
#define KYBD_ROW_COUNT 4
#define KYBD_COL_COUNT 4
#define KYBD_DBTIME    4 /* Tscan ~ 10 msec*/
#define KYBD_BUFLEN    2
#define KYBD_SCAN_PD_MS 20

// external keyboard FIFO. Add key entries 
// using queue_push, remove keys using queue_pop
// The queue has an internal key-repeat check feature
// to prevent fast repeated key entries from occuring
#define KEYBUFFER_LEN           16
#define KEYBUFFER_TICK_PD_MS    KYBD_SCAN_PD_MS
#define KEYBUFFER_DLY_TMOUT_MS  200  /* key-repeat-delay [msec] */

/* ** [GPIO] ZeroCrossingAC ---------------- */
#define AC_ZC_INPUT GP10

/* ** [GPIO] HeaterControl ----------------- */
#define HTR_CTRL_ON_L    GP11  /* Enable heater power pulses */
#define HTR_CTRL_OFF_L   GP12  /* Heater shutoff - hold active while not heating */
#define HTR_CTL_OFF 1  /* not active */
#define HTR_CTL_ON  0  /* active     */

/* ** [GPIO] Analog Power Supply Controller  */
#define APSU_P16V_ON_L          GP13  /* [out] active (low) - charge +16V cap */
#define APSU_N16V_ON_L          GP14  /* [out] active (low) - charge -16V cap */
#define APSU_P16V_CHARGE_STATE  GP15  /* [in] active (low)  - +16V cap charged */
#define APSU_N16V_CHARGE_STATE  GP21  /* [in] active (low)  - -16V cap charged */
#define APSU_P16V_CHARGE_STATE_USE_PU 0 /* external PU resistor */
#define APSU_P16V_CHARGE_STATE_USE_PD 0
#define APSU_N16V_CHARGE_STATE_USE_PU 0 /* external PU resistor */
#define APSU_N16V_CHARGE_STATE_USE_PD 0
#define APSU_X16V_ENABLE        0
#define APSU_X16V_DISABLE       1
#define APSU_X16V_CHG_OVER      0
#define APSU_X16V_CHG_UNDER     1

/* ** [GPIO] OPTION - JBC IRON IN-CRADLE DETECT (active low) */
#define IRON_ONHOOK_DET_L       GP22
#define IRON_ONHOOK_DET_USE_PU  0 /* external PU resistor */
#define IRON_ONHOOK_DET_USE_PD  0
#define IRON_ONHOOK             0
#define IRON_OFFHOOK            1

/* ** [ADC]  Temp -------------------------- */
#define ADC_TEMP    GP26


/* System Definitions and Maximums */

#define IRON_MAX_TEMP           800 /* F */
#define IRON_MAX_WATT           200
#define MAX_TEMP_PRESETS        4   /* 'A', 'B', 'C', 'D' */

#endif /* BOARD_H */
