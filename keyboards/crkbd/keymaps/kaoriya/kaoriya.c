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

#include QMK_KEYBOARD_H

#define KEYMAP_VERSION 8

#include <stdio.h>

char kaoriya_verinfo[22] = {};

void keyboard_post_init_user(void) {
    // compose version info string.
    snprintf(kaoriya_verinfo, sizeof(kaoriya_verinfo), "+KaoriYa/%s+m%d",
        STR(KAORIYA_FIRMWARE_VERSION), KEYMAP_VERSION);
    int l = strlen(kaoriya_verinfo);
    int m = sizeof(kaoriya_verinfo) - 1 - l;
    if (m > 0) {
        memmove(kaoriya_verinfo + m, kaoriya_verinfo, l);
        memset(kaoriya_verinfo, ' ', m);
    }
}
