/*
This is the c configuration file for the keymap

Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2015 Jack Humbert

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

// Detect hand configuration (left or right) by matrix.
#define SPLIT_HAND_MATRIX_GRID D7, B3
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_RIGHT

// Detect master/slave configuration by USB instead of VBUS.
#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 500

#define TAPPING_FORCE_HOLD
#define TAPPING_TERM 200

#undef RGBLED_NUM
#define RGBLIGHT_ANIMATIONS
#define RGBLED_NUM 54
#define RGBLIGHT_LIMIT_VAL 100
#define RGBLIGHT_HUE_STEP 17
#define RGBLIGHT_SAT_STEP 17
#define RGBLIGHT_VAL_STEP 10

// Use the lily version to get the uzu42 logo instead of the qmk logo
#define OLED_FONT_H "keymaps/default/glcdfont_uzu42.c"
