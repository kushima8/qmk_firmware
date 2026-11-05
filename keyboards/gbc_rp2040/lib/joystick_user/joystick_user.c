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
#include "eeprom.h"          /* eeprom_read_block / eeprom_update_block (キャリブレーション保存) */
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

/* --- 可動域 (カバー等で機械的に制限された端の ADC 絶対値) ---
 * config.h で JOY_X_MIN / JOY_X_MAX / JOY_Y_MIN / JOY_Y_MAX (カバーに当たって
 * 止まる位置の ADC 生値) を定義する。未定義なら 0 / 1023 (理論フルスケール)。
 *
 * 中心 (静止位置) は起動時と長押しリセット時に毎回自動取得する。実測では
 * 端は ±3 カウントで再現する一方、静止位置はバネ戻りのヒステリシスで
 * 挿し直すたびに 20 カウント以上動くため、保存するのは端だけにし、
 * 「中心から端までの距離 (span)」は中心が決まるたびに計算し直す。
 * span には JOY_CAL_SPAN_MARGIN_PCT (97%) を掛け、端の少し手前で
 * フルスケールに達するようにする。
 */
#ifndef JOY_X_MIN
#    define JOY_X_MIN 0
#endif
#ifndef JOY_X_MAX
#    define JOY_X_MAX JOY_ADC_MAX
#endif
#ifndef JOY_Y_MIN
#    define JOY_Y_MIN 0
#endif
#ifndef JOY_Y_MAX
#    define JOY_Y_MAX JOY_ADC_MAX
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

/* --- ポーリング間隔 (デジタルモード) --- */
#define JOY_POLL_INTERVAL_MS      10

/* マウスモードだけは短い間隔で読み取り・送信し、カーソルを滑らかに動かす。
 * 移動量は実際に経過した時間に比例させるため、間隔を変えても
 * 1 秒あたりの移動量 (マウス最大速度の体感) は変わらない。
 *   マウス最大速度 = 基準時間 JOY_MOUSE_SPEED_BASE_MS あたりの最大移動カウント
 * 読み取りが遅れた場合 (EEPROM 書き込み中など) に一度に大きく飛ばないよう、
 * 1 回に扱う経過時間は JOY_MOUSE_DT_MAX_MS で頭打ちにする。
 */
#define JOY_MOUSE_POLL_INTERVAL_MS 4    /* 250Hz */

/* アナログモード (ゲーム向け) も入力遅延を抑えるため 4ms ごとに更新する。
 * 倒し具合をそのまま送るだけなので、間隔を変えても出力値は変わらない。
 * (値が変わったときだけ QMK 本体のメインループが HID レポートを送る)
 */
#define JOY_ANALOG_POLL_INTERVAL_MS 4   /* 250Hz */
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

    /* 可動域の端 (ADC 絶対値)。既定は config.h の JOY_*_MIN/MAX、
     * キャリブレーション保存後は実測値。 */
    uint16_t   x_min, x_max;
    uint16_t   y_min, y_max;
    /* 中心から端までの距離 (97% マージン込み)。中心が決まるたびに
     * joy_update_spans() で端から計算する。 */
    uint16_t   span_xn;
    uint16_t   span_xp;
    uint16_t   span_yn;
    uint16_t   span_yp;

    /* キャリブレーション (JOY_CALIB キー) */
    uint8_t    cal_phase;      /* JOY_CAL_PHASE_* */
    uint8_t    cal_result;     /* JOY_CAL_RESULT_* (CFG_SHOW 表示用) */
    bool       cal_saved;      /* EEPROM の実測値を使用中 */
    bool       cal_span_saved; /* 端も実測値 (false なら中心の参照値のみ保存、端は既定値) */
    uint16_t   cal_done_at;    /* 直前の確定/失敗/リセット時刻 (OLED 表示用) */
    bool       cal_show;       /* OLED に結果を表示中 (表示期間が過ぎたら false)。
                                * timer_elapsed() は 16bit で約 65 秒ごとに巻き戻るため、
                                * 時刻比較だけで判定すると 65 秒おきに結果が再表示される */
    bool       cal_moved;      /* 可動域フェーズ中にスティックが動いた */
    bool       cal_key_held;   /* JOY_CALIB 押下中 (長押し判定用) */
    bool       cal_key_long;   /* 長押しリセット済み (離したときの短押し動作を抑止) */
    uint16_t   cal_at;         /* フェーズ開始時刻 */
    uint16_t   cal_key_at;     /* JOY_CALIB 押下時刻 (長押し判定用) */
    uint16_t   cal_n;          /* 中心サンプル数 */
    int32_t    cal_sum_x;
    int32_t    cal_sum_y;
    uint16_t   cal_min_x, cal_max_x;
    uint16_t   cal_min_y, cal_max_y;
    uint16_t   cal_prev_cx, cal_prev_cy; /* 開始前の中心 (失敗時に戻す) */
} joy = {
    .mode            = JOY_MODE_DIGITAL,
    .x_center        = JOY_ADC_CENTER_DEFAULT,
    .y_center        = JOY_ADC_CENTER_DEFAULT,
    .x_min           = JOY_X_MIN,
    .x_max           = JOY_X_MAX,
    .y_min           = JOY_Y_MIN,
    .y_max           = JOY_Y_MAX,
    /* span_* は joy_init() の joy_update_spans() で計算 */
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

/* ============================================================
 * キャリブレーション値の永続化 (EEPROM 末尾の専用領域)
 *
 * eeconfig_user は上の 4 バイトで満杯、eeconfig_kb はトラックボール構成が
 * 使用、EECONFIG_USER_DATA_SIZE / VIA_EEPROM_CUSTOM_CONFIG_SIZE は
 * VIA/Vial の領域をずらして既存ユーザーのキーマップを壊す。
 * そのため config.h で DYNAMIC_KEYMAP_EEPROM_MAX_ADDR を末尾から
 * JOY_CAL_EEPROM_SIZE だけ手前に下げ (マクロバッファが同量だけ縮む)、
 * 空いた末尾に書く。既存のキーマップ・マクロの先頭アドレスは変わらない。
 *
 * レイアウト (14 バイト):
 *   [0]    magic (JOY_CAL_EE_MAGIC)
 *   [1]    flags (bit0: 端も実測値。0 なら中心の参照値のみで端は既定値)
 *   [2,3]  中心 X (参照値)   [4,5]  中心 Y (参照値)
 *   [6,7]  X 端 min  [8,9]  X 端 max  [10,11] Y 端 min  [12,13] Y 端 max
 * 中心の参照値は起動時に自動取得した中心の妥当性判定にだけ使う
 * (挿すときにスティックに触れていた場合は参照値へフォールバック)。
 * ============================================================ */

#ifndef JOY_CAL_EEPROM_ADDR
#    error "JOY_CAL_EEPROM_ADDR must be defined in config.h"
#endif

#define JOY_CAL_EE_MAGIC        0xC7  /* 0xC5/0xC6 = span 保存の旧形式 (未リリース) */
#define JOY_CAL_EE_FLAG_SPAN    0x01

#define JOY_CAL_PHASE_IDLE      0
#define JOY_CAL_PHASE_CENTER    1   /* 触らない状態で中心をサンプリング中 */
#define JOY_CAL_PHASE_RANGE     2   /* 回してもらって最小/最大を記録中 */

#define JOY_CAL_RESULT_NONE     0   /* 起動後まだ実行していない */
#define JOY_CAL_RESULT_OK       1   /* 直前の実行で保存した */
#define JOY_CAL_RESULT_CENTER   2   /* 中心だけ保存 (可動域フェーズで動かさなかった) */
#define JOY_CAL_RESULT_FAILED   3   /* 可動域が小さすぎて保存しなかった / 時間切れ */
#define JOY_CAL_RESULT_RESET    4   /* 長押しで既定値に戻した */

#define JOY_CAL_CENTER_MS       500   /* 中心フェーズの長さ */
#define JOY_CAL_RANGE_TIMEOUT_MS 30000 /* 可動域フェーズを放置したときの中断 */
#define JOY_CAL_MOVE_THRESHOLD  48    /* これ以上中心から離れたら「動かした」(ADC カウント) */
#define JOY_CAL_SPAN_MIN_PCT    50    /* 実測可動域が config.h の既定端から求めた可動域の 50% 未満なら失敗 */
#define JOY_CAL_CENTER_TOL_PCT  50    /* 中心が参照位置から「既定可動域の最小値 x 50%」以上ずれていたら異常 (触れていた) */
#define JOY_CAL_SPAN_MARGIN_PCT 97    /* 中心→端の距離の 97% を span にする (端の少し手前でフルスケール) */
#define JOY_CAL_LONG_PRESS_MS   2000  /* 長押しリセットのしきい値 */
#define JOY_CAL_RESULT_SHOW_MS  5000  /* OLED に直前の結果を出しておく時間 */

typedef struct {
    uint8_t  magic;
    uint8_t  flags;
    uint16_t cx, cy;
    uint16_t xmin, xmax, ymin, ymax;
} __attribute__((packed)) joy_cal_ee_t;

/* 既定の端 (config.h) から、ADC 中央を中心と仮定したときの 4 方向の可動域の最小値。
 * 妥当性判定の基準に使う。 */
static uint16_t joy_cal_min_default_span(void) {
    uint16_t m = JOY_ADC_CENTER_DEFAULT - JOY_X_MIN;
    if (JOY_X_MAX - JOY_ADC_CENTER_DEFAULT < m) m = JOY_X_MAX - JOY_ADC_CENTER_DEFAULT;
    if (JOY_ADC_CENTER_DEFAULT - JOY_Y_MIN < m) m = JOY_ADC_CENTER_DEFAULT - JOY_Y_MIN;
    if (JOY_Y_MAX - JOY_ADC_CENTER_DEFAULT < m) m = JOY_Y_MAX - JOY_ADC_CENTER_DEFAULT;
    return m;
}

/* 「中心→端」の距離が、既定端から求めた距離の JOY_CAL_SPAN_MIN_PCT % 以上あること。
 * 端まで倒していない / 別の個体の値 / 壊れた EEPROM を弾く。 */
static bool joy_cal_span_ok(int16_t measured, int16_t dflt) {
    return measured >= (int16_t)((int32_t)dflt * JOY_CAL_SPAN_MIN_PCT / 100) && measured <= JOY_ADC_MAX;
}

static bool joy_cal_extremes_ok(uint16_t cx, uint16_t cy, uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax) {
    if (xmin >= cx || xmax <= cx || ymin >= cy || ymax <= cy) return false;
    return joy_cal_span_ok((int16_t)(cx - xmin), JOY_ADC_CENTER_DEFAULT - JOY_X_MIN) &&
           joy_cal_span_ok((int16_t)(xmax - cx), JOY_X_MAX - JOY_ADC_CENTER_DEFAULT) &&
           joy_cal_span_ok((int16_t)(cy - ymin), JOY_ADC_CENTER_DEFAULT - JOY_Y_MIN) &&
           joy_cal_span_ok((int16_t)(ymax - cy), JOY_Y_MAX - JOY_ADC_CENTER_DEFAULT);
}

/* 現在の中心と端から span を計算する。中心が端の外側にあるなど不正なら、
 * その方向は 1 以上に丸めて 0 除算を避ける。 */
static uint16_t joy_span_from(int16_t d) {
    if (d < 2) return 1;
    return (uint16_t)((uint32_t)d * JOY_CAL_SPAN_MARGIN_PCT / 100);
}
static void joy_update_spans(void) {
    joy.span_xn = joy_span_from((int16_t)(joy.x_center - joy.x_min));
    joy.span_xp = joy_span_from((int16_t)(joy.x_max - joy.x_center));
    joy.span_yn = joy_span_from((int16_t)(joy.y_center - joy.y_min));
    joy.span_yp = joy_span_from((int16_t)(joy.y_max - joy.y_center));
}

/* 中心が参照位置から大きくずれていないか (触れていなかったか) */
static bool joy_center_plausible(uint16_t cx, uint16_t cy, uint16_t ref_x, uint16_t ref_y) {
    uint16_t tol = joy_cal_min_default_span() * JOY_CAL_CENTER_TOL_PCT / 100;
    return abs((int16_t)cx - (int16_t)ref_x) <= tol && abs((int16_t)cy - (int16_t)ref_y) <= tol;
}

/* EEPROM の実測値を読み込む。有効なら joy の端 (と参照中心) を上書きして true。
 * 中心そのものは呼び出し側が自動取得する。 */
static uint16_t joy_cal_ref_cx, joy_cal_ref_cy;   /* 保存されていた参照中心 */
static bool joy_cal_load(void) {
    joy_cal_ee_t c;
    eeprom_read_block(&c, (const void *)JOY_CAL_EEPROM_ADDR, sizeof(c));
    if (c.magic != JOY_CAL_EE_MAGIC) return false;
    if (c.cx > JOY_ADC_MAX || c.cy > JOY_ADC_MAX) return false;
    bool span = (c.flags & JOY_CAL_EE_FLAG_SPAN) != 0;
    if (span && !joy_cal_extremes_ok(c.cx, c.cy, c.xmin, c.xmax, c.ymin, c.ymax)) return false;
    joy_cal_ref_cx = c.cx;
    joy_cal_ref_cy = c.cy;
    if (span) {
        joy.x_min = c.xmin;
        joy.x_max = c.xmax;
        joy.y_min = c.ymin;
        joy.y_max = c.ymax;
    }
    joy.cal_span_saved = span;
    return true;
}

static void joy_cal_save(void) {
    joy_cal_ee_t c = {
        .magic = JOY_CAL_EE_MAGIC,
        .flags = joy.cal_span_saved ? JOY_CAL_EE_FLAG_SPAN : 0,
        .cx = joy.x_center, .cy = joy.y_center,
        .xmin = joy.x_min, .xmax = joy.x_max,
        .ymin = joy.y_min, .ymax = joy.y_max,
    };
    eeprom_update_block(&c, (void *)JOY_CAL_EEPROM_ADDR, sizeof(c));
    joy_cal_ref_cx = joy.x_center;
    joy_cal_ref_cy = joy.y_center;
    joy.cal_saved  = true;
}

static void joy_cal_erase(void) {
    eeprom_update_byte((uint8_t *)JOY_CAL_EEPROM_ADDR, 0xFF);
    joy.cal_saved      = false;
    joy.cal_span_saved = false;
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
 * span は ADC 生値の差分なので、analogReadPin() が返す解像度 (QMK は AVR /
 * RP2040 とも 10bit に揃えて返す) と同じ単位で config.h に書くこと。
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
    int16_t nx = joy_norm_axis(dx, joy.span_xn, joy.span_xp);
    int16_t ny = joy_norm_axis(dy, joy.span_yn, joy.span_yp);

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

    int16_t  nx  = joy_norm_axis(dx, joy.span_xn, joy.span_xp);
    int16_t  ny  = joy_norm_axis(dy, joy.span_yn, joy.span_yp);
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
 * ADC 差分を scale_axis() で可動域基準の HID 軸値 (±JOY_JS_AXIS_MAX) に変換する。
 * ============================================================ */

static void process_analog(uint16_t x, uint16_t y) {
    release_last_dir();

    int32_t dx = (int32_t)x - (int32_t)joy.x_center;
    int32_t dy = (int32_t)y - (int32_t)joy.y_center;

    /* 反転は「変換後」に行う。変換前に dy の符号を反転させると、
     * scale_axis() 内の span_neg / span_pos の選択が生 ADC の方向と
     * 食い違ってしまうため。
     */
    int16_t ax = scale_axis(dx, joy.span_xn, joy.span_xp);
    int16_t ay = scale_axis(dy, joy.span_yn, joy.span_yp);

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

/* 起動時 (と長押しリセット時) の中心自動取得: 複数回サンプリングして平均を
 * センター値とする。約 32ms ブロックする。 */
static void joy_auto_center(void) {
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

void joy_init(void) {
    joy_ee_load();

    /* 端: EEPROM に実測値があればそれ、無ければ config.h の既定値。
     * 中心: 毎回起動時に自動取得 (静止位置は挿し直すたびに動くため)。
     * ただし保存済みの参照中心から大きくずれていれば「挿すときに触れていた」
     * とみなし、参照中心にフォールバックする。 */
    joy.cal_saved = joy_cal_load();
    joy_auto_center();
    if (joy.cal_saved && !joy_center_plausible(joy.x_center, joy.y_center, joy_cal_ref_cx, joy_cal_ref_cy)) {
        joy.x_center = joy_cal_ref_cx;
        joy.y_center = joy_cal_ref_cy;
    }
    joy_update_spans();
}

/* ============================================================
 * キャリブレーション状態機械 (joy_task から 10ms ごとに呼ばれる)
 *
 *   IDLE --JOY_CALIB 短押し--> CENTER (500ms、触らない)
 *        --自動--> RANGE (回してもらう)
 *        --JOY_CALIB 短押し--> 確定 (動かしていなければ中心のみ保存) → IDLE
 *        --30 秒放置--> 失敗 → IDLE
 *   いつでも JOY_CALIB 2 秒長押し → EEPROM 消去・既定値へ → IDLE
 * ============================================================ */

static void joy_cal_finish(uint8_t result) {
    joy.cal_phase   = JOY_CAL_PHASE_IDLE;
    joy.cal_result  = result;
    joy.cal_done_at = timer_read();
    joy.cal_show    = true;
    /* 中心/可動域が変わったので次回ポーリングで必ず再評価させる */
    joy.last_dir = DIR_CENTER;
    joy.last_ax  = JOY_JS_AXIS_MIN;
    joy.last_ay  = JOY_JS_AXIS_MIN;
    reset_mouse_accum();
}

static void joy_cal_poll(uint16_t x, uint16_t y) {
    switch (joy.cal_phase) {
        case JOY_CAL_PHASE_CENTER:
            joy.cal_sum_x += x;
            joy.cal_sum_y += y;
            joy.cal_n++;
            if (timer_elapsed(joy.cal_at) < JOY_CAL_CENTER_MS) return;
            joy.x_center = (uint16_t)(joy.cal_sum_x / joy.cal_n);
            joy.y_center = (uint16_t)(joy.cal_sum_y / joy.cal_n);
            /* 中心取得中にスティックに触れていた場合を弾く: 開始前の中心からの
             * ずれが既定可動域の最小値の JOY_CAL_CENTER_TOL_PCT % を超えていたら失敗。
             * (静止位置のばらつきは 20〜30 カウント程度なので誤検出しない) */
            if (!joy_center_plausible(joy.x_center, joy.y_center, joy.cal_prev_cx, joy.cal_prev_cy)) {
                joy.x_center = joy.cal_prev_cx;
                joy.y_center = joy.cal_prev_cy;
                joy_update_spans();
                joy_cal_finish(JOY_CAL_RESULT_FAILED);
                return;
            }
            joy.cal_min_x = joy.cal_max_x = joy.x_center;
            joy.cal_min_y = joy.cal_max_y = joy.y_center;
            joy.cal_moved = false;
            joy.cal_phase = JOY_CAL_PHASE_RANGE;
            joy.cal_at    = timer_read();
            return;

        case JOY_CAL_PHASE_RANGE:
            if (timer_elapsed(joy.cal_at) >= JOY_CAL_RANGE_TIMEOUT_MS) {
                joy.x_center = joy.cal_prev_cx;   /* 中断: 中心も開始前に戻す */
                joy.y_center = joy.cal_prev_cy;
                joy_update_spans();
                joy_cal_finish(JOY_CAL_RESULT_FAILED);
                return;
            }
            if (x < joy.cal_min_x) joy.cal_min_x = x;
            if (x > joy.cal_max_x) joy.cal_max_x = x;
            if (y < joy.cal_min_y) joy.cal_min_y = y;
            if (y > joy.cal_max_y) joy.cal_max_y = y;
            if (!joy.cal_moved &&
                (abs((int16_t)x - (int16_t)joy.x_center) > JOY_CAL_MOVE_THRESHOLD ||
                 abs((int16_t)y - (int16_t)joy.y_center) > JOY_CAL_MOVE_THRESHOLD)) {
                joy.cal_moved = true;
            }
            return;

        default:
            return;
    }
}

/* JOY_CALIB 短押し: 開始 / 確定 */
static void joy_cal_toggle(void) {
    switch (joy.cal_phase) {
        case JOY_CAL_PHASE_IDLE:
            cleanup_current_mode();
            joy.cal_prev_cx = joy.x_center;
            joy.cal_prev_cy = joy.y_center;
            joy.cal_sum_x = 0;
            joy.cal_sum_y = 0;
            joy.cal_n     = 0;
            joy.cal_phase = JOY_CAL_PHASE_CENTER;
            joy.cal_at    = timer_read();
            break;

        case JOY_CAL_PHASE_RANGE: {
            if (!joy.cal_moved) {
                /* 中心だけ取り直したいケース: 端はそのまま、中心を参照値として保存 */
                joy_update_spans();
                joy_cal_save();
                joy_cal_finish(JOY_CAL_RESULT_CENTER);
                break;
            }
            if (!joy_cal_extremes_ok(joy.x_center, joy.y_center,
                                     joy.cal_min_x, joy.cal_max_x, joy.cal_min_y, joy.cal_max_y)) {
                /* 4 方向のどれかが既定の半分に満たない (端まで回していない等)。
                 * 何も保存せず、中心も開始前の値に戻す。 */
                joy.x_center = joy.cal_prev_cx;
                joy.y_center = joy.cal_prev_cy;
                joy_update_spans();
                joy_cal_finish(JOY_CAL_RESULT_FAILED);
                break;
            }
            joy.x_min = joy.cal_min_x;
            joy.x_max = joy.cal_max_x;
            joy.y_min = joy.cal_min_y;
            joy.y_max = joy.cal_max_y;
            joy.cal_span_saved = true;
            joy_update_spans();
            joy_cal_save();
            joy_cal_finish(JOY_CAL_RESULT_OK);
            break;
        }

        default: /* CENTER フェーズ中の押下は無視 */
            break;
    }
}

/* JOY_CALIB 2 秒長押し: 既定値に戻す */
static void joy_cal_reset(void) {
    joy_cal_erase();
    joy.x_min = JOY_X_MIN;
    joy.x_max = JOY_X_MAX;
    joy.y_min = JOY_Y_MIN;
    joy.y_max = JOY_Y_MAX;
    cleanup_current_mode();
    /* 中心もその場で取り直し、リセット直後を「挿し直した直後」と同じ状態にする */
    joy_auto_center();
    joy_update_spans();
    joy_cal_finish(JOY_CAL_RESULT_RESET);
}

void joy_calib_key(bool pressed) {
    if (pressed) {
        joy.cal_key_held = true;
        joy.cal_key_long = false;
        joy.cal_key_at   = timer_read();
        return;
    }
    joy.cal_key_held = false;
    if (joy.cal_key_long) return;        /* 長押しリセット済み: 短押し動作はしない */
    joy_cal_toggle();
}

bool joy_cal_is_running(void) {
    return joy.cal_phase != JOY_CAL_PHASE_IDLE;
}

joy_cal_status_t joy_cal_status(void) {
    if (joy.cal_phase == JOY_CAL_PHASE_CENTER) return JOY_CAL_STATUS_CENTER;
    if (joy.cal_phase == JOY_CAL_PHASE_RANGE) return JOY_CAL_STATUS_RANGE;
    if (joy.cal_show && timer_elapsed(joy.cal_done_at) >= JOY_CAL_RESULT_SHOW_MS) {
        joy.cal_show = false;   /* 期限切れを記録し、タイマー巻き戻りで再表示されないようにする */
    }
    if (joy.cal_show) {
        switch (joy.cal_result) {
            case JOY_CAL_RESULT_OK:     return JOY_CAL_STATUS_OK;
            case JOY_CAL_RESULT_CENTER: return JOY_CAL_STATUS_CENTER_ONLY;
            case JOY_CAL_RESULT_FAILED: return JOY_CAL_STATUS_FAILED;
            case JOY_CAL_RESULT_RESET:  return JOY_CAL_STATUS_RESET;
        }
    }
    return JOY_CAL_STATUS_IDLE;
}

void joy_task(void) {
    joy_ee_flush();

    /* 長押しリセット判定 (どのフェーズでも有効) */
    if (joy.cal_key_held && !joy.cal_key_long &&
        timer_elapsed(joy.cal_key_at) >= JOY_CAL_LONG_PRESS_MS) {
        joy.cal_key_long = true;
        joy_cal_reset();
    }

    uint16_t interval = (joy.cal_phase != JOY_CAL_PHASE_IDLE) ? JOY_POLL_INTERVAL_MS
                      : (joy.mode == JOY_MODE_MOUSE)  ? JOY_MOUSE_POLL_INTERVAL_MS
                      : (joy.mode == JOY_MODE_ANALOG) ? JOY_ANALOG_POLL_INTERVAL_MS
                                                      : JOY_POLL_INTERVAL_MS;
    uint16_t elapsed  = timer_elapsed(joy.last_poll);
    if (elapsed < interval) return;
    joy.last_poll = timer_read();

    uint16_t x = analogReadPin(JOY_X_PIN);
    uint16_t y = analogReadPin(JOY_Y_PIN);

    /* キャリブレーション中は通常の入力処理を止める (回している間に
     * 方向キーやマウス移動が出ないように)。 */
    if (joy.cal_phase != JOY_CAL_PHASE_IDLE) {
        joy_cal_poll(x, y);
        return;
    }

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
/* 4 値をスペース区切りで出力 (range / span 行用。コードサイズ節約のため共通化) */
static void joy_send_dec4(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
    joy_send_dec(a);
    send_string_P(PSTR(" "));
    joy_send_dec(b);
    send_string_P(PSTR(" "));
    joy_send_dec(c);
    send_string_P(PSTR(" "));
    joy_send_dec(d);
}

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

    /* キャリブレーション: 中心 (x y)、端 (xmin xmax ymin ymax)、span (x- x+ y- y+)、状態 */
    send_string_P(PSTR("center          "));
    joy_send_dec(joy.x_center);
    send_string_P(PSTR(" "));
    joy_send_dec(joy.y_center);
    send_string_P(PSTR("\nrange           "));
    joy_send_dec4(joy.x_min, joy.x_max, joy.y_min, joy.y_max);
    send_string_P(PSTR("\nspan            "));
    joy_send_dec4(joy.span_xn, joy.span_xp, joy.span_yn, joy.span_yp);
    send_string_P(PSTR("\ncalib           "));
    if (joy.cal_phase != JOY_CAL_PHASE_IDLE) {
        send_string_P(PSTR("running\n"));
    } else {
        send_string_P(!joy.cal_saved ? PSTR("default") : joy.cal_span_saved ? PSTR("saved") : PSTR("saved center only"));
        switch (joy.cal_result) {
            case JOY_CAL_RESULT_OK:     send_string_P(PSTR(", last ok\n"));          break;
            case JOY_CAL_RESULT_CENTER: send_string_P(PSTR(", last center only\n")); break;
            case JOY_CAL_RESULT_FAILED: send_string_P(PSTR(", last failed\n"));      break;
            case JOY_CAL_RESULT_RESET:  send_string_P(PSTR(", last reset\n"));       break;
            default:                    send_string_P(PSTR("\n"));                   break;
        }
    }

    /* 変更から EEPROM 書き込みまで 2 秒の遅延があるため、その間は auto saving */
    send_string_P(joy.ee_dirty ? PSTR("eeprom          auto saving\n")
                               : PSTR("eeprom          saved\n"));
}

#endif /* JOYSTICK_ENABLE */
