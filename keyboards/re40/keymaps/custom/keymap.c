/* Copyright 2019 kushima8
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
#include "keymap_jp.h"

enum layer_number {
    _FIRST = 0,
    _SECOND,
    _LOWER,
    _RAISE,
    _ADJUST
};

#define FIRST TO(_FIRST)
#define SECOND TO(_SECOND)
#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)

/* Hold=>GUI, Tap=>ESC */
#define GUI_ESC LGUI_T(KC_ESC)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_FIRST] = LAYOUT(
        GUI_ESC,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, KC_BSPC,
        KC_LCTL,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, KC_QUOT,
        KC_LALT,    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                      KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, KC_RSFT,
                                              LOWER,  KC_SPC, KC_MUTE,  SECOND,  KC_ENT,   RAISE
    ),
    [_SECOND] = LAYOUT(
        GUI_ESC,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, KC_BSPC,
        KC_LCTL,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, KC_QUOT,
        KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                      KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, KC_RALT,
                                              LOWER,  KC_SPC, _______,   FIRST,  KC_ENT,   RAISE
    ),
    [_LOWER] = LAYOUT(
        KC_LGUI, _______, KC_UP,   _______, _______, _______,                      KC_7,    KC_8,    KC_9, KC_MINS, KC_SLSH,  KC_DEL,
        _______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______,                      KC_4,    KC_5,    KC_6, KC_PPLS, KC_PAST, _______,
        _______, _______, _______, _______, _______, _______,                      KC_1,    KC_2,    KC_3,  KC_DOT, KC_COMM, _______,
                                            _______,  KC_TAB, _______, _______,    KC_0, _______
    ),
    [_RAISE] = LAYOUT(
        _______, JP_EXLM, JP_AT,   JP_HASH, JP_DLR,  JP_PERC,                   JP_CIRC, JP_AMPR, JP_ASTR, JP_LPRN, JP_RPRN, JP_TILD,
        _______, _______, RGB_TOG, RGB_HUI, RGB_SAI, RGB_VAI,                   _______, JP_MINS, JP_PLUS,  JP_EQL, JP_LCBR, JP_RCBR,
        _______, _______, RGB_MOD, RGB_HUD, RGB_SAD, RGB_VAD,                   _______, _______, _______, _______, JP_BSLS, _______, // TODO: JP_BSLS does not work well.
                                            _______, _______, _______, _______,  KC_APP, _______
    ),
    [_ADJUST] = LAYOUT(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,     KC_F5,                     KC_F6,   KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,
        KC_CAPS, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______,  KC_F12,
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______,
                                            _______,  KC_SPC, _______, _______,  KC_ENT, _______
    ),
};

uint32_t layer_state_set_user(uint32_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

static bool is_alt_pressed;
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LOWER:
            // Close task switcher with Alt-Tab
            if (!(record->event.pressed) && is_alt_pressed) {
                unregister_code(KC_LALT);
                is_alt_pressed = false;
            }
            break;
    }
    return true;
}

#ifdef OLED_DRIVER_ENABLE

static void render_logo(void) {
    static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };

    oled_write_P(qmk_logo, false);
}

static void print_status_narrow(void) {
    // Print current mode
    oled_write_P(PSTR("\n\n"), false);
    oled_write_ln_P(PSTR("MODE"), false);
    oled_write_ln_P(PSTR(""), false);
    if (keymap_config.swap_lctl_lgui) {
        oled_write_ln_P(PSTR("MAC"), false);
    } else {
        oled_write_ln_P(PSTR("WIN"), false);
    }

    switch (get_highest_layer(default_layer_state)) {
        case _FIRST:
            oled_write_ln_P(PSTR("1"), false);
            break;
        case _SECOND:
            oled_write_ln_P(PSTR("2"), false);
            break;
        case _LOWER:
            oled_write_ln_P(PSTR("↓"), false);
            break;
        case _RAISE:
            oled_write_ln_P(PSTR("↑"), false);
            break;
        case _ADJUST:
            oled_write_ln_P(PSTR("-"), false);
            break;
        default:
            oled_write_P(PSTR("Undef"), false);
    }
    oled_write_P(PSTR("\n\n"), false);
    // Print current layer
    oled_write_ln_P(PSTR("LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _FIRST:
            oled_write_P(PSTR("1st"), false);
            break;
        case _SECOND:
            oled_write_P(PSTR("2nd"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("Low"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("Rai"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("Adj"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Undef"), false);
    }
    oled_write_P(PSTR("\n\n"), false);
    led_t led_usb_state = host_keyboard_led_state();
    oled_write_ln_P(PSTR("CPSLK"), led_usb_state.caps_lock);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    return rotation;
}

void oled_task_user(void) {
    if (is_keyboard_master()) {
        print_status_narrow();
    } else {
        render_logo();
    }
}

#endif

#ifdef ENCODER_ENABLE

/* Rotary encoder settings */
void encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        // Left rotary
        switch (get_highest_layer(layer_state)) {
            case _FIRST:
            case _SECOND:
                tap_code(clockwise ? KC_VOLU : KC_VOLD);
                break;
            case _LOWER:
                if (!is_alt_pressed) {
                    register_code(KC_LALT); // unregister in process_record_user().
                    is_alt_pressed = true;
                }
                tap_code16(clockwise ? KC_TAB : S(KC_TAB));
                break;
            default:
                break;
        }
    } else if (index == 1) {
        // Right rotary
        switch (get_highest_layer(layer_state)) {
            case _FIRST:
            case _SECOND:
                tap_code(clockwise ? KC_WH_U : KC_WH_D);
                break;
            case _RAISE:
                tap_code(clockwise ? KC_PGUP : KC_PGDN);
                break;
            default:
                break;
        }
    }
}

#endif
