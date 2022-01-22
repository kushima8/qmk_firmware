#include "keyball46.h"

#include <string.h>

#include "quantum.h"
#include "pointing_device.h"
#include "transactions.h"

#include "trackball.h"

#ifndef KEYBALL_SCROLL_DIVIDER
#    define KEYBALL_SCROLL_DIVIDER 10
#endif
#if KEYBALL_SCROLL_DIVIDER <= 0
#    error "KEYBALL_SCROLL_DIVIDER should be larger than zero"
#endif

#ifndef KEYBALL_POINTER_DIVIDER
#    define KEYBALL_POINTER_DIVIDER 1
#endif
#if KEYBALL_POINTER_DIVIDER <= 0
#    error "KEYBALL_POINTER_DIVIDER should be larger than zero"
#endif

// clang-format off
matrix_row_t matrix_mask[MATRIX_ROWS] = {
    0b0111111,
    0b0111111,
    0b0011111,
    0b0111111,

    0b0111111,
    0b0111111,
    0b0011111,
    0b0111111,
};
// clang-format on

static void adjust_matrix_mask_primary(void) {
    int base = is_keyboard_left() ? 0 : 4;
    matrix_mask[base + 2] = 0b0111111;
    matrix_mask[base + 3] = 0b0011111;
}

static void adjust_matrix_mask_secondary(void) {
    int base = is_keyboard_left() ? 4 : 0;
    matrix_mask[base + 2] = 0b0111111;
    matrix_mask[base + 3] = 0b0011111;
    keyball_adjust_trackball_handness();
}

static uint8_t peek_matrix_intersection(pin_t out_pin, pin_t in_pin) {
    setPinInputHigh(in_pin);
    setPinOutput(out_pin);
    writePinLow(out_pin);
    wait_us(1);
    uint8_t pin_state = readPin(in_pin);
    setPinInputHigh(out_pin);
    return pin_state;
}

bool is_keyboard_left(void) {
    return !peek_matrix_intersection(trackball_has() ? F7: F6, B5);
}

static int  primary        = 0;
static int  secondary      = 1;
static bool is_scroll_mode = false;

void keyboard_pre_init_kb() {
    trackball_init();
}

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) { return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v; }

static trackball_delta_t ball1, ball2;

__attribute__((weak)) void pointing_device_task(void) {
    trackball_delta_t d0 = {0}, d1 = {0};
    bool              c0 = trackball_consume_delta(primary, is_scroll_mode ? KEYBALL_SCROLL_DIVIDER : KEYBALL_POINTER_DIVIDER, &d0);
    bool              c1 = trackball_consume_delta(secondary, KEYBALL_SCROLL_DIVIDER, &d1);
    if (c0 || c1) {
        ball1 = d0;
        ball2 = d1;
        keyball_process_trackball_user(&d0, &d1);
    }
}

static uint16_t last_keycode;
static uint8_t  last_row;
static uint8_t  last_col;

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    last_keycode = keycode;
    last_row     = record->event.key.row;
    last_col     = record->event.key.col;
    if (!process_record_user(keycode, record)) {
        return false;
    }

    // translate KC_BTN? event to mouse report
    if (IS_MOUSEKEY_BUTTON(keycode)) {
        report_mouse_t r = {};
        r                = pointing_device_get_report();
        if (record->event.pressed) {
            r.buttons |= 1 << (keycode - KC_MS_BTN1);
        } else {
            r.buttons &= ~(1 << (keycode - KC_MS_BTN1));
        }
        pointing_device_set_report(r);
        return false;
    }
    return true;
}

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

#endif

//////////////////////////////////////////////////////////////////////////////
// tasks for secondary trackball

typedef struct {
    bool              has;
    trackball_delta_t delta;
} trackball_data_t;

trackball_data_t secondary_trackball = {.has = false};

static void get_trackball_data_secondary_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    trackball_data_t *data = (trackball_data_t *)out_data;
    *data                  = secondary_trackball;
}

void matrix_scan_kb(void) {
    // fetch trackball sensor on primary, and apply it.
    trackball_delta_t delta;
    bool              has = trackball_fetch_sensor(&delta);
    trackball_apply_delta(0, has ? &delta : NULL);
    // apply secondary trackball sensor.
    if (secondary_trackball.has) {
        adjust_matrix_mask_secondary();
    }
    trackball_apply_delta(1, secondary_trackball.has ? &secondary_trackball.delta : NULL);
    // delegate to user function.
    matrix_scan_user();
}

void matrix_slave_scan_kb(void) {
    // fetch trackball sensor on secondary.
    trackball_delta_t delta;
    bool              has = trackball_fetch_sensor(&delta);
    // prepare to send data to primary.
    if (has) {
        secondary_trackball.has   = true;
        secondary_trackball.delta = delta;
    }
    // delegate to user function.
    matrix_slave_scan_user();
}

void housekeeping_task_kb(void) {
    // receive secondary trackball data .
    if (is_keyboard_master()) {
        trackball_data_t data;
        if (transaction_rpc_recv(GET_TRACKBALL_DATA, sizeof(data), &data)) {
            secondary_trackball = data;
        }
    }
}

void keyboard_post_init_kb(void) {
    if (trackball_has()) {
        adjust_matrix_mask_primary();
    }
    // register transaction to get trackball data.
    transaction_register_rpc(GET_TRACKBALL_DATA, get_trackball_data_secondary_handler);
    // delegate to user function.
    keyboard_post_init_user();
}

//////////////////////////////////////////////////////////////////////////////
// Keyball API

bool keyball_get_scroll_mode(void) { return is_scroll_mode; }

void keyball_set_scroll_mode(bool mode) {
    if (is_scroll_mode != mode) {
        is_scroll_mode = mode;
        trackball_reset_delta(primary);
    }
}

void keyball_process_trackball_default(const trackball_delta_t *primary, const trackball_delta_t *secondary) {
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

__attribute__((weak)) void keyball_process_trackball_user(const trackball_delta_t *primary, const trackball_delta_t *secondary) { keyball_process_trackball_default(primary, secondary); }

static bool should_swap_primary_trackball(void) {
    // TODO: support trackball handness.
    return !trackball_has() || is_keyboard_left();
}

void keyball_adjust_trackball_handness(void) {
    if (should_swap_primary_trackball()) {
        primary   = 1;
        secondary = 0;
    }
}

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Ball:{ball#1 x}{ball#1 y}{ball#2 x}{ball#2 y}
    //
    // Output example:
    //
    //     Ball: -12  34   0   0
    //
    oled_write_P(PSTR("Ball:"), false);
    oled_write(format_4d(ball1.x), false);
    oled_write(format_4d(ball1.y), false);
    oled_write(format_4d(ball2.x), false);
    oled_write(format_4d(ball2.y), false);
#endif
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

void keyball_oled_render_keyinfo(void) {
#ifdef OLED_ENABLE
    // Format: `Key:   R{row}  C{col} K{kc}  '{name}`
    //
    // Where `kc` is lower 8 bit of keycode.
    // Where `name` is readable label for `kc`, valid between 4 and 56.
    //
    // It is aligned to fit with output of keyball_oled_render_ballinfo().
    // For example:
    //
    //     Ball:   0   0   0   0
    //     Key:   R2  C3 K06  'c
    //
    char    name    = '\0';
    uint8_t keycode = last_keycode;
    if (keycode >= 4 && keycode < 53) {
        name = pgm_read_byte(code_to_name + keycode - 4);
    }

    oled_write_P(PSTR("Key:   R"), false);
    oled_write_char(to_1x(last_row), false);
    oled_write_P(PSTR("  C"), false);
    oled_write_char(to_1x(last_col), false);
    if (keycode) {
        oled_write_P(PSTR(" K"), false);
        oled_write_char(to_1x(keycode >> 4), false);
        oled_write_char(to_1x(keycode), false);
        // oled_write(format_02x((uint8_t)keycode), false);
    }
    if (name) {
        oled_write_P(PSTR("  '"), false);
        oled_write_char(name, false);
    }
#endif
}
