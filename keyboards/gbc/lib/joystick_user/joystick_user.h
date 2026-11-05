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

/* ============================================================
 * アナログジョイスティック 3モード制御モジュール
 *
 *   JOY_MODE_DIGITAL : 4方向 仮想キー入力 (VIA 上の行/列位置で編集可能)
 *   JOY_MODE_MOUSE   : マウスカーソル移動 (円形デッドゾーン、可変速度)
 *   JOY_MODE_ANALOG  : ゲームパッド アナログ軸入力 (±127)
 *
 * モード状態・キャリブレーション・マウス速度はこのモジュール内部で保持。
 * 入力モード・マウス最大速度・デッドゾーンは EEPROM に自動保存する。
 * ============================================================ */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef JOYSTICK_ENABLE

typedef enum {
    JOY_MODE_DIGITAL = 0,
    JOY_MODE_MOUSE,
    JOY_MODE_ANALOG,
    JOY_MODE_COUNT,
} joy_mode_t;

/* ---- 初期化 / 周期処理 ---- */

/* 起動時キャリブレーション (スティックに触れていないこと) */
void joy_init(void);

/* 周期ポーリング (housekeeping_task_user から呼ぶ) */
void joy_task(void);

/* ---- モード制御 ---- */

joy_mode_t joy_get_mode(void);
void       joy_set_mode(joy_mode_t mode);

/* ---- マウス最大速度 (OLED 表示・キーでの増減用) ---- */

uint8_t joy_get_mouse_speed(void);
void    joy_mouse_speed_up(void);
void    joy_mouse_speed_down(void);

/* ---- デッドゾーン (現在のモードに作用、EEPROM 永続化) ----
 * 単位は割合(%)。どのモードも倒した方向の実可動域に対する割合で、
 * 倒した距離で判定する (円形デッドゾーン)。
 * joy_deadzone_reset() も現在のモードの値だけを既定値に戻す。
 */
uint8_t joy_get_deadzone(void);
void    joy_deadzone_up(void);
void    joy_deadzone_down(void);
void    joy_deadzone_reset(void);

/* ---- キャリブレーション (JOY_CALIB キー用) ----
 * 中心位置と可動域 (config.h の JOY_*_SPAN_*) を実機で測り直し、
 * EEPROM 末尾の専用領域 (config.h の JOY_CAL_EEPROM_ADDR) に保存する。
 * 保存済みなら起動時の自動センタリングは行わず保存値を使う。
 *
 *   短押し (IDLE 時)  : 開始。約 0.5 秒スティックに触らない → 中心を取得
 *                        → そのままスティックを端に当てながら 2〜3 周回す
 *   短押し (回した後) : 確定・保存。動かしていなければ中心だけ保存
 *   2 秒長押し        : 保存値を消して既定値に戻す
 *   30 秒放置         : 中断 (保存しない)
 * 計測中は通常の入力 (デジタル/マウス/アナログ) を出さない。
 * 結果は CFG_SHOW の center / span / calib 行で確認できる。
 */
void joy_calib_key(bool pressed);   /* process_record_user から press/release 両方で呼ぶ */
bool joy_cal_is_running(void);

/* ---- 設定値のテキスト出力 (CFG_SHOW キー用) ----
 * 現在のモード / マウス最大速度 / 3 モード分のデッドゾーン / EEPROM 保存状態を
 * 1 項目 1 行でキー入力としてホストへタイプする (先頭行 "GBC24 joystick")。
 * US / JIS どちらの配列でも同じ文字になる記号だけを使う。
 */
void    joy_send_status(void);

#endif /* JOYSTICK_ENABLE */
