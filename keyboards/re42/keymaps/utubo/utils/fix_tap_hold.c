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
#include "fix_tap_hold.h"
#include "keymap_jp_util.h"

tap_state_t tap_state;

static struct {
    uint16_t keycode;
    uint16_t time;
} last_pressed = {0, 0};

static void process_tap_state(uint16_t keycode, keyrecord_t *record) {
    bool in_tapping_term = (record->event.time - last_pressed.time) <= TAPPING_TERM;
    bool is_same_keycode = keycode == last_pressed.keycode;
    if (record->event.pressed) {
        last_pressed.keycode = keycode;
        last_pressed.time = record->event.time;
        tap_state = (is_same_keycode && in_tapping_term) ? DOUBLE_HOLD : SINGLE_HOLD;
    } else if (is_same_keycode && tap_state != DOUBLE_HOLD){
        tap_state = in_tapping_term ? SINGLE_TAP : LONG_TAP;
    } else {
        tap_state = KEYUP;
    }
}

static uint8_t quick_MT_mod = 0;
static void process_quick_MT(void) {
    if (quick_MT_mod && tap_state == SINGLE_HOLD) {
        register_mods(quick_MT_mod);
        quick_MT_mod = 0;
    }
}
bool quick_MT(uint16_t mod_key, uint16_t keycode) {
    if (tap_state != SINGLE_HOLD) {
        quick_MT_mod = 0;
        unregister_mods(MOD_BIT(mod_key));
    }
    switch (tap_state) {
        // register mod_key when another key is pressed in process_quick_MT.
        case SINGLE_HOLD: quick_MT_mod = MOD_BIT(mod_key); break;
        case DOUBLE_HOLD: register_code16(keycode); break;
        case SINGLE_TAP:  tap_code16(keycode); break;
        case LONG_TAP:    tap_code16(mod_key); break;
        default: unregister_code16(keycode);
    }
    return false;
}

bool quick_LT(uint8_t layer, uint16_t keycode) {
    if (tap_state != SINGLE_HOLD) {
        layer_off(layer);
    }
    switch (tap_state) {
        case SINGLE_HOLD: layer_on(layer); break;
        case DOUBLE_HOLD: register_code16(keycode); break;
        case SINGLE_TAP:  tap_code16(keycode); break;
        default: unregister_code16(keycode); break;
    }
    return false;
}


bool sh_t16(uint16_t keycode) {
    #ifdef SWAP_HANDS_ENABLE
    swap_hands = !swap_hands;
    #endif
    if (tap_state == SINGLE_TAP) {
        tap_code16jp(keycode);
        //tap_code16(keycode);
    }
    return false;
}

void process_fix_tap_hold(uint16_t keycode, keyrecord_t *record) {
    process_tap_state(keycode, record);
    process_quick_MT();
}

