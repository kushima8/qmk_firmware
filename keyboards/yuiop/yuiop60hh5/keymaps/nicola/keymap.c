/*
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

#include "ws2812.h"

enum nicola_keycodes {
    NC_TOG = SAFE_RANGE,
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
      KC_ESC  ,    KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0, KC_MINS, KC_EQL , KC_BSLS, KC_GRV ,
      KC_TAB      ,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, KC_LBRC, KC_RBRC,      KC_BSPC,
      KC_LCTL       ,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, KC_QUOT,             KC_ENT ,
      KC_LSFT           ,    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M, KC_COMM, KC_DOT , KC_SLSH,         KC_RSFT, MO(1)  ,
                   MO(1)   ,  KC_LALT ,                         KC_SPC                         ,   KC_RALT  , KC_RGUI
    ),

    [1] = LAYOUT(
      XXXXXXX , KC_F1  , KC_F2  , KC_F3  , KC_F4  , KC_F5  , KC_F6  , KC_F7  , KC_F8  , KC_F9  , KC_F10 , KC_F11 , KC_F12 , KC_INS , KC_DEL ,
      KC_CAPS     , XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_NLCK, KC_PSCR, KC_SLCK, KC_PAUS, KC_UP  , XXXXXXX,      XXXXXXX,
      _______       , KC_VOLD, KC_VOLU, KC_MUTE, XXXXXXX, XXXXXXX, KC_PAST, KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT,             XXXXXXX,
      _______           , XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_PPLS, KC_PMNS, KC_END , KC_PGDN, KC_DOWN,         _______, _______,
                   _______ ,  _______ ,                         NC_TOG                         ,   _______  , _______
    ),
};
// clang-format on

static LED_TYPE indicator[3] = {0};
static bool nicola_enable = false;
static bool nicola_left = false;
static bool nicola_right = false;

#if 0
static bool nicola_passthrough = false;
#endif

static void nicola_on(void) {
    LED_TYPE v = {.r = 0xff, .g = 0xff, .b=0xff};
    indicator[2] = v;
    ws2812_setleds(indicator, 3);

    tap_code(KC_LANG1); // enable IME
}

static void nicola_off(void) {
    LED_TYPE v = {.r = 0x00, .g = 0x00, .b=0x00};
    indicator[2] = v;
    ws2812_setleds(indicator, 3);

    tap_code(KC_LANG2); // disable IME
}

static void nicola_send_string(bool pressed, const char *normal, const char *left, const char *right) {
    if (!pressed) {
        return;
    }
    if (nicola_left && left) {
        send_string(left);
    } else if (nicola_right && right) {
        send_string(right);
    } else if (normal) {
        send_string(normal);
    }
}

static bool nicola_process(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;
    // capture ALT pressed status
    switch (keycode) {
        case KC_LALT:
            nicola_left = pressed;
            return false;
        case KC_RALT:
            nicola_right = pressed;
            return false;
    }
    switch (keycode) {

        // left hand

        case KC_1: /*TODO*/ break;
        case KC_2: /*TODO*/ break;
        case KC_3: /*TODO*/ break;
        case KC_4: /*TODO*/ break;
        case KC_5: /*TODO*/ break;

        case KC_Q: /*TODO*/ break;
        case KC_W: /*TODO*/ break;
        case KC_E: /*TODO*/ break;
        case KC_R: /*TODO*/ break;
        case KC_T: /*TODO*/ break;

        case KC_A: nicola_send_string(pressed, "u", "wo", NULL); break;
        case KC_S: nicola_send_string(pressed, "si", "a", "ji"); break;
        case KC_D: nicola_send_string(pressed, "te", "na", "de"); break;
        case KC_F: nicola_send_string(pressed, "ke", "xyu", "ge"); break;
        case KC_G: nicola_send_string(pressed, "se", "mo", "ze"); break;

        case KC_Z: /*TODO*/ break;
        case KC_X: /*TODO*/ break;
        case KC_C: /*TODO*/ break;
        case KC_V: /*TODO*/ break;
        case KC_B: /*TODO*/ break;

        // right hand

        case KC_6: /*TODO*/ break;
        case KC_7: /*TODO*/ break;
        case KC_8: /*TODO*/ break;
        case KC_9: /*TODO*/ break;
        case KC_0: /*TODO*/ break;

        case KC_Y: /*TODO*/ break;
        case KC_U: /*TODO*/ break;
        case KC_I: /*TODO*/ break;
        case KC_O: /*TODO*/ break;
        case KC_P: /*TODO*/ break;

        case KC_H: /*TODO*/ break;
        case KC_J: /*TODO*/ break;
        case KC_K: /*TODO*/ break;
        case KC_L: /*TODO*/ break;
        case KC_SCLN: /*TODO*/ break;

        case KC_N: /*TODO*/ break;
        case KC_M: /*TODO*/ break;
        case KC_COMM: /*TODO*/ break;
        case KC_DOT: /*TODO*/ break;
        case KC_SLSH: /*TODO*/ break;

        default:
#if 0
            if (pressed) {
                if (nicola_left) {
                    register_mods(MOD_BIT(KC_LEFT_ALT));
                }
                if (nicola_right) {
                    register_mods(MOD_BIT(KC_RIGHT_ALT));
                }
                nicola_passthrough = true;
            }
#endif
            return true;
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case NC_TOG:
            if (record->event.pressed) {
                (nicola_enable = !nicola_enable) ? nicola_on() : nicola_off();
            }
            return false;
    }
    if (nicola_enable) {
        return nicola_process(keycode, record);
    }
    return true;
}

#if 0
void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (nicola_passthrough) {
        nicola_passthrough = false;
        if (nicola_left) {
            unregister_mods(MOD_BIT(KC_LEFT_ALT));
        }
        if (nicola_right) {
            unregister_mods(MOD_BIT(KC_RIGHT_ALT));
        }
    }
}
#endif
