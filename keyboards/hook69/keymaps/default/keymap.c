/* Copyright 2022 kushima8
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
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "keymap_japanese.h"
#include "eeprom.h"
#if defined(RGBLIGHT_ENABLE)
#include "rgblight.h"
#endif

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _1st,
    _2nd,
    _3rd,
    _4th
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_1st] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        KC_ESC   ,JP_ZKHK ,KC_Q    ,KC_W    ,KC_E    ,KC_R    ,KC_T    ,KC_Y    ,KC_SPC  ,KC_U    ,KC_I    ,KC_O    ,KC_P    ,JP_AT   ,JP_LBRC ,KC_RGUI ,KC_BSPC ,KC_1    ,KC_2    ,KC_3    ,KC_0    ,JP_MINS ,JP_CIRC ,KC_A    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_TAB   ,KC_LSFT ,KC_A    ,KC_S    ,KC_D    ,KC_F    ,KC_G    ,KC_H    ,KC_SPC  ,KC_J    ,KC_K    ,KC_L    ,JP_SCLN ,JP_COLN ,JP_RBRC ,KC_RSFT ,KC_ENT  ,KC_4    ,KC_5    ,KC_6    ,KC_PSCR ,KC_UP   ,JP_YEN  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_CAPS  ,KC_LCTL ,KC_LALT ,KC_Z    ,KC_X    ,KC_C    ,KC_V    ,KC_B    ,KC_SPC  ,KC_N    ,KC_M    ,KC_COMM ,KC_DOT  ,KC_SLSH ,JP_BSLS ,MO(_2nd),KC_RCTL ,KC_7    ,KC_8    ,KC_9    ,KC_LEFT ,KC_DOWN ,KC_RIGHT
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_2nd] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        _______  ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,KC_F1   ,KC_F2   ,KC_F3   ,KC_F10  ,KC_F11  ,KC_F12  ,KC_B    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        _______  ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,KC_F4   ,KC_F5   ,KC_F6   ,KC_HOME ,KC_PGUP ,KC_END  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        _______  ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,KC_F7   ,KC_F8   ,KC_F9   ,KC_INS  ,KC_PGDN ,KC_DEL
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_3rd] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        KC_ESC   ,JP_ZKHK ,KC_Q    ,KC_W    ,KC_E    ,KC_R    ,KC_T    ,KC_Y    ,KC_SPC  ,KC_U    ,KC_I    ,KC_O    ,KC_P    ,JP_AT   ,JP_LBRC ,KC_RGUI ,KC_BSPC ,KC_1    ,KC_2    ,KC_3    ,KC_0    ,JP_MINS ,JP_CIRC ,KC_A    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_TAB   ,KC_LSFT ,KC_A    ,KC_S    ,KC_D    ,KC_F    ,KC_G    ,KC_H    ,KC_SPC  ,KC_J    ,KC_K    ,KC_L    ,JP_SCLN ,JP_COLN ,JP_RBRC ,KC_RSFT ,KC_ENT  ,KC_4    ,KC_5    ,KC_6    ,KC_PSCR ,KC_UP   ,JP_YEN  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_CAPS  ,KC_LCTL ,KC_LALT ,KC_Z    ,KC_X    ,KC_C    ,KC_V    ,KC_B    ,KC_SPC  ,KC_N    ,KC_M    ,KC_COMM ,KC_DOT  ,KC_SLSH ,JP_BSLS ,MO(_2nd),KC_RCTL ,KC_7    ,KC_8    ,KC_9    ,KC_LEFT ,KC_DOWN ,KC_RIGHT
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_4th] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        KC_ESC   ,JP_ZKHK ,KC_Q    ,KC_W    ,KC_E    ,KC_R    ,KC_T    ,KC_Y    ,KC_SPC  ,KC_U    ,KC_I    ,KC_O    ,KC_P    ,JP_AT   ,JP_LBRC ,KC_RGUI ,KC_BSPC ,KC_1    ,KC_2    ,KC_3    ,KC_0    ,JP_MINS ,JP_CIRC ,KC_A    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_TAB   ,KC_LSFT ,KC_A    ,KC_S    ,KC_D    ,KC_F    ,KC_G    ,KC_H    ,KC_SPC  ,KC_J    ,KC_K    ,KC_L    ,JP_SCLN ,JP_COLN ,JP_RBRC ,KC_RSFT ,KC_ENT  ,KC_4    ,KC_5    ,KC_6    ,KC_PSCR ,KC_UP   ,JP_YEN  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_CAPS  ,KC_LCTL ,KC_LALT ,KC_Z    ,KC_X    ,KC_C    ,KC_V    ,KC_B    ,KC_SPC  ,KC_N    ,KC_M    ,KC_COMM ,KC_DOT  ,KC_SLSH ,JP_BSLS ,MO(_2nd),KC_RCTL ,KC_7    ,KC_8    ,KC_9    ,KC_LEFT ,KC_DOWN ,KC_RIGHT
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    )
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_1st] = { ENCODER_CCW_CW(KC_B, KC_A ) },
    [_2nd] = { ENCODER_CCW_CW(KC_D, KC_C ) },
    [_3rd] = { ENCODER_CCW_CW(KC_F, KC_E ) },
    [_4th] = { ENCODER_CCW_CW(KC_H, KC_G ) },
};
#endif

#ifdef OLED_ENABLE

static void render_logo(void) {
    static const char PROGMEM qmk_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00
    };

    oled_write_P(qmk_logo, false);
}

static void print_status_narrow(void) {
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _1st:
            oled_write_ln_P(PSTR("1st"), false);
            break;
        case _2nd:
            oled_write_ln_P(PSTR("2nd"), false);
            break;
        case _3rd:
            oled_write_ln_P(PSTR("3rd"), false);
            break;
        case _4th:
            oled_write_ln_P(PSTR("4th"), false);
            break;
        default:
            oled_write_P(PSTR("Undef"), false);
    }

    #ifdef RGBLIGHT_ENABLE
        oled_write_ln_P(PSTR(""), false);
        oled_write_ln_P(PSTR("LED"), false);
        oled_write_ln_P(PSTR(""), false);
        char rgbMode[6] = {0};
        snprintf(rgbMode, sizeof(rgbMode), "M:%-3d", rgblight_get_mode());
        oled_write(rgbMode, false);
        static char rgbHue[6] = {0};
        snprintf(rgbHue, sizeof(rgbHue), "H:%-3d", rgblight_get_hue());
        oled_write(rgbHue, false);
        static char rgbSat[6] = {0};
        snprintf(rgbSat, sizeof(rgbSat), "S:%-3d", rgblight_get_sat());
        oled_write(rgbSat, false);
        static char rgbVal[6] = {0};
        snprintf(rgbVal, sizeof(rgbVal), "V:%-3d", rgblight_get_val());
            oled_write(rgbVal, false);
    #endif
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    return rotation;
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        print_status_narrow();
    } else {
        render_logo();
    }
    return false;
}

#endif

layer_state_t layer_state_set_user(layer_state_t state) {
    uint8_t layer = get_highest_layer(state);
    if (layer < DYNAMIC_KEYMAP_LAYER_COUNT) {
#if defined(RGBLIGHT_ENABLE)
        rgblight_update_qword(eeprom_read_dword((const uint32_t *)(VIA_RGBLIGHT_USER_ADDR + 4 * layer)));
#endif
    }

    return state;
}
