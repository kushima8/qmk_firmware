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

#ifdef POINTING_DEVICE_ENABLE
/* PMW3360 トラックボールの軸補正 (キーボード側で対応、ドライバは無修正)
 *
 * 同梱ドライバの pointing_device_driver_get_report() は
 *   x = d.y, y = d.x
 * を返すが、GBC24 のセンサ搭載向きではこれが
 *   上に転がす → 左 / 右に転がす → 上
 * になってしまう。ドライバ出力 (ox, oy) に対して
 *   x = -oy, y = ox
 * の回転を掛けることで 上→上 / 右→右 に補正する。
 */
report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    mouse_xy_report_t ox = mouse_report.x;
    mouse_xy_report_t oy = mouse_report.y;
    mouse_report.x = -oy;
    mouse_report.y = ox;
    return pointing_device_task_user(mouse_report);
}
#endif
