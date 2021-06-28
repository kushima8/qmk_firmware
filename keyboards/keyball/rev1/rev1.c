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

static const char * format_i16(int16_t n, int w) {
    static char buf[7];
    buf[w] = '\0';
    --w;
    bool minus = n < 0;
    if (minus) {
        n = -n;
    }
    do {
        buf[w] = (n % 10) + '0';
        w--;
        n /= 10;
    } while(n != 0 && w >= 0);
    if (minus && w >= 0) {
        buf[w] = '-';
        w--;
    }
    while (w >= 0) {
        buf[w] = ' ';
        w--;
    }
    return buf;
}

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_DRIVER_ENABLE
    oled_write_P(PSTR("Ball:"), false);
    oled_write(format_i16(ball1.x, 4), false);
    oled_write(format_i16(ball1.y, 4), false);
    oled_write(format_i16(ball2.x, 4), false);
    oled_write(format_i16(ball2.y, 4), false);
#endif
}
