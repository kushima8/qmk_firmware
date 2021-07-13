/*
Copyright 2021 MURAOKA Taro (aka KoRoN, @kaoriya)

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

#include "yuiop2pd.h"

#include "pointing_device.h"
#include "analog.h"

const int16_t margin        = 32;
const int16_t div_pointer   = 16;
const int16_t div_scroll    = 256;
const int16_t interval      = 16;

void pointing_device_init(void) {
    analogReference(ADC_REF_POWER);
}

static int16_t add_i16(int16_t a, int16_t b) {
    int16_t r = a + b;
    if (a >= 0 && b >= 0 && r < 0) {
        r = 32767;
    } else if (a < 0 && b < 0 && r >= 0) {
        r = -32768;
    }
    return r;
}

static int16_t add_i8(int8_t a, int8_t b) {
    int16_t r = a + b;
    if (a >= 0 && b >= 0 && r < 0) {
        r = 127;
    } else if (a < 0 && b < 0 && r >= 0) {
        r = -128;
    }
    return r;
}

static bool is_far(int16_t x, int16_t y, int16_t d) {
    if (x >= d || x <= -d || y >= d || y <= -d) {
        return true;
    }
    return x*x + y*y >= d*d;
}

static int16_t consume(int16_t *p, int16_t div) {
    int16_t r = *p / div;
    *p %= div;
    return r;
}

static int8_t to_i8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

static int16_t cum_x = 0;
static int16_t cum_y = 0;
static int16_t cum_v = 0;
static int16_t cum_h = 0;

void pointing_device_task(void) {
    static int8_t count = 0;

    int16_t dx = analogReadPin(F7) - 512;
    int16_t dy = analogReadPin(F6) - 512;
    int16_t dv = analogReadPin(F4) - 512;
    int16_t dh = analogReadPin(F5) - 512;
    if (is_far(dx, dy, margin)) {
        cum_x = add_i16(cum_x, -dx);
        cum_y = add_i16(cum_y, -dy);
    }
    if (is_far(dv, dh, margin)) {
        cum_v = add_i16(cum_v, dv);
        cum_h = add_i16(cum_h, -dh);
    }
    count++;

    if (count >= interval) {
        count = 0;
        report_mouse_t r = pointing_device_get_report();
        r.x = add_i8(r.x, to_i8(consume(&cum_x, div_pointer * interval)));
        r.y = add_i8(r.y, to_i8(consume(&cum_y, div_pointer * interval)));
        r.v = add_i8(r.v, to_i8(consume(&cum_v, div_scroll * interval)));
        r.h = add_i8(r.h, to_i8(consume(&cum_h, div_scroll * interval)));
        pointing_device_set_report(r);
    }

    pointing_device_send();
}
