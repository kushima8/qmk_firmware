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

#include QMK_KEYBOARD_H
#include "joystick_user.h"
#ifdef GBC_CORE_ENABLE
#  include "lib/gbc/gbc.h"   /* gbc_send_status() (トラックボール構成) */
#endif

#ifdef OLED_ENABLE
#  include <stdio.h>   /* snprintf */
#endif

/* ============================================================
 * レイヤー / カスタムキーコード
 * ============================================================ */

enum layer_names {
    _0th,
    _1st,
    _2nd,
    _3rd,
};

enum custom_keycodes {
    JOY_DIGI = QK_KB_16,     /* VIA custom keycode range (user00) */
    JOY_MOUS,                /* ジョイスティック → マウス */
    JOY_ANLG,                /* ジョイスティック → アナログ (ゲームパッド軸) */
    JOY_SPD_UP,              /* マウス最大速度 上げる */
    JOY_SPD_DN,              /* マウス最大速度 下げる */
    JOY_DZ_UP,               /* 現在のモードのデッドゾーン 広げる (鈍くする) */
    JOY_DZ_DN,               /* 現在のモードのデッドゾーン 狭める (敏感にする) */
    JOY_DZ_RST,              /* 3 モードのデッドゾーンを既定値に戻す */
    CFG_SHOW,                /* 現在の設定値をテキストとしてタイプ出力 (QK_KB_24) */
};

#ifdef JOYSTICK_ENABLE
/* デバッグ用: 最後に受信したカスタム keycode 種別
 *   0 = 未受信
 *   1 = JOY_DIGI
 *   2 = JOY_MOUS
 *   3 = JOY_ANLG
 *   4 = JOY_SPD_UP
 *   5 = JOY_SPD_DN
 *   6 = JOY_DZ_UP
 *   7 = JOY_DZ_DN
 *   8 = JOY_DZ_RST
 */
static uint8_t last_joy_event = 0;
#endif

/* ============================================================
 * キーマップ
 * ============================================================ */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
    [_0th] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,
                          KC_T,    KC_U,    KC_V,
                                                              KC_W,    KC_X,    KC_Y
    ),
    [_1st] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,
                          KC_T,    KC_U,    KC_V,
                                                              KC_W,    KC_X,    KC_Y
    ),
    [_2nd] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,
                          KC_T,    KC_U,    KC_V,
                                                              KC_W,    KC_X,    KC_Y
    ),
    [_3rd] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,
                          KC_T,    KC_U,    KC_V,
                                                              KC_W,    KC_X,    KC_Y
    ),
    // `--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------'
};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_0th] = { ENCODER_CCW_CW(KC_1, KC_2) },
    [_1st] = { ENCODER_CCW_CW(KC_3, KC_4) },
    [_2nd] = { ENCODER_CCW_CW(KC_5, KC_6) },
    [_3rd] = { ENCODER_CCW_CW(KC_7, KC_8) },
};
#endif

/* ============================================================
 * OLED
 * ============================================================ */

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

static void oled_render_layer(void) {
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("LAYER"), false);
    switch (get_highest_layer(layer_state)) {
        case _0th: oled_write_ln_P(PSTR("0th"),   false); break;
        case _1st: oled_write_ln_P(PSTR("1st"),   false); break;
        case _2nd: oled_write_ln_P(PSTR("2nd"),   false); break;
        case _3rd: oled_write_ln_P(PSTR("3rd"),   false); break;
        default:   oled_write_P(   PSTR("Undef"), false); break;
    }
}

#ifdef JOYSTICK_ENABLE
static void oled_render_joystick(void) {
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("JOY"), false);
    switch (joy_get_mode()) {
        case JOY_MODE_DIGITAL:
            oled_write_ln_P(PSTR("DIGI"), false);
            break;
        case JOY_MODE_MOUSE:
            oled_write_ln_P(PSTR("MOUS"), false);
            break;
        case JOY_MODE_ANALOG:
            oled_write_ln_P(PSTR("ANLG"), false);
            break;
        default: break;
    }
    /* マウス速度は常に表示 (JOY_SPD_UP/DN の効果をどのモードでも確認可能) */
    char buf[6];
    snprintf(buf, sizeof(buf), "S:%u", joy_get_mouse_speed());
    oled_write_ln(buf, false);
    /* 現在のモードのデッドゾーン (%) */
    snprintf(buf, sizeof(buf), "D:%u", joy_get_deadzone());
    oled_write_ln(buf, false);
    /* デバッグ: 最後に受信した JOY 系 keycode の種別 */
    snprintf(buf, sizeof(buf), "E:%u", last_joy_event);
    oled_write_ln(buf, false);
}
#endif

bool oled_task_user(void) {
    oled_render_layer();
#ifdef JOYSTICK_ENABLE
    oled_render_joystick();
#endif
    return false;
}

#endif /* OLED_ENABLE */

/* ============================================================
 * DIP スイッチ
 * DIP0 を切り替えると、VIA 編集可能な仮想キー位置
 * (DIP0_ROW, DIP0_COL) にキーイベントが発生する。
 * ============================================================ */

#ifdef DIP_SWITCH_ENABLE

#define DIP0_ROW 4
#define DIP0_COL 3

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    switch (index) {
        case 0:
            action_exec(MAKE_KEYEVENT(DIP0_ROW, DIP0_COL, active));
            break;
        default:
            break;
    }
    return false;
}

#endif /* DIP_SWITCH_ENABLE */

/* ============================================================
 * 設定値のテキスト出力 (CFG_SHOW)
 *
 * OLED が無い個体でも現在の設定を確認できるよう、設定値を
 * キー入力としてホストへタイプする (1 項目 1 行、Enter で改行)。
 * メモ帳などのテキストエディタを開いて押すこと
 * (チャット欄などでは改行で送信されてしまう)。
 *
 * 出力例 (ジョイスティック構成):
 *   GBC24 joystick
 *   mode            mouse
 *   mouse speed     15 / 50
 *   deadzone digi   54%
 *   deadzone mouse  24%   <- current
 *   deadzone analog 12%
 *   eeprom          saved
 *
 * 出力例 (トラックボール構成):
 *   GBC24 trackball
 *   cpi             500
 *   scroll mode     off
 *   scroll div      4  1/8
 *   scroll snap     vertical
 *   auto mouse      on
 *   aml timeout     650ms
 *   eeprom          saved
 *
 * 日本語入力 ON のままだとかなに変換されるので、IME を OFF にしてから押す。
 * config.h で CFG_SHOW_SEND_LNG2 を定義すると、出力前に KC_LNG2
 * (Mac の「英数」キー) を送って IME を自動で OFF にする。
 * ============================================================ */

static void send_config_status(void) {
    /* 押下中の修飾キーがあると大文字化・ショートカット化するため一時的に外す */
    const uint8_t mods     = get_mods();
    const uint8_t osm_mods = get_oneshot_mods();
    clear_mods();
    clear_weak_mods();
    clear_oneshot_mods();
    send_keyboard_report();

#ifdef CFG_SHOW_SEND_LNG2
    tap_code(KC_LNG2);
#endif

#if defined(JOYSTICK_ENABLE)
    joy_send_status();
#elif defined(GBC_CORE_ENABLE)
    gbc_send_status();
#else
    send_string_P(PSTR("GBC24\nno pointing device\n"));
#endif

    set_mods(mods);
    set_oneshot_mods(osm_mods);
    send_keyboard_report();
}

/* ============================================================
 * QMK フック
 * ============================================================ */

void keyboard_post_init_user(void) {
#ifdef JOYSTICK_ENABLE
    joy_init();
#endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    switch (keycode) {
#ifdef JOYSTICK_ENABLE
        case JOY_DIGI:   joy_set_mode(JOY_MODE_DIGITAL); last_joy_event = 1; return false;
        case JOY_MOUS:   joy_set_mode(JOY_MODE_MOUSE);   last_joy_event = 2; return false;
        case JOY_ANLG:   joy_set_mode(JOY_MODE_ANALOG);  last_joy_event = 3; return false;
        case JOY_SPD_UP: joy_mouse_speed_up();           last_joy_event = 4; return false;
        case JOY_SPD_DN: joy_mouse_speed_down();         last_joy_event = 5; return false;
        case JOY_DZ_UP:  joy_deadzone_up();              last_joy_event = 6; return false;
        case JOY_DZ_DN:  joy_deadzone_down();            last_joy_event = 7; return false;
        case JOY_DZ_RST: joy_deadzone_reset();           last_joy_event = 8; return false;
#endif
        case CFG_SHOW:   send_config_status();                               return false;
        default: break;
    }
    return true;
}

void housekeeping_task_user(void) {
#ifdef DIP_SWITCH_ENABLE
    /* 初回のみ DIP スイッチの GND 側ピンを出力 Low に設定 (ハードウェア初期化) */
    static bool dip_init_done = false;
    if (!dip_init_done) {
        setPinOutput(DIP_SWITCH_GND_PIN);
        writePinLow(DIP_SWITCH_GND_PIN);
        dip_init_done = true;
    }
#endif

#ifdef JOYSTICK_ENABLE
    joy_task();
#endif
}
