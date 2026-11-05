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
#ifdef MOUSEKEY_ENABLE
#    include "mousekey.h"   /* mousekey_get_report (ドラッグ中のボタン状態) */
#endif

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

/* 倒し量の正規化スケール: どの方向も、カバーに当たる位置 (span) まで倒すと
 * JOY_NORM_FULL になる。デジタル / マウスモードはこのスケールで判定するため、
 * 方向ごとに可動域が違っても「倒せる距離の何%か」で同じように反応する。
 * (アナログモードは scale_axis() で同じ考え方の HID 軸スケールに変換する)
 */
#define JOY_NORM_FULL         1024

/* --- デッドゾーン (実行時可変) ---
 * 3 モードそれぞれのデッドゾーンを「割合(%)」として実行時に保持する。
 * JOY_DZ_UP / JOY_DZ_DN キーで現在のモードの値を増減し、EEPROM に保存する。
 *
 *   どのモードも「倒した方向の実可動域 (中心からカバーに当たるまで)」に対する割合。
 *   判定は倒した距離 (ベクトルの大きさ) で行う円形デッドゾーン。
 *     デジタル / マウス : JOY_NORM_FULL (正規化スケール) 基準
 *     アナログ          : JOY_JS_AXIS_MAX (HID 軸フルスケール = 可動域の端) 基準
 *
 * デジタルモード:
 *   デッドゾーン = 入力が始まる位置 (発火点)。方向は上下・左右のうち
 *   大きく倒れているほうの軸で決める (45° が境目、上下左右に優先順位なし)。
 *   境目でのチャタリングを防ぐため、JOY_DIGITAL_HYST の余裕 (ヒステリシス) を持つ:
 *     - 入力中は、発火点より JOY_DIGITAL_HYST だけ内側に戻すまで離さない
 *     - 方向の切り替えは、新しい方向の成分が今の方向より JOY_DIGITAL_HYST 以上
 *       大きくなるまで行わない
 */
#define JOY_DZ_STEP                 2   /* キー 1 回あたりの増減量 (%) */

#define JOY_DZ_DIGITAL_MIN          6
#define JOY_DZ_DIGITAL_MAX         90
#define JOY_DZ_DIGITAL_DEFAULT     54   /* 可動域の 54% で入力開始 */

#define JOY_DZ_MOUSE_MIN            4
#define JOY_DZ_MOUSE_MAX           90
#define JOY_DZ_MOUSE_DEFAULT       24   /* 可動域の 24% で移動開始 */

#define JOY_DZ_ANALOG_MIN           2
#define JOY_DZ_ANALOG_MAX          90
#define JOY_DZ_ANALOG_DEFAULT      12   /* AXIS_MAX=511 で 61 */

/* デジタルモードのヒステリシス幅 (可動域の %)。
 * ADC の読み取りの揺れ (正規化スケールで数カウント〜十数カウント) より十分大きく、
 * 操作感を損なわない程度の値。最小デッドゾーン 6% より小さいこと。
 */
#define JOY_DIGITAL_HYST_PCT        4

/* 実行時のしきい値 (正規化スケール / HID 軸値)
 * AVR は int が 16bit のため、乗算は int32_t に広げてから行う
 * (16bit 解像度時 AXIS_MAX=32767 で桁溢れするため)
 */
#define JOY_DEAD_DIGITAL       ((int16_t)((int32_t)JOY_NORM_FULL * joy.dz_digital / 100))
#define JOY_DIGITAL_HYST       ((int16_t)((int32_t)JOY_NORM_FULL * JOY_DIGITAL_HYST_PCT / 100))
#define JOY_DEAD_MOUSE         ((uint16_t)((int32_t)JOY_NORM_FULL * joy.dz_mouse / 100))
#define JOY_DEAD_ANALOG        ((int16_t)((int32_t)JOY_JS_AXIS_MAX * joy.dz_analog / 100))

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

/* マウスモードだけは短い間隔で読み取り・送信し、カーソルを滑らかに動かす。
 * 移動量は実際に経過した時間に比例させるため、間隔を変えても
 * 1 秒あたりの移動量 (マウス最大速度の体感) は変わらない。
 *   マウス最大速度 = 基準時間 JOY_MOUSE_SPEED_BASE_MS あたりの最大移動カウント
 * 読み取りが遅れた場合 (EEPROM 書き込み中など) に一度に大きく飛ばないよう、
 * 1 回に扱う経過時間は JOY_MOUSE_DT_MAX_MS で頭打ちにする。
 */
#define JOY_MOUSE_POLL_INTERVAL_MS 4    /* 250Hz */
#define JOY_MOUSE_SPEED_BASE_MS   10
#define JOY_MOUSE_DT_MAX_MS       20

/* --- マウス HID レポート軸範囲 (int8_t 固定、解像度に依存しない) --- */
#define JOY_MOUSE_AXIS_MAX       127
#define JOY_MOUSE_AXIS_MIN      (-127)

/* --- ジョイスティック HID 軸範囲 (JOYSTICK_AXIS_RESOLUTION から導出) ---
 *   8 bit → -128..127
 *  10 bit → -512..511
 *  16 bit → -32768..32767
 */
#define JOY_JS_AXIS_MAX   ((int16_t)((1L << (JOYSTICK_AXIS_RESOLUTION - 1)) - 1))
/* QMK 本体は USB ディスクリプタで論理範囲を -JOYSTICK_MAX_VALUE ..
 * +JOYSTICK_MAX_VALUE (対称) と申告する (tmk_core/protocol/usb_descriptor.c)。
 * 下限を -(1<<n-1) にすると申告値を 1 下回り、ホスト側で -1.00196 のような
 * 超過値になるため、上限と対称に定義する。
 */
#define JOY_JS_AXIS_MIN   ((int16_t)(-JOY_JS_AXIS_MAX))

/* アナログモードのデッドゾーンは上部の実行時可変ブロックで定義済み
 * (JOY_DEAD_ANALOG / JOY_DZ_ANALOG_*)。解像度に比例してスケールされる。
 */

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

    /* デッドゾーン (割合%、実行時可変) */
    uint8_t    dz_digital;
    uint8_t    dz_mouse;
    uint8_t    dz_analog;

    /* EEPROM 遅延書き込み: 変更後 JOY_EE_SAVE_DELAY_MS 経過で 1 回だけ書く */
    bool       ee_dirty;
    uint16_t   ee_dirty_at;

    /* 冗長な HID レポートを避けるための前回値 */
    joy_dir_t  last_dir;
    int16_t    last_ax;       /* 解像度 > 8bit に対応するため int16_t */
    int16_t    last_ay;

    /* マウスモードの移動量の端数 (1/256 カウント単位)。
     * 1 ポーリングで 1 カウントに満たない移動を次回へ持ち越す。 */
    int16_t    mouse_acc_x;
    int16_t    mouse_acc_y;
} joy = {
    .mode            = JOY_MODE_DIGITAL,
    .x_center        = JOY_ADC_CENTER_DEFAULT,
    .y_center        = JOY_ADC_CENTER_DEFAULT,
    .mouse_speed_max = JOY_MOUSE_SPEED_DEFAULT,
    .last_poll       = 0,
    .dz_digital      = JOY_DZ_DIGITAL_DEFAULT,
    .dz_mouse        = JOY_DZ_MOUSE_DEFAULT,
    .dz_analog       = JOY_DZ_ANALOG_DEFAULT,
    .ee_dirty        = false,
    .ee_dirty_at     = 0,
    .last_dir        = DIR_CENTER,
    .last_ax         = 0,
    .last_ay         = 0,
    .mouse_acc_x     = 0,
    .mouse_acc_y     = 0,
};

/* ============================================================
 * EEPROM 永続化 (eeconfig_user: 4 バイト)
 *
 * eeconfig_kb は gbc.c がトラックボール設定で使用中のため、
 * ユーザーデータブロック側を使う。
 * 書き込み回数を抑えるため、変更から一定時間後に 1 回だけ書く。
 *
 * 4 バイト目は下位 6bit がマウス最大速度 (1〜50)、上位 2bit が入力モード。
 * 旧ファームの保存データは上位 2bit が 0 (= デジタル) なので、そのまま読める。
 * ============================================================ */

#define JOY_EE_SAVE_DELAY_MS   2000

typedef union {
    uint32_t raw;
    struct {
        uint8_t dz_digital;
        uint8_t dz_mouse;
        uint8_t dz_analog;
        uint8_t speed_mode;   /* bit0-5: マウス最大速度 / bit6-7: 入力モード */
    };
} joy_eeconfig_t;

#define JOY_EE_SPEED_MASK   0x3F
#define JOY_EE_MODE_SHIFT   6

static void joy_ee_load(void) {
    if (!eeconfig_is_enabled()) return;

    joy_eeconfig_t c = {.raw = eeconfig_read_user()};

    /* 未初期化 (0x00 / 0xFF) や範囲外は既定値のまま使う */
    if (c.dz_digital >= JOY_DZ_DIGITAL_MIN && c.dz_digital <= JOY_DZ_DIGITAL_MAX) {
        joy.dz_digital = c.dz_digital;
    }
    if (c.dz_mouse >= JOY_DZ_MOUSE_MIN && c.dz_mouse <= JOY_DZ_MOUSE_MAX) {
        joy.dz_mouse = c.dz_mouse;
    }
    if (c.dz_analog >= JOY_DZ_ANALOG_MIN && c.dz_analog <= JOY_DZ_ANALOG_MAX) {
        joy.dz_analog = c.dz_analog;
    }
    uint8_t speed = c.speed_mode & JOY_EE_SPEED_MASK;
    if (speed >= JOY_MOUSE_SPEED_MIN && speed <= JOY_MOUSE_SPEED_MAX) {
        joy.mouse_speed_max = speed;
        /* 速度が有効なときだけモードも採用する (未初期化 0xFF を誤読しないため) */
        uint8_t mode = c.speed_mode >> JOY_EE_MODE_SHIFT;
        if (mode < JOY_MODE_COUNT) {
            joy.mode = (joy_mode_t)mode;
        }
    }
}

static void joy_ee_mark_dirty(void) {
    joy.ee_dirty    = true;
    joy.ee_dirty_at = timer_read();
}

static void joy_ee_flush(void) {
    if (!joy.ee_dirty) return;
    if (timer_elapsed(joy.ee_dirty_at) < JOY_EE_SAVE_DELAY_MS) return;

    joy.ee_dirty = false;
    if (!eeconfig_is_enabled()) return;

    joy_eeconfig_t c = {
        .dz_digital  = joy.dz_digital,
        .dz_mouse    = joy.dz_mouse,
        .dz_analog   = joy.dz_analog,
        .speed_mode  = (uint8_t)((joy.mouse_speed_max & JOY_EE_SPEED_MASK) |
                                 ((uint8_t)joy.mode << JOY_EE_MODE_SHIFT)),
    };
    eeconfig_update_user(c.raw);
}

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

/* 現在押されているマウスボタン (KC_BTN1 など)。
 * 移動レポートにボタン状態を載せないと、ホスト側でボタンが離されたことになり
 * ドラッグが途切れるため、MOUSEKEY が保持している状態を引き継ぐ。
 */
static uint8_t current_mouse_buttons(void) {
#ifdef MOUSEKEY_ENABLE
    return mousekey_get_report().buttons;
#else
    return 0;
#endif
}

static void reset_mouse_accum(void) {
    joy.mouse_acc_x = 0;
    joy.mouse_acc_y = 0;
}

static void stop_mouse(void) {
    reset_mouse_accum();
    report_mouse_t report = {0};
    report.buttons = current_mouse_buttons();
    host_mouse_send(&report);
}

/* ベクトルの大きさの近似: max(|a|,|b|) + 3/8 * min(|a|,|b|)
 * sqrt 不要、誤差最大約 4%
 */
static uint16_t joy_magnitude(int16_t a, int16_t b) {
    uint16_t aa = (uint16_t)abs(a);
    uint16_t ab = (uint16_t)abs(b);
    return (aa > ab) ? (uint16_t)(aa + ab * 3 / 8) : (uint16_t)(ab + aa * 3 / 8);
}

/* ADC 差分を方向ごとの可動域で正規化する (span 位置で ±JOY_NORM_FULL)。
 * 可動域が方向によって違っても、倒し切ったときの速さが揃う。
 */
static int16_t joy_norm_axis(int16_t delta, uint16_t span_neg, uint16_t span_pos) {
    uint16_t span = (delta < 0) ? span_neg : span_pos;
    if (span == 0) span = 1;
    int32_t v = (int32_t)delta * JOY_NORM_FULL / (int32_t)span;
    return (int16_t)JOY_CLAMP(v, -2 * JOY_NORM_FULL, 2 * JOY_NORM_FULL);
}

/* 1/256 単位の移動量を端数ごと積算し、整数カウント分だけ取り出す */
static int8_t joy_take_mouse_count(int16_t *acc, int32_t v_q8) {
    int32_t a   = (int32_t)*acc + v_q8;
    int32_t out = a / 256;                 /* 0 方向へ切り捨て、残りは持ち越し */
    out         = JOY_CLAMP(out, JOY_MOUSE_AXIS_MIN, JOY_MOUSE_AXIS_MAX);
    *acc        = (int16_t)(a - out * 256);
    return (int8_t)out;
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

/* 指定した方向への倒し量 (正規化スケール、反対向きなら負) */
static int16_t dir_component(joy_dir_t dir, int16_t nx, int16_t ny) {
    switch (dir) {
        case DIR_N: return ny;
        case DIR_S: return (int16_t)-ny;
        case DIR_E: return nx;
        case DIR_W: return (int16_t)-nx;
        default:    return 0;
    }
}

static joy_dir_t compute_direction(uint16_t x, uint16_t y) {
    int16_t dx = (int16_t)x - (int16_t)joy.x_center;
    int16_t dy = (int16_t)y - (int16_t)joy.y_center;

    /* 方向ごとの可動域で正規化 (どの方向も倒し切りで JOY_NORM_FULL) */
    int16_t nx = joy_norm_axis(dx, JOY_X_SPAN_NEG, JOY_X_SPAN_POS);
    int16_t ny = joy_norm_axis(dy, JOY_Y_SPAN_NEG, JOY_Y_SPAN_POS);

    /* センター判定: 倒した距離で判定 (円形デッドゾーン、斜めも同じ距離で反応)。
     * 入力中は発火点より JOY_DIGITAL_HYST だけ内側に戻すまで離さない。
     */
    int16_t threshold = JOY_DEAD_DIGITAL;
    if (joy.last_dir != DIR_CENTER) {
        threshold -= JOY_DIGITAL_HYST;
        if (threshold < 0) threshold = 0;
    }
    if (joy_magnitude(nx, ny) <= (uint16_t)threshold) {
        return DIR_CENTER;
    }

    /* 方向: 大きく倒れているほうの軸で決める (45° が境目、優先順位なし) */
    joy_dir_t cand;
    if (abs(nx) > abs(ny)) cand = (nx > 0) ? DIR_E : DIR_W;
    else                   cand = (ny > 0) ? DIR_N : DIR_S;

    /* 方向の切り替えは、新しい方向の成分が今の方向より
     * JOY_DIGITAL_HYST 以上大きくなるまで行わない (45° 付近での入れ替わり防止)
     */
    if (joy.last_dir != DIR_CENTER && cand != joy.last_dir) {
        int16_t cur = dir_component(joy.last_dir, nx, ny);
        int16_t nxt = dir_component(cand, nx, ny);
        if (cur > 0 && nxt - cur < JOY_DIGITAL_HYST) {
            return joy.last_dir;
        }
    }
    return cand;
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
 * - 倒し量は方向ごとの可動域で正規化する (どの方向も倒し切りで最大速度)
 * - 速度は 1/256 カウント単位で計算し、端数を次回へ持ち越す
 *   (低速時や斜め方向でも移動が切り捨てで消えない)
 * - 速度は mouse_speed_max を超えない
 * - 移動レポートには現在のマウスボタン状態を載せる (ドラッグ対応)
 * - 4ms ごとに送信し、移動量は経過時間に比例させる (滑らかさと速度の両立)
 *
 * マウス HID は int8_t 固定 (±127) なので JOYSTICK_AXIS_RESOLUTION の
 * 影響を受けない。
 * ============================================================ */

static void process_mouse(uint16_t x, uint16_t y, uint16_t dt_ms) {
    release_last_dir();
    reset_analog_axes();

    int16_t dx = (int16_t)x - (int16_t)joy.x_center;
    int16_t dy = (int16_t)y - (int16_t)joy.y_center;

    int16_t  nx  = joy_norm_axis(dx, JOY_X_SPAN_NEG, JOY_X_SPAN_POS);
    int16_t  ny  = joy_norm_axis(dy, JOY_Y_SPAN_NEG, JOY_Y_SPAN_POS);
    uint16_t mag = joy_magnitude(nx, ny);

    if (mag <= JOY_DEAD_MOUSE) {
        reset_mouse_accum();
        return;
    }

    /* デッドゾーン境界 = 速度 1、倒し切り = mouse_speed_max (超えない) */
    uint16_t max_eff = JOY_NORM_FULL - JOY_DEAD_MOUSE;   /* デッドゾーン上限 90% でも > 0 */
    uint16_t eff     = mag - JOY_DEAD_MOUSE;
    if (eff > max_eff) eff = max_eff;

    int32_t speed_q8 = (int32_t)eff * joy.mouse_speed_max * 256 / max_eff;
    if (speed_q8 < 256) speed_q8 = 256;

    /* 方向は nx/mag, ny/mag。Y は反転 (上 = カーソル上)。
     * speed_q8 は基準時間あたりの量なので、実際の経過時間に比例させる。
     * (最大でも 2048 x 12800 x 20 ≒ 5.2e8 で int32_t に収まる)
     */
    if (dt_ms > JOY_MOUSE_DT_MAX_MS) dt_ms = JOY_MOUSE_DT_MAX_MS;
    int32_t den   = (int32_t)mag * JOY_MOUSE_SPEED_BASE_MS;
    int32_t vx_q8 = (int32_t)nx * speed_q8 * dt_ms / den;
    int32_t vy_q8 = (int32_t)(-ny) * speed_q8 * dt_ms / den;

    int8_t mx = joy_take_mouse_count(&joy.mouse_acc_x, vx_q8);
    int8_t my = joy_take_mouse_count(&joy.mouse_acc_y, vy_q8);

    if (mx == 0 && my == 0) return;

    report_mouse_t report = {0};
    report.x       = mx;
    report.y       = my;
    report.buttons = current_mouse_buttons();
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
     * 軸レンジは対称なので反転で範囲外にはならないが、念のためクランプする。
     */
#if JOY_ANALOG_INVERT_Y
    ay = (int16_t)JOY_CLAMP(-(int32_t)ay, JOY_JS_AXIS_MIN, JOY_JS_AXIS_MAX);
#endif

    /* 円形デッドゾーン + 引き伸ばし (HID 軸スケール)
     *   - 倒した距離で判定するため、浅い斜めの入力も片軸が 0 に張り付かない
     *   - デッドゾーンを超えた分を 0〜フルスケールへ引き伸ばすため、
     *     出力は 0 から連続的に始まる (段差が出ない)
     * 演算順は 16bit 解像度・デッドゾーン 90% でも int32_t に収まるようにしている。
     */
    uint16_t mag = joy_magnitude(ax, ay);
    if (mag <= JOY_DEAD_ANALOG) {
        ax = 0;
        ay = 0;
    } else {
        int32_t num = (int32_t)mag - JOY_DEAD_ANALOG;
        int32_t den = (int32_t)JOY_JS_AXIS_MAX - JOY_DEAD_ANALOG;
        int32_t sx  = (int32_t)ax * num / (int32_t)mag * JOY_JS_AXIS_MAX / den;
        int32_t sy  = (int32_t)ay * num / (int32_t)mag * JOY_JS_AXIS_MAX / den;
        ax = (int16_t)JOY_CLAMP(sx, JOY_JS_AXIS_MIN, JOY_JS_AXIS_MAX);
        ay = (int16_t)JOY_CLAMP(sy, JOY_JS_AXIS_MIN, JOY_JS_AXIS_MAX);
    }

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

    joy_ee_load();
}

void joy_task(void) {
    joy_ee_flush();

    uint16_t interval = (joy.mode == JOY_MODE_MOUSE) ? JOY_MOUSE_POLL_INTERVAL_MS
                                                     : JOY_POLL_INTERVAL_MS;
    uint16_t elapsed  = timer_elapsed(joy.last_poll);
    if (elapsed < interval) return;
    joy.last_poll = timer_read();

    uint16_t x = analogReadPin(JOY_X_PIN);
    uint16_t y = analogReadPin(JOY_Y_PIN);

    switch (joy.mode) {
        case JOY_MODE_DIGITAL: process_digital(x, y); break;
        case JOY_MODE_MOUSE:   process_mouse(x, y, elapsed); break;
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
    joy_ee_mark_dirty();   /* 入力モードも約 2 秒後に EEPROM へ保存 */
}

uint8_t joy_get_mouse_speed(void) {
    return joy.mouse_speed_max;
}

void joy_mouse_speed_up(void) {
    uint8_t cur  = joy.mouse_speed_max;
    int16_t next = (int16_t)(cur / JOY_MOUSE_SPEED_STEP + 1) * JOY_MOUSE_SPEED_STEP;
    joy.mouse_speed_max = (uint8_t)JOY_CLAMP(next, JOY_MOUSE_SPEED_MIN, JOY_MOUSE_SPEED_MAX);
    joy_ee_mark_dirty();
}

void joy_mouse_speed_down(void) {
    uint8_t cur  = joy.mouse_speed_max;
    int16_t next = (cur % JOY_MOUSE_SPEED_STEP)
                       ? (int16_t)(cur / JOY_MOUSE_SPEED_STEP) * JOY_MOUSE_SPEED_STEP
                       : (int16_t)cur - JOY_MOUSE_SPEED_STEP;
    joy.mouse_speed_max = (uint8_t)JOY_CLAMP(next, JOY_MOUSE_SPEED_MIN, JOY_MOUSE_SPEED_MAX);
    joy_ee_mark_dirty();
}

/* ============================================================
 * デッドゾーン (現在のモードに作用)
 * ============================================================ */

/* 現在のモードのデッドゾーン割合への参照と可動範囲を取得 */
static uint8_t *joy_dz_target(uint8_t *lo, uint8_t *hi) {
    switch (joy.mode) {
        case JOY_MODE_MOUSE:
            *lo = JOY_DZ_MOUSE_MIN;   *hi = JOY_DZ_MOUSE_MAX;
            return &joy.dz_mouse;
        case JOY_MODE_ANALOG:
            *lo = JOY_DZ_ANALOG_MIN;  *hi = JOY_DZ_ANALOG_MAX;
            return &joy.dz_analog;
        case JOY_MODE_DIGITAL:
        default:
            *lo = JOY_DZ_DIGITAL_MIN; *hi = JOY_DZ_DIGITAL_MAX;
            return &joy.dz_digital;
    }
}

uint8_t joy_get_deadzone(void) {
    uint8_t lo, hi;
    return *joy_dz_target(&lo, &hi);
}

static void joy_dz_adjust(int8_t delta) {
    uint8_t  lo, hi;
    uint8_t *p    = joy_dz_target(&lo, &hi);
    int16_t  next = (int16_t)*p + delta;

    *p = (uint8_t)JOY_CLAMP(next, lo, hi);

    /* しきい値が変わったので次回ポーリングで必ず再評価・再送させる */
    release_last_dir();
    joy.last_ax = JOY_JS_AXIS_MIN;
    joy.last_ay = JOY_JS_AXIS_MIN;

    joy_ee_mark_dirty();
}

void joy_deadzone_up(void) {
    joy_dz_adjust(JOY_DZ_STEP);
}

void joy_deadzone_down(void) {
    joy_dz_adjust(-JOY_DZ_STEP);
}

/* 現在のモードのデッドゾーンだけを既定値に戻す (JOY_DZ_UP / DN と同じく現在のモードに作用) */
void joy_deadzone_reset(void) {
    switch (joy.mode) {
        case JOY_MODE_MOUSE:  joy.dz_mouse   = JOY_DZ_MOUSE_DEFAULT;   break;
        case JOY_MODE_ANALOG: joy.dz_analog  = JOY_DZ_ANALOG_DEFAULT;  break;
        case JOY_MODE_DIGITAL:
        default:              joy.dz_digital = JOY_DZ_DIGITAL_DEFAULT; break;
    }
    joy_dz_adjust(0);
}

/* ============================================================
 * 設定値のテキスト出力 (CFG_SHOW キー用)
 *
 * snprintf を使うとフラッシュを 1KB 以上消費するため、10 進変換は自前で行う。
 * 出力に使う記号は「US / JIS 配列で同じ位置にあるもの」(空白 , . / - %) のみ。
 * ':' '=' '(' などは JIS 配列のホストで別の文字に化けるため使わない。
 * ============================================================ */

static void joy_send_dec(uint16_t v) {
    char    buf[6];
    uint8_t i = sizeof(buf) - 1;
    buf[i]    = '\0';
    do {
        buf[--i] = (char)('0' + v % 10);
        v /= 10;
    } while (v != 0 && i != 0);
    send_string(buf + i);
}

/* デッドゾーン 1 行分。現在のモードの行には "<- current" を付ける */
static void joy_send_dz_line(const char *label, uint8_t value, bool current) {
    send_string_P(label);
    joy_send_dec(value);
    send_string_P(current ? PSTR("%   <- current\n") : PSTR("%\n"));
}

/* 出力形式 (項目名は 16 桁で揃える、行頭字下げなし、最終行も改行):
 *   GBC24 joystick
 *   mode            mouse
 *   mouse speed     15 / 50
 *   deadzone digi   54%
 *   deadzone mouse  24%   <- current
 *   deadzone analog 12%
 *   eeprom          saved
 */
void joy_send_status(void) {
    send_string_P(PSTR("GBC24 joystick\n"));

    send_string_P(PSTR("mode            "));
    switch (joy.mode) {
        case JOY_MODE_DIGITAL: send_string_P(PSTR("digi\n"));    break;
        case JOY_MODE_MOUSE:   send_string_P(PSTR("mouse\n"));   break;
        case JOY_MODE_ANALOG:  send_string_P(PSTR("analog\n"));  break;
        default:               send_string_P(PSTR("unknown\n")); break;
    }

    send_string_P(PSTR("mouse speed     "));
    joy_send_dec(joy.mouse_speed_max);
    send_string_P(PSTR(" / "));
    joy_send_dec(JOY_MOUSE_SPEED_MAX);
    send_string_P(PSTR("\n"));

    joy_send_dz_line(PSTR("deadzone digi   "), joy.dz_digital, joy.mode == JOY_MODE_DIGITAL);
    joy_send_dz_line(PSTR("deadzone mouse  "), joy.dz_mouse,   joy.mode == JOY_MODE_MOUSE);
    joy_send_dz_line(PSTR("deadzone analog "), joy.dz_analog,  joy.mode == JOY_MODE_ANALOG);

    /* 変更から EEPROM 書き込みまで 2 秒の遅延があるため、その間は auto saving */
    send_string_P(joy.ee_dirty ? PSTR("eeprom          auto saving\n")
                               : PSTR("eeprom          saved\n"));
}

#endif /* JOYSTICK_ENABLE */
