/*
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)
Copyright 2023 kushima8 (@kushima8)

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
#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#include "reex.h"
#include "drivers/pmw3360/pmw3360.h"

#include <string.h>

const uint8_t CPI_DEFAULT    = REEX_CPI_DEFAULT / 100;
const uint8_t CPI_MAX        = pmw3360_MAXCPI + 1;
const uint8_t SCROLL_DIV_MAX = 7;

const uint16_t AML_TIMEOUT_MIN = 100;
const uint16_t AML_TIMEOUT_MAX = 1000;
const uint16_t AML_TIMEOUT_QU  = 50;   // Quantization Unit

static const char BL = '\xB0'; // Blank indicator character
static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";

reex_t reex = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,
    .negotiated     = false,

    .ex_this_have_ball = false,
    .ex_that_have_ball = false,

    .this_motion = {0},
    .that_motion = {0},
    .ex_this_motion = {0},
    .ex_that_motion = {0},

    .cpi_value   = 0,
    .cpi_changed = false,

    .scroll_mode = false,
    .scroll_div  = 0,

    .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
};

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void reex_on_adjust_layout(reex_adjust_t v) {}

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

// divmod16 divides *v by div, returns the quotient, and assigns the remainder
// to *v.
static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

#ifdef OLED_ENABLE
static const char *format_4d(int8_t d) {
    static char buf[5] = {0}; // max width (4) + NUL (1)
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
#endif

static void add_cpi(int8_t delta) {
    int16_t v = reex_get_cpi() + delta;
    reex_set_cpi(v < 1 ? 1 : v);
}

static void add_scroll_div(int8_t delta) {
    int8_t v = reex_get_scroll_div() + delta;
    reex_set_scroll_div(v < 1 ? 1 : v);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

void pointing_device_driver_init(void) {
    reex.this_have_ball = pmw3360_init(0);
    if (reex.this_have_ball) {
#if defined(REEX_PMW3360_UPLOAD_SROM_ID)
#    if REEX_PMW3360_UPLOAD_SROM_ID == 0x04
        pmw3360_srom_upload(0,pmw3360_srom_0x04);
#    elif REEX_PMW3360_UPLOAD_SROM_ID == 0x81
        pmw3360_srom_upload(0,pmw3360_srom_0x81);
#    else
#        error Invalid value for REEX_PMW3360_UPLOAD_SROM_ID. Please choose 0x04 or 0x81 or disable it.
#    endif
#endif
        pmw3360_cpi_set(CPI_DEFAULT - 1);
    }
}

uint16_t pointing_device_driver_get_cpi(void) {
    return reex_get_cpi();
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    reex_set_cpi(cpi);
}

__attribute__((weak)) void reex_on_apply_motion_to_mouse_move(reex_motion_t *m, report_mouse_t *r, bool is_left) {
    r->x = -clip2int8(m->x);
    r->y = clip2int8(m->y);
    // clear motion
    m->x = 0;
    m->y = 0;
}

__attribute__((weak)) void reex_on_apply_motion_to_mouse_scroll(reex_motion_t *m, report_mouse_t *r, bool is_left) {
    // consume motion of trackball.
    int16_t div = 1 << (reex_get_scroll_div() - 1);
    int16_t x = divmod16(&m->x, div);
    int16_t y = divmod16(&m->y, div);

    // apply to mouse report.
    r->h = -clip2int8(x);
    r->v = -clip2int8(y);

    // Scroll snapping
#if REEX_SCROLLSNAP_ENABLE == 1
    // Old behavior up to 1.3.2)
    uint32_t now = timer_read32();
    if (r->h != 0 || r->v != 0) {
        reex.scroll_snap_last = now;
    } else if (TIMER_DIFF_32(now, reex.scroll_snap_last) >= REEX_SCROLLSNAP_RESET_TIMER) {
        reex.scroll_snap_tension_h = 0;
    }
    if (abs(reex.scroll_snap_tension_h) < REEX_SCROLLSNAP_TENSION_THRESHOLD) {
        reex.scroll_snap_tension_h += y;
        r->h = 0;
    }
#elif REEX_SCROLLSNAP_ENABLE == 2
    // New behavior
    switch (reex_get_scrollsnap_mode()) {
        case REEX_SCROLLSNAP_MODE_VERTICAL:
            r->h = 0;
            break;
        case REEX_SCROLLSNAP_MODE_HORIZONTAL:
            r->v = 0;
            break;
        default:
            // pass by without doing anything
            break;
    }
#endif
}

static void motion_to_mouse(reex_motion_t *m, report_mouse_t *r, bool is_left, bool as_scroll) {
    if (as_scroll) {
        reex_on_apply_motion_to_mouse_scroll(m, r, is_left);
    } else {
        reex_on_apply_motion_to_mouse_move(m, r, is_left);
    }
}

static inline bool should_report(void) {
    uint32_t now = timer_read32();
#if defined(REEX_REPORTMOUSE_INTERVAL) && REEX_REPORTMOUSE_INTERVAL > 0
    // throttling mouse report rate.
    static uint32_t last = 0;
    if (TIMER_DIFF_32(now, last) < REEX_REPORTMOUSE_INTERVAL) {
        return false;
    }
    last = now;
#endif
#if defined(REEX_SCROLLBALL_INHIVITOR) && REEX_SCROLLBALL_INHIVITOR > 0
    if (TIMER_DIFF_32(now, reex.scroll_mode_changed) < REEX_SCROLLBALL_INHIVITOR) {
        reex.this_motion.x = 0;
        reex.this_motion.y = 0;
        reex.that_motion.x = 0;
        reex.that_motion.y = 0;
    }
#endif
    return true;
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t rep) {
    // fetch from optical sensor.
    if (reex.this_have_ball) {
        pmw3360_motion_t d = {0};
        if (pmw3360_motion_burst(0,&d)) {
            ATOMIC_BLOCK_FORCEON {
                reex.this_motion.x = add16(reex.this_motion.x, d.x);
                reex.this_motion.y = add16(reex.this_motion.y, d.y);
            }
        }
    }
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master() && should_report()) {
        // modify mouse report by PMW3360 motion.
        motion_to_mouse(&reex.this_motion, &rep, is_keyboard_left(), reex.scroll_mode);
        motion_to_mouse(&reex.that_motion, &rep, !is_keyboard_left(), reex.scroll_mode ^ reex.this_have_ball);
        // store mouse report for OLED.
        reex.last_mouse = rep;
    }
    return rep;
}

//////////////////////////////////////////////////////////////////////////////
// Split RPC

#ifdef SPLIT_KEYBOARD

static void rpc_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    reex_info_t info = {
        .ballcnt = reex.this_have_ball ? 1 : 0,
    };
    *(reex_info_t *)out_data = info;
    reex_on_adjust_layout(REEX_ADJUST_SECONDARY);
}

static void rpc_get_info_invoke(void) {
    //static bool     negotiated = false;
    static uint32_t last_sync  = 0;
    static int      round      = 0;
    uint32_t        now        = timer_read32();
    if (reex.negotiated || TIMER_DIFF_32(now, last_sync) < REEX_TX_GETINFO_INTERVAL) {
        return;
    }
    last_sync = now;
    round++;
    reex_info_t recv = {0};
    if (!transaction_rpc_exec(REEX_GET_INFO, 0, NULL, sizeof(recv), &recv)) {
        if (round < REEX_TX_GETINFO_MAXTRY) {
            dprintf("reex:rpc_get_info_invoke: missed #%d\n", round);
            return;
        }
    }
    reex.negotiated     = true;
    reex.that_enable    = true;
    reex.that_have_ball = recv.ballcnt > 0;
    dprintf("reex:rpc_get_info_invoke: negotiated #%d %d\n", round, reex.that_have_ball);

    // split keyboard negotiation completed.

#    ifdef VIA_ENABLE
    // adjust VIA layout options according to current combination.
    uint8_t  layouts = (reex.this_have_ball ? (is_keyboard_left() ? 0x02 : 0x01) : 0x00) | (reex.that_have_ball ? (is_keyboard_left() ? 0x01 : 0x02) : 0x00);
    uint32_t curr    = via_get_layout_options();
    uint32_t next    = (curr & ~0x3) | layouts;
    if (next != curr) {
        via_set_layout_options(next);
    }
#    endif

    reex_on_adjust_layout(REEX_ADJUST_PRIMARY);
}

static void rpc_get_motion_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    *(reex_motion_t *)out_data = reex.this_motion;
    // clear motion
    reex.this_motion.x = 0;
    reex.this_motion.y = 0;
}

static void rpc_get_motion_invoke(void) {
    static uint32_t last_sync = 0;
    uint32_t        now       = timer_read32();
    if (TIMER_DIFF_32(now, last_sync) < REEX_TX_GETMOTION_INTERVAL) {
        return;
    }
    reex_motion_t recv = {0};
    if (transaction_rpc_exec(REEX_GET_MOTION, 0, NULL, sizeof(recv), &recv)) {
        reex.that_motion.x = add16(reex.that_motion.x, recv.x);
        reex.that_motion.y = add16(reex.that_motion.y, recv.y);
    }
    last_sync = now;
    return;
}

static void rpc_set_cpi_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    reex_set_cpi(*(reex_cpi_t *)in_data);
}

static void rpc_set_cpi_invoke(void) {
    if (!reex.cpi_changed) {
        return;
    }
    reex_cpi_t req = reex.cpi_value;
    if (!transaction_rpc_send(REEX_SET_CPI, sizeof(req), &req)) {
        return;
    }
    reex.cpi_changed = false;
}

#endif

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

void reex_oled_render_ballinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Ball:{mouse x}{mouse y}{mouse h}{mouse v}`
    //
    // Output example:
    //
    //     Ball: -12  34   0   0

    // 1st line, "Ball" label, mouse x, y, h, and v.
    oled_write_P(PSTR("Ball\xB1"), false);
    oled_write(format_4d(reex.last_mouse.x), false);
    oled_write(format_4d(reex.last_mouse.y), false);
    oled_write(format_4d(reex.last_mouse.h), false);
    oled_write(format_4d(reex.last_mouse.v), false);

    // 2nd line, empty label and CPI
    oled_write_P(PSTR("    \xB1\xBC\xBD"), false);
    oled_write(format_4d(reex_get_cpi()) + 1, false);
    oled_write_P(PSTR("00 "), false);

    // indicate scroll snap mode: "VT" (vertical), "HN" (horiozntal), and "SCR" (free)
#if 1 && REEX_SCROLLSNAP_ENABLE == 2
    switch (reex_get_scrollsnap_mode()) {
        case REEX_SCROLLSNAP_MODE_VERTICAL:
            oled_write_P(PSTR("VT"), false);
            break;
        case REEX_SCROLLSNAP_MODE_HORIZONTAL:
            oled_write_P(PSTR("HO"), false);
            break;
        default:
            oled_write_P(PSTR("\xBE\xBF"), false);
            break;
    }
#else
    oled_write_P(PSTR("\xBE\xBF"), false);
#endif
    // indicate scroll mode: on/off
    if (reex.scroll_mode) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    // indicate scroll divider:
    oled_write_P(PSTR(" \xC0\xC1"), false);
    oled_write_char('0' + reex_get_scroll_div(), false);
#endif
}

void reex_oled_render_ballsubinfo(void) {
#ifdef OLED_ENABLE
#endif
}

void reex_oled_render_keyinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Key :  R{row}  C{col} K{kc} {name}{name}{name}`
    //
    // Where `kc` is lower 8 bit of keycode.
    // Where `name`s are readable labels for pressing keys, valid between 4 and 56.
    //
    // `row`, `col`, and `kc` indicates the last processed key,
    // but `name`s indicate unreleased keys in best effort.
    //
    // It is aligned to fit with output of reex_oled_render_ballinfo().
    // For example:
    //
    //     Key :  R2  C3 K06 abc
    //     Ball:   0   0   0   0

    // "Key" Label
    oled_write_P(PSTR("Key \xB1"), false);

    // Row and column
    oled_write_char('\xB8', false);
    oled_write_char(to_1x(reex.last_pos.row), false);
    oled_write_char('\xB9', false);
    oled_write_char(to_1x(reex.last_pos.col), false);

    // Keycode
    oled_write_P(PSTR("\xBA\xBB"), false);
    oled_write_char(to_1x(reex.last_kc >> 4), false);
    oled_write_char(to_1x(reex.last_kc), false);

    // Pressing keys
    oled_write_P(PSTR("  "), false);
    oled_write(reex.pressing_keys, false);
#endif
}

void reex_oled_render_layerinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Layer:{layer state}`
    //
    // Output example:
    //
    //     Layer:-23------------
    //
    oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
    for (uint8_t i = 1; i < 8; i++) {
        oled_write_char((layer_state_is(i) ? to_1x(i) : BL), false);
    }
    oled_write_char(' ', false);

#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    oled_write_P(PSTR("\xC2\xC3"), false);
    if (get_auto_mouse_enable()) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    oled_write(format_4d(get_auto_mouse_timeout() / 10) + 1, false);
    oled_write_char('0', false);
#    else
    oled_write_P(PSTR("\xC2\xC3\xB4\xB5 ---"), false);
#    endif
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Public API functions

bool reex_get_scroll_mode(void) {
    return reex.scroll_mode;
}

void reex_set_scroll_mode(bool mode) {
    if (mode != reex.scroll_mode) {
        reex.scroll_mode_changed = timer_read32();
    }
    reex.scroll_mode = mode;
}

reex_scrollsnap_mode_t reex_get_scrollsnap_mode(void) {
#if REEX_SCROLLSNAP_ENABLE == 2
    return reex.scrollsnap_mode;
#else
    return 0;
#endif
}

void reex_set_scrollsnap_mode(reex_scrollsnap_mode_t mode) {
#if REEX_SCROLLSNAP_ENABLE == 2
    reex.scrollsnap_mode = mode;
#endif
}

uint8_t reex_get_scroll_div(void) {
    return reex.scroll_div == 0 ? REEX_SCROLL_DIV_DEFAULT : reex.scroll_div;
}

void reex_set_scroll_div(uint8_t div) {
    reex.scroll_div = div > SCROLL_DIV_MAX ? SCROLL_DIV_MAX : div;
}

uint8_t reex_get_cpi(void) {
    return reex.cpi_value == 0 ? CPI_DEFAULT : reex.cpi_value;
}

void reex_set_cpi(uint8_t cpi) {
    if (cpi > CPI_MAX) {
        cpi = CPI_MAX;
    }
    reex.cpi_value   = cpi;
    reex.cpi_changed = true;
    if (reex.this_have_ball) {
        pmw3360_cpi_set(cpi == 0 ? CPI_DEFAULT - 1 : cpi - 1);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks

void keyboard_post_init_kb(void) {
#ifdef SPLIT_KEYBOARD
    // register transaction handlers on secondary.
    if (!is_keyboard_master()) {
        transaction_register_rpc(REEX_GET_INFO, rpc_get_info_handler);
        transaction_register_rpc(REEX_GET_MOTION, rpc_get_motion_handler);
        transaction_register_rpc(REEX_SET_CPI, rpc_set_cpi_handler);
    }
#endif

    // read reex configuration from EEPROM
    if (eeconfig_is_enabled()) {
        reex_config_t c = {.raw = eeconfig_read_kb()};
        reex_set_cpi(c.cpi);
        reex_set_scroll_div(c.sdiv);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        set_auto_mouse_enable(c.amle);
        set_auto_mouse_timeout(c.amlto == 0 ? AUTO_MOUSE_TIME : (c.amlto + 1) * AML_TIMEOUT_QU);
#endif
#if REEX_SCROLLSNAP_ENABLE == 2
        reex_set_scrollsnap_mode(c.ssnap);
#endif
    }

    reex_on_adjust_layout(REEX_ADJUST_PENDING);
    keyboard_post_init_user();
}

#if SPLIT_KEYBOARD
void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        rpc_get_info_invoke();
        if (reex.that_have_ball) {
            rpc_get_motion_invoke();
            rpc_set_cpi_invoke();
        }
    }
}
#endif

static void pressing_keys_update(uint16_t keycode, keyrecord_t *record) {
    // Process only valid keycodes.
    if (keycode >= 4 && keycode < 57) {
        char value = pgm_read_byte(code_to_name + keycode - 4);
        char where = BL;
        if (!record->event.pressed) {
            // Swap `value` and `where` when releasing.
            where = value;
            value = BL;
        }
        // Rewrite the last `where` of pressing_keys to `value` .
        for (int i = 0; i < REEX_OLED_MAX_PRESSING_KEYCODES; i++) {
            if (reex.pressing_keys[i] == where) {
                reex.pressing_keys[i] = value;
                break;
            }
        }
    }
}

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
bool is_mouse_record_kb(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case SCRL_MO:
            return true;
    }
    return is_mouse_record_user(keycode, record);
}
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // store last keycode, row, and col for OLED
    reex.last_kc  = keycode;
    reex.last_pos = record->event.key;

    pressing_keys_update(keycode, record);

    if (!process_record_user(keycode, record)) {
        return false;
    }

    // strip QK_MODS part.
    if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
        keycode &= 0xff;
    }

    switch (keycode) {
#ifndef MOUSEKEY_ENABLE
        // process KC_MS_BTN1~8 by myself
        // See process_action() in quantum/action.c for details.
        case KC_MS_BTN1 ... KC_MS_BTN8: {
            extern void register_mouse(uint8_t mouse_keycode, bool pressed);
            register_mouse(keycode, record->event.pressed);
            // to apply QK_MODS actions, allow to process others.
            return true;
        }
#endif

        case SCRL_MO:
            reex_set_scroll_mode(record->event.pressed);
            // process_auto_mouse may use this in future, if changed order of
            // processes.
            return true;
    }

    // process events which works on pressed only.
    if (record->event.pressed) {
        switch (keycode) {
            case REC_RST:
                reex_set_cpi(0);
                reex_set_scroll_div(0);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                set_auto_mouse_enable(false);
                set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
                break;
            case REC_SAVE: {
                reex_config_t c = {
                    .cpi   = reex.cpi_value,
                    .sdiv  = reex.scroll_div,
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                    .amle  = get_auto_mouse_enable(),
                    .amlto = (get_auto_mouse_timeout() / AML_TIMEOUT_QU) - 1,
#endif
#if REEX_SCROLLSNAP_ENABLE == 2
                    .ssnap = reex_get_scrollsnap_mode(),
#endif
                };
                eeconfig_update_kb(c.raw);
            } break;

            case CPI_I100:
                add_cpi(1);
                break;
            case CPI_D100:
                add_cpi(-1);
                break;
            case CPI_I1K:
                add_cpi(10);
                break;
            case CPI_D1K:
                add_cpi(-10);
                break;

            case SCRL_TO:
                reex_set_scroll_mode(!reex.scroll_mode);
                break;
            case SCRL_DVI:
                add_scroll_div(1);
                break;
            case SCRL_DVD:
                add_scroll_div(-1);
                break;

#if REEX_SCROLLSNAP_ENABLE == 2
            case SSNP_HOR:
                reex_set_scrollsnap_mode(REEX_SCROLLSNAP_MODE_HORIZONTAL);
                break;
            case SSNP_VRT:
                reex_set_scrollsnap_mode(REEX_SCROLLSNAP_MODE_VERTICAL);
                break;
            case SSNP_FRE:
                reex_set_scrollsnap_mode(REEX_SCROLLSNAP_MODE_FREE);
                break;
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            case AML_TO:
                set_auto_mouse_enable(!get_auto_mouse_enable());
                break;
            case AML_I50:
                {
                    uint16_t v = get_auto_mouse_timeout() + 50;
                    set_auto_mouse_timeout(MIN(v, AML_TIMEOUT_MAX));
                }
                break;
            case AML_D50:
                {
                    uint16_t v = get_auto_mouse_timeout() - 50;
                    set_auto_mouse_timeout(MAX(v, AML_TIMEOUT_MIN));
                }
                break;
#endif

            default:
                return true;
        }
        return false;
    }

    return true;
}

// Disable functions keycode_config() and mod_config() in keycode_config.c to
// reduce size.  These functions are provided for customizing magic keycode.
// These two functions are mostly unnecessary if `MAGIC_KEYCODE_ENABLE = no` is
// set.
//
// If `MAGIC_KEYCODE_ENABLE = no` and you want to keep these two functions as
// they are, define the macro REEX_KEEP_MAGIC_FUNCTIONS.
//
// See: https://docs.qmk.fm/#/squeezing_avr?id=magic-functions
//
#if !defined(MAGIC_KEYCODE_ENABLE) && !defined(REEX_KEEP_MAGIC_FUNCTIONS)

uint16_t keycode_config(uint16_t keycode) {
    return keycode;
}

uint8_t mod_config(uint8_t mod) {
    return mod;
}

#endif
