/*
Copyright 2022 @Yowkees
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

#include QMK_KEYBOARD_H

#include "quantum.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_universal(
    _______  , _______  , _______  , _______  , _______  , _______  ,                                  KC_1     , KC_2     , KC_3     , KC_4     , KC_5     , KC_ESC   ,
    _______  , _______  , _______  , _______  , _______  , _______  ,                                  KC_Z     , KC_U     , KC_W     , KC_I     , KC_F3    , KC_TAB   ,
    _______  , _______  , _______  , _______  , _______  , _______  ,                                  KC_S     ,C(KC_BTN1), KC_BTN2  , KC_D     , KC_LBRC  , KC_RBRC  ,
    _______  , _______  , _______  , _______  , _______  , _______  , _______  ,            KC_LSFT  , KC_ENT   , KC_M     , KC_A     , KC_Q     , KC_MINS  , KC_EQL   ,
    _______  , _______  , _______  , _______  , _______  , _______  , _______  ,            KC_SPC   , KC_BTN1  , _______  , _______  , _______  , KC_PGDN  , KC_PGUP
  ),
};
// clang-format on
//
void keyboard_post_init_user() {
#ifdef RGBLIGHT_ENABLE
    rgblight_disable_noeeprom();
#endif
    keyball_set_cpi(19);
    keyball_set_scroll_div(5);
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
}
#endif
