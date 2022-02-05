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

#include "yuiop22rs.h"

__attribute__((weak)) void rotary_switch_update_state_kb(uint8_t state) { rotary_switch_update_state_user(state); }

__attribute__((weak)) void rotary_switch_update_state_user(uint8_t state) {}

uint8_t rotary_switch_state = 0;

bool dip_switch_update_mask_kb(uint32_t state) {
    rotary_switch_state = (uint32_t)state;
    rotary_switch_update_state_kb(rotary_switch_state);
    return dip_switch_update_mask_user(state);
}

matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b11000,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b00000,
};
