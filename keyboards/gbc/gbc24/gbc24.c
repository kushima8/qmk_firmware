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

#include "gbc24.h"
#include "lib/duplexmatrix/duplexmatrix.h"
#include "joystick_user.h"

#ifdef JOYSTICK_ENABLE
/* ジョイスティック押し込みスイッチ (config.h の JOY_SW_* 参照)
 *
 * matrix_init_kb() は matrix_init() の末尾で呼ばれ、quantum_init() →
 * bootmagic() の最初の matrix_scan() より前に実行されるため、
 * ここでピンを設定しておけば初回スキャンから正しい値が読める。
 */
void matrix_init_kb(void) {
    setPinOutput(JOY_SW_GND_PIN);
    writePinLow(JOY_SW_GND_PIN);
    setPinInputHigh(JOY_SW_PIN);
    matrix_init_user();
}

/* duplex_scan_raw() の末尾で毎スキャン呼ばれる。生マトリクスに押し込み
 * スイッチの状態を合流させるだけで、デバウンスは matrix_scan() 側の
 * debounce() が通常キーと同じように行う。
 */
void duplex_scan_raw_post_kb(matrix_row_t out_matrix[]) {
    if (!readPin(JOY_SW_PIN)) {
        out_matrix[JOY_SW_ROW] |= (matrix_row_t)1 << JOY_SW_COL;
    }
}
#endif /* JOYSTICK_ENABLE */

/* ---- Bootmagic: 2 キー同時押しのときだけ動作させる ----
 *
 * 既定の Bootmagic は「左上キー (0,0) を押しながら USB 接続」で EEPROM を
 * 初期化してブートローダーへ入る。1 キーだけでは、ケーブルを挿すときに
 * たまたま触れただけで設定が消えるため、条件を次のように絞る。
 *
 *   上段の左端 (0,0) と 上段の右端 (1,2) の 2 キー「だけ」が押されている
 *
 * ほかのキーが 1 つでも押されていれば何もしない (手のひらでキー部全体を
 * 押さえながらケーブルを挿した場合の誤動作を防ぐ)。左上キーだけを押して
 * 接続した場合も、何もせず通常どおり起動する。
 *
 * QMK 版と Vial 版ではベースの QMK が異なり、上書きする関数名が違う。
 *   古いベース (QMK 版) : bootmagic_lite() を丸ごと置き換える
 *   新しいベース (Vial 版): bootmagic_scan() が bootmagic_should_reset() を呼ぶ
 * 両方を定義しておけば同じソースを共有できる (使われないほうは LTO で消える)。
 */
/* マクロ名もベースで異なる: 古いベースは BOOTMAGIC_LITE、新しいベースは BOOTMAGIC_ENABLE */
#if defined(BOOTMAGIC_ENABLE) || defined(BOOTMAGIC_LITE)
#    define GBC24_BOOT_A_ROW 0 /* 上段の左端 (K00) */
#    define GBC24_BOOT_A_COL 0
#    define GBC24_BOOT_B_ROW 1 /* 上段の右端 (K12) */
#    define GBC24_BOOT_B_COL 2

/* 行 0 は左端だけ、行 1 は右端だけ、行 2 以降は何も押されていないこと。
 * (2 つのキーが別の行にある前提の書き方。AVR 版 Vial はフラッシュの空きが
 *  100 バイト前後しかないため、汎用のループより小さくなるこの形にしている) */
_Static_assert(GBC24_BOOT_A_ROW == 0 && GBC24_BOOT_B_ROW == 1, "gbc24_boot_combo() assumes rows 0 and 1");

static bool gbc24_boot_combo(void) {
    if (matrix_get_row(0) != ((matrix_row_t)1 << GBC24_BOOT_A_COL)) return false;
    if (matrix_get_row(1) != ((matrix_row_t)1 << GBC24_BOOT_B_COL)) return false;
    for (uint8_t r = 2; r < MATRIX_ROWS; r++) {
        if (matrix_get_row(r)) return false;
    }
    return true;
}

bool bootmagic_should_reset(void);
bool bootmagic_should_reset(void) {
    return gbc24_boot_combo();
}

void bootmagic_lite(void);
void bootmagic_lite(void) {
    /* デバウンスを無効にできないので 2 回スキャンする (QMK 標準と同じ) */
    matrix_scan();
#    if defined(DEBOUNCE) && DEBOUNCE > 0
    wait_ms(DEBOUNCE * 2);
#    else
    wait_ms(30);
#    endif
    matrix_scan();

    if (gbc24_boot_combo()) {
        eeconfig_disable();
        bootloader_jump();
    }
}
#endif /* BOOTMAGIC_ENABLE || BOOTMAGIC_LITE */

/* トラックボール構成の軸補正 (x = -d.x, y = d.y) は lib/gbc/gbc.c の
 * gbc_on_apply_motion_to_mouse_move() で行う。キーボード側では何もしない。 */

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
 *   center          541 501
 *   range           248 787 223 737
 *   span            284 238 269 228
 *   calib           saved, last ok
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
 * QMK フック (キーボードレベル)
 *
 * 独自キーコードの処理と、ジョイスティックの初期化・周期処理をここで行う。
 * キーマップ側に process_record_user() などを書かなくても動くため、
 * default 以外のキーマップ (自作キーマップなど) でも同じように使える。
 * ============================================================ */

void keyboard_post_init_kb(void) {
#ifdef JOYSTICK_ENABLE
    joy_init();
#endif
#ifdef GBC_CORE_ENABLE
    gbc_keyboard_post_init();
#endif
    keyboard_post_init_user();
}

/* housekeeping_task_user() は QMK 本体が別途呼ぶので、ここからは呼ばない */
void housekeeping_task_kb(void) {
#ifdef JOYSTICK_ENABLE
    joy_task();
#endif
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_user(keycode, record)) {
        return false;
    }
#ifdef GBC_CORE_ENABLE
    /* トラックボール用キーコード (REC_RST 〜 SSNP_FRE) */
    if (!gbc_process_record(keycode, record)) {
        return false;
    }
#endif
#ifdef JOYSTICK_ENABLE
    /* 長押し判定のため press / release の両方を渡す */
    if (keycode == JOY_CALIB) {
        joy_calib_key(record->event.pressed);
        return false;
    }
#endif
    if (!record->event.pressed) return true;

    switch (keycode) {
#ifdef JOYSTICK_ENABLE
        case JOY_DIGI:   joy_set_mode(JOY_MODE_DIGITAL); return false;
        case JOY_MOUS:   joy_set_mode(JOY_MODE_MOUSE);   return false;
        case JOY_ANLG:   joy_set_mode(JOY_MODE_ANALOG);  return false;
        case JOY_SPD_UP: joy_mouse_speed_up();           return false;
        case JOY_SPD_DN: joy_mouse_speed_down();         return false;
        case JOY_DZ_UP:  joy_deadzone_up();              return false;
        case JOY_DZ_DN:  joy_deadzone_down();            return false;
        case JOY_DZ_RST: joy_deadzone_reset();           return false;
#endif
        case CFG_SHOW:   send_config_status();           return false;
        default: break;
    }
    return true;
}

/* ---- Magic 機能のスタブ (MAGIC_ENABLE = no) ----
 * keycode_config() / mod_config() は Ctrl⇔Caps 入れ替えなどの Magic 設定を
 * 全キーに適用する関数で、QMK 本体では必ずリンクされる。Magic キーコードを
 * 無効にすると設定を変える手段がなくなるので、「そのまま返す」スタブに
 * 置き換えて約 570 バイト節約する (QMK 公式 "Squeezing AVR" の手法)。
 * トラックボール構成では lib/gbc/gbc.c が同じスタブを定義しているため、
 * ジョイスティック構成 (GBC_CORE_ENABLE 未定義) のときだけここで定義する。
 */
#if !defined(MAGIC_KEYCODE_ENABLE) && !defined(QMK_SETTINGS) && \
    !defined(GBC_CORE_ENABLE) && !defined(GBC_KEEP_MAGIC_FUNCTIONS)
uint16_t keycode_config(uint16_t keycode) {
    return keycode;
}
uint8_t mod_config(uint8_t mod) {
    return mod;
}
#endif
