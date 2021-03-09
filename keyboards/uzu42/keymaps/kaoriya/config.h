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

#define KAORIYA_FIRMWARE_VERSION r1

#undef PRODUCT
#define PRODUCT uzu42 +KaoriYa/KAORIYA_FIRMWARE_VERSION

// Minimal layer state for small binary
#define LAYER_STATE_8BIT

// Detect hand configuration (left or right) by matrix.
#define SPLIT_HAND_MATRIX_GRID D7, B3
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_RIGHT
#define MATRIX_MASKED

// Detect master/slave configuration by USB instead of VBUS.
#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 500

#define TAPPING_FORCE_HOLD
#define TAPPING_TERM 200

#undef RGBLED_NUM
#define RGBLIGHT_ANIMATIONS
#define RGBLED_NUM 54
#define RGBLIGHT_LIMIT_VAL 120
#define RGBLIGHT_HUE_STEP 17
#define RGBLIGHT_SAT_STEP 17
#define RGBLIGHT_VAL_STEP 12

// Disable some QMK Features as default.
#define NO_ACTION_ONESHOT
#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION

#define OLED_FONT_H "keymaps/default/glcdfont_uzu42.c"
