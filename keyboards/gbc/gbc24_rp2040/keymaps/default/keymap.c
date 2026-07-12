/* Copyright 2025 kushima8
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
#include "quantum.h"
#include "gpio.h"
#include "eeprom.h"
#include "analog.h"

enum layer_names {
    _0th,
    _1st,
    _2nd,
    _3rd
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_0th] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,KC_F    ,KC_G    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_H    ,KC_I    ,KC_J    ,KC_K    ,KC_L    ,KC_M    ,KC_N    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                 KC_O    ,KC_P    ,KC_Q    ,KC_R    ,KC_S    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                          KC_T    ,KC_U    ,KC_V    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                       KC_W 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                               ,KC_X 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                                        ,KC_Y
    // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_1st] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,KC_F    ,KC_G    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_H    ,KC_I    ,KC_J    ,KC_K    ,KC_L    ,KC_M    ,KC_N    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                 KC_O    ,KC_P    ,KC_Q    ,KC_R    ,KC_S    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                          KC_T    ,KC_U    ,KC_V    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                       KC_W 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                               ,KC_X 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                                        ,KC_Y
    // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_2nd] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,KC_F    ,KC_G    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_H    ,KC_I    ,KC_J    ,KC_K    ,KC_L    ,KC_M    ,KC_N    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                 KC_O    ,KC_P    ,KC_Q    ,KC_R    ,KC_S    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                          KC_T    ,KC_U    ,KC_V    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                       KC_W 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                               ,KC_X 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                                        ,KC_Y
    // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    ),
    [_3rd] = LAYOUT(
    // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_A    ,KC_B    ,KC_C    ,KC_D    ,KC_E    ,KC_F    ,KC_G    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
        KC_H    ,KC_I    ,KC_J    ,KC_K    ,KC_L    ,KC_M    ,KC_N    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                 KC_O    ,KC_P    ,KC_Q    ,KC_R    ,KC_S    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                          KC_T    ,KC_U    ,KC_V    ,
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                       KC_W 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                               ,KC_X 
    // |--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
                                                                                        ,KC_Y
    // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
    )
};

#ifdef ENCODER_MAP_ENABLE

const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_0th] = { ENCODER_CCW_CW(KC_1, KC_2 ) },
    [_1st] = { ENCODER_CCW_CW(KC_3, KC_4 ) },
    [_2nd] = { ENCODER_CCW_CW(KC_5, KC_6 ) },
    [_3rd] = { ENCODER_CCW_CW(KC_7, KC_8 ) },
};

#endif

#ifdef OLED_ENABLE

static void print_status_narrow(void) {
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _0th:
            oled_write_ln_P(PSTR("0th"), false);
            break;
        case _1st:
            oled_write_ln_P(PSTR("1st"), false);
            break;
        case _2nd:
            oled_write_ln_P(PSTR("2nd"), false);
            break;
        case _3rd:
            oled_write_ln_P(PSTR("3rd"), false);
            break;
        default:
            oled_write_P(PSTR("Undef"), false);
    }
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

bool oled_task_user(void) {
    print_status_narrow();
    return false;
}

#endif

#ifdef DIP_SWITCH_ENABLE

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)){
        return false;
    }
    switch (index) {
        case 0:
            action_exec(MAKE_KEYEVENT(4, 3, active));
        break;
    }
    return false;
}
#endif

enum {
    DIR_C = 0,
    DIR_N, DIR_NE, DIR_E, DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW
};

#define JOY_X_CENTER 512
#define JOY_Y_CENTER 512
#define JOY_DEAD     120
#define JOY_DIAG     70

static uint8_t joy_dir(uint16_t x, uint16_t y) {
    int16_t dx = (int16_t)x - JOY_X_CENTER;
    int16_t dy = (int16_t)y - JOY_Y_CENTER;

    bool east  = dx >  JOY_DEAD;
    bool west  = dx < -JOY_DEAD;

    bool south = dy < -JOY_DEAD;
    bool north = dy >  JOY_DEAD;

    bool de = dx >  JOY_DIAG;
    bool dw = dx < -JOY_DIAG;
    bool ds = dy < -JOY_DIAG;
    bool dn = dy >  JOY_DIAG;

    if (!(de || dw || dn || ds)) return DIR_C;

    //if (dn && dw) return DIR_NW;
    //if (dn && de) return DIR_NE;
    //if (ds && dw) return DIR_SW;
    //if (ds && de) return DIR_SE;

    if (north) return DIR_N;
    if (south) return DIR_S;
    if (west)  return DIR_W;
    if (east)  return DIR_E;

    return (abs(dx) > abs(dy)) ? ((dx > 0) ? DIR_E : DIR_W)
                               : ((dy > 0) ? DIR_N : DIR_S);
}

static void send_dir_key(uint8_t dir, bool active) {
    switch (dir) {
        case DIR_N:  action_exec(MAKE_KEYEVENT(1, 7, active)); break; // UP
        case DIR_W:  action_exec(MAKE_KEYEVENT(2, 5, active)); break; // LEFT
        case DIR_S:  action_exec(MAKE_KEYEVENT(3, 4, active)); break; // DOWN
        case DIR_E:  action_exec(MAKE_KEYEVENT(2, 6, active)); break; // RIGHT
        //case DIR_NW: action_exec(MAKE_KEYEVENT(1, 6, active)); break;
        //case DIR_NE: action_exec(MAKE_KEYEVENT(2, 4, active)); break;
        //case DIR_SW: action_exec(MAKE_KEYEVENT(2, 7, active)); break;
        //case DIR_SE: action_exec(MAKE_KEYEVENT(3, 5, active)); break;
        default: break;
    }
}

static uint16_t last_poll;
static uint8_t  last_dir = DIR_C;

void housekeeping_task_user(void) {
#ifdef DIP_SWITCH_ENABLE
    static bool dip_ini_flg = true;
    if(dip_ini_flg){
        setPinOutput(GP18);
        writePinLow(GP18);
        dip_ini_flg = false;
    }
#endif

    if (timer_elapsed(last_poll) < 10) return;
    last_poll = timer_read();

    uint16_t x = analogReadPin(JOY_X_PIN);
    uint16_t y = analogReadPin(JOY_Y_PIN);

    uint8_t dir = joy_dir(x, y);
    if (dir == last_dir) return;

    send_dir_key(last_dir, false);
    send_dir_key(dir, true);
    last_dir = dir;

    //uprintf("JOY2 x=%u y=%u dir=%u\n", x, y, dir);
}
