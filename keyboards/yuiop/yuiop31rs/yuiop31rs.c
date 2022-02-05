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

bool dip_switch_update_mask_kb(uint32_t state) {
    rotary_switch_state = (uint32_t)state;
    rotary_switch_update_state_kb(rotary_switch_state);
    return dip_switch_update_mask_user(state);
}

// clang-format off
matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b0111100,
    0b0111100,
    0b0111111,
    0b0111111,
    0b0111111,
    0b0111110,
};
// clang-format on

#ifdef RGB_MATRIX_ENABLE
// clang-format off
led_config_t g_led_config = {
    // Key Matrix to LED Index.
    {
        { NO_LED, NO_LED,      0,      1,      2,      3, NO_LED },
        { NO_LED, NO_LED,      7,      6,      5,      4, NO_LED },
        {      8,      9,     10,     11,     12,     13, NO_LED },
        {     19,     18,     17,     16,     15,     14, NO_LED },
        {     20,     21,     22,     23,     24,     25, NO_LED },
        {     30,     29,     28,     27,     26, NO_LED, NO_LED },
    },
    // LED Index to Physical Position
    {
                        { 67,  0}, {112,  0}, {157,  0}, {202,  0},
                              {224, 13}, {179, 13}, {134, 13}, { 90, 13},
        {  0, 26}, { 45, 26}, { 90, 26}, {134, 26}, {179, 26}, {224, 26},
        {224, 38}, {179, 38}, {134, 38}, { 90, 38}, { 45, 38}, {  0, 38},
        {  0, 51}, { 45, 51}, { 90, 51}, {134, 51}, {179, 51}, {224, 51},
             {202, 64}, {157, 64}, {112, 64}, { 67, 64}, { 22, 64},
    },
    // LED Index to Flag
    {
             4, 4, 4, 4,
              4, 4, 4, 4,
        4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4,
          4, 4, 4, 4, 4,
    }
};
// clang-format on
#endif  // RGB_MATRIX_ENABLE
