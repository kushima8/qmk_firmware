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

#define KEYBALL_CPI_DEFAULT 500

//////////////////////////////////////////////////////////////////////////////

#define TX_GETINFO_INTERVAL 500
#define TX_GETINFO_MAXTRY 10
#define TX_GETMOTION_INTERVAL 5

//////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint16_t vid;
    uint16_t pid;
    uint8_t  ballcnt;  // count of balls: support only 0 or 1, for now
} keyball_info_t;

typedef uint8_t keyball_motion_id_t;

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t n;  // accumulative scan count after last consume.
} keyball_motion_t;

typedef uint16_t keyball_cpi_t;

//////////////////////////////////////////////////////////////////////////////

static bool this_have_ball = false;
static bool that_have_ball = false;

static keyball_motion_t this_motion = {0};
static keyball_motion_t that_motion = {0};

static uint16_t cpi_value   = KEYBALL_CPI_DEFAULT;
static bool     cpi_changed = false;

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
    0b11110011,
};
// clang-format on

static void matrix_adjust_this(void) {
    if (this_have_ball) {
        matrix_mask[is_keyboard_left() ? 4 : 9] = 0b11000011;
    }
    // FIXME: adjust other matrix related
}

static void matrix_adjust_that(void) {
    if (that_have_ball) {
        matrix_mask[is_keyboard_left() ? 9 : 4] = 0b11000011;
    }
    // FIXME: adjust other matrix related
}

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

// incU8 increments a uint8_t with clipping.
static inline uint8_t incU8(uint8_t a) { return a < 0xff ? a + 1 : 0xff; }

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) { return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v; }

static void motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r) {
    if (m->n == 0) {
        return;
    }
    // TODO: consider direction of ball.
    r->x = clip2int8(m->y);
    r->y = clip2int8(m->x);
    // clear motion
    m->x = 0;
    m->y = 0;
    m->n = 0;
}

static void motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r) {
    if (m->n == 0) {
        return;
    }
    // TODO: consider direction of ball.
    r->h = clip2int8(m->x);
    r->v = clip2int8(m->y);
    m->x = 0;
    m->y = 0;
    m->n = 0;
}

void pointing_device_driver_init(void) {
    this_have_ball = pmw3360_init();
    if (this_have_ball) {
        pmw3360_cpi_set(KEYBALL_CPI_DEFAULT);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t rep) {
    // fetch from optical sensor.
    if (this_have_ball) {
        pmw3360_motion_t d = {0};
        if (pmw3360_motion_burst(&d)) {
            ATOMIC_BLOCK_FORCEON {
                this_motion.x = add16(this_motion.x, d.x);
                this_motion.y = add16(this_motion.y, d.y);
                this_motion.n = incU8(this_motion.n);
            }
        }
    }
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master()) {
        if (this_have_ball) {
            motion_to_mouse_move(&this_motion, &rep);
            if (that_have_ball) {
                // dual ball
                motion_to_mouse_scroll(&this_motion, &rep);
            }
        } else if (that_have_ball) {
            // only that ball
            motion_to_mouse_move(&that_motion, &rep);
        }
    }
    return rep;
}

uint16_t pointing_device_driver_get_cpi(void) { return cpi_value; }

void pointing_device_driver_set_cpi(uint16_t cpi) {
    if (this_have_ball) {
        pmw3360_cpi_set(cpi);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
    cpi_value   = cpi;
    cpi_changed = true;
}

//////////////////////////////////////////////////////////////////////////////

static void keyball_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    keyball_info_t info = {
        .vid     = VENDOR_ID,
        .pid     = PRODUCT_ID,
        .ballcnt = this_have_ball ? 1 : 0,
    };
    memcpy(out_data, &info, sizeof(info));
}

static void keyball_get_info_invoke(void) {
    static bool     negotiated = false;
    static uint32_t last_sync  = 0;
    static int      round      = 0;
    if (negotiated || timer_elapsed32(last_sync) < TX_GETINFO_INTERVAL) {
        return;
    }
    last_sync = timer_read32();
    round++;
    keyball_info_t recv = {0};
    if (!transaction_rpc_recv(KEYBALL_GET_INFO, sizeof(recv), &recv)) {
        if (round < TX_GETINFO_MAXTRY) {
            dprintf("keyball_get_info_invoke: missed #%d\n", round);
            return;
        }
    }
    negotiated = true;
    if (recv.vid == VENDOR_ID && recv.pid == PRODUCT_ID) {
        that_have_ball = recv.ballcnt > 0;
    }
    dprintf("keyball_get_info_invoke: negotiated #%d %d\n", round, that_have_ball);
    matrix_adjust_that();
}

static void keyball_get_motion_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (this_have_ball && *((keyball_motion_id_t *)in_data) == 0) {
        *(keyball_motion_t *)out_data = this_motion;
        // clear motion
        this_motion.x = 0;
        this_motion.y = 0;
        this_motion.n = 0;
    }
}

static void keyball_get_motion_invoke(void) {
    static uint32_t last_sync = 0;
    if (!that_have_ball || that_motion.n != 0 || timer_elapsed32(last_sync) < TX_GETMOTION_INTERVAL) {
        return;
    }
    keyball_motion_id_t req  = 0;
    keyball_motion_t    recv = {0};
    if (transaction_rpc_exec(KEYBALL_GET_MOTION, sizeof(req), &req, sizeof(recv), &recv)) {
        ATOMIC_BLOCK_FORCEON { that_motion = recv; }
    } else {
        dprintf("keyball_get_motion_invoke: failed");
    }
    last_sync = timer_read32();
    return;
}

static void keyball_set_cpi_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (this_have_ball) {
        pmw3360_cpi_set(*(keyball_cpi_t *)in_data);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }
}

static void keyball_set_cpi_invoke(void) {
    if (!that_have_ball || !cpi_changed) {
        return;
    }
    keyball_cpi_t req = cpi_value;
    if (!transaction_rpc_send(KEYBALL_SET_CPI, sizeof(req), &req)) {
        return;
    }
    cpi_changed = false;
}

void keyball_set_cpi(uint16_t cpi) {
    if (cpi_value != cpi) {
        cpi_value   = cpi;
        cpi_changed = true;
    }
}

//////////////////////////////////////////////////////////////////////////////

void keyboard_post_init_kb(void) {
    // register transaction handlers on secondary.
    if (!is_keyboard_master()) {
        transaction_register_rpc(KEYBALL_GET_INFO, keyball_get_info_handler);
        transaction_register_rpc(KEYBALL_GET_MOTION, keyball_get_motion_handler);
        transaction_register_rpc(KEYBALL_SET_CPI, keyball_set_cpi_handler);
    }
    matrix_adjust_this();
    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        keyball_get_info_invoke();
        keyball_get_motion_invoke();
        keyball_set_cpi_invoke();
    }
}

#if 0  // for debug
bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    static bool first = true;
    if (first) {
        first = false;
#    ifdef CONSOLE_ENABLE
        uprintf("Keyball Hello #%d\n", 10);
        uprintf("ball: this=%d that=%d\n", this_have_ball, that_have_ball);
#    endif
    }
    return true;
}
#endif
