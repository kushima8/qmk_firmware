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

#ifdef SSD1306OLED
  #include "ssd1306.h"
  #include <string.h>
  #include "print.h"
  #include "common/glcdfont.c"
  #include "sendchar.h"
  #include "common/oled_helper.h"
#endif

// Defines the keycodes used by our macros in process_record_user
enum custom_keycodes {
  RGB_RST = SAFE_RANGE, //Reset RGB State (Red) 
  KC_P00, // Double Zero
  JU_2,  
  JU_6, 
  JU_7,
  JU_8,
  JU_9,
  JU_0,
  JU_MINS,
  JU_EQL,
  JU_LBRC,
  JU_RBRC,
  JU_BSLS,
  JU_SCLN,
  JU_QUOT,
  JU_GRV,
  JU_CAPS
};

// Defines names for use in layer keycodes and the keymap
enum layer_number {
    _ANSI = 0,
    _JIS,
    _FUNC,
    _ADJUST,
};

// Defines the keycodes used by our macros in process_record_user
#define FN      MO(_FUNC)
#define ADJUST  MO(_ADJUST) 
#define ANSI    DF(_ANSI)  //For 104 Keyboard in OS Setting
#define JIS     DF(_JIS)   //For 109 Keyboard in OS Setting
#define KANA    LALT(KC_GRV)  // Zenkaku_Hankaku
#define UNDO    LCTL(KC_Z)
#define CUT     LCTL(KC_X)
#define COPY    LCTL(KC_C)
#define PASTE   LCTL(KC_V)


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
 /* ANSI Mode
   * ,-----------------------------------------------------------------------------------------------------------------------.
   * | Esc   |   1   |   2   |   3   |   4   |   5   | CAPS  |   6   |   7   |   8   |   9   |   0   |   -   |   =   | ADJUST|
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Tab   |   Q   |   W   |   E   |   R   |   T   |  B/S  |   Y   |   U   |   I   |   O   |   P   |   [   |   ]   |  B/S  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |   A   |   S   |   D   |   F   |   G   |-------|   H   |   J   |   K   |   L   |   ;   |   '   |       |  Del  |
   * |- - - -|-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------|       |-------|
   * | Shift |   Z   |   X   |   C   |   V   |   B   | Enter |   N   |   M   |   ,   |   .   |   /   |   \   | Shift |  Ins  |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |  Win  | Menu  |  Alt  |     Space     | Menu  |     Space     | PGUP  |  Home |  UP   |  End  |       | Ctrl  |
   * |-------+-------+-------+-------+---------------------------------------+-------+-------+-------+-------|       |-------|
   * |  Fn   |    KANA   |   Ctrl    |                                       | PGDN  |  Left | Down  | Right | Enter |  Alt  |
   * `-------------------------------'　　　　　　　　　　　　　　　　          `-----------------------------------------------'
   */
[_ANSI] = LAYOUT_ortho_6x15( /* Base ANSI Mode */
 KC_ESC , KC_1   , KC_2   , KC_3   , KC_4   , KC_5   , KC_CAPS, KC_6   , KC_7   , KC_8   , KC_9   , KC_0   , KC_MINS, KC_EQL , ADJUST ,\
 KC_TAB , KC_Q   , KC_W   , KC_E   , KC_R   , KC_T   , KC_BSPC, KC_Y   , KC_U   , KC_I   , KC_O   , KC_P   , KC_LBRC, KC_RBRC, KC_BSPC,\
 KC_LCTL, KC_A   , KC_S   , KC_D   , KC_F   , KC_G   ,          KC_H   , KC_J   , KC_K   , KC_L   , KC_SCLN, KC_QUOT,          KC_DEL ,\
 KC_LSFT, KC_Z   , KC_X   , KC_C   , KC_V   , KC_B   , KC_ENT , KC_N   , KC_M   , KC_COMM, KC_DOT , KC_SLSH, KC_BSLS, KC_RSFT, KC_INS ,\
 KC_LCTL, KC_LGUI, KC_APP , KC_LALT,   KC_SPC        , KC_APP ,     KC_SPC      , KC_PGUP, KC_HOME, KC_UP  , KC_END ,          KC_RCTL,\
 FN     ,       KANA  ,   KC_LCTL  ,                                              KC_PGDN, KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT , KC_RALT \
),

/* JIS Mode
   * ,-----------------------------------------------------------------------------------------------------------------------.
   * | Esc   |   1   |  JU2  |   3   |   4   |   5   |JU_CAPS|  JU6  |  JU7  |  JU8  |  JU9  |  JU0  |  JU-  |  JU=  | ADJUST|
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Tab   |   Q   |   W   |   E   |   R   |   T   |  B/S  |   Y   |   U   |   I   |   O   |   P   |  JU[  |  JU]  |  B/S  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |   A   |   S   |   D   |   F   |   G   |-------|   H   |   J   |   K   |   L   |  JU;  |  JU'  |       |  Del  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------|       |-------|
   * | Shift |   Z   |   X   |   C   |   V   |   B   | Enter |   N   |   M   |   ,   |   .   |   /   |  JU\  | Shift |  Ins  |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |  Win  | Menu  |  Alt  |     Space     | Menu  |     Space     | PGUP  |  Home |  UP   |  End  |       | Ctrl  |
   * |-------+-------+-------+-------+---------------------------------------+-------+-------+-------+-------|       |-------|
   * |  Fn   |    KANA   |    Ctrl   |                                       | PGDN  |  Left | Down  | Right | Enter |  Alt  |
   * `-------------------------------'　　　　　　　　　　　　　　　　          `-----------------------------------------------'
   */
[_JIS] = LAYOUT_ortho_6x15( /* JIS Mode */
 _______, _______, JU_2   , _______, _______, _______, JU_CAPS, JU_6   , JU_7   , JU_8   , JU_9   , JU_0   , JU_MINS, JU_EQL , ADJUST ,\
 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, JU_LBRC, JU_RBRC, _______,\
 _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, JU_SCLN, JU_QUOT,          _______,\
 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, JU_BSLS, _______, _______,\
 _______, _______, _______, _______,   _______       , _______,     _______     , _______, _______, _______, _______,          _______,\
 _______,      _______,  _______   ,                                              _______, _______, _______, _______, _______, _______ \
),

/* FUNC
   * ,-----------------------------------------------------------------------------------------------------------------------.
   * |   `   |  F1   |  F2   |  F3   |  F4   |  F5   |  Ins  |  F6   |  F7   |  F8   |  F9   |  F10  |  F11  |  F12  |NumLock|
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Tab   |       |       |       |       |       |  Del  |       |       |       |   =   |  P/   |  P*   |   P-  |  B/S  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |       |       |       |       |       |-------| Home  |  UP   |  End  |   P7  |  P8   |  P9   |       |  Del  |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------|       |-------|
   * | Shift | Undo  |  Cut  | Copy  | Paste |       | _____ |  LEFT |  DOWN | RIGHT |   P4  |  P5   |  P6   |   P+  |  Ins  |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * | Ctrl  |  Win  | Menu  |  Alt  |     Space     | Menu  |     Space     | PGUP  |   P1  |  P2   |  P3   |       | Ctrl  |
   * |-------+-------+-------+-------+---------------------------------------+-------+-------+-------+-------|       |-------|
   * |  Fn   |    KANA   |   Ctrl    |                                       | PGDN  |   P0  |  P00  |  P.   | PEnter|  Alt  |
   * `-------------------------------'　　　　　　　　　　　　　　　　          `-----------------------------------------------'
   */
[_FUNC] = LAYOUT_ortho_6x15( /* Function */
 KC_GRV , KC_F1  , KC_F2  , KC_F3  , KC_F4  , KC_F5  , KC_INS , KC_F6  , KC_F7  , KC_F8  , KC_F9  , KC_F10 , KC_F11 , KC_F12 , KC_NLCK,\
 _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_DEL , XXXXXXX, XXXXXXX, XXXXXXX, KC_EQL , KC_PSLS, KC_PAST, KC_PMNS, _______,\
 _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          KC_HOME, KC_UP  , KC_END , KC_P7  , KC_P8  , KC_P9  ,          _______,\
 _______, UNDO   , CUT    , COPY   , PASTE  , XXXXXXX, _______, KC_LEFT, KC_DOWN, KC_RGHT, KC_P4  , KC_P5  , KC_P6  , KC_PPLS, _______,\
 _______, _______, _______, _______,   _______       , _______,     _______     , _______, KC_P1  , KC_P2  , KC_P3  ,          _______,\
 _______,      _______,   _______  ,                                              _______, KC_P0  , KC_P00 , KC_PDOT, KC_PENT, _______ \
),

/* ADJUST
   * ,-----------------------------------------------------------------------------------------------------------------------.
   * |RGB_RST|       |       |       |       |       |       |       |       |       |       | PrtSC |ScrLock| Pause | _____ |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * |       |       |  HUE+ |  SAT+ |  VAL+ |  MOD+ |       |       |       |       |       |       |       |       |       |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------+-------+-------|
   * |       |       |  HUE- |  SAT- |  VAL- |  MOD- |-------|       |  MUTE |  VOL- |  VOL+ |       |       |       |       |
   * |-------+-------+-------+-------+-------+-------|       |-------+-------+-------+-------+-------+-------|       |-------|
   * |       |       |       |       |       |RGB_TOG|       |       |       |       |       |       |       |       |       |
   * |-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------|
   * |       |       |       |       |    ANSI_MODE  |       |    JIS_MODE   |       |       |       |       |       |       |
   * |-------+-------+-------+-------+---------------------------------------+-------+-------+-------+-------|       |-------|
   * |       |           |           |                                       |       |       |       |       |       |       |
   * `-------------------------------'　　　　　　　　　　　　　　　　          `-----------------------------------------------'
   */
[_ADJUST] = LAYOUT_ortho_6x15( /* Adjust */
 RGB_RST, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_PSCR, KC_SLCK, KC_PAUS, _______,\
 XXXXXXX, XXXXXXX, RGB_HUI, RGB_SAI, RGB_VAI, RGB_MOD, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,\
 XXXXXXX, XXXXXXX, RGB_HUD, RGB_SAD, RGB_VAD, RGB_RMOD,         XXXXXXX, KC_MUTE, KC_VOLD, KC_VOLU, XXXXXXX, XXXXXXX,          XXXXXXX,\
 XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RGB_TOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,\
 XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,     ANSI        , XXXXXXX,       JIS       , XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,          XXXXXXX,\
 XXXXXXX,      XXXXXXX,   XXXXXXX  ,                                              XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX \
)

};

#ifdef OLED_DRIVER_ENABLE
void oled_task_user(void) {
  oled_write_P(PSTR("Mode :"), false);
  switch (get_highest_layer(default_layer_state)) {

    case _ANSI:
      oled_write_P(PSTR("ANSI(104)\n"), false);
      break;
    case _JIS:
      oled_write_P(PSTR("JIS(109)\n"), false);
      break;
  }

  oled_write_P(PSTR("Layer:"), false);
  switch (get_highest_layer(layer_state)) {
    case _FUNC:
      oled_write_P(PSTR("FUNCTION\n"), false);
      break;
    case _ADJUST:
      oled_write_P(PSTR("ADJUST\n"), false);
      break;
    default:
      // Or use the write_ln shortcut over adding 'n' to the end of your string
      oled_write_ln_P(PSTR("QWERTY"), false);
       break;
  }

  // Host Keyboard LED Status
  oled_write_P(IS_HOST_LED_ON(USB_LED_CAPS_LOCK) ? PSTR("\n<CAPS> ") : PSTR("\n       "), false);
  oled_write_P(IS_HOST_LED_ON(USB_LED_NUM_LOCK) ? PSTR("<NUM> ") : PSTR("      "), false);
  oled_write_P(IS_HOST_LED_ON(USB_LED_SCROLL_LOCK) ? PSTR("<SCR> ") : PSTR("      "), false);
}
#endif

/* Rotary encoder settings */
void encoder_update_user(uint8_t index, bool clockwise) {
  if (index == 0) { /* Left encoder */

//    switch (layer_state) {
    switch (get_highest_layer(layer_state)) {
     case _FUNC:
        if (clockwise) {
          tap_code(KC_LEFT);
        } else {
          tap_code(KC_RGHT);
        }
        break;
     case _ADJUST:
        if (clockwise) {
          rgblight_decrease_val_noeeprom();
        } else {
          rgblight_increase_val_noeeprom();
        }
        break;
     default:
        if (clockwise) {
          tap_code(KC_UP);
        } else {
          tap_code(KC_DOWN);
        }
        break;
    }
  }
}


#ifdef RGBLIGHT_ENABLE
//Following line allows macro to read current RGB settings
extern rgblight_config_t rgblight_config;

// The first three LEDs are used as indicators for CAPS_LOCK, NUM_LOCK and LAYER_STATE.
void keyboard_post_init_user(void) {

	rgblight_set_effect_range(3, RGBLED_NUM-3);
	led_set_user((1<<USB_LED_CAPS_LOCK)|(1<<USB_LED_NUM_LOCK));
    wait_ms(200);
    led_set_user(0);
}

#define HSV_CAPS        0, 255,  96
#define HSV_CAPS_OFF    0, 255,  16
#define HSV_NUMLK      43, 255,  96
#define HSV_NUMLK_OFF  43, 255,  32

void led_set_user(uint8_t usb_led) {
    if (usb_led & (1<<USB_LED_CAPS_LOCK)) {
        sethsv(HSV_CAPS , (LED_TYPE *)&led[0]);
    } else {
        sethsv(HSV_CAPS_OFF, (LED_TYPE *)&led[0]);
    }
    	if (usb_led & (1<<USB_LED_NUM_LOCK)) {
        sethsv(HSV_NUMLK, (LED_TYPE *)&led[1]);
    } else {
        sethsv(HSV_NUMLK_OFF, (LED_TYPE *)&led[1]);
    }
    rgblight_set();
}

#define HSV_ANSI     85, 255,  16 //Green
#define HSV_JIS     169, 255,  48 //Blue
#define HSV_FUNC     85, 255,  96 //Green 
#define HSV_ADJUST  148, 192, 128 //Azure

layer_state_t layer_state_set_user(layer_state_t state) {

  switch (get_highest_layer(default_layer_state)) {
    case _ANSI:
        sethsv(HSV_ANSI, (LED_TYPE *)&led[2]);
      break;
    case _JIS:
        sethsv(HSV_JIS, (LED_TYPE *)&led[2]);
      break;
    default:
        sethsv(HSV_ANSI, (LED_TYPE *)&led[2]);
      break;
  }

  switch (get_highest_layer(state)) {
    case _FUNC:
        sethsv(HSV_FUNC, (LED_TYPE *)&led[2]);
      break;
    case _ADJUST:
        sethsv(HSV_ADJUST, (LED_TYPE *)&led[2]);
      break;
  }

return state;
}

#endif


int RGB_current_mode;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  static bool lshift = false; // added
  static bool rshift = false; // added
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

   case KC_P00:
        if (record->event.pressed) {
      } else {
        tap_code( KC_P0 );   // send 0
        tap_code( KC_P0 );   // send 0 twice without macro
      }
      return false;

    case JU_2:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LBRC);
          unregister_code(KC_LBRC);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_2);
          unregister_code(KC_2);
        }
      }
      return false;
    case JU_6:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_EQL);
          unregister_code(KC_EQL);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_6);
          unregister_code(KC_6);
        }
      }
      return false;
    case JU_7:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_6);
          unregister_code(KC_6);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_7);
          unregister_code(KC_7);
        }
      }
      return false;
    case JU_8:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_QUOT);
          unregister_code(KC_QUOT);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_8);
          unregister_code(KC_8);
        }
      }
      return false;
    case JU_9:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_8);
          unregister_code(KC_8);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_9);
          unregister_code(KC_9);
        }
      }
      return false;
    case JU_0:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_9);
          unregister_code(KC_9);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_0);
          unregister_code(KC_0);
        }
      }
      return false;
    case JU_MINS:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_INT1);
          unregister_code(KC_INT1);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_MINS);
          unregister_code(KC_MINS);
        }
      }
      return false;
    case JU_EQL:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_SCLN);
          unregister_code(KC_SCLN);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_LSFT);
          register_code(KC_MINS);
          unregister_code(KC_MINS);
          unregister_code(KC_LSFT);
        }
      }
      return false;
    case JU_LBRC:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_RBRC);
          unregister_code(KC_RBRC);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_RBRC);
          unregister_code(KC_RBRC);
        }
      }
      return false;
    case JU_RBRC:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_NUHS);
          unregister_code(KC_NUHS);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_NUHS);
          unregister_code(KC_NUHS);
        }
      }
      return false;
    case JU_BSLS:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_INT3);
          unregister_code(KC_INT3);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_INT1);
          unregister_code(KC_INT1);
        }
      }
      return false;
    case JU_SCLN:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_QUOT);
          unregister_code(KC_QUOT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_SCLN);
          unregister_code(KC_SCLN);
        }
      }
      return false;
    case JU_QUOT:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_2);
          unregister_code(KC_2);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_LSFT);
          register_code(KC_7);
          unregister_code(KC_7);
          unregister_code(KC_LSFT);
        }
      }
      return false;
    case JU_GRV:
      if (record->event.pressed) {
        lshift = keyboard_report->mods & MOD_BIT(KC_LSFT);
        rshift = keyboard_report->mods & MOD_BIT(KC_RSFT);
        if (lshift || rshift) {
          if (lshift) unregister_code(KC_LSFT);
          if (rshift) unregister_code(KC_RSFT);
          register_code(KC_LSFT);
          register_code(KC_LBRC);
          unregister_code(KC_LBRC);
          unregister_code(KC_LSFT);
          if (lshift) register_code(KC_LSFT);
          if (rshift) register_code(KC_RSFT);
        } else {
          register_code(KC_LSFT);
          register_code(KC_EQL);
          unregister_code(KC_EQL);
          unregister_code(KC_LSFT);
        }
      }
      return false;
    case JU_CAPS:
      if (record->event.pressed) {
        register_code(KC_LSFT);
        register_code(KC_CAPS);
        unregister_code(KC_CAPS);
        unregister_code(KC_LSFT);
      }
      return false;
      break;
  }
  return true;
}


void matrix_init_user(void) {
}

void matrix_scan_user(void) {
}

bool led_update_user(led_t led_state) {
    return true;
}

