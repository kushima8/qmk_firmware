/*
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

#include "quantum.h"
#include "transactions.h"

#include "keyball.h"
#include "drivers/pmw3360/pmw3360.h"

const uint8_t CPI_DEFAULT = KEYBALL_CPI_DEFAULT / 100;
const uint8_t CPI_MAX     = pmw3360_MAXCPI + 1;

keyball_t keyball = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,

    .this_motion = {0},
    .that_motion = {0},

    .cpi_value   = 0,
    .cpi_changed = false,

    .scroll_mode = false,
    .scroll_div  = 0,
};

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void keyball_on_adjust_layout(keyball_adjust_t v) {}

//////////////////////////////////////////////////////////////////////////////
// Static utilities

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

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) { return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v; }

static const char *format_4d(int8_t d) {
    static char buf[5] = {0};  // max width (4) + NUL (1)
    char        lead   = ' ';
    if (d < 0) {
        d    = -d;
        lead = '-';
    }
    buf[3] = (d % 10) + '0';
    d /= 10;
    if (d == 0) {
        buf[2] = lead;
        lead   = ' ';
    } else {
        buf[2] = (d % 10) + '0';
        d /= 10;
    }
    if (d == 0) {
        buf[1] = lead;
        lead   = ' ';
    } else {
        buf[1] = (d % 10) + '0';
        d /= 10;
    }
    buf[0] = lead;
    return buf;
}

static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? x + '0' : x + 'a' - 10;
}

static void add_cpi(int8_t delta) {
    int16_t v = keyball_get_cpi() + delta;
    if (v < 1) {
        v = 1;
    }
    keyball_set_cpi(v);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

void pointing_device_driver_init(void) {
    keyball.this_have_ball = pmw3360_init();
    if (keyball.this_have_ball) {
        pmw3360_cpi_set(CPI_DEFAULT - 1);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
}

uint16_t pointing_device_driver_get_cpi(void) { return keyball_get_cpi(); }

void pointing_device_driver_set_cpi(uint16_t cpi) { keyball_set_cpi(cpi); }

static void motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    r->x = clip2int8(m->y);
    r->y = clip2int8(m->x);
    if (is_left) {
        r->x = -r->x;
        r->y = -r->y;
    }
    // clear motion
    m->x = 0;
    m->y = 0;
}

static void motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    int16_t x = m->x >> keyball.scroll_div;
    m->x -= x << keyball.scroll_div;
    int16_t y = m->y >> keyball.scroll_div;
    m->y -= y << keyball.scroll_div;
    r->h = clip2int8(y);
    r->v = clip2int8(x);
    if (!is_left) {
        r->h = -r->h;
        r->v = -r->v;
    }
}

static void motion_to_mouse(keyball_motion_t *m, report_mouse_t *r, bool is_left, bool as_scroll) {
    if (as_scroll) {
        motion_to_mouse_scroll(m, r, is_left);
    } else {
        motion_to_mouse_move(m, r, is_left);
    }
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t rep) {
    // fetch from optical sensor.
    if (keyball.this_have_ball) {
        pmw3360_motion_t d = {0};
        if (pmw3360_motion_burst(&d)) {
            ATOMIC_BLOCK_FORCEON {
                keyball.this_motion.x = add16(keyball.this_motion.x, d.x);
                keyball.this_motion.y = add16(keyball.this_motion.y, d.y);
            }
        }
    }
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master()) {
        if (keyball.this_have_ball) {
            motion_to_mouse(&keyball.this_motion, &rep, is_keyboard_left(), keyball.scroll_mode);
        }
        if (keyball.that_have_ball) {
            motion_to_mouse(&keyball.that_motion, &rep, !is_keyboard_left(), keyball.scroll_mode ^ keyball.this_have_ball);
        }
    }
    return rep;
}

//////////////////////////////////////////////////////////////////////////////
// Split RPC

static void rpc_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
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
    *(keyball_info_t *)out_data = info;
    keyball_on_adjust_layout(KEYBALL_ADJUST_SECONDARY);
}

static void rpc_get_info_invoke(void) {
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
            dprintf("keyball:rpc_get_info_invoke: missed #%d\n", round);
            return;
        }
    }
    negotiated = true;
    if (recv.vid == VENDOR_ID && recv.pid == PRODUCT_ID) {
        keyball.that_enable    = true;
        keyball.that_have_ball = recv.ballcnt > 0;
    }
    dprintf("keyball:rpc_get_info_invoke: negotiated #%d %d\n", round, keyball.that_have_ball);

    // split keyboard negotiation completed.

#ifdef VIA_ENABLE
    // adjust VIA layout options according to current combination.
    uint8_t  layouts = (keyball.this_have_ball ? (is_keyboard_left() ? 0x02 : 0x01) : 0x00) | (keyball.that_have_ball ? (is_keyboard_left() ? 0x01 : 0x02) : 0x00);
    uint32_t curr    = via_get_layout_options();
    uint32_t next    = (curr & ~0x3) | layouts;
    if (next != curr) {
        via_set_layout_options(next);
    }
#endif

    keyball_on_adjust_layout(KEYBALL_ADJUST_PRIMARY);
}

static void rpc_get_motion_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (keyball.this_have_ball && *((keyball_motion_id_t *)in_data) == 0) {
        *(keyball_motion_t *)out_data = keyball.this_motion;
        // clear motion
        keyball.this_motion.x = 0;
        keyball.this_motion.y = 0;
    }
}

static void rpc_get_motion_invoke(void) {
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
        dprintf("keyball:rpc_get_motion_invoke: failed");
    }
    last_sync = timer_read32();
    return;
}

static void rpc_set_cpi_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (keyball.this_have_ball) {
        keyball_set_cpi(*(keyball_cpi_t *)in_data);
    }
}

static void rpc_set_cpi_invoke(void) {
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
// OLED utility

#ifdef OLED_ENABLE
// clang-format off
const char PROGMEM code_to_name[] = {
    'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i',  'j',
    'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't',
    'u', 'v', 'w', 'x', 'y', 'z',  '1', '2', '3',  '4',
    '5', '6', '7', '8', '9', '0',  'R', 'E', 'B',  'T',
    '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`',
    ',', '.', '/',
};
// clang-format on
#endif

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Ball:{mouse x}{mouse y}{mouse h}{mouse v}`
    //         `    CPI{CPI} S{SCROLL_MODE} D{SCROLL_DIV}`
    //
    // Output example:
    //
    //     Ball: -12  34   0   0
    //
    oled_write_P(PSTR("Ball:"), false);
    oled_write(format_4d(keyball.last_mouse.x), false);
    oled_write(format_4d(keyball.last_mouse.y), false);
    oled_write(format_4d(keyball.last_mouse.h), false);
    oled_write(format_4d(keyball.last_mouse.v), false);
    // CPI
    oled_write_P(PSTR("     CPI"), false);
    oled_write(format_4d(keyball_get_cpi()) + 1, false);
    oled_write_P(PSTR("00  S"), false);
    oled_write_char(keyball.scroll_mode ? '1' : '0', false);
    oled_write_P(PSTR("  D"), false);
    oled_write_char('0' + keyball.scroll_div, false);
#endif
}

void keyball_oled_render_keyinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Key :  R{row}  C{col} K{kc}  '{name}`
    //
    // Where `kc` is lower 8 bit of keycode.
    // Where `name` is readable label for `kc`, valid between 4 and 56.
    //
    // It is aligned to fit with output of keyball_oled_render_ballinfo().
    // For example:
    //
    //     Key :  R2  C3 K06  'c
    //     Ball:   0   0   0   0
    //
    uint8_t keycode = keyball.last_kc;

    oled_write_P(PSTR("Key :  R"), false);
    oled_write_char(to_1x(keyball.last_pos.row), false);
    oled_write_P(PSTR("  C"), false);
    oled_write_char(to_1x(keyball.last_pos.col), false);
    if (keycode) {
        oled_write_P(PSTR(" K"), false);
        oled_write_char(to_1x(keycode >> 4), false);
        oled_write_char(to_1x(keycode), false);
    }
    if (keycode >= 4 && keycode < 57) {
        oled_write_P(PSTR("  '"), false);
        char name = pgm_read_byte(code_to_name + keycode - 4);
        oled_write_char(name, false);
    } else {
        oled_advance_page(true);
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////

bool keyball_get_scroll_mode(void) { return keyball.scroll_mode; }

void keyball_set_scroll_mode(bool mode) { keyball.scroll_mode = mode; }

uint8_t keyball_get_cpi(void) { return keyball.cpi_value == 0 ? CPI_DEFAULT : keyball.cpi_value; }

void keyball_set_cpi(uint8_t cpi) {
    if (cpi > CPI_MAX) {
        cpi = CPI_MAX;
    }
    keyball.cpi_value   = cpi;
    keyball.cpi_changed = true;
    if (keyball.this_have_ball) {
        pmw3360_cpi_set(cpi == 0 ? CPI_DEFAULT - 1 : cpi - 1);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////

void keyboard_post_init_kb(void) {
    // register transaction handlers on secondary.
    if (!is_keyboard_master()) {
        transaction_register_rpc(KEYBALL_GET_INFO, rpc_get_info_handler);
        transaction_register_rpc(KEYBALL_GET_MOTION, rpc_get_motion_handler);
        transaction_register_rpc(KEYBALL_SET_CPI, rpc_set_cpi_handler);
    }

    // read keyball configuration from EEPROM
    keyball_config_t c = {.raw = eeconfig_read_kb()};
    if (c.cpi != 0) {
        keyball_set_cpi(c.cpi);
    }
    keyball.scroll_div = c.sdiv;

    keyball_on_adjust_layout(KEYBALL_ADJUST_PENDING);
    keyboard_post_init_user();
}

void eeconfig_init_kb(void) {
    keyball_config_t c = {
        .cpi  = 0,
        .sdiv = KEYBALL_SCROLL_DIV_DEFAULT,
    };
    eeconfig_update_kb(c.raw);
    eeconfig_init_user();
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        rpc_get_info_invoke();
        rpc_get_motion_invoke();
        rpc_set_cpi_invoke();
    }
}

report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    // store mouse report for OLED.
    keyball.last_mouse = pointing_device_task_user(mouse_report);
    return keyball.last_mouse;
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // store last keycode, row, and col for OLED
    keyball.last_kc  = keycode;
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
                keyball_set_cpi(0);
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
