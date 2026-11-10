/*
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)
Copyright 2023 kushima8 (@kushima8)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Configurations

#ifndef GBC_CPI_DEFAULT
#    define GBC_CPI_DEFAULT 500
#endif

#ifndef GBC_SCROLL_DIV_DEFAULT
#    define GBC_SCROLL_DIV_DEFAULT 4 // 4: 1/8 (1/2^(n-1))
#endif

#ifndef GBC_REPORTMOUSE_INTERVAL
#    define GBC_REPORTMOUSE_INTERVAL 8 // mouse report rate: 125Hz
#endif

#ifndef GBC_SCROLLBALL_INHIVITOR
#    define GBC_SCROLLBALL_INHIVITOR 50
#endif

/// To disable scroll snap feature, define 0 in your config.h
#ifndef GBC_SCROLLSNAP_ENABLE
#    define GBC_SCROLLSNAP_ENABLE 2
#endif

#ifndef GBC_SCROLLSNAP_RESET_TIMER
#    define GBC_SCROLLSNAP_RESET_TIMER 100
#endif

#ifndef GBC_SCROLLSNAP_TENSION_THRESHOLD
#    define GBC_SCROLLSNAP_TENSION_THRESHOLD 12
#endif

/// Specify SROM ID to be uploaded PMW3360DW (optical sensor).  It will be
/// enabled high CPI setting or so.  Valid valus are 0x04 or 0x81.  Define this
/// in your config.h to be enable.  Please note that using this option will
/// increase the firmware size by more than 4KB.
//#define GBC_PMW3360_UPLOAD_SROM_ID 0x04
//#define GBC_PMW3360_UPLOAD_SROM_ID 0x81

/// Defining this macro keeps two functions intact: keycode_config() and
/// mod_config() in keycode_config.c.
///
/// These functions customize the magic key code and are useless if the magic
/// key code is disabled.  Therefore, GBC automatically disables it.
/// However, there may be cases where you still need these functions even after
/// disabling the magic key code. In that case, define this macro.
//#define GBC_KEEP_MAGIC_FUNCTIONS

//////////////////////////////////////////////////////////////////////////////
// Constants

#define GBC_TX_GETINFO_INTERVAL 300
#define GBC_TX_GETINFO_MAXTRY 20
#define GBC_TX_GETMOTION_INTERVAL 4


//////////////////////////////////////////////////////////////////////////////
// Types

enum gbc_keycodes {
    REC_RST  = QK_KB_0, // GBC configuration: reset to default
    REC_SAVE = QK_KB_1, // GBC configuration: save to EEPROM

    CPI_I100 = QK_KB_2, // CPI +100 CPI
    CPI_D100 = QK_KB_3, // CPI -100 CPI
    CPI_I1K  = QK_KB_4, // CPI +1000 CPI
    CPI_D1K  = QK_KB_5, // CPI -1000 CPI

    // In scroll mode, motion from primary trackball is treated as scroll
    // wheel.
    SCRL_TO  = QK_KB_6, // Toggle scroll mode
    SCRL_MO  = QK_KB_7, // Momentary scroll mode
    SCRL_DVI = QK_KB_8, // Increment scroll divider
    SCRL_DVD = QK_KB_9, // Decrement scroll divider

    SSNP_VRT = QK_KB_13, // Set scroll snap mode as vertical
    SSNP_HOR = QK_KB_14, // Set scroll snap mode as horizontal
    SSNP_FRE = QK_KB_15, // Set scroll snap mode as disable (free scroll)

    // Auto mouse layer control keycodes.
    // Only works when POINTING_DEVICE_AUTO_MOUSE_ENABLE is defined.
    AML_TO   = QK_KB_10, // Toggle automatic mouse layer
    AML_I50  = QK_KB_11, // Increment automatic mouse layer timeout
    AML_D50  = QK_KB_12, // Decrement automatic mouse layer timeout

    // User customizable 32 keycodes.
    GBC_SAFE_RANGE = QK_USER_0,
};

/* EEPROM (eeconfig_kb, 4 バイト) のレイアウト。
 * 以前は uint8_t/uint16_t 混在のビットフィールドで、かつ #ifdef で
 * フィールドの有無が変わっていたため、ビルド設定やコンパイラで配置が
 * 変わり得た。全フィールドを常に確保し、バイト境界に揃えて固定する。
 *   [0] cpi   (0 = 既定値, 1..CPI_MAX)
 *   [1] bit0-2 sdiv / bit3 amle / bit4-5 ssnap / bit6-7 予約
 *   [2] amlto (0 = 既定値, それ以外 (amlto+1)*50 ms)
 *   [3] 予約
 * 旧レイアウトとは互換がないため、更新後にトラックボール設定は
 * 一度既定値に戻る (REC_SAVE で保存し直す)。 */
typedef union {
    uint32_t raw;
    struct {
        uint8_t cpi;
        uint8_t sdiv : 3;  // scroll divider
        uint8_t amle : 1;  // automatic mouse layer enabled
        uint8_t ssnap : 2; // scroll snap mode
        uint8_t _rsv1 : 2;
        uint8_t amlto;     // automatic mouse layer timeout
        uint8_t _rsv2;
    };
} gbc_config_t;
_Static_assert(sizeof(gbc_config_t) == 4, "gbc_config_t must fit eeconfig_kb (4 bytes)");

typedef struct {
    int16_t x;
    int16_t y;
} gbc_motion_t;

typedef uint8_t gbc_cpi_t;

typedef enum {
    GBC_SCROLLSNAP_MODE_VERTICAL   = 0,
    GBC_SCROLLSNAP_MODE_HORIZONTAL = 1,
    GBC_SCROLLSNAP_MODE_FREE       = 2,
} gbc_scrollsnap_mode_t;

typedef struct {
    bool have_ball;      // PMW3360 の初期化に成功した

    gbc_motion_t motion; // センサからの未処理の移動量 (レポート間隔ごとに消費)

    uint8_t cpi_value;

    bool     scroll_mode;     // 実際のスクロールモード (= scroll_toggle || scroll_mo_count > 0)
    bool     scroll_toggle;   // SCRL_TO / gbc_set_scroll_mode() で切り替える状態
    uint8_t  scroll_mo_count; // 押下中の SCRL_MO キーの数
    uint32_t scroll_mode_changed;
    uint8_t  scroll_div;

#if GBC_SCROLLSNAP_ENABLE == 1
    uint32_t scroll_snap_last;
    int8_t   scroll_snap_tension_h;
#elif GBC_SCROLLSNAP_ENABLE == 2
    gbc_scrollsnap_mode_t scrollsnap_mode;
#endif
} gbc_t;


//////////////////////////////////////////////////////////////////////////////
// Exported values (touch carefully)

extern gbc_t gbc;

//////////////////////////////////////////////////////////////////////////////
// Hook points


/// gbc_on_apply_motion_to_mouse_move applies trackball's motion m to r as
/// mouse movement.
/// You can change the default algorithm by override this function.
void gbc_on_apply_motion_to_mouse_move(gbc_motion_t *m, report_mouse_t *r, bool is_left);

/// gbc_on_apply_motion_to_mouse_scroll applies trackball's motion m to r
/// as mouse scroll.
/// You can change the default algorithm by override this function.
void gbc_on_apply_motion_to_mouse_scroll(gbc_motion_t *m, report_mouse_t *r, bool is_left);

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks (call these from the keyboard's own *_kb functions)

/// gbc_keyboard_post_init loads trackball settings from EEPROM.
/// Call it from keyboard_post_init_kb().
void gbc_keyboard_post_init(void);

/// gbc_process_record handles the trackball keycodes (REC_RST .. SSNP_FRE).
/// Call it from process_record_kb() after process_record_user(); it returns
/// false when the keycode was consumed.
bool gbc_process_record(uint16_t keycode, keyrecord_t *record);

//////////////////////////////////////////////////////////////////////////////
// Public API functions

/// gbc_send_status types current trackball settings to the host as text.
/// It is intended for boards without OLED (see CFG_SHOW keycode).
/// It prints one item per line (header "GBC24 trackball", cpi, scroll mode,
/// scroll div, scroll snap, auto mouse, aml timeout, eeprom).
/// Only characters that are identical on US and JIS layouts are used.
void gbc_send_status(void);

/// gbc_get_scroll_mode gets current scroll mode.
bool gbc_get_scroll_mode(void);

/// gbc_set_scroll_mode modify scroll mode.
void gbc_set_scroll_mode(bool mode);

/// gbc_get_scrollsnap_mode gets current scroll snap mode.
gbc_scrollsnap_mode_t gbc_get_scrollsnap_mode(void);

/// gbc_set_scrollsnap_mode change scroll snap mode.
void gbc_set_scrollsnap_mode(gbc_scrollsnap_mode_t mode);

/// gbc_get_scroll_div gets current scroll divider.
/// See also gbc_set_scroll_div for the scroll divider's detail.
uint8_t gbc_get_scroll_div(void);

/// gbc_set_scroll_div changes scroll divider.
///
/// The scroll divider is the number that divides the raw value when applying
/// trackball motion to scrolling.  The CPI value of the trackball is very
/// high, so if you apply it to scrolling as is, it will scroll too much.
/// In order to adjust the scroll amount to be appropriate, it is applied after
/// dividing it by a scroll divider.  The actual denominator is determined by
/// the following formula:
///
///   denominator = 2 ^ (div - 1)      (1 -> 1/1, 4 -> 1/8, 7 -> 1/64)
///
/// Valid values are between 1 and 7 (larger values are clamped to 7),
/// GBC_SCROLL_DIV_DEFAULT is used when 0 is specified.
void gbc_set_scroll_div(uint8_t div);

/// gbc_get_cpi gets current CPI of trackball in units of 100 CPI.
/// The actual CPI value is the returned value multiplied by 100:
///
///     CPI = v * 100                  (5 -> 500 CPI)
///
/// It never returns 0: GBC_CPI_DEFAULT / 100 is returned while the default
/// is in use.
uint8_t gbc_get_cpi(void);

/// gbc_set_cpi changes CPI of trackball.
/// Valid values are between 1 and 35, and the actual CPI value is the set
/// value multiplied by 100:
///
///     CPI = v * 100                  (1 -> 100 CPI, 35 -> 3500 CPI)
///
/// 0 selects GBC_CPI_DEFAULT.  Values above 35 are clamped to 35, which is
/// the limit of the bundled PMW3360 driver (pmw3360_MAXCPI + 1).
void gbc_set_cpi(uint8_t cpi);
