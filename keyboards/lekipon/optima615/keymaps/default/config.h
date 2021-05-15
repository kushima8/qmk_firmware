/* Copyright 2021 Lekipon
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

#define TAPPING_TERM 150    // TapからHoldに切り替わるまでの押下時間(ms)。デフォルト値は200。
#define PERMISSIVE_HOLD     // TAPPING_TERM経過前に他のキーを押すと、Holdとして扱われるようになる。キーの同時押しが速い人向け。
//#define RETRO_TAPPING       // TAPPING_TERMを経過しても、他のキーを押していなければTapとして扱われるようになる。
                            // TAPPING_TERMが短くてTapのつもりがHold扱いになってしまう場合の対策になるが、好み次第。
                            // TAPPING_TERMを500以上に設定していると、自動的に有効になる。
//#define TAPPING_FORCE_HOLD  // Mod Tap keyをTap後にすぐにHoldしたときに、Holdとして扱われるようになる
//#define IGNORE_MOD_TAP_INTERRUPT    // Mod Tap keyと通常キーを連続して高速に押したとき、両方のキーをTapとして扱うよう になる。

// place overrides here
#define ENCODER_RESOLUTION 2  //For Using ALPS EC11 Series

// /*== all animations enable ==*/
//#define RGBLIGHT_ANIMATIONS
#undef RGBLIGHT_ANIMATIONS
// /*== or choose animations ==*/
  #define RGBLIGHT_EFFECT_BREATHING
  #define RGBLIGHT_EFFECT_RAINBOW_MOOD
  #define RGBLIGHT_EFFECT_RAINBOW_SWIRL
  #define RGBLIGHT_EFFECT_CHRISTMAS
  #define RGBLIGHT_EFFECT_STATIC_GRADIENT
  #define RGBLIGHT_EFFECT_TWINKLE
//   #define RGBLIGHT_EFFECT_KNIGHT
//   #define RGBLIGHT_EFFECT_SNAKE
//   #define RGBLIGHT_EFFECT_RGB_TEST
//   #define RGBLIGHT_EFFECT_ALTERNATING
