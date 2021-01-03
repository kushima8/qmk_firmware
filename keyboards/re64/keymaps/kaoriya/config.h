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

#undef PRODUCT
#define PRODUCT Re64 +KaoriYa r1

#define LAYER_STATE_8BIT

#define TAPPING_FORCE_HOLD
#define TAPPING_TERM 200

#define NO_ACTION_ONESHOT

#ifdef RGBLIGHT_ENABLE
#    undef RGBLED_NUM
#    undef RGBLIGHT_LIMIT_VAL
#    define RGBLED_NUM 34
#    define RGBLIGHT_LIMIT_VAL 112
#    define RGBLIGHT_HUE_STEP 17
#    define RGBLIGHT_SAT_STEP 17
#    define RGBLIGHT_VAL_STEP 16
#    define RGBLIGHT_ANIMATIONS
#endif
