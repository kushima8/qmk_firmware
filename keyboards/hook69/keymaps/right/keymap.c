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
#elif defined(RGB_MATRIX_ENABLE)
    #include "rgb_matrix.h"
    extern rgb_config_t rgb_matrix_config;
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
        JP_CIRC  ,JP_MINS ,KC_0    ,KC_3    ,KC_2    ,KC_1    ,KC_BSPC ,KC_RGUI ,JP_LBRC ,JP_AT   ,KC_P    ,KC_O    ,KC_I    ,KC_U    ,KC_SPC  ,KC_Y    ,KC_T    ,KC_R    ,KC_E    ,KC_W    ,KC_Q    ,JP_ZKHK ,KC_ESC  ,KC_A    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        JP_YEN   ,KC_UP   ,KC_PSCR ,KC_6    ,KC_5    ,KC_4    ,KC_ENT  ,KC_RSFT ,JP_RBRC ,JP_COLN ,JP_SCLN ,KC_L    ,KC_K    ,KC_J    ,KC_SPC  ,KC_H    ,KC_G    ,KC_F    ,KC_D    ,KC_S    ,KC_A    ,KC_LSFT ,KC_TAB  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_RIGHT ,KC_DOWN ,KC_LEFT ,KC_9    ,KC_8    ,KC_7    ,KC_RCTRL,MO(_2nd),JP_BSLS ,KC_SLSH ,KC_DOT  ,KC_COMM ,KC_M    ,KC_N    ,KC_SPC  ,KC_B    ,KC_V    ,KC_C    ,KC_X    ,KC_Z    ,KC_LALT ,KC_LCTRL,KC_CAPS
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_2nd] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        KC_F12   ,KC_F11  ,KC_F10  ,KC_F3   ,KC_F2   ,KC_F1   ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,KC_B    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_END   ,KC_PGUP ,KC_HOME ,KC_F6   ,KC_F5   ,KC_F4   ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_DEL   ,KC_PGDN ,KC_INS  ,KC_F9   ,KC_F8   ,KC_F7   ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______ ,_______
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_3rd] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        JP_CIRC  ,JP_MINS ,KC_0    ,KC_3    ,KC_2    ,KC_1    ,KC_BSPC ,KC_RGUI ,JP_LBRC ,JP_AT   ,KC_P    ,KC_O    ,KC_I    ,KC_U    ,KC_SPC  ,KC_Y    ,KC_T    ,KC_R    ,KC_E    ,KC_W    ,KC_Q    ,JP_ZKHK ,KC_ESC  ,KC_A    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        JP_YEN   ,KC_UP   ,KC_PSCR ,KC_6    ,KC_5    ,KC_4    ,KC_ENT  ,KC_RSFT ,JP_RBRC ,JP_COLN ,JP_SCLN ,KC_L    ,KC_K    ,KC_J    ,KC_SPC  ,KC_H    ,KC_G    ,KC_F    ,KC_D    ,KC_S    ,KC_A    ,KC_LSFT ,KC_TAB  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_RIGHT ,KC_DOWN ,KC_LEFT ,KC_9    ,KC_8    ,KC_7    ,KC_RCTRL,MO(_2nd),JP_BSLS ,KC_SLSH ,KC_DOT  ,KC_COMM ,KC_M    ,KC_N    ,KC_SPC  ,KC_B    ,KC_V    ,KC_C    ,KC_X    ,KC_Z    ,KC_LALT ,KC_LCTRL,KC_CAPS
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_4th] = LAYOUT(
     // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------.
        JP_CIRC  ,JP_MINS ,KC_0    ,KC_3    ,KC_2    ,KC_1    ,KC_BSPC ,KC_RGUI ,JP_LBRC ,JP_AT   ,KC_P    ,KC_O    ,KC_I    ,KC_U    ,KC_SPC  ,KC_Y    ,KC_T    ,KC_R    ,KC_E    ,KC_W    ,KC_Q    ,JP_ZKHK ,KC_ESC  ,KC_A    ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        JP_YEN   ,KC_UP   ,KC_PSCR ,KC_6    ,KC_5    ,KC_4    ,KC_ENT  ,KC_RSFT ,JP_RBRC ,JP_COLN ,JP_SCLN ,KC_L    ,KC_K    ,KC_J    ,KC_SPC  ,KC_H    ,KC_G    ,KC_F    ,KC_D    ,KC_S    ,KC_A    ,KC_LSFT ,KC_TAB  ,
     // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_RIGHT ,KC_DOWN ,KC_LEFT ,KC_9    ,KC_8    ,KC_7    ,KC_RCTRL,MO(_2nd),JP_BSLS ,KC_SLSH ,KC_DOT  ,KC_COMM ,KC_M    ,KC_N    ,KC_SPC  ,KC_B    ,KC_V    ,KC_C    ,KC_X    ,KC_Z    ,KC_LALT ,KC_LCTRL,KC_CAPS
     // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    )
};

#ifdef ENCODER_ENABLE

/* Rotary encoder settings */

keyevent_t encoder_ccw  = {
    .key = (keypos_t){.row = 5, .col = 5},
    .pressed = false
};

keyevent_t encoder_cw  = {
    .key = (keypos_t){.row = 11, .col = 5},
    .pressed = false
};

void matrix_scan_user(void) {
    if (IS_PRESSED(encoder_ccw)) {
        encoder_ccw.pressed = false;
        encoder_ccw.time = (timer_read() | 1);
        action_exec(encoder_ccw);
    }

    if (IS_PRESSED(encoder_cw)) {
        encoder_cw.pressed = false;
        encoder_cw.time = (timer_read() | 1);
        action_exec(encoder_cw);
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (!clockwise){
        encoder_cw.pressed = true;
        encoder_cw.time = (timer_read() | 1);
        action_exec(encoder_cw);
    } else {
        encoder_ccw.pressed = true;
        encoder_ccw.time = (timer_read() | 1);
        action_exec(encoder_ccw);
    }
    return true;
}

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
                rgblight_update_dword(eeprom_read_dword((const uint32_t *)(VIA_RGBLIGHT_USER_ADDR + 4 * layer)));
        #elif defined(RGB_MATRIX_ENABLE)
                rgb_matrix_config.raw = eeprom_read_dword((const uint32_t *)(VIA_RGBLIGHT_USER_ADDR + 4 * layer));
        #endif
    }

    return state;
}
