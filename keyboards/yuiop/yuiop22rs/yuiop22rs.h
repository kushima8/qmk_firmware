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

#pragma once

#include "quantum.h"

#define LAYOUT( \
                   K03, K04, \
    K10, K11, K12, K13, K14, \
    K20, K21, K22, K23, K24, \
    K30, K31, K32, K33, K34, \
    K40, K41, K42, K43, K44  \
) \
{ \
    { KC_NO, KC_NO, KC_NO,   K03,   K04 }, \
    {   K10,   K11,   K12,   K13,   K14 }, \
    {   K20,   K21,   K22,   K23,   K24 }, \
    {   K30,   K31,   K32,   K33,   K34 }, \
    {   K40,   K41,   K42,   K43,   K44 }, \
    { KC_NO, KC_NO, KC_NO, KC_NO, KC_NO }, \
}

extern uint8_t rotary_switch_state;

__attribute__((weak)) void rotary_switch_update_state_kb(uint8_t state);
__attribute__((weak)) void rotary_switch_update_state_user(uint8_t state);
