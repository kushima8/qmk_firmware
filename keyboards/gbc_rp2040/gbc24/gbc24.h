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
/* 物理配置順。J17/J25/J26/J34/J43 はジョイスティック (上/左/右/下) と押し込みの
 * 仮想マトリクス位置 (lib/joystick_user の dir_matrix_pos、gbc24.c の JOY_SW_*)。
 * VIA/Vial の gbc24.json / vial.json と同じ 30 位置。 */
#define LAYOUT( \
    K00, K01, K02, K03, K10, K11, K12, \
    K13, K20, K21, K22, K23, K30, K31, \
         K32, K33, K50, K51, K52,                 J17, \
              K53, K60, K61,                 J25, J43, J26, \
                                  K40,            J34, \
                                       K41, K42 \
) \
{ \
    { K00,   K01,   K02,   K03,   K50,   K51,   K52,   K53 }, \
    { K10,   K11,   K12,   K13,   K60,   K61,   KC_NO, J17 }, \
    { K20,   K21,   K22,   K23,   KC_NO, J25,   J26,   KC_NO }, \
    { K30,   K31,   K32,   K33,   J34,   KC_NO, KC_NO, KC_NO }, \
    { K40,   K41,   K42,   J43,   KC_NO, KC_NO, KC_NO, KC_NO }  \
}

