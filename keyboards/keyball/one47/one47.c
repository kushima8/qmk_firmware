/*
Copyright 2022 @Yowkees
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

#include "lib/keyball/keyball.h"
#include "lib/duplexmatrix/duplexmatrix.h"

//////////////////////////////////////////////////////////////////////////////

// clang-format off
matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b111111111111,
    0b111111111111,
    0b111111111111,
    0b011111111111,
};
// clang-format on

static uint8_t peek_matrix_intersection(pin_t out_pin, pin_t in_pin) {
    extern void matrix_io_delay(void);
    setPinInputHigh(in_pin);
    setPinOutput(out_pin);
    writePinLow(out_pin);
    wait_us(1);
    uint8_t pin_state = readPin(in_pin);
    setPinInputHigh(out_pin);
    matrix_io_delay();
    return pin_state;
}

static uint16_t bitrev12(uint16_t bits) {
    return bitrev16(bits) >> 4;
}

static uint16_t bitrev11(uint16_t bits) {
    return bitrev16(bits) >> 5;
}

static bool isLeftBall = false;

// ball on left side.
bool is_keyboard_left(void) {
    return isLeftBall;
}

void keyboard_pre_init_kb(void) {
    isLeftBall = !peek_matrix_intersection(B4, F7);
    keyboard_pre_init_user();
}

void duplex_scan_raw_post_kb(matrix_row_t out_matrix[]) {
    if (isLeftBall) {
        out_matrix[0] = bitrev12(out_matrix[0]);
        out_matrix[1] = bitrev12(out_matrix[1]);
        out_matrix[2] = bitrev12(out_matrix[2]);
        // FIXME:
        out_matrix[3] = bitrev11(out_matrix[3]);
    }
}
