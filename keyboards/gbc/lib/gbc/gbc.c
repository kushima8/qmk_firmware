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

#include "gbc.h"
#include "drivers/pmw3360/pmw3360.h"

#include <string.h>

const uint8_t CPI_DEFAULT    = GBC_CPI_DEFAULT / 100;
const uint8_t CPI_MAX        = pmw3360_MAXCPI + 1;
const uint8_t SCROLL_DIV_MAX = 7;

const uint16_t AML_TIMEOUT_MIN = 100;
const uint16_t AML_TIMEOUT_MAX = 1000;
const uint16_t AML_TIMEOUT_QU  = 50;   // Quantization Unit

gbc_t gbc = {
    .have_ball = false,
    .motion    = {0},
    .cpi_value = 0,

    .scroll_mode = false,
    .scroll_mode_before_mo = false,
    .scroll_div  = 0,
};

//////////////////////////////////////////////////////////////////////////////
// Hook points


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

static void add_cpi(int8_t delta) {
    int16_t v = gbc_get_cpi() + delta;
    gbc_set_cpi(v < 1 ? 1 : v);
}

static void add_scroll_div(int8_t delta) {
    int8_t v = gbc_get_scroll_div() + delta;
    gbc_set_scroll_div(v < 1 ? 1 : v);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

void pointing_device_driver_init(void) {
    // 添付 pmw3360 ドライバはセンサー index を取る API のため、
    // 単一センサー構成の GBC24 では index 0 を指定する。
    gbc.have_ball = pmw3360_init(0);
    if (gbc.have_ball) {
#if defined(GBC_PMW3360_UPLOAD_SROM_ID)
#    if GBC_PMW3360_UPLOAD_SROM_ID == 0x04
        pmw3360_srom_upload(0, pmw3360_srom_0x04);
#    elif GBC_PMW3360_UPLOAD_SROM_ID == 0x81
        pmw3360_srom_upload(0, pmw3360_srom_0x81);
#    else
#        error Invalid value for GBC_PMW3360_UPLOAD_SROM_ID. Please choose 0x04 or 0x81 or disable it.
#    endif
#endif
        pmw3360_cpi_set(CPI_DEFAULT - 1);
    }
}

uint16_t pointing_device_driver_get_cpi(void) {
    return gbc_get_cpi();
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    gbc_set_cpi(cpi);
}

__attribute__((weak)) void gbc_on_apply_motion_to_mouse_move(gbc_motion_t *m, report_mouse_t *r, bool is_left) {
    r->x = -clip2int8(m->x);
    r->y = clip2int8(m->y);
    // clear motion
    m->x = 0;
    m->y = 0;
}

__attribute__((weak)) void gbc_on_apply_motion_to_mouse_scroll(gbc_motion_t *m, report_mouse_t *r, bool is_left) {
    // consume motion of trackball.
    int16_t div = 1 << (gbc_get_scroll_div() - 1);
    int16_t x = divmod16(&m->x, div);
    int16_t y = divmod16(&m->y, div);

    // apply to mouse report.
    r->h = -clip2int8(x);
    r->v = -clip2int8(y);

    // Scroll snapping
#if GBC_SCROLLSNAP_ENABLE == 1
    // Old behavior up to 1.3.2)
    uint32_t now = timer_read32();
    if (r->h != 0 || r->v != 0) {
        gbc.scroll_snap_last = now;
    } else if (TIMER_DIFF_32(now, gbc.scroll_snap_last) >= GBC_SCROLLSNAP_RESET_TIMER) {
        gbc.scroll_snap_tension_h = 0;
    }
    if (abs(gbc.scroll_snap_tension_h) < GBC_SCROLLSNAP_TENSION_THRESHOLD) {
        gbc.scroll_snap_tension_h += y;
        r->h = 0;
    }
#elif GBC_SCROLLSNAP_ENABLE == 2
    // New behavior
    switch (gbc_get_scrollsnap_mode()) {
        case GBC_SCROLLSNAP_MODE_VERTICAL:
            r->h = 0;
            break;
        case GBC_SCROLLSNAP_MODE_HORIZONTAL:
            r->v = 0;
            break;
        default:
            // pass by without doing anything
            break;
    }
#endif
}

static void motion_to_mouse(gbc_motion_t *m, report_mouse_t *r, bool is_left, bool as_scroll) {
    if (as_scroll) {
        gbc_on_apply_motion_to_mouse_scroll(m, r, is_left);
    } else {
        gbc_on_apply_motion_to_mouse_move(m, r, is_left);
    }
}

static inline bool should_report(void) {
    uint32_t now = timer_read32();
#if defined(GBC_REPORTMOUSE_INTERVAL) && GBC_REPORTMOUSE_INTERVAL > 0
    // throttling mouse report rate.
    static uint32_t last = 0;
    if (TIMER_DIFF_32(now, last) < GBC_REPORTMOUSE_INTERVAL) {
        return false;
    }
    last = now;
#endif
#if defined(GBC_SCROLLBALL_INHIVITOR) && GBC_SCROLLBALL_INHIVITOR > 0
    if (TIMER_DIFF_32(now, gbc.scroll_mode_changed) < GBC_SCROLLBALL_INHIVITOR) {
        gbc.motion.x = 0;
        gbc.motion.y = 0;
    }
#endif
    return true;
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t rep) {
    // fetch from optical sensor.
    if (gbc.have_ball) {
        pmw3360_motion_t d = {0};
        if (pmw3360_motion_burst(0, &d)) {
            ATOMIC_BLOCK_FORCEON {
                gbc.motion.x = add16(gbc.motion.x, d.x);
                gbc.motion.y = add16(gbc.motion.y, d.y);
            }
        }
    }
    if (should_report()) {
        // modify mouse report by PMW3360 motion.
        motion_to_mouse(&gbc.motion, &rep, true, gbc.scroll_mode);
    }
    return rep;
}

//////////////////////////////////////////////////////////////////////////////
// Text status output (for boards without OLED)
//
// 出力に使う記号は US / JIS 配列で同じ文字になるもの (空白 , . / - %) のみ。
// snprintf はフラッシュを大きく消費するため 10 進変換は自前で行う。

static void send_dec(uint16_t v) {
    char    buf[6];
    uint8_t i = sizeof(buf) - 1;
    buf[i]    = '\0';
    do {
        buf[--i] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0 && i != 0);
    send_string(buf + i);
}

static void send_onoff(bool on) {
    send_string_P(on ? PSTR("on") : PSTR("off"));
}

// is_config_saved reports whether the effective settings stored in EEPROM
// equal the current ones.  Stored values are decoded the same way as
// keyboard_post_init_kb() does, so an untouched (all zero) EEPROM matches
// the defaults.
static bool is_config_saved(void) {
    if (!eeconfig_is_enabled()) {
        return false;
    }
    gbc_config_t c = {.raw = eeconfig_read_kb()};

    uint8_t saved_cpi = c.cpi == 0 ? CPI_DEFAULT : (c.cpi > CPI_MAX ? CPI_MAX : c.cpi);
    if (saved_cpi != gbc_get_cpi()) {
        return false;
    }
    uint8_t saved_sdiv = c.sdiv == 0 ? GBC_SCROLL_DIV_DEFAULT : c.sdiv;
    if (saved_sdiv != gbc_get_scroll_div()) {
        return false;
    }
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    if ((bool)c.amle != get_auto_mouse_enable()) {
        return false;
    }
    uint16_t saved_amlto = c.amlto == 0 ? AUTO_MOUSE_TIME : (c.amlto + 1) * AML_TIMEOUT_QU;
    if (saved_amlto != get_auto_mouse_timeout()) {
        return false;
    }
#endif
#if GBC_SCROLLSNAP_ENABLE == 2
    if ((gbc_scrollsnap_mode_t)c.ssnap != gbc_get_scrollsnap_mode()) {
        return false;
    }
#endif
    return true;
}

// Output format (labels padded to 16 columns, no indent, newline after
// every line including the last one):
//
//   GBC24 trackball
//   cpi             500
//   scroll mode     off
//   scroll div      4  1/8
//   scroll snap     vertical
//   auto mouse      on
//   aml timeout     650ms
//   eeprom          saved
void gbc_send_status(void) {
    send_string_P(PSTR("GBC24 trackball\n"));

    // Stored CPI value v means v * 100 CPI.
    send_string_P(PSTR("cpi             "));
    if (gbc.have_ball) {
        send_dec((uint16_t)gbc_get_cpi() * 100);
        send_string_P(PSTR("\n"));
    } else {
        send_string_P(PSTR("none\n"));
    }

    send_string_P(PSTR("scroll mode     "));
    send_onoff(gbc.scroll_mode);
    send_string_P(PSTR("\n"));

    // Divider n means 1/2^(n-1).
    send_string_P(PSTR("scroll div      "));
    send_dec(gbc_get_scroll_div());
    send_string_P(PSTR("  1/"));
    send_dec((uint16_t)1 << (gbc_get_scroll_div() - 1));
    send_string_P(PSTR("\n"));

    send_string_P(PSTR("scroll snap     "));
#if GBC_SCROLLSNAP_ENABLE == 2
    switch (gbc_get_scrollsnap_mode()) {
        case GBC_SCROLLSNAP_MODE_VERTICAL:
            send_string_P(PSTR("vertical\n"));
            break;
        case GBC_SCROLLSNAP_MODE_HORIZONTAL:
            send_string_P(PSTR("horizontal\n"));
            break;
        default:
            send_string_P(PSTR("free\n"));
            break;
    }
#elif GBC_SCROLLSNAP_ENABLE == 1
    send_string_P(PSTR("auto\n"));
#else
    send_string_P(PSTR("off\n"));
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    send_string_P(PSTR("auto mouse      "));
    send_onoff(get_auto_mouse_enable());
    send_string_P(PSTR("\naml timeout     "));
    send_dec(get_auto_mouse_timeout());
    send_string_P(PSTR("ms\n"));
#else
    send_string_P(PSTR("auto mouse      n/a\naml timeout     n/a\n"));
#endif

    // Trackball settings are written to EEPROM only by REC_SAVE.
    send_string_P(is_config_saved() ? PSTR("eeprom          saved\n")
                                    : PSTR("eeprom          not saved, press REC SAVE\n"));
}

//////////////////////////////////////////////////////////////////////////////
// Public API functions

bool gbc_get_scroll_mode(void) {
    return gbc.scroll_mode;
}

void gbc_set_scroll_mode(bool mode) {
    if (mode != gbc.scroll_mode) {
        gbc.scroll_mode_changed = timer_read32();
    }
    gbc.scroll_mode = mode;
}

gbc_scrollsnap_mode_t gbc_get_scrollsnap_mode(void) {
#if GBC_SCROLLSNAP_ENABLE == 2
    return gbc.scrollsnap_mode;
#else
    return 0;
#endif
}

void gbc_set_scrollsnap_mode(gbc_scrollsnap_mode_t mode) {
#if GBC_SCROLLSNAP_ENABLE == 2
    gbc.scrollsnap_mode = mode;
#endif
}

uint8_t gbc_get_scroll_div(void) {
    return gbc.scroll_div == 0 ? GBC_SCROLL_DIV_DEFAULT : gbc.scroll_div;
}

void gbc_set_scroll_div(uint8_t div) {
    gbc.scroll_div = div > SCROLL_DIV_MAX ? SCROLL_DIV_MAX : div;
}

uint8_t gbc_get_cpi(void) {
    return gbc.cpi_value == 0 ? CPI_DEFAULT : gbc.cpi_value;
}

void gbc_set_cpi(uint8_t cpi) {
    if (cpi > CPI_MAX) {
        cpi = CPI_MAX;
    }
    gbc.cpi_value = cpi;
    if (gbc.have_ball) {
        pmw3360_cpi_set(cpi == 0 ? CPI_DEFAULT - 1 : cpi - 1);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks

void keyboard_post_init_kb(void) {
    // read gbc configuration from EEPROM
    if (eeconfig_is_enabled()) {
        gbc_config_t c = {.raw = eeconfig_read_kb()};
        gbc_set_cpi(c.cpi);
        gbc_set_scroll_div(c.sdiv);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        set_auto_mouse_enable(c.amle);
        set_auto_mouse_timeout(c.amlto == 0 ? AUTO_MOUSE_TIME : (c.amlto + 1) * AML_TIMEOUT_QU);
#endif
#if GBC_SCROLLSNAP_ENABLE == 2
        gbc_set_scrollsnap_mode(c.ssnap);
#endif
    }

    keyboard_post_init_user();
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
            /* 押している間だけスクロール。離したら押下前の状態に戻す
             * (SCRL_TO で ON にしていた場合は ON のまま)。 */
            if (record->event.pressed) {
                gbc.scroll_mode_before_mo = gbc.scroll_mode;
                gbc_set_scroll_mode(true);
            } else {
                gbc_set_scroll_mode(gbc.scroll_mode_before_mo);
            }
            // process_auto_mouse may use this in future, if changed order of
            // processes.
            return true;
    }

    // process events which works on pressed only.
    if (record->event.pressed) {
        switch (keycode) {
            case REC_RST:
                gbc_set_cpi(0);
                gbc_set_scroll_div(0);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                set_auto_mouse_enable(false);
                set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
#if GBC_SCROLLSNAP_ENABLE == 2
                gbc_set_scrollsnap_mode(GBC_SCROLLSNAP_MODE_VERTICAL);
#endif
                break;
            case REC_SAVE: {
                gbc_config_t c = {
                    .cpi   = gbc.cpi_value,
                    .sdiv  = gbc.scroll_div,
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                    .amle  = get_auto_mouse_enable(),
                    .amlto = (get_auto_mouse_timeout() / AML_TIMEOUT_QU) - 1,
#endif
#if GBC_SCROLLSNAP_ENABLE == 2
                    .ssnap = gbc_get_scrollsnap_mode(),
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
                gbc_set_scroll_mode(!gbc.scroll_mode);
                break;
            case SCRL_DVI:
                add_scroll_div(1);
                break;
            case SCRL_DVD:
                add_scroll_div(-1);
                break;

#if GBC_SCROLLSNAP_ENABLE == 2
            case SSNP_HOR:
                gbc_set_scrollsnap_mode(GBC_SCROLLSNAP_MODE_HORIZONTAL);
                break;
            case SSNP_VRT:
                gbc_set_scrollsnap_mode(GBC_SCROLLSNAP_MODE_VERTICAL);
                break;
            case SSNP_FRE:
                gbc_set_scrollsnap_mode(GBC_SCROLLSNAP_MODE_FREE);
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
// they are, define the macro GBC_KEEP_MAGIC_FUNCTIONS.
//
// See: https://docs.qmk.fm/#/squeezing_avr?id=magic-functions
//
#if !defined(MAGIC_KEYCODE_ENABLE) && !defined(GBC_KEEP_MAGIC_FUNCTIONS)

uint16_t keycode_config(uint16_t keycode) {
    return keycode;
}

uint8_t mod_config(uint8_t mod) {
    return mod;
}

#endif
