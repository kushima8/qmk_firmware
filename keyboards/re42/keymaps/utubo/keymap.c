/* Copyright 2020 kushima8
 * Copyright 2020 utubo
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
#include "utils/keymap_jp_util.h"
#include "utils/fix_tap_hold.h"

enum layer_number {
    _DEFAULT= 0,
    _TENKEY,
    _CONFIG,
    _LOWER,
    _RAISE,
    _ADJUST
};

enum custom_keycodes {
    RT_ENT = SAFE_RANGE,  // hold=>RAISE, tap=>KC_ENT
    LT_MHEN, // hold=>LOWER,   double-hold=>RAISE, tap=>Mukenkan
    LT_HENK, // hold=>LOWER,   double-hold=>RAISE, tap=>Henkan
    LT_P0,   // hold=>LOWER,   double-hold=>RAISE, tap=>KC_P0
    AT_ESC , // hold=>KC_ALT,  tap=>KC_ESC
    GT_ZKHK, // hold=>KC_LGUI, tap=>JP_ZKHK
    SH_JPQT, // SH_T(JP_QUOT)
    SH_TAB,  // SH_T(KC_TAB)
};

#define DEFAULT TO(_DEFAULT)
#define LOWER   MO(_LOWER)
#define RAISE   MO(_RAISE)
#define TENKEY  TG(_TENKEY)
#define CONFIG  TO(_CONFIG)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_DEFAULT] = LAYOUT(
    // ,-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------.
        AT_ESC ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,                     KC_Y   ,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_BSPC,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        SH_TAB ,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,                     KC_H   ,KC_J   ,KC_K   ,KC_L   ,JP_SCLN,SH_JPQT,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,                     KC_N   ,KC_M   ,KC_COMM,KC_DOT ,KC_SLSH,RSFT_T(JP_MINS),
    // |-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------|
        KC_LCTL,                        LT_MHEN,KC_SPC ,CONFIG ,     CONFIG ,RT_ENT ,LT_HENK                        ,KC_RCTL
    // `-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------'
    ),
    [_TENKEY] = LAYOUT(
    // ,-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------.
        KC_ESC ,_______,KC_UP  ,_______,_______,_______,                     KC_P7  ,KC_P8  ,KC_P9  ,JP_EQL ,KC_PPLS,_______,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,KC_LEFT,KC_DOWN,KC_RGHT,_______,_______,                     KC_P4  ,KC_P5  ,KC_P6  ,KC_COMM,KC_PAST,SH_TAB ,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,_______,_______,_______,_______,_______,                     KC_P1  ,KC_P2  ,KC_P3  ,KC_DOT ,_______,_______,
    // |-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------|
        _______,                        _______,_______,_______,     _______,_______,LT_P0                          ,_______
    // `-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------'
    ),
    [_CONFIG] = LAYOUT(
    // ,-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------.
        _______,DM_REC1,DM_REC2,_______,_______,_______,                     _______,_______,_______,_______,_______,_______,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,KC_CAPS,RGB_TOG,RGB_HUI,RGB_SAI,RGB_VAI,                     _______,_______,_______,_______,_______,_______,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,KC_NLCK,RGB_MOD,RGB_HUD,RGB_SAD,RGB_VAD,                     _______,_______,_______,_______,_______,_______,
    // |-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------|
        _______,                        _______,_______,DEFAULT,     DEFAULT,_______,_______                        ,_______
    // `-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------'
    ),
    [_LOWER] = LAYOUT( // KC_SLASH is for input a date.
    // ,-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------.
        GT_ZKHK,JP_1   ,JP_2   ,JP_3   ,JP_4   ,JP_5   ,                     JP_6   ,JP_7   ,JP_8   ,JP_9   ,JP_0   ,_______,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        KC_TAB ,KC_F1  ,KC_F2  ,KC_F3  ,KC_F4  ,KC_F5 ,                      KC_LEFT,KC_DOWN,KC_UP  ,KC_RGHT,KC_DEL ,KC_INS ,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,KC_F6  ,KC_F7  ,KC_F8  ,KC_F9  ,KC_F10 ,                     KC_PSCR,KC_APP ,KC_HOME,KC_END ,KC_SLSH,_______,
    // |-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------|
        _______,                        _______,RAISE  ,_______,     _______,_______,_______                        ,_______
    // `-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------'
    ),
    [_RAISE] = LAYOUT(
    // ,-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------.
        JP_GRV ,JP_EXLM,JP_AT  ,JP_HASH,JP_DLR ,JP_PERC,                     JP_CIRC,JP_AMPR,JP_ASTR,JP_LPRN,JP_RPRN,_______,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,KC_F11 ,KC_F12 ,KC_F13 ,KC_F14 ,KC_F15 ,                     JP_EQL ,JP_LCBR,JP_RCBR,JP_LBRC,JP_RBRC,JP_YEN ,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,_______,_______,_______,_______,_______,                     JP_PLUS,JP_UNDS,JP_LABK,JP_RABK,JP_QUES,_______,
    // |-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------|
        _______,                        _______,_______,_______,     _______,_______,_______                        ,_______
    // `-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------'
    ),
    [_ADJUST] = LAYOUT(
    // ,-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------.
        KC_LGUI,DM_PLY1,DM_PLY2,_______,_______,_______,                     _______,_______,_______,_______,_______,_______,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        SH_TG  ,KC_VOLU,KC_VOLD,KC_MUTE,RGB_TOG,_______,                     KC_MS_L,KC_MS_D,KC_MS_U,KC_MS_R,_______,SH_TG  ,
    // |-------+-------+-------+-------+-------+-------|                    |-------+-------+-------+-------+-------+-------|
        _______,KC_BTN1,KC_BTN2,_______,_______,_______,                     _______,_______,_______,_______,_______,LSFT_T(KC_PWR),
    // |-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------|
        TENKEY ,                        _______,_______,_______,     _______,_______,_______                        ,TENKEY
    // `-------+-------+-------+-------+-------+-------+-------|    |-------+-------+-------+-------+-------+-------+-------'
    ),
};

uint32_t layer_state_set_user(uint32_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

const keypos_t hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
    // Left
    {{5, 4}, {0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}},
    {{5, 5}, {0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}},
    {{5, 6}, {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}},
    {{5, 7}, {4, 7}, {3, 7}, {2, 7}, {1, 7}, {0, 7}},
    // Right
    {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {0, 0}},
    {{1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {0, 1}},
    {{1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {0, 2}},
    {{5, 3}, {4, 3}, {3, 3}, {2, 3}, {1, 3}, {0, 3}},
};

char tap_count[] = "00000";
void countup_taps(void) {
    for (int i = 4; i >= 0; i --) {
        if (tap_count[i] >= '9') {
            tap_count[i] = '0';
        } else {
            tap_count[i] = tap_count[i] + 1;
            break;
        }
    }
}

static uint16_t lower_keycode = 0;
static bool custom_lower(uint16_t keycode) {
    switch (tap_state) {
        case SINGLE_HOLD:
            lower_keycode = LOWER;
            break;
        case DOUBLE_HOLD:
            lower_keycode = RAISE;
            tap_state = SINGLE_HOLD;
            break;
        default: break;
    }
    switch (lower_keycode) {
        case LOWER: return quick_LT(_LOWER, keycode);
        case RAISE: return quick_LT(_RAISE, keycode);
        case KC_LALT: // Close the task-switcher that is opened with LOWER-Tab.
            unregister_code(KC_LALT);
            layer_off(_LOWER);
            lower_keycode = 0;
            return false;
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    process_fix_tap_hold(keycode, record);

    if (record->event.pressed) {
        countup_taps();
    }

    switch (keycode) {
        case KC_TAB: // LOWER-Tab => Alt-Tab
            if (record->event.pressed && lower_keycode == LOWER) {
                lower_keycode = KC_LALT;
                register_code(KC_LALT);
            }
            return true;
        case LT_MHEN: return custom_lower(JP_MHEN);
        case LT_HENK: return custom_lower(JP_HENK);
        case LT_P0:   return custom_lower(KC_P0);
        case RT_ENT:  return quick_LT(_RAISE, KC_ENT);
        case AT_ESC:  return quick_MT(KC_LALT, KC_ESC);
        case GT_ZKHK: return quick_MT(KC_LGUI, JP_ZKHK);
        case SH_TAB:  return sh_t16(KC_TAB);
        case SH_JPQT: return sh_t16(JP_QUOT);
    }

    return process_jp_symbols(keycode, record);
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

    // Print current layer
    oled_write_ln_P(PSTR("LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _DEFAULT: oled_write_P(PSTR("Dflt "), false); break;
        case _LOWER:   oled_write_P(PSTR("Low  "), false); break;
        case _RAISE:   oled_write_P(PSTR("Rai  "), false); break;
        case _ADJUST:  oled_write_P(PSTR("Adj  "), false); break;
        case _TENKEY:  oled_write_P(PSTR("10Key"), false); break;
        case _CONFIG:  oled_write_P(PSTR("Conf "), false); break;
        default:       oled_write_P(PSTR("Error"), false);
    }

    // Print swap hands
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("SWAP"), swap_hands);

    // Print lock
    led_t led_usb_state = host_keyboard_led_state();
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("NUM"), led_usb_state.num_lock);
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("CAPS"), led_usb_state.caps_lock);
    oled_write_ln_P(PSTR(""), false);
    oled_write_P(PSTR("SCROL"), led_usb_state.scroll_lock);
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("KANA"), led_usb_state.kana);

    // Print tap count
    oled_write_ln_P(PSTR(""), false);
    oled_write(tap_count, false);
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
    clockwise = clockwise ^ (index == 1);
    switch (get_highest_layer(layer_state)) {
        case _LOWER:  tap_code(clockwise ? KC_PGDN : KC_PGUP); break;
        case _RAISE:  tap_code16(clockwise ? A(KC_PGDN) : A(KC_PGUP)); break;
        case _ADJUST: tap_code16(clockwise ? C(KC_PGDN) : C(KC_PGUP)); break;
        default: tap_code(clockwise ? KC_WH_D : KC_WH_U);
    }
}

#endif
