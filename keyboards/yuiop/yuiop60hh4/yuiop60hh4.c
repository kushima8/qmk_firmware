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

#include "yuiop60hh4.h"

matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b111111111111111,
    0b101111111111111,
    0b100111111111111,
    0b110111111111101,
    0b000110001000110,
};

//////////////////////////////////////////////////////////////////////////////

#ifdef LED_ARRAY_STATE

#include <string.h>
#include "ws2812.h"

static bool led_states_dirty;
static LED_TYPE led_states[5];

static void LAS_set(int n, uint8_t r, uint8_t g, uint8_t b) {
    LED_TYPE next = { .r = r, .g = g, .b = b };
    led_states[n] = next;
    led_states_dirty = true;
}

static void LAS_clear(int n) {
    LAS_set(n, 0, 0, 0);
}

static void LAS_update(int n, uint8_t r, uint8_t g, uint8_t b, bool enable) {
    if (enable) {
        LAS_set(n, r, g, b);
    } else {
        LAS_clear(n);
    }
}

static void led_states_commit(void) {
    if (!led_states_dirty) {
        return;
    }
    ws2812_setleds(led_states, 5);
}

static void led_array_state_init(void) {
    for (int i = 0; i < 5; i++) {
        LAS_clear(i);
    }
    led_states_commit();
}

bool led_update_kb(led_t led_state) {
    bool res = led_update_user(led_state);
    if (!res) {
        return false;
    }
    LAS_update(0, 0xff, 0x00, 0x00, led_state.caps_lock);
    LAS_update(1, 0xcc, 0x00, 0xcc, led_state.scroll_lock);
    LAS_update(2, 0x00, 0x00, 0xff, led_state.num_lock);
    LAS_update(3, 0x00, 0xcc, 0xcc, led_state.compose);
    LAS_update(4, 0x00, 0xff, 0x00, led_state.kana);
    led_states_commit();
    return true;
}

#endif

//////////////////////////////////////////////////////////////////////////////

void keyboard_post_init_kb(void) {
#ifdef LED_ARRAY_STATE
    led_array_state_init();
#endif
    keyboard_post_init_user();
}
