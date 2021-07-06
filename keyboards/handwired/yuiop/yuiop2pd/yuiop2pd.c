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

void pointing_device_init(void) {
    analogReference(ADC_REF_POWER);
}

typedef struct {
    int16_t v;
    int16_t div;
} vd_t;

static int16_t vd_put(vd_t *p, int16_t delta) {
    p->v += delta;
    // FIXME: consider overflow.
    int16_t r = p->v / p->div;
    p->v %= p->div;
    return r;
}

static vd_t x = {0, 16};
static vd_t y = {0, 16};
static vd_t v = {0, 128};
static vd_t h = {0, 128};

static int16_t filter(int16_t v, int16_t d) {
    return (v >= d || v <= -d) ? v : 0;
}

void pointing_device_task(void) {
    report_mouse_t r = pointing_device_get_report();
    r.x -= vd_put(&x, filter(analogReadPin(F7) - 512, 16));
    r.y -= vd_put(&y, filter(analogReadPin(F6) - 512, 16));
    r.v -= vd_put(&v, filter(analogReadPin(F5) - 512, 16));
    r.h += vd_put(&h, filter(analogReadPin(F4) - 512, 16));
    pointing_device_set_report(r);

    pointing_device_send();
}
