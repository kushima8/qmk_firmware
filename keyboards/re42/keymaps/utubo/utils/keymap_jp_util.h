/* Copyright 2020 utubo
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

/*
 * SHIFT+JP_XXXをANSI配列のように対応させます。(対応させた場合、ホールドは適用されません)
 * 以下のように実行してください。
 *     bool process_record_user(uint16_t keycode, keyrecord_t *record) {
 *         return bool process_jp_symbols(keycode, record);
 *     }
 */
bool process_jp_symbols(uint16_t keycode, keyrecord_t *record);

/* tap_code16を上記に対応させたものです。 */
#define TAP_CODE16JP
void tap_code16jp(uint16_t keycode);

