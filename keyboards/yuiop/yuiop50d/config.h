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
#define VENDOR_ID       0x7890
#define PRODUCT_ID      0x0003
#define DEVICE_VER      0x0001
#define MANUFACTURER    koron

#define PRODUCT         YUIOP50D Rev.1
#define DESCRIPTION     Experimental Keyboard

// Key matrix size and pins
#define MATRIX_ROWS 5
#define MATRIX_COLS 11
#define MATRIX_ROW_PINS { F7, B1, B3, B2, B6 }
#define MATRIX_COL_PINS { F4, D1, F5, D0, F6, D4, C6, D7, E6, B4, B5 }
#define DIODE_DIRECTION COL2ROW

// For input stability
#define DEBOUNCE 5

// RGB LED settings
#define RGB_DI_PIN D3
#define RGBLED_NUM 50
#define RGBLIGHT_ANIMATIONS
#define RGBLIGHT_LIMIT_VAL 100
#define RGBLIGHT_HUE_STEP 17
#define RGBLIGHT_SAT_STEP 17
#define RGBLIGHT_VAL_STEP 10

// RGB matrix settings
#ifdef RGB_MATRIX_ENABLE
# define DRIVER_LED_TOTAL 50
# define RGB_MATRIX_KEYPRESSES
# define RGB_MATRIX_FRAMEBUFFER_EFFECTS
# define RGB_MATRIX_MAXIMUM_BRIGHTNESS 100
#endif
