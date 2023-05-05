#include QMK_KEYBOARD_H
#include "quantum.h"
#include <stdio.h>

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------.
        KC_ESC  ,KC_1    ,KC_2    ,KC_3    ,KC_4    ,KC_5    ,         KC_6    ,KC_7    ,KC_8    ,KC_9    ,KC_0    ,KC_MINS ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_TAB  ,KC_Q    ,KC_W    ,KC_E    ,KC_R    ,KC_T    ,         KC_Y    ,KC_U    ,KC_I    ,KC_O    ,KC_P    ,KC_BSPC ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_LCTL ,KC_A    ,KC_S    ,KC_D    ,KC_F    ,KC_G    ,         KC_H    ,KC_J    ,KC_K    ,KC_L    ,KC_SCLN ,KC_QUOT ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_RSFT ,KC_Z    ,KC_X    ,KC_C    ,KC_V    ,KC_B    ,         KC_N    ,KC_M    ,KC_COMM ,KC_DOT  ,KC_SLSH ,KC_ENT  ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_LCTL ,KC_LGUI ,KC_LALT                   ,KC_SPC  ,         KC_SPC                    ,KC_RALT ,KC_RGUI ,KC_APP
    // `--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+---------------------'
    )
};
// clang-format on

void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    // Force RGB lights to show test animation without writing EEPROM.
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_RGB_TEST);
#endif
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    reex_oled_render_keyinfo();
    reex_oled_render_ballinfo();
}
#endif
