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

static void adjust_board_as_this(void) {
    adjust_rgblight_ranges();

    keyball_config_t c;
    c.raw = eeconfig_read_kb();
    if (c.cpi != 0) {
        pointing_device_set_cpi(c.cpi);
    }
    keyball.scroll_div = c.sdiv;
}

static void adjust_board_on_primary(void) {
    adjust_rgblight_ranges();

#ifdef VIA_ENABLE
    // adjust layout options value according to current combination.
    bool     left    = is_keyboard_left();
    uint8_t  layouts = (keyball.this_have_ball ? (left ? 0x02 : 0x01) : 0x00) | (keyball.that_have_ball ? (left ? 0x01 : 0x02) : 0x00);
    uint32_t curr    = via_get_layout_options();
    uint32_t next    = (curr & ~0x3) | layouts;
    if (next != curr) {
        via_set_layout_options(next);
    }
#endif
}

static void adjust_board_on_secondary(void) { adjust_rgblight_ranges(); }

//////////////////////////////////////////////////////////////////////////////

// add16 adds two int16_t with clipping.
static int16_t add16(int16_t a, int16_t b) {
    int16_t r = a + b;
    if (a >= 0 && b >= 0 && r < 0) {
        r = 32767;
    } else if (a < 0 && b < 0 && r >= 0) {
        r = -32768;
    }
    return r;
}

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

static void keyball_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    keyball_info_t *that = (keyball_info_t *)in_data;
    if (that->vid == VENDOR_ID && that->pid == PRODUCT_ID) {
        keyball.that_enable    = true;
        keyball.that_have_ball = that->ballcnt > 0;
    }
    keyball_info_t info = {
        .vid     = VENDOR_ID,
        .pid     = PRODUCT_ID,
        .ballcnt = keyball.this_have_ball ? 1 : 0,
    };
    memcpy(out_data, &info, sizeof(info));
    adjust_board_on_secondary();
}

static void keyball_get_info_invoke(void) {
    static bool     negotiated = false;
    static uint32_t last_sync  = 0;
    static int      round      = 0;
    if (negotiated || timer_elapsed32(last_sync) < KEYBALL_TX_GETINFO_INTERVAL) {
        return;
    }
    last_sync = timer_read32();
    round++;
    keyball_info_t req = {
        .vid     = VENDOR_ID,
        .pid     = PRODUCT_ID,
        .ballcnt = keyball.this_have_ball ? 1 : 0,
    };
    keyball_info_t recv = {0};
    if (!transaction_rpc_exec(KEYBALL_GET_INFO, sizeof(req), &req, sizeof(recv), &recv)) {
        if (round < KEYBALL_TX_GETINFO_MAXTRY) {
            dprintf("keyball_get_info_invoke: missed #%d\n", round);
            return;
        }
    }
    negotiated = true;
    if (recv.vid == VENDOR_ID && recv.pid == PRODUCT_ID) {
        keyball.that_enable    = true;
        keyball.that_have_ball = recv.ballcnt > 0;
    }
    dprintf("keyball_get_info_invoke: negotiated #%d %d\n", round, keyball.that_have_ball);
    if (is_keyboard_master()) {
        adjust_board_on_primary();
    } else {
        adjust_board_on_secondary();
    }
}

static void keyball_get_motion_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (keyball.this_have_ball && *((keyball_motion_id_t *)in_data) == 0) {
        *(keyball_motion_t *)out_data = keyball.this_motion;
        // clear motion
        keyball.this_motion.x = 0;
        keyball.this_motion.y = 0;
    }
}

static void keyball_get_motion_invoke(void) {
    static uint32_t last_sync = 0;
    if (!keyball.that_have_ball || timer_elapsed32(last_sync) < KEYBALL_TX_GETMOTION_INTERVAL) {
        return;
    }
    keyball_motion_id_t req  = 0;
    keyball_motion_t    recv = {0};
    if (transaction_rpc_exec(KEYBALL_GET_MOTION, sizeof(req), &req, sizeof(recv), &recv)) {
        ATOMIC_BLOCK_FORCEON {
            keyball.that_motion.x = add16(keyball.that_motion.x, recv.x);
            keyball.that_motion.y = add16(keyball.that_motion.y, recv.y);
        }
    } else {
        dprintf("keyball_get_motion_invoke: failed");
    }
    last_sync = timer_read32();
    return;
}

static void keyball_set_cpi_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (keyball.this_have_ball) {
        pmw3360_cpi_set(*(keyball_cpi_t *)in_data);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
}

static void keyball_set_cpi_invoke(void) {
    if (!keyball.that_have_ball || !keyball.cpi_changed) {
        return;
    }
    keyball_cpi_t req = keyball.cpi_value;
    if (!transaction_rpc_send(KEYBALL_SET_CPI, sizeof(req), &req)) {
        return;
    }
    keyball.cpi_changed = false;
}

//////////////////////////////////////////////////////////////////////////////

bool keyball_get_scroll_mode(void) { return keyball.scroll_mode; }

void keyball_set_scroll_mode(bool mode) {
    keyball.scroll_mode = mode;
}

//////////////////////////////////////////////////////////////////////////////

void keyboard_post_init_kb(void) {
    // register transaction handlers on secondary.
    if (!is_keyboard_master()) {
        transaction_register_rpc(KEYBALL_GET_INFO, keyball_get_info_handler);
        transaction_register_rpc(KEYBALL_GET_MOTION, keyball_get_motion_handler);
        transaction_register_rpc(KEYBALL_SET_CPI, keyball_set_cpi_handler);
    }
    adjust_board_as_this();
    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        keyball_get_info_invoke();
        keyball_get_motion_invoke();
        keyball_set_cpi_invoke();
    }
}

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

report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    // store mouse report for OLED.
    keyball.last_mouse = pointing_device_task_user(mouse_report);
    return keyball.last_mouse;
}

void eeconfig_init_kb(void) {
    keyball_config_t c = {
        .cpi  = 0,
        .sdiv = KEYBALL_SCROLL_DIV_DEFAULT,
    };
    eeconfig_update_kb(c.raw);
    eeconfig_init_user();
}
