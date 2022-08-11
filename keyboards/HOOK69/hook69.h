/* Copyright 2022 kushima8
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "quantum.h"

/* This is a shortcut to help you visually see your layout.
 *
 * The first section contains all of the arguments representing the physical
 * layout of the board and position of the keys.
 *
 * The second converts the arguments into a two-dimensional array which
 * represents the switch matrix.
 */
#define LAYOUT( \
    L00, L03, L10, L13, L20, L23, L30, L33, L40, L43, L50, L53, R01, R04, R11, R14, R21, R24, R31, R34, R41, R44, R51, R54, \
    L01, L04, L11, L14, L21, L24, L31, L34, L41, L44, L51, L54, R02, R05, R12, R15, R22, R25, R32, R35, R42, R45, R52, \
    L02, L05, L12, L15, L22, L25, L32, L35, L42, L45, L52, R00, R03, R10, R13, R20, R23, R30, R33, R40, R43, R50, R53  \
) \
{ \
    { L00,    L01,    L02,    L03,    L04,    L05   }, \
    { L10,    L11,    L12,    L13,    L14,    L15   }, \
    { L20,    L21,    L22,    L23,    L24,    L25   }, \
    { L30,    L31,    L32,    L33,    L34,    L35   }, \
    { L40,    L41,    L42,    L43,    L44,    L45   }, \
    { L50,    L51,    L52,    L53,    L54,    KC_NO }, \
    { R00,    R01,    R02,    R03,    R04,    R05   }, \
    { R10,    R11,    R12,    R13,    R14,    R15   }, \
    { R20,    R21,    R22,    R23,    R24,    R25   }, \
    { R30,    R31,    R32,    R33,    R34,    R35   }, \
    { R40,    R41,    R42,    R43,    R44,    R45   }, \
    { R50,    R51,    R52,    R53,    R54,    KC_NO } \
}
