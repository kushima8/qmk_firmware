/*
Copyright 2023 kushima8 (@kushima8)

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
#include "drivers/pmw3360/pmw3360.h"
#include "lib/reex/reex.c"

#define MANUAL  TO(0)
#define AUTO   TO(1)
#define FN  MO(2)
#define MOUSE  MO(3)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------.
        KC_ESC  ,KC_1    ,KC_2    ,KC_3    ,KC_4    ,KC_5    ,         KC_6    ,KC_7    ,KC_8    ,KC_9    ,KC_0    ,KC_MINS ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_TAB  ,KC_Q    ,KC_W    ,KC_E    ,KC_R    ,KC_T    ,         KC_Y    ,KC_U    ,KC_I    ,KC_O    ,KC_P    ,KC_BSPC ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_LCTL ,KC_A    ,KC_S    ,KC_D    ,KC_F    ,KC_G    ,         KC_H    ,KC_J    ,KC_K    ,KC_L    ,KC_SCLN ,KC_QUOT ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_RSFT ,KC_Z    ,KC_X    ,KC_C    ,KC_V    ,KC_B    ,         KC_N    ,KC_M    ,KC_COMM ,KC_DOT  ,KC_SLSH ,SFT_T(KC_ENT),
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_LCTL ,AUTO    ,KC_LALT                   ,MOUSE   ,         KC_SPC                    ,KC_RALT ,KC_RGUI ,FN ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
                 KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,         KC_F    ,KC_G    ,KC_H    ,KC_I    ,KC_J
    // `--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+---------------------'
    ),
    [1] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------.
        KC_ESC  ,KC_1    ,KC_2    ,KC_3    ,KC_4    ,KC_5    ,         KC_6    ,KC_7    ,KC_8    ,KC_9    ,KC_0    ,KC_MINS ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_TAB  ,KC_Q    ,KC_W    ,KC_E    ,KC_R    ,KC_T    ,         KC_Y    ,KC_U    ,KC_I    ,KC_O    ,KC_P    ,KC_BSPC ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_LCTL ,KC_A    ,KC_S    ,KC_D    ,KC_F    ,KC_G    ,         KC_H    ,KC_J    ,KC_K    ,KC_L    ,KC_SCLN ,KC_QUOT ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_RSFT ,KC_Z    ,KC_X    ,KC_C    ,KC_V    ,KC_B    ,         KC_N    ,KC_M    ,KC_COMM ,KC_DOT  ,KC_SLSH ,SFT_T(KC_ENT),
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        KC_LCTL ,MANUAL  ,KC_LALT                   ,KC_SPC  ,         KC_SPC                    ,KC_RALT ,KC_RGUI ,FN ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
                 KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,         KC_F    ,KC_G    ,KC_H    ,KC_I    ,KC_J
    // `--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+---------------------'
    ),
    [2] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------.
        _______ ,KC_F1   ,KC_F2   ,KC_F3   ,KC_F4   ,KC_F5   ,         KC_F6   ,KC_F7   ,KC_F8   ,KC_F9   ,KC_F10  ,KC_F11  ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        _______ ,_______ ,_______ ,_______ ,_______ ,AML_TO  ,         SSNP_VRT,_______ ,_______ ,_______ ,_______ ,KC_F12  ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        REC_RST ,SCRL_TO ,CPI_I100,CPI_I1K ,SCRL_DVI,AML_I50 ,         SSNP_HOR,_______ ,RGB_TOG ,RGB_HUI ,RGB_SAI ,RGB_VAI ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        REC_SAVE,SCRL_MO ,CPI_D100,CPI_D1K ,SCRL_DVD,AML_D50 ,         SSNP_FRE,_______ ,RGB_MOD ,RGB_HUD ,RGB_SAD ,RGB_VAD ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        _______ ,_______ ,_______                   ,_______ ,         _______                   ,_______ ,_______ ,_______,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
                 KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,         KC_F    ,KC_G    ,KC_H    ,KC_I    ,KC_J
    // `--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+---------------------'
    ),
    [3] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------.
        _______ ,_______ ,_______ ,_______ ,_______ ,_______ ,         _______ ,_______ ,_______ ,_______ ,_______ ,_______ ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        _______ ,_______ ,_______ ,_______ ,_______ ,_______ ,         _______ ,_______ ,_______ ,_______ ,_______ ,_______ ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        _______ ,_______ ,_______ ,_______ ,_______ ,_______ ,         KC_BTN4 ,KC_BTN1 ,KC_BTN2 ,KC_BTN3 ,_______ ,_______ ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        _______ ,_______ ,_______ ,_______ ,_______ ,_______ ,         KC_BTN5 ,_______ ,_______ ,_______ ,_______ ,_______ ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
        _______ ,_______ ,_______                   ,_______ ,         _______                   ,_______ ,_______ ,_______ ,
    // |--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+--------+-------------|
                 KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,         KC_F    ,KC_G    ,KC_H    ,KC_I    ,KC_J
    // `--------+--------+--------+--------+--------+--------|        |--------+--------+--------+--------+---------------------'
    )
};
// clang-format on

#ifdef OLED_ENABLE
#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    reex_oled_render_layerinfo();
    reex_oled_render_keyinfo();
    reex_oled_render_ballinfo();
}
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
layer_state_t layer_state_set_user(layer_state_t state) {
    switch(get_highest_layer(remove_auto_mouse_layer(state, true))) {
        case 0:
        case 2:
            state = remove_auto_mouse_layer(state, false);
            set_auto_mouse_enable(false);
            break;
        default:
            set_auto_mouse_enable(true);
            break;
    }
    return state;
}
#endif

#if defined(ENCODER_ENABLE) && defined(DIP_SWITCH_ENABLE)
void keyboard_post_init_user(void) {
    if(!is_keyboard_master()){
        if(!reex.this_have_ball){
            encoder_init();
            dip_switch_init();
            gpio_set_pin_output(GP26);
            gpio_write_pin_low(GP26);
        }
    }
}

void housekeeping_task_user(void){
    static bool encoder_ini_flg = true;
    if(reex.negotiated && encoder_ini_flg){
        if(is_keyboard_master()){
            if(!reex.this_have_ball){
                encoder_init();
                dip_switch_init();
                gpio_set_pin_output(GP26);
                gpio_write_pin_low(GP26);
                encoder_ini_flg = false;
            }
        }
    }
}
#endif

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_B, KC_A ) },
    [1] = { ENCODER_CCW_CW(KC_D, KC_C ) },
    [2] = { ENCODER_CCW_CW(KC_F, KC_E ) },
    [3] = { ENCODER_CCW_CW(KC_H, KC_G ) },
};
#endif

#ifdef DIP_SWITCH_ENABLE
void matrix_slave_scan_kb(void) {
    dip_switch_read(false);
    matrix_slave_scan_user();
}

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)){
        return false;
    }
    if(!reex.negotiated){
        return false;
    }
    switch (index) {
        case 0:
              action_exec(MAKE_KEYEVENT(5, 0, active));
//            action_exec(MAKE_KEYEVENT(isLeftHand ? 5 : 11, 0, active));
            break;
    }
    return false;
}
#endif

#ifdef POINTING_DEVICE_ENABLE

bool ex_have_ball = false;

void pointing_device_init_kb(void) {
    ex_have_ball = pmw3360_init(1);
#if defined(REEX_PMW3360_UPLOAD_SROM_ID)
#    if REEX_PMW3360_UPLOAD_SROM_ID == 0x04
        pmw3360_srom_upload(0,pmw3360_srom_0x04);
#    elif REEX_PMW3360_UPLOAD_SROM_ID == 0x81
        pmw3360_srom_upload(0,pmw3360_srom_0x81);
#    else
#        error Invalid value for REEX_PMW3360_UPLOAD_SROM_ID. Please choose 0x04 or 0x81 or disable it.
#    endif
#endif
}

report_mouse_t pointing_device_task_kb(report_mouse_t rep) {
    if (ex_have_ball && reex.this_have_ball) {
        pmw3360_motion_t d = {0};
        if (pmw3360_motion_burst(1,&d)) {
            ATOMIC_BLOCK_FORCEON {
                reex.ex_this_motion.x = add16(reex.ex_this_motion.x, d.x);
                reex.ex_this_motion.y = add16(reex.ex_this_motion.y, d.y);
            }
        }
    }
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master() && should_report()) {
        // modify mouse report by PMW3360 motion.
        motion_to_mouse(&reex.ex_this_motion, &rep, is_keyboard_left(), !reex.scroll_mode);
    }
    return rep;
}

#endif