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

#ifdef OLED_ENABLE

#include <stdio.h>

static char keylog[24] = {};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }
    snprintf(keylog, sizeof(keylog), "K:%04X R:%d C:%d",
            keycode, record->event.key.row, record->event.key.col);
    return true;
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  return rotation;
}

bool oled_task_user(void) {
    oled_write_ln_P(PSTR(STR(PRODUCT)), false);
    oled_write_ln(keylog, false);
    return true;
}

#endif // OLED_ENABLE
