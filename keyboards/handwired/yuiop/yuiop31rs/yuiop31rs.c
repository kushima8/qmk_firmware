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

#include "yuiop31rs.h"

__attribute__((weak)) void rotary_switch_update_state_kb(uint8_t state) {
    if (!rotary_switch_update_state_user(state)) {
        return;
    }
    layer_state_t next = state > 0 ? (layer_state_t)1 << state : 0;
    dprintf("state=%d layer_state=%08lX next=%08lX\n", state, layer_state, next);
    if (next != layer_state) {
        layer_state_set(next);
    }
}

__attribute__((weak)) bool rotary_switch_update_state_user(uint8_t state) { return true; }

uint8_t rotary_switch_state = 0;

void dip_switch_update_mask_kb(uint32_t state) { 
    rotary_switch_state = (uint32_t)state;
    rotary_switch_update_state_kb(rotary_switch_state);
}

matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b0111100,
    0b0111100,
    0b0111111,
    0b0111111,
    0b0111111,
    0b0111110,
};
