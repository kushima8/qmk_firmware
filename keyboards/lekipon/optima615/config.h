/*
Copyright 2021 Lekipon

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "config_common.h"

/* USB Device descriptor parameter */
#define VENDOR_ID 0xFEED
#define PRODUCT_ID 0x0615
#define DEVICE_VER 0x0001
#define MANUFACTURER Lekipon
#define PRODUCT OPTIMA615
#define DESCRIPTION A custom keyboard

/* key matrix size */
#define MATRIX_ROWS 16
#define MATRIX_COLS 5

/*
 * Keyboard Matrix Assignments
 *
 * Change this to how you wired your keyboard
 * COLS: AVR pins used for columns, left to right
 * ROWS: AVR pins used for rows, top to bottom
 * DIODE_DIRECTION: COL2ROW = COL = Anode (+), ROW = Cathode (-, marked on diode)
 *                  ROW2COL = ROW = Anode (+), COL = Cathode (-, marked on diode)
 *
 */
#define MATRIX_ROW_PINS { B5, B4, E6, D7, C6, D4, D2, F6 }
#define MATRIX_COL_PINS { F7, B1, B3, B2, B6 }
#define UNUSED_PINS

/* COL2ROW, ROW2COL*/
/*#define DIODE_DIRECTION COL2ROW*/

/* Rotary encoder define*/
#define ENCODERS_PAD_A { F4 }
#define ENCODERS_PAD_B { F5 }

/* RGB BackLights define*/
#define RGB_DI_PIN D3
#ifdef RGB_DI_PIN
#define RGBLED_NUM 88
#define RGBLIGHT_HUE_STEP 4
#define RGBLIGHT_SAT_STEP 8
#define RGBLIGHT_VAL_STEP 8
#define RGBLIGHT_LIMIT_VAL 127 /* The maximum brightness level */
#define RGBLIGHT_SLEEP  /* If defined, the RGB lighting will be switched off when the host goes to sleep */
#define RGBLIGHT_ANIMATIONS
/*== customize RGB Animation effect ==*/
   #define RGBLIGHT_EFFECT_TWINKLE_LIFE 127
   #define RGBLIGHT_RAINBOW_SWIRL_RANGE 127
   #define RGBLIGHT_EFFECT_CHRISTMAS_STEP 1
/*== customize breathing effect ==*/
//   #define RGBLIGHT_BREATHE_TABLE_SIZE 64     // 256(default) or 128 or 64
//   #define RGBLIGHT_EFFECT_BREATHE_CENTER 1.2  // 1 to 2.7
//   #define RGBLIGHT_EFFECT_BREATHE_MAX    96   // 0 to 255
#endif

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#define DEBOUNCE 5

/* define if matrix has ghost (lacks anti-ghosting diodes) */
//#define MATRIX_HAS_GHOST

/* Mechanical locking support. Use KC_LCAP, KC_LNUM or KC_LSCR instead in keymap */
#define LOCKING_SUPPORT_ENABLE
/* Locking resynchronize hack */
#define LOCKING_RESYNC_ENABLE
