/*
Copyright 2021 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

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

#include "quantum.h"
#include "drivers/pmw3360/pmw3360.h"
#include "print.h"

// clang-format off
matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b01110111,
    0b01110111,
    0b01110111,
    0b11110111,
    0b11110111,
    0b01110111,
    0b01110111,
    0b01110111,
    0b11110111,
    0b11000011,
};
// clang-format on

extern bool pmw3360_has;

void keyboard_post_init_kb() {
    rgblight_enable_noeeprom();
    rgblight_sethsv_noeeprom(pmw3360_has ? 0xaa : 0, 255, 100);
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);

    keyboard_post_init_user();
}
