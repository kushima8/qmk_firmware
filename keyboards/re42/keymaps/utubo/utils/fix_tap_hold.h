/* Copyright 2021 utubo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include QMK_KEYBOARD_H

typedef enum {
    KEYUP = 0,
    SINGLE_TAP,
    LONG_TAP,
    SINGLE_HOLD,
    DOUBLE_HOLD
} tap_state_t;

extern tap_state_t tap_state;

void process_fix_tap_hold(uint16_t keycode, keyrecord_t *record);
bool quick_MT(uint16_t mod_key, uint16_t keycode);
bool quick_LT(uint8_t layer, uint16_t keycode);
bool sh_t16(uint16_t keycode);

