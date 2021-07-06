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

void pointing_device_task(void) {
    report_mouse_t r = pointing_device_get_report();

    int16_t dx = analogReadPin(F7) - 512;
    r.x -= dx / 16;
    int16_t dy = analogReadPin(F6) - 512;
    r.y -= dy / 16;

    int16_t dv = analogReadPin(F5) - 512;
    r.v -= dv / 128;
    int16_t dh = analogReadPin(F4) - 512;
    r.h += dh / 128;

    // TODO: modify r
    pointing_device_set_report(r);
    pointing_device_send();
}
