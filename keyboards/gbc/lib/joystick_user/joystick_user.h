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

#endif /* JOYSTICK_ENABLE */
