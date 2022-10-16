/* Copyright 2021 yushakobo
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
#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT (
    KC_PSCR , KC_SLCK  , KC_PAUS ,   KC_MUTE ,
    KC_INS  , KC_HOME  , KC_PGUP ,   KC_NUM  ,  KC_PSLS ,  KC_PAST ,  KC_PMNS ,
    KC_DEL  , KC_END   , KC_PGDN ,   KC_P7   ,  KC_P8   ,  KC_P9   ,  KC_PPLS ,
                                     KC_P4   ,  KC_P5   ,  KC_P6   ,  KC_PPLS ,
    MO(1)   , KC_UP    , MO(2)   ,   KC_P1   ,  KC_P2   ,  KC_P3   ,  KC_PENT ,
    KC_LEFT , KC_DOWN  , KC_RGHT ,   KC_P0   ,  KC_P0   ,  KC_PDOT ,  KC_PENT
  ),

  [_FN1] = LAYOUT (
    _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,
                                     _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  MO(3)   ,  _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______
  ),

  [_FN2] = LAYOUT (
    _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  KC_F10  ,  KC_F11  ,  KC_F12  ,
    _______ ,  _______ ,  _______ ,  KC_F7   ,  KC_F8   ,  KC_F9   ,  _______ ,
                                     KC_F4   ,  KC_F5   ,  KC_F6   ,  _______ ,
    MO(3)   ,  _______ ,  _______ ,  KC_F1   ,  KC_F2   ,  KC_F3   ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______
  ),

  [_FN3] = LAYOUT (
    _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,
                                     _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,
    _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______ ,  _______
  )
};

#ifdef RGBLIGHT_LAYERS
# define LOCK_COLOR_1 HSV_SPRINGGREEN

const rgblight_segment_t PROGMEM navpad_capslock[] = RGBLIGHT_LAYER_SEGMENTS(
  {4, 1, LOCK_COLOR_1}
);

const rgblight_segment_t PROGMEM navpad_numlock[] = RGBLIGHT_LAYER_SEGMENTS(
  {0, 1, LOCK_COLOR_1},
  {5, 1, LOCK_COLOR_1}
);

const rgblight_segment_t PROGMEM navpad_scrolllock[] = RGBLIGHT_LAYER_SEGMENTS(
  {2, 1, LOCK_COLOR_1},
  {6, 1, LOCK_COLOR_1}
);


bool led_update_user(led_t led_state) {
    rgblight_set_layer_state(0, host_keyboard_led_state().caps_lock);
    rgblight_set_layer_state(1, (host_keyboard_led_state().num_lock && IS_LAYER_ON(_BASE)));
    rgblight_set_layer_state(2, host_keyboard_led_state().scroll_lock);
    return true;
}

const rgblight_segment_t* const PROGMEM navpad_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
  navpad_capslock,
  navpad_numlock,
  navpad_scrolllock
);

void keyboard_post_init_user(void) {
    rgblight_sethsv_noeeprom(HSV_OFF);
    // Enable the LED layers
    rgblight_layers = navpad_rgb_layers;
}
#endif
