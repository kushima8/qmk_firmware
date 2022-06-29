/*
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

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

// USB Device descriptor parameter
#define VENDOR_ID 0x7890
#define PRODUCT_ID 0x000b
#define DEVICE_VER 0x0001
#define MANUFACTURER koron

#define PRODUCT YUIOP32DM
#define DESCRIPTION Experimental Keyboard

// Key matrix size and pins
#define MATRIX_ROWS 4
#define MATRIX_COLS (4 * 2) // duplex matrix
#define MATRIX_ROW_PINS \
    { D7, E6, B4, B5 }
#define MATRIX_COL_PINS \
    { B1, B3, B2, B6 }
#define DEBOUNCE 5

// RGB LED common settings
#define RGB_DI_PIN D3

// RGB LED settings
#ifdef RGBLIGHT_ENABLE
#    define RGBLED_NUM 32
#    define RGBLIGHT_LIMIT_VAL 255
#    define RGBLIGHT_HUE_STEP 17
#    define RGBLIGHT_SAT_STEP 17
#    define RGBLIGHT_VAL_STEP 17
#    define RGBLIGHT_ANIMATIONS
#endif

// RGB MATRIX settings
#ifdef RGB_MATRIX_ENABLE
#    define DRIVER_LED_TOTAL 32
#    define RGB_MATRIX_KEYPRESSES
#    define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#    define RGB_MATRIX_MAXIMUM_BRIGHTNESS 255
#endif

// To squeeze firmware size
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE
#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION
