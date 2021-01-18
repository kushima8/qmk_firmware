/*
This is the c configuration file for the keymap

Copyright 2021 MURAOKA Taro <koron.kaoriya@gmail.com>

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
#define PRODUCT DZ60RGB +KaoriYa/KAORIYA_FIRMWARE_VERSION

#define LAYER_STATE_8BIT

#define TAPPING_FORCE_HOLD
#define TAPPING_TERM 200

#undef DEBOUNCE
#define DEBOUNCE 5

#define NO_ACTION_ONESHOT
#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION

#ifdef RGB_MATRIX_ENABLE
#    define RGB_MATRIX_MAXIMUM_BRIGHTNESS 100
#    define RGB_MATRIX_HUE_STEP 17
#    define RGB_MATRIX_SAT_STEP 17
#    define RGB_MATRIX_VAL_STEP 10
#    define RGB_MATRIX_SPD_STEP 10
#endif
