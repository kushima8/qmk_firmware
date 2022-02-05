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
#define PRODUCT_ID      0x0006
#define DEVICE_VER      0x0001
#define MANUFACTURER    koron

#define PRODUCT         YUIOP60HH3 Rev.1
#define DESCRIPTION     Experimental Keyboard

// Key matrix size and pins
#define MATRIX_ROWS 5
#define MATRIX_COLS 15
#define MATRIX_ROW_PINS { F7, F6, F5, F4, F1 }
#define MATRIX_COL_PINS { B0, B7, D0, D1, D2, D3, D5, D4, D6, D7, B4, B5, B6, C6, C7 }
#define DIODE_DIRECTION COL2ROW

#define MATRIX_MASKED

// For input stability
#define DEBOUNCE 5

// RGB LED settings
#define RGB_DI_PIN E6
#define RGBLED_NUM 5
#define RGBLIGHT_ANIMATIONS
#define RGBLIGHT_LIMIT_VAL 200
#define RGBLIGHT_HUE_STEP 17
#define RGBLIGHT_SAT_STEP 17
#define RGBLIGHT_VAL_STEP 20

// RGB matrix settings
#ifdef RGB_MATRIX_ENABLE
# define DRIVER_LED_TOTAL 5
# define RGB_MATRIX_KEYPRESSES
# define RGB_MATRIX_FRAMEBUFFER_EFFECTS
# define RGB_MATRIX_MAXIMUM_BRIGHTNESS 200
#endif
