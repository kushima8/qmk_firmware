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

#include QMK_KEYBOARD_H

#include "keyball.h"
#include "drivers/pmw3360/pmw3360.h"

keyball_t keyball = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,

    .this_motion = {0},
    .that_motion = {0},

    .cpi_value = 0,
    .cpi_changed = false,

    .scroll_mode = false,
    .scroll_div = 0,
};

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

void pointing_device_driver_init(void) {
    keyball.this_have_ball = pmw3360_init();
    if (keyball.this_have_ball) {
        //keyball.cpi_value = KEYBALL_CPI_DEFAULT;
        pmw3360_reg_write(pmw3360_Config1, KEYBALL_CPI_DEFAULT);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
}

uint16_t pointing_device_driver_get_cpi(void) { return keyball.cpi_value; }

void pointing_device_driver_set_cpi(uint16_t cpi) {
    if (keyball.this_have_ball) {
        pmw3360_cpi_set(cpi);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
    keyball.cpi_value   = cpi;
    keyball.cpi_changed = true;
}

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
        bool is_left = is_keyboard_left();
        if (keyball.this_have_ball) {
            motion_to_mouse(&keyball.this_motion, &rep, is_left, keyball.scroll_mode);
        }
        if (keyball.that_have_ball) {
            motion_to_mouse(&keyball.that_motion, &rep, !is_left, keyball.scroll_mode ^ keyball.this_have_ball);
        }
    }
    return rep;
}

//////////////////////////////////////////////////////////////////////////////
// OLED utility

#ifdef OLED_ENABLE

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
    oled_write(format_4d(keyball.cpi_value + 1) + 1, false);
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
