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

#include "joystick_user.h"

#ifdef JOYSTICK_ENABLE

#include <stdlib.h>          /* abs */
#include "quantum.h"
#include "analog.h"
#include "host.h"
#include "joystick.h"

/* 必須設定チェック (config.h で定義) */
#ifndef JOY_X_PIN
#  error "JOY_X_PIN must be defined in config.h"
#endif
#ifndef JOY_Y_PIN
#  error "JOY_Y_PIN must be defined in config.h"
#endif
#ifndef JOYSTICK_AXIS_RESOLUTION
#  error "JOYSTICK_AXIS_RESOLUTION must be defined in config.h"
#endif

/* ============================================================
 * 定数
 * ============================================================ */

/* --- ADC: AVR の analogReadPin() は常に 10bit (0..1023) を返す --- */
#define JOY_ADC_BITS             10
#define JOY_ADC_MAX              ((1 << JOY_ADC_BITS) - 1)         /* 1023 */
#define JOY_ADC_CENTER_DEFAULT   (1 << (JOY_ADC_BITS - 1))         /*  512 */
#define JOY_ADC_RANGE_HALF       (1 << (JOY_ADC_BITS - 1))         /*  512 */

/* --- 可動域 (カバー等で機械的に制限された、中心からの ADC 差分の絶対値) ---
 * config.h で JOY_X_SPAN_NEG / JOY_X_SPAN_POS / JOY_Y_SPAN_NEG / JOY_Y_SPAN_POS
 * を定義すると、その位置まで倒したときに HID 軸値が ±フルスケール (1.00) になる。
 * 未定義の場合は従来どおり ±512 (ADC 10bit の理論フルスケール) を仮定する。
 */
#ifndef JOY_X_SPAN_NEG
#    define JOY_X_SPAN_NEG JOY_ADC_RANGE_HALF
#endif
#ifndef JOY_X_SPAN_POS
#    define JOY_X_SPAN_POS JOY_ADC_RANGE_HALF
#endif
#ifndef JOY_Y_SPAN_NEG
#    define JOY_Y_SPAN_NEG JOY_ADC_RANGE_HALF
#endif
#ifndef JOY_Y_SPAN_POS
#    define JOY_Y_SPAN_POS JOY_ADC_RANGE_HALF
#endif

/* 4軸方向のうち最小の可動量。デジタル/マウスモードの基準に使う。
 * (QMK 側の MIN マクロ有無に依存しないよう独自名で定義)
 */
#define JOY_MIN2(a, b)   ((a) < (b) ? (a) : (b))
#define JOY_SPAN_MIN     JOY_MIN2(JOY_MIN2(JOY_X_SPAN_NEG, JOY_X_SPAN_POS), \
                                  JOY_MIN2(JOY_Y_SPAN_NEG, JOY_Y_SPAN_POS))

/* --- デッドゾーン (ADC 生値ベース、可動域に対する割合で定義) ---
 * 割合は従来の固定値 (120/512 ≈ 24%, 70/512 ≈ 14%) を踏襲している。
 * 反応を軽くしたい場合はパーセントを下げること。
 */
#define JOY_DEAD_DIGITAL_PERCENT       24   /* デジタルモード メイン方向 */
#define JOY_DEAD_DIGITAL_DIAG_PERCENT  14   /* デジタルモード センター判定 */
#define JOY_DEAD_MOUSE_PERCENT         24   /* マウスモード 円形デッドゾーン */

#define JOY_DEAD_DIGITAL       (JOY_SPAN_MIN * JOY_DEAD_DIGITAL_PERCENT / 100)
#define JOY_DEAD_DIGITAL_DIAG  (JOY_SPAN_MIN * JOY_DEAD_DIGITAL_DIAG_PERCENT / 100)
#define JOY_DEAD_MOUSE         (JOY_SPAN_MIN * JOY_DEAD_MOUSE_PERCENT / 100)

/* --- マウス最大速度 --- */
#define JOY_MOUSE_SPEED_MIN        1
#define JOY_MOUSE_SPEED_MAX       50   /* 40 → 50 (上限を少し拡張) */
#define JOY_MOUSE_SPEED_STEP       5   /* 2 → 5 (体感差を明確化) */
#define JOY_MOUSE_SPEED_DEFAULT   15

/* --- 起動時キャリブレーション --- */
#define JOY_CALIB_SAMPLES         16
#define JOY_CALIB_WAIT_MS          2

/* --- ポーリング間隔 --- */
#define JOY_POLL_INTERVAL_MS      10

/* --- マウス HID レポート軸範囲 (int8_t 固定、解像度に依存しない) --- */
#define JOY_MOUSE_AXIS_MAX       127
#define JOY_MOUSE_AXIS_MIN      (-127)

/* --- ジョイスティック HID 軸範囲 (JOYSTICK_AXIS_RESOLUTION から導出) ---
 *   8 bit → -128..127
 *  10 bit → -512..511
 *  16 bit → -32768..32767
 */
#define JOY_JS_AXIS_MAX   ((int16_t)((1L << (JOYSTICK_AXIS_RESOLUTION - 1)) - 1))
#define JOY_JS_AXIS_MIN   ((int16_t)(-JOY_JS_AXIS_MAX))

/* --- アナログモード デッドゾーン (HID 軸範囲に対する割合) ---
 *   解像度に比例してスケールされる。
 *   10bit → 511 * 12 / 100 ≈ 61
 *    8bit → 127 * 12 / 100 ≈ 15
 */
#define JOY_DEAD_ANALOG_PERCENT   12
#define JOY_DEAD_ANALOG           (JOY_JS_AXIS_MAX * JOY_DEAD_ANALOG_PERCENT / 100)

/* --- アナログモード Y 軸の向き ---
 * HID の Generic Desktop Y (Usage 0x31) は「下方向が正」と定義されている。
 * 本ファイルは dy > 0 を「スティック上」として扱っている
 * (compute_direction() の DIR_N、process_mouse() の -dy 補正を参照) ため、
 * アナログモードでも符号を反転させないと上下が逆になる。
 * 配線の都合で ADC の増加方向が逆なボードでは 0 にすること。
 */
#ifndef JOY_ANALOG_INVERT_Y
#    define JOY_ANALOG_INVERT_Y 1
#endif

#define JOY_CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))

/* ============================================================
 * デジタルモード 方向定義
 *
 * 各方向を VIA キーマップ上の行/列位置にマップする。
 * これらの位置に好きなキーコードを割り当てることで、
 * ジョイスティックの方向入力の挙動をユーザーが VIA 上で
 * 編集できる。
 * ============================================================ */

typedef enum {
    DIR_CENTER = 0,
    DIR_N,
    DIR_E,
    DIR_S,
    DIR_W,
    DIR_COUNT,
} joy_dir_t;

typedef struct {
    uint8_t row;
    uint8_t col;
} matrix_pos_t;

static const matrix_pos_t dir_matrix_pos[DIR_COUNT] = {
    [DIR_CENTER] = { 0, 0 },   /* 未使用 */
    [DIR_N]      = { 1, 7 },
    [DIR_E]      = { 2, 6 },
    [DIR_S]      = { 3, 4 },
    [DIR_W]      = { 2, 5 },
};

/* ============================================================
 * 内部状態
 * ============================================================ */

static struct {
    joy_mode_t mode;
    uint16_t   x_center;
    uint16_t   y_center;
    uint8_t    mouse_speed_max;
    uint16_t   last_poll;

    /* 冗長な HID レポートを避けるための前回値 */
    joy_dir_t  last_dir;
    int16_t    last_ax;       /* 解像度 > 8bit に対応するため int16_t */
    int16_t    last_ay;
} joy = {
    .mode            = JOY_MODE_DIGITAL,
    .x_center        = JOY_ADC_CENTER_DEFAULT,
    .y_center        = JOY_ADC_CENTER_DEFAULT,
    .mouse_speed_max = JOY_MOUSE_SPEED_DEFAULT,
    .last_poll       = 0,
    .last_dir        = DIR_CENTER,
    .last_ax         = 0,
    .last_ay         = 0,
};

/* QMK が参照する軸定義 (2軸、どちらも仮想) */
joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
    JOYSTICK_AXIS_VIRTUAL,   /* X */
    JOYSTICK_AXIS_VIRTUAL,   /* Y */
};

/* ============================================================
 * 下位ヘルパー
 * ============================================================ */

static void send_dir_key(joy_dir_t dir, bool pressed) {
    if (dir <= DIR_CENTER || dir >= DIR_COUNT) return;
    const matrix_pos_t pos = dir_matrix_pos[dir];
    action_exec(MAKE_KEYEVENT(pos.row, pos.col, pressed));
}

static void release_last_dir(void) {
    if (joy.last_dir != DIR_CENTER) {
        send_dir_key(joy.last_dir, false);
        joy.last_dir = DIR_CENTER;
    }
}

static void reset_analog_axes(void) {
    if (joy.last_ax != 0 || joy.last_ay != 0) {
        joystick_set_axis(0, 0);
        joystick_set_axis(1, 0);
        joy.last_ax = 0;
        joy.last_ay = 0;
    }
}

static void stop_mouse(void) {
    report_mouse_t report = {0};
    host_mouse_send(&report);
}

/* ADC 生値の符号付き差分を、実測した可動域 (span) を基準に HID 軸値へ線形変換。
 *
 *   delta == +span_pos  ->  +JOY_JS_AXIS_MAX  (= 1.00)
 *   delta == -span_neg  ->  -JOY_JS_AXIS_MAX  (= -1.00)
 *
 * 正方向と負方向で span を分けているのは、カバーの成形誤差やスティックの
 * 取り付け位置で可動域が非対称になることが多いため。
 *
 * span を基準に割るため、ADC の解像度 (AVR:10bit / RP2040:12bit) に
 * 依存しない。旧 adc_to_axis() のシフト処理は不要になったため廃止した。
 */
static inline int16_t scale_axis(int32_t delta, uint16_t span_neg, uint16_t span_pos) {
    uint16_t span = (delta < 0) ? span_neg : span_pos;
    if (span == 0) span = 1;   /* ゼロ除算防止 */

    int32_t v = delta * (int32_t)JOY_JS_AXIS_MAX / (int32_t)span;
    return (int16_t)JOY_CLAMP(v, JOY_JS_AXIS_MIN, JOY_JS_AXIS_MAX);
}

/* ============================================================
 * デジタルモード: 4方向判定 + 仮想キー送信
 * ============================================================ */

static joy_dir_t compute_direction(uint16_t x, uint16_t y) {
    int16_t dx = (int16_t)x - (int16_t)joy.x_center;
    int16_t dy = (int16_t)y - (int16_t)joy.y_center;

    /* センター判定: どの軸も斜め許容値を超えていない */
    if (abs(dx) <= JOY_DEAD_DIGITAL_DIAG &&
        abs(dy) <= JOY_DEAD_DIGITAL_DIAG) {
        return DIR_CENTER;
    }

    /* メイン方向: しきい値を明確に超えている軸を優先 */
    if (dy >  JOY_DEAD_DIGITAL) return DIR_N;
    if (dy < -JOY_DEAD_DIGITAL) return DIR_S;
    if (dx >  JOY_DEAD_DIGITAL) return DIR_E;
    if (dx < -JOY_DEAD_DIGITAL) return DIR_W;

    /* 斜め領域: 偏差が大きい方の軸で採決 */
    if (abs(dx) > abs(dy)) return (dx > 0) ? DIR_E : DIR_W;
    else                   return (dy > 0) ? DIR_N : DIR_S;
}

static void process_digital(uint16_t x, uint16_t y) {
    reset_analog_axes();

    joy_dir_t dir = compute_direction(x, y);
    if (dir == joy.last_dir) return;

    /* 前方向を離してから新方向を押す */
    send_dir_key(joy.last_dir, false);
    send_dir_key(dir, true);
    joy.last_dir = dir;
}

/* ============================================================
 * マウスモード: 倒し量比例速度、円形デッドゾーン
 *
 * magnitude 近似:
 *   mag ≈ max(|dx|,|dy|) + 3/8 * min(|dx|,|dy|)
 *   sqrt 不要、誤差最大約 4%
 *
 * マウス HID は int8_t 固定 (±127) なので JOYSTICK_AXIS_RESOLUTION の
 * 影響を受けない。速度は JOY_SPAN_MIN (実可動域) を基準に正規化する。
 * ============================================================ */

static void process_mouse(uint16_t x, uint16_t y) {
    release_last_dir();
    reset_analog_axes();

    int16_t dx = (int16_t)x - (int16_t)joy.x_center;
    int16_t dy = (int16_t)y - (int16_t)joy.y_center;

    uint16_t adx = abs(dx);
    uint16_t ady = abs(dy);
    uint16_t mag = (adx > ady) ? (adx + (ady * 3 / 8))
                               : (ady + (adx * 3 / 8));

    if (mag <= JOY_DEAD_MOUSE) return;

    /* デッドゾーン境界=速度 1、フル傾倒=mouse_speed_max */
    uint16_t eff     = mag - JOY_DEAD_MOUSE;
    /* 分母は実可動域。旧コードは ±512 前提だったため、カバーで可動域を
     * 制限していると mouse_speed_max に到達できなかった。
     */
    uint16_t max_eff = JOY_SPAN_MIN - JOY_DEAD_MOUSE;
    int16_t  speed   = (int16_t)((uint32_t)eff * joy.mouse_speed_max / max_eff);
    if (speed < 1) speed = 1;

    /* dx/mag, dy/mag で正規化 → speed 倍。Y は反転 (上=カーソル上)。 */
    int8_t mx = (int8_t)JOY_CLAMP(
        (int32_t)dx * speed / (int32_t)mag,
        JOY_MOUSE_AXIS_MIN, JOY_MOUSE_AXIS_MAX);
    int8_t my = (int8_t)JOY_CLAMP(
        (int32_t)(-dy) * speed / (int32_t)mag,
        JOY_MOUSE_AXIS_MIN, JOY_MOUSE_AXIS_MAX);

    if (mx == 0 && my == 0) return;

    report_mouse_t report = {0};
    report.x = mx;
    report.y = my;
    host_mouse_send(&report);
}

/* ============================================================
 * アナログモード: JOYSTICK_AXIS_RESOLUTION 分解能のゲームパッド軸を送信
 *
 * ADC 10bit と JOYSTICK_AXIS_RESOLUTION 10bit の場合は 1:1 マッピング。
 * 解像度が異なる場合は adc_to_axis() 内でシフト調整される。
 * ============================================================ */

static void process_analog(uint16_t x, uint16_t y) {
    release_last_dir();

    int32_t dx = (int32_t)x - (int32_t)joy.x_center;
    int32_t dy = (int32_t)y - (int32_t)joy.y_center;

    /* 反転は「変換後」に行う。変換前に dy の符号を反転させると、
     * scale_axis() 内の span_neg / span_pos の選択が生 ADC の方向と
     * 食い違ってしまうため。
     */
    int16_t ax = scale_axis(dx, JOY_X_SPAN_NEG, JOY_X_SPAN_POS);
    int16_t ay = scale_axis(dy, JOY_Y_SPAN_NEG, JOY_Y_SPAN_POS);

    /* HID ゲームパッドの Y は下方向が正。dy>0 (上) を負値へ写す。
     * -JOY_JS_AXIS_MIN は JOY_JS_AXIS_MAX を 1 だけ超えるため再クランプする。
     */
#if JOY_ANALOG_INVERT_Y
    ay = (int16_t)JOY_CLAMP(-(int32_t)ay, JOY_JS_AXIS_MIN, JOY_JS_AXIS_MAX);
#endif

    /* 変換後デッドゾーン (HID 軸スケール) */
    if (ax > -JOY_DEAD_ANALOG && ax < JOY_DEAD_ANALOG) ax = 0;
    if (ay > -JOY_DEAD_ANALOG && ay < JOY_DEAD_ANALOG) ay = 0;

    /* 前回と同値なら USB レポートを省略 */
    if (ax == joy.last_ax && ay == joy.last_ay) return;

    joy.last_ax = ax;
    joy.last_ay = ay;
    joystick_set_axis(0, ax);
    joystick_set_axis(1, ay);
}

/* ============================================================
 * モード切り替え時のクリーンアップ
 *   - 押しっぱなしの仮想キー解放
 *   - マウス停止レポート送信
 *   - アナログ軸ゼロクリア
 * ============================================================ */

static void cleanup_current_mode(void) {
    switch (joy.mode) {
        case JOY_MODE_DIGITAL: release_last_dir();   break;
        case JOY_MODE_MOUSE:   stop_mouse();         break;
        case JOY_MODE_ANALOG:  reset_analog_axes();  break;
        default: break;
    }
}

/* ============================================================
 * 公開 API
 * ============================================================ */

void joy_init(void) {
    /* 複数回サンプリングして平均をセンター値とする */
    int32_t sum_x = 0;
    int32_t sum_y = 0;
    for (uint8_t i = 0; i < JOY_CALIB_SAMPLES; i++) {
        sum_x += analogReadPin(JOY_X_PIN);
        sum_y += analogReadPin(JOY_Y_PIN);
        wait_ms(JOY_CALIB_WAIT_MS);
    }
    joy.x_center = (uint16_t)(sum_x / JOY_CALIB_SAMPLES);
    joy.y_center = (uint16_t)(sum_y / JOY_CALIB_SAMPLES);
}

void joy_task(void) {
    if (timer_elapsed(joy.last_poll) < JOY_POLL_INTERVAL_MS) return;
    joy.last_poll = timer_read();

    uint16_t x = analogReadPin(JOY_X_PIN);
    uint16_t y = analogReadPin(JOY_Y_PIN);

    switch (joy.mode) {
        case JOY_MODE_DIGITAL: process_digital(x, y); break;
        case JOY_MODE_MOUSE:   process_mouse(x, y);   break;
        case JOY_MODE_ANALOG:  process_analog(x, y);  break;
        default: break;
    }
}

joy_mode_t joy_get_mode(void) {
    return joy.mode;
}

void joy_set_mode(joy_mode_t mode) {
    if (mode >= JOY_MODE_COUNT || mode == joy.mode) return;
    cleanup_current_mode();
    joy.mode = mode;
}

uint8_t joy_get_mouse_speed(void) {
    return joy.mouse_speed_max;
}

void joy_mouse_speed_up(void) {
    int16_t next = (int16_t)joy.mouse_speed_max + JOY_MOUSE_SPEED_STEP;
    joy.mouse_speed_max = (uint8_t)JOY_CLAMP(
        next, JOY_MOUSE_SPEED_MIN, JOY_MOUSE_SPEED_MAX);
}

void joy_mouse_speed_down(void) {
    int16_t next = (int16_t)joy.mouse_speed_max - JOY_MOUSE_SPEED_STEP;
    joy.mouse_speed_max = (uint8_t)JOY_CLAMP(
        next, JOY_MOUSE_SPEED_MIN, JOY_MOUSE_SPEED_MAX);
}

#endif /* JOYSTICK_ENABLE */