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
#include "joystick_user.h"   /* OLED 表示用 (joy_get_mode など) */

/* NOTE: snprintf (avr-libc vfprintf) はフラッシュを約 1KB 消費するため、
 * 数値表示は軽量な自前フォーマッタ (oled_write_tag_u8) で行う。 */

/* ============================================================
 * レイヤー
 * (JOY_* / CFG_SHOW などの独自キーコードは gbc24.h で定義、gbc24.c で処理)
 * ============================================================ */

enum layer_names {
    _0th,
    _1st,
    _2nd,
    _3rd,
};

/* ============================================================
 * キーマップ
 * ============================================================ */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // 右側の 5 つ (UP / LEFT ENT RGHT / DOWN) はジョイスティックのデジタルモード 4 方向と押し込み。
    // ,--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------|
    [_0th] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,                      KC_UP,
                          KC_T,    KC_U,    KC_V,                      KC_LEFT, KC_ENT,  KC_RGHT,
                                                              KC_W,             KC_DOWN,
                                                                       KC_X,    KC_Y
    ),
    [_1st] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,                      KC_UP,
                          KC_T,    KC_U,    KC_V,                      KC_LEFT, KC_ENT,  KC_RGHT,
                                                              KC_W,             KC_DOWN,
                                                                       KC_X,    KC_Y
    ),
    [_2nd] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,                      KC_UP,
                          KC_T,    KC_U,    KC_V,                      KC_LEFT, KC_ENT,  KC_RGHT,
                                                              KC_W,             KC_DOWN,
                                                                       KC_X,    KC_Y
    ),
    [_3rd] = LAYOUT(
        KC_A,    KC_B,    KC_C,    KC_D,    KC_E,    KC_F,    KC_G,
        KC_H,    KC_I,    KC_J,    KC_K,    KC_L,    KC_M,    KC_N,
                 KC_O,    KC_P,    KC_Q,    KC_R,    KC_S,                      KC_UP,
                          KC_T,    KC_U,    KC_V,                      KC_LEFT, KC_ENT,  KC_RGHT,
                                                              KC_W,             KC_DOWN,
                                                                       KC_X,    KC_Y
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

/* snprintf を使わない軽量な "X:nnn" 表示 (フラッシュ節約) */
static void oled_write_tag_u8(char tag, uint8_t v) {
    char buf[6];
    uint8_t i = 0;
    buf[i++] = tag;
    buf[i++] = ':';
    if (v >= 100) buf[i++] = (char)('0' + v / 100);
    if (v >= 10) buf[i++] = (char)('0' + (v / 10) % 10);
    buf[i++] = (char)('0' + v % 10);
    buf[i]   = '\0';
    oled_write_ln(buf, false);
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
    oled_write_tag_u8('S', joy_get_mouse_speed());
    /* 現在のモードのデッドゾーン (%) */
    oled_write_tag_u8('D', joy_get_deadzone());
    /* キャリブレーション状態 (測定中と、確定後 5 秒間の結果)。
     * oled_task_user は画面をクリアせず上書きするだけなので、表示する行数を
     * 常に 3 行に固定し、不要な行は空行で上書きして古い表示を残さない。 */
    const char *l1 = PSTR(""), *l2 = PSTR(""), *l3 = PSTR("");
    switch (joy_cal_status()) {
        case JOY_CAL_STATUS_CENTER:      l1 = PSTR("CAL"); l2 = PSTR("CTR"); break;
        case JOY_CAL_STATUS_RANGE:       l1 = PSTR("CAL"); l2 = PSTR("RNG"); break;
        case JOY_CAL_STATUS_OK:          l1 = PSTR("CAL"); l2 = PSTR("OK");  break;
        case JOY_CAL_STATUS_CENTER_ONLY: l1 = PSTR("CAL"); l2 = PSTR("CTR"); l3 = PSTR("ONLY"); break;
        case JOY_CAL_STATUS_FAILED:      l1 = PSTR("CAL"); l2 = PSTR("NG");  break;
        case JOY_CAL_STATUS_RESET:       l1 = PSTR("CAL"); l2 = PSTR("RST"); break;
        default: break;
    }
    oled_write_ln_P(l1, false);
    oled_write_ln_P(l2, false);
    oled_write_ln_P(l3, false);
}
#endif

#ifdef GBC_CORE_ENABLE
/* トラックボール構成: CPI / スクロール除数 / モード / 自動マウスレイヤー / スナップ
 * (液晶は縦向き 5 文字幅なので 1 項目 1〜2 行) */
static void oled_render_trackball(void) {
    oled_write_ln_P(PSTR(""), false);
    oled_write_ln_P(PSTR("TRACK"), false);
    /* CPI は 100 単位の内部値 (1..35) を表示: "C:35" = 3500 */
    oled_write_tag_u8('C', gbc_get_cpi());
    oled_write_tag_u8('D', gbc_get_scroll_div());
    oled_write_ln_P(gbc_get_scroll_mode() ? PSTR("SCRL") : PSTR("MOVE"), false);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    oled_write_ln_P(get_auto_mouse_enable() ? PSTR("AML+") : PSTR("AML-"), false);
#endif
    switch (gbc_get_scrollsnap_mode()) {
        case GBC_SCROLLSNAP_MODE_VERTICAL:   oled_write_ln_P(PSTR("SN:V"), false); break;
        case GBC_SCROLLSNAP_MODE_HORIZONTAL: oled_write_ln_P(PSTR("SN:H"), false); break;
        default:                             oled_write_ln_P(PSTR("SN:F"), false); break;
    }
}
#endif

bool oled_task_user(void) {
    oled_render_layer();
#ifdef JOYSTICK_ENABLE
    oled_render_joystick();
#endif
#ifdef GBC_CORE_ENABLE
    oled_render_trackball();
#endif
    return false;
}

#endif /* OLED_ENABLE */

/* ジョイスティック押し込みスイッチは gbc24.c (duplex_scan_raw_post_kb) で
 * 生マトリクスの (4,3) に合流させている。keymap 側の処理は不要。 */
