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

#pragma once

#include "quantum.h"

// clang-format off

#define LAYOUT( \
    L00, L01, L02, \
    L10, L11, L12, \
    L20, L21, L22, \
    L30, L31, L32, \
    L40, L41, L42  \
    ) \
    { \
        { L00, L01, L02, KC_NO }, \
        { L10, L11, L12, KC_NO }, \
        { L20, L21, L22, KC_NO }, \
        { L30, L31, L32, KC_NO }, \
        { L40, L41, L42, KC_NO }  \
    }

// clang-format on
