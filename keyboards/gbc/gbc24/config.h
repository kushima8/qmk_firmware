/*
Copyright 2022 kushima8

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

//#include "config_common.h"

/* USB Device descriptor parameter */
//#define VENDOR_ID    0x3938
//#define PRODUCT_ID   0x3635
//#define DEVICE_VER   0x0001
//#define MANUFACTURER kushima8
//#define PRODUCT      HOOK64

/* key matrix size */
#define MATRIX_ROWS 5
#define MATRIX_COLS (4 * 2)  // duplex matrix

// VIA config
#define DYNAMIC_KEYMAP_LAYER_COUNT 4

/*
 * Keyboard Matrix Assignments
 *
 * Change this to how you wired your keyboard
 * COLS: AVR pins used for columns, left to right
 * ROWS: AVR pins used for rows, top to bottom
 * DIODE_DIRECTION: COL2ROW = COL = Anode (+), ROW = Cathode (-, marked on diode)
 *                  ROW2COL = ROW = Anode (+), COL = Cathode (-, marked on diode)
 *
 */
#define MATRIX_ROW_PINS { D4, C6, D7, E6, B4 }
#define MATRIX_COL_PINS { B5, F4, F5, B6 }
//#define UNUSED_PINS

/* COL2ROW, ROW2COL */
//#define DIODE_DIRECTION COL2ROW
//#define DIODE_DIRECTION CUSTOM_MATRIX
/*
 * Split Keyboard specific options, make sure you have 'SPLIT_KEYBOARD = yes' in your rules.mk, and define SOFT_SERIAL_PIN.
 */
//#define SOFT_SERIAL_PIN D2  // or D1, D2, D3, E6

/* Rotary encoder define*/
#define ENCODERS_PAD_A { D3 }
#define ENCODERS_PAD_B { D2 }
#define ENCODER_RESOLUTION 6

#define DIP_SWITCH_PINS { B2 }

/* ---- アナログジョイスティック構成 (POINTING_INPUT = joystick) ---- */
#ifdef JOYSTICK_ENABLE
#    define JOY_X_PIN F6
#    define JOY_Y_PIN F7
#    define JOYSTICK_BUTTON_COUNT 1
#    define JOYSTICK_AXIS_COUNT 2
#    define JOYSTICK_AXIS_RESOLUTION 10
#endif

/* ---- PMW3360 トラックボール構成 (POINTING_INPUT = trackball) ----
 * ジョイスティック接続部を交換してトラックボールモジュールを装着する。
 * SPI はハードウェア SPI を使用:
 *   SCK  = B1
 *   MOSI = B2
 *   MISO = B3
 *   NCS  = F7 (JOY_Y と同じピンを流用。ドライバのデフォルトと同一)
 */
#ifdef POINTING_DEVICE_ENABLE
#    define PMW3360_NCS_PINS { F7 }

/* ---- 同梱 pmw3360 ドライバを「無修正」で QMK 0.22.x でビルドするための互換対応 ----
 * (config.h は全ソースに -include されるため、ここに置けばドライバ側は変更不要)
 *
 * 1) pmw3360.h が size_t を使うが stddef.h を include していない
 *    → ここで強制 include (.S ファイルでは typedef が使えないため C のみ)
 * 2) pmw3360.c が新 GPIO API 名 gpio_set_pin_output() を使うが 0.22.x には無い
 *    → 旧 API setPinOutput() へのエイリアスを定義
 */
#    ifndef __ASSEMBLER__
#        include <stddef.h>
#    endif
#    define gpio_set_pin_output(pin) setPinOutput(pin)
#endif

//#define LED_NUM_LOCK_PIN B0
//#define LED_CAPS_LOCK_PIN B1
//#define LED_SCROLL_LOCK_PIN B2
//#define LED_COMPOSE_PIN B3
//#define LED_KANA_PIN B4

//#define BACKLIGHT_PIN B7
//#define BACKLIGHT_LEVELS 3
//#define BACKLIGHT_BREATHING

//#define WS2812_DI_PIN D3
//#ifdef RGBLIGHT_ENABLE
//#    define RGBLED_NUM 1
//#    define RGBLIGHT_HUE_STEP 8
//#    define RGBLIGHT_SAT_STEP 8
//#    define RGBLIGHT_VAL_STEP 8
//#    define RGBLIGHT_LIMIT_VAL 255 /* The maximum brightness level */
//#    define RGBLIGHT_SLEEP  /* If defined, the RGB lighting will be switched off when the host goes to sleep */
/*== all animations enable ==*/
//#    define RGBLIGHT_ANIMATIONS
/*== or choose animations ==*/
//#    define RGBLIGHT_EFFECT_BREATHING
//#    define RGBLIGHT_EFFECT_RAINBOW_MOOD
//#    define RGBLIGHT_EFFECT_RAINBOW_SWIRL
//#    define RGBLIGHT_EFFECT_SNAKE
//#    define RGBLIGHT_EFFECT_KNIGHT
//#    define RGBLIGHT_EFFECT_CHRISTMAS
//#    define RGBLIGHT_EFFECT_STATIC_GRADIENT
//#    define RGBLIGHT_EFFECT_RGB_TEST
//#    define RGBLIGHT_EFFECT_ALTERNATING
/*== customize breathing effect ==*/
/*==== (DEFAULT) use fixed table instead of exp() and sin() ====*/
//#    define RGBLIGHT_BREATHE_TABLE_SIZE 256      // 256(default) or 128 or 64
/*==== use exp() and sin() ====*/
//#    define RGBLIGHT_EFFECT_BREATHE_CENTER 1.85  // 1 to 2.7
//#    define RGBLIGHT_EFFECT_BREATHE_MAX    255   // 0 to 255
//#endif

/* Debounce reduces chatter (unintended double-presses) - set 0 if debouncing is not needed */
#define DEBOUNCE 5

/* define if matrix has ghost (lacks anti-ghosting diodes) */
//#define MATRIX_HAS_GHOST

/* Mechanical locking support. Use KC_LCAP, KC_LNUM or KC_LSCR instead in keymap */
#define LOCKING_SUPPORT_ENABLE
/* Locking resynchronize hack */
#define LOCKING_RESYNC_ENABLE

/* If defined, GRAVE_ESC will always act as ESC when CTRL is held.
 * This is useful for the Windows task manager shortcut (ctrl+shift+esc).
 */
//#define GRAVE_ESC_CTRL_OVERRIDE

/*
 * Force NKRO
 *
 * Force NKRO (nKey Rollover) to be enabled by default, regardless of the saved
 * state in the bootmagic EEPROM settings. (Note that NKRO must be enabled in the
 * makefile for this to work.)
 *
 * If forced on, NKRO can be disabled via magic key (default = LShift+RShift+N)
 * until the next keyboard reset.
 *
 * NKRO may prevent your keystrokes from being detected in the BIOS, but it is
 * fully operational during normal computer usage.
 *
 * For a less heavy-handed approach, enable NKRO via magic key (LShift+RShift+N)
 * or via bootmagic (hold SPACE+N while plugging in the keyboard). Once set by
 * bootmagic, NKRO mode will always be enabled until it is toggled again during a
 * power-up.
 *
 */
//#define FORCE_NKRO

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT

/* disable these deprecated features by default */
//#define NO_ACTION_MACRO
//#define NO_ACTION_FUNCTION

/* Bootmagic Lite key configuration */
//#define BOOTMAGIC_LITE_ROW 0
//#define BOOTMAGIC_LITE_COLUMN 0

#if !defined(LAYER_STATE_8BIT) && !defined(LAYER_STATE_16BIT) && !defined(LAYER_STATE_32BIT)
#    define LAYER_STATE_8BIT
#endif

//#ifdef RGBLIGHT_ENABLE
//#    define RGBLIGHT_EFFECT_BREATHING
//#    define RGBLIGHT_EFFECT_RAINBOW_MOOD
//#    define RGBLIGHT_EFFECT_RAINBOW_SWIRL
//#    define RGBLIGHT_MODE_SNAKE
//#    define RGBLIGHT_MODE_KNIGHT
//#    define RGBLIGHT_MODE_CHRISTMAS
//#    define RGBLIGHT_MODE_STATIC_GRADIENT
//#    define RGBLIGHT_EFFECT_RGB_TEST
//#    define RGBLIGHT_MODE_ALTERNATING
//#    define RGBLIGHT_MODE_TWINKLE
//#endif

#define TAP_CODE_DELAY 5

