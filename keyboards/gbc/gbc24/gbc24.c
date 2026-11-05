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

/* GBC_CORE_ENABLE (lib/gbc/gbc.c 経由) の場合、gbc.c の
 * gbc_on_apply_motion_to_mouse_move() が x = -d.x, y = d.y を出力する。
 * これは「旧ドライバ出力 (x=d.y, y=d.x) + 上記回転補正」の最終結果と同一の
 * マッピングであるため、キーボード側の追加補正は不要 (二重適用防止のため無効化)。
 */

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
