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

#include QMK_KEYBOARD_H

#include <stdio.h>

#ifdef OLED_ENABLE

static char keylog[2][24] = {};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }
    snprintf(keylog[0], sizeof(keylog[0]), "K:%04X R:%d C:%d",
            keycode, record->event.key.row, record->event.key.col);
    return true;
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  return OLED_ROTATION_180;
}

bool oled_task_user(void) {
    oled_write_ln_P(PSTR(STR(PRODUCT)), false);
    oled_write_ln(keylog[0], false);
    oled_write_ln(keylog[1], false);
    return true;
}

#endif // OLED_ENABLE

void rotary_switch_update_state_user(uint8_t state) {
    // Change layer_state by rotary switch state.
    layer_state_t next = layer_state & ~0b11111111110;
    if (state >= 1 && state <= 9) {
        next |= 1 << state;
    }
    if (next != layer_state) {
        layer_state_set(next);
    }

#ifdef OLED_ENABLE
    snprintf(keylog[1], sizeof(keylog[1]), "RS:%02X L:%04lX", state, (uint32_t)layer_state);
    oled_set_cursor(0, 2);
    oled_write_ln(keylog[1], false);
    oled_render();
#endif // OLED_ENABLE
}
