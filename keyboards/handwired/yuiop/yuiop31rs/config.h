/*
Copyright 2021 MURAOKA Taro (aka KoRoN, @kaoriya)

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
#define PRODUCT_ID 0x0008
#define DEVICE_VER 0x0001
#define MANUFACTURER koron

#define PRODUCT YUIOP31RS
#define DESCRIPTION Experimental Keyboard

// Key matrix size and pins
#define MATRIX_ROWS 6
#define MATRIX_COLS 7
#define MATRIX_ROW_PINS \
    { B4, E6, D7, C6, F6, F5 }
#define MATRIX_COL_PINS \
    { F4, F7, B1, B3, B2, B6, D4 }
#define DIODE_DIRECTION COL2ROW

// For input stability
#define DEBOUNCE 5

// Rotary switch
#define DIP_SWITCH_MATRIX_GRID           \
    {                                    \
        {2, 6}, {1, 6}, {3, 6}, { 0, 6 } \
    }
#define MATRIX_MASKED

// RGB LED common settings
#define RGB_DI_PIN B5

// RGB LED settings
#ifdef RGBLIGHT_ENABLE
#    define RGBLED_NUM 31
#    define RGBLIGHT_LIMIT_VAL 150
#    define RGBLIGHT_HUE_STEP 17
#    define RGBLIGHT_SAT_STEP 17
#    define RGBLIGHT_VAL_STEP 15
#    define RGBLIGHT_ANIMATIONS
#endif

// RGB MATRIX settings
#ifdef RGB_MATRIX_ENABLE
#    define DRIVER_LED_TOTAL 31
#    define RGB_MATRIX_KEYPRESSES
#    define RGB_MATRIX_FRAMEBUFFER_EFFECTS
#    define RGB_MATRIX_MAXIMUM_BRIGHTNESS 150
#endif
