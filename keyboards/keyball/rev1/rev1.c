#include "rev1.h"

#include "quantum.h"
#include "pointing_device.h"

#include "trackball.h"

#ifndef KEYBALL_SCROLL_DIVIDER
#    define KEYBALL_SCROLL_DIVIDER 10
#endif
#if KEYBALL_SCROLL_DIVIDER <= 0
#    error "KEYBALL_SCROLL_DIVIDER should be larger than zero"
#endif

static int primary = 0;
static int secondary = 1;
static bool is_scroll_mode = false;

__attribute__((weak)) void pointing_device_init(void) {
    trackball_init();
}

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

static trackball_delta_t ball1, ball2;

__attribute__((weak)) void pointing_device_task(void) {
    trackball_delta_t d0 = {0}, d1 = {0};
    bool c0 = trackball_consume_delta(primary, is_scroll_mode ? KEYBALL_SCROLL_DIVIDER : 1, &d0);
    bool c1 = trackball_consume_delta(secondary, KEYBALL_SCROLL_DIVIDER, &d1);
    if (c0 || c1) {
        ball1 = d0;
        ball2 = d1;
        keyball_process_trackball_user(&d0, &d1);
    }
}

static uint16_t last_keycode;
static uint8_t last_row;
static uint8_t last_col;

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    last_keycode = keycode;
    last_row = record->event.key.row;
    last_col = record->event.key.col;
    return process_record_user(keycode, record);
}

const char digits[] = "0123456789abcdef";

static char format_buf[5]; // max width (4) + NUL (1)

static const char * format_d(int16_t d, int w) {
    format_buf[w] = '\0';
    --w;
    bool minus = d < 0;
    if (minus) {
        d = -d;
    }
    do {
        format_buf[w] = (d % 10) + '0';
        d /= 10;
        w--;
    } while(d != 0 && w >= 0);
    if (minus && w >= 0) {
        format_buf[w] = '-';
        w--;
    }
    while (w >= 0) {
        format_buf[w] = ' ';
        w--;
    }
    return format_buf;
}

static const char * format_02x(uint8_t x) {
    format_buf[0] = digits[(x >> 4) & 0x0f];
    format_buf[1] = digits[x & 0x0f];
    format_buf[2] = '\0';
    return format_buf;
}

//////////////////////////////////////////////////////////////////////////////
// Keyball API

bool keyball_get_scroll_mode(void) {
    return is_scroll_mode;
}

void keyball_set_scroll_mode(bool mode) {
    if (is_scroll_mode != mode) {
        is_scroll_mode = mode;
        trackball_reset_delta(primary);
    }
}

void keyball_process_trackball_default(
        const trackball_delta_t *primary,
        const trackball_delta_t *secondary)
{
    report_mouse_t r = pointing_device_get_report();
    if (primary) {
        if (!is_scroll_mode) {
            r.x += clip2int8(primary->x);
            r.y += clip2int8(primary->y);
        } else {
            r.h += clip2int8(primary->x);
            r.v -= clip2int8(primary->y);
        }
    }
    if (secondary) {
        r.h += clip2int8(secondary->x);
        r.v -= clip2int8(secondary->y);
    }
    pointing_device_set_report(r);
    pointing_device_send();
}

__attribute__((weak)) void keyball_process_trackball_user(
        const trackball_delta_t *primary,
        const trackball_delta_t *secondary) {
    keyball_process_trackball_default(primary, secondary);
}

static bool should_swap_primary_trackball(void) {
    // TODO: support trackball handness.
    return !trackball_has() || is_keyboard_left();
}

void keyball_adjust_trackball_handness(void) {
    if (should_swap_primary_trackball()) {
        primary = 1;
        secondary = 0;
    }
}

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_DRIVER_ENABLE
    // Format: `Ball:{ball#1 x}{ball#1 y}{ball#2 x}{ball#2 y}
    //
    // Output example:
    //
    //     Ball: -12  34   0   0
    //
    oled_write_P(PSTR("Ball:"), false);
    oled_write(format_d(ball1.x, 4), false);
    oled_write(format_d(ball1.y, 4), false);
    oled_write(format_d(ball2.x, 4), false);
    oled_write(format_d(ball2.y, 4), false);
#endif
}

const char PROGMEM code_to_name[] = {
    'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i',  'j',
    'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't',
    'u', 'v', 'w', 'x', 'y', 'z',  '1', '2', '3',  '4',
    '5', '6', '7', '8', '9', '0',  'R', 'E', 'B',  'T',
    '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`',
    ',', '.', '/',
};

void keyball_oled_render_keyinfo(void) {
#ifdef OLED_DRIVER_ENABLE
    // Format: `Key:   R{row}  C{col} K{key code}  '{key name}`
    //
    // It is aligned to fit with output of keyball_oled_render_ballinfo().
    // For example:
    //
    //     Ball:   0   0   0   0
    //     Key:   R3  C2 K57  'B
    //
    char name = '\0';
    uint16_t keycode = last_keycode;
    if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) ||
        (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
        keycode &= 0xFF;
    }
    if (keycode >= 4 && keycode < 53) {
        name = pgm_read_byte(code_to_name + keycode - 4);
    }

    oled_write_P(PSTR("Key:   R"), false);
    oled_write(format_d(last_row, 1), false);
    oled_write_P(PSTR("  C"), false);
    oled_write(format_d(last_col, 1), false);
    if (keycode) {
        oled_write_P(PSTR(" K"), false);
        oled_write(format_02x((uint8_t)keycode), false);
    }
    if (name) {
        oled_write_P(PSTR("  '"), false);
        oled_write_char(name, false);
    }
#endif
}
