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

#include <string.h>

#include "quantum.h"

#include "transactions.h"
#include "drivers/pmw3360/pmw3360.h"

//////////////////////////////////////////////////////////////////////////////

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
    0b11110111,
};
// clang-format on

static void adjust_rgblight_ranges(void) {
#ifdef RGBLIGHT_ENABLE
    // adjust RGBLIGHT's clipping and effect ranges
    uint8_t lednum_this = keyball.this_have_ball ? 34 : 37;
    uint8_t lednum_that = !keyball.that_enable ? 0 : keyball.that_have_ball ? 34 : 37;
    rgblight_set_clipping_range(is_keyboard_left() ? 0 : lednum_that, lednum_this);
    rgblight_set_effect_range(0, lednum_this + lednum_that);
#endif
}

void keyball_post_init_kb(void) {
    adjust_rgblight_ranges();
}

void keyball_adjust_as_primary(void) {
    adjust_rgblight_ranges();
}

void keyball_adjust_as_secondary(void) { adjust_rgblight_ranges(); }

//////////////////////////////////////////////////////////////////////////////

static void add_cpi(int8_t delta) {
    int16_t v = keyball.cpi_value + delta;
    if (v < 0) {
        v = 0;
    } else if (v > pmw3360_MAXCPI) {
        v = pmw3360_MAXCPI;
    }
    pointing_device_set_cpi(v);
}

//////////////////////////////////////////////////////////////////////////////

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // store last keycode, row, and col for OLED
    keyball.last_kc = keycode;
    keyball.last_pos = record->event.key;

    if (!process_record_user(keycode, record)) {
        return false;
    }

    switch (keycode) {
        // process KC_MS_BTN1~8 by myself
        // See process_action() in quantum/action.c for details.
#ifndef MOUSEKEY_ENABLE
        case KC_MS_BTN1 ... KC_MS_BTN8: {
            extern void register_button(bool, enum mouse_buttons);
            register_button(record->event.pressed, MOUSE_BTN_MASK(keycode - KC_MS_BTN1));
            break;
        }
#endif

        case KBC_RST:
            if (record->event.pressed) {
                pointing_device_set_cpi(KEYBALL_CPI_DEFAULT);
                keyball.scroll_div = KEYBALL_SCROLL_DIV_DEFAULT;
            }
            break;
        case KBC_SAVE:
            if (record->event.pressed) {
                keyball_config_t c = {
                    .cpi  = keyball.cpi_value,
                    .sdiv = keyball.scroll_div,
                };
                eeconfig_update_kb(c.raw);
            }
            break;

        case CPI_I100:
            if (record->event.pressed) {
                add_cpi(1);
            }
            break;
        case CPI_D100:
            if (record->event.pressed) {
                add_cpi(-1);
            }
            break;
        case CPI_I1K:
            if (record->event.pressed) {
                add_cpi(10);
            }
            break;
        case CPI_D1K:
            if (record->event.pressed) {
                add_cpi(-10);
            }
            break;

        case SCRL_TO:
            if (record->event.pressed) {
                keyball.scroll_mode = !keyball.scroll_mode;
            }
            break;
        case SCRL_MO:
            keyball.scroll_mode = record->event.pressed;
            break;
        case SCRL_DVI:
            if (record->event.pressed && keyball.scroll_div < 7) {
                keyball.scroll_div++;
            }
            break;
        case SCRL_DVD:
            if (record->event.pressed && keyball.scroll_div > 0) {
                keyball.scroll_div--;
            }
            break;

        default:
            return true;
    }
    return false;
}
