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
#include QMK_KEYBOARD_H

#ifdef RGBLIGHT_ENABLE
//Following line allows macro to read current RGB settings
extern rgblight_config_t rgblight_config;
#endif

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
  RGB_RST = SAFE_RANGE
};

// Defines names for use in layer keycodes and the keymap
enum layer_number {
    _ANSI = 0,
};

// Defines the keycodes used by our macros in process_record_user
#define KANA    LALT(KC_GRV)

int RGB_current_mode;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  switch (keycode) {
    case RGB_RST:
      #ifdef RGBLIGHT_ENABLE
        if (record->event.pressed) {
          eeconfig_update_rgblight_default();
          rgblight_enable();
          RGB_current_mode = rgblight_config.mode;
      }
      #endif
      return false;
      break;
  }
  return true;
}


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

 /* ANSI Mode
   * ,-----------------------------------------------------------------------------------------------------------------------.
   * | Esc   |   1   |   2   |   3   |   4   |   5   | CAPS  |   6   |   7   |   8   |   9   |   0   |   -   |   =   |  Num  |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Tab   |   Q   |   W   |   E   |   R   |   T   |  B/S  |   Y   |   U   |   I   |   O   |   P   |   [   |   ]   |  B/S  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |   A   |   S   |   D   |   F   |   G   |-------|   H   |   J   |   K   |   L   |   ;   |   '   |       |  Del  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------|       |-------|
   * | Shift |   Z   |   X   |   C   |   V   |   B   | Enter |   N   |   M   |   ,   |   .   |   /   |   \   | Shift |  Ins  |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |  Win  | Menu  |  Alt  |     Space     | Menu  |     Space     | PGUP  |  Home |  UP   |  End  |       | Ctrl  |
   * |-------+-------+-------+-------+---------------------------------------+-------+-------+-------+-------|       |-------|
   * | RGBRST|    KANA   |  Shift    |                                       | PGDN  |  Left | Down  | Right | Enter |  Alt  |
   * `-------------------------------'　　　　　　　　　　　　　　　　          `-----------------------------------------------'
   */
[_ANSI] = LAYOUT_ortho_6x15( /* Base ANSI */
 KC_ESC , KC_1   , KC_2   , KC_3   , KC_4   , KC_5   , KC_CAPS, KC_6   , KC_7   , KC_8   , KC_9   , KC_0   , KC_MINS, KC_EQL , KC_NLCK,\
 KC_TAB , KC_Q   , KC_W   , KC_E   , KC_R   , KC_T   , KC_BSPC, KC_Y   , KC_U   , KC_I   , KC_O   , KC_P   , KC_LBRC, KC_RBRC, KC_BSPC,\
 KC_LCTL, KC_A   , KC_S   , KC_D   , KC_F   , KC_G   ,          KC_H   , KC_J   , KC_K   , KC_L   , KC_SCLN, KC_QUOT,          KC_DEL ,\
 KC_LSFT, KC_Z   , KC_X   , KC_C   , KC_V   , KC_B   , KC_ENT , KC_N   , KC_M   , KC_COMM, KC_DOT , KC_SLSH, KC_BSLS, KC_RSFT, KC_INS ,\
 KC_LCTL, KC_LGUI, KC_APP , KC_LALT,   KC_SPC        , KC_APP ,     KC_SPC      , KC_PGUP, KC_HOME, KC_UP  , KC_END ,          KC_RCTL,\
 RGB_RST,       KANA  ,     KC_LSFT,                                              KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT , KC_RALT \
)
};

#ifdef RGBLIGHT_ENABLE
void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom();
    //rgblight_mode(RGBLIGHT_MODE_RAINBOW_SWIRL);
    rgblight_mode_noeeprom(RGBLIGHT_MODE_RGB_TEST);
}
#endif

/*
void matrix_init_user(void) {
}

void matrix_scan_user(void) {
}

bool led_update_user(led_t led_state) {
    return true;
}*/


