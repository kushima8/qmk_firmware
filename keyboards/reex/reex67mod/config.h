// Copyright 2023 kushima8 (@kushima8)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// USB Device descriptor parameters
//#define VENDOR_ID           0x3938
//#define PRODUCT_ID          0x5236
//#define DEVICE_VER          0x0001
//#define MANUFACTURER        kushima8
//#define PRODUCT             Reex67mod

/* key matrix size */
#define MATRIX_ROWS         (6 * 2)  // split keyboard
#define MATRIX_COLS         (4 * 2)  // duplex matrix
#define MATRIX_ROW_PINS     { D4, C6, D7, E6, B4, F6 }
#define MATRIX_COL_PINS     { B5, F4, F5, B6 }
#define MATRIX_MASKED
#define DEBOUNCE            5

#define ENCODERS_PAD_A { B2 }
#define ENCODERS_PAD_B { B3 }
#define ENCODER_RESOLUTION 4
//#define ENCODERS_PAD_A_RIGHT { B3 }
//#define ENCODERS_PAD_B_RIGHT { B2 }
//#define ENCODER_RESOLUTIONS_RIGHT { 4 }
#define ENCODER_MAP_KEY_DELAY 10

#define DIP_SWITCH_PINS { B1 }
//#define DIP_SWITCH_PINS_RIGHT { F7 }

#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 3
#define AUTO_MOUSE_TIME 650
//#define AUTO_MOUSE_DELAY 200
//#define AUTO_MOUSE_DEBOUNCE 25

#define DYNAMIC_KEYMAP_LAYER_COUNT 4

// VIA config
#define VIA_CUSTOM_LIGHTING_ENABLE
#define VIA_RGBLIGHT_USER_ADDR (EECONFIG_SIZE)
#define VIA_EEPROM_MAGIC_ADDR (VIA_RGBLIGHT_USER_ADDR + DYNAMIC_KEYMAP_LAYER_COUNT * 4)  // Layer * 4bytes(RGB Light config)

// Split parameters
#define SOFT_SERIAL_PIN         D2
#define SPLIT_HAND_MATRIX_GRID  B4, B6
#define SPLIT_USB_DETECT
//#define SPLIT_USB_TIMEOUT       500
#ifdef OLED_ENABLE
#    define SPLIT_OLED_ENABLE
#endif

// If your PC does not recognize Reex, try setting this macro. This macro
// increases the firmware size by 200 bytes, so it is disabled by default, but
// it has been reported to work well in such cases.
//#define SPLIT_WATCHDOG_ENABLE

#define SPLIT_TRANSACTION_IDS_KB REEX_GET_INFO, REEX_GET_MOTION, REEX_SET_CPI

// RGB LED settings
#define WS2812_DI_PIN       D3
#ifdef RGBLIGHT_ENABLE
#    define RGBLED_NUM      75
#    define RGBLED_SPLIT    { 33, 42 }
#    ifndef RGBLIGHT_LIMIT_VAL
#        define RGBLIGHT_LIMIT_VAL  100 // limitated for power consumption
#    endif
#    ifndef RGBLIGHT_VAL_STEP
#        define RGBLIGHT_VAL_STEP   10
#    endif
#    ifndef RGBLIGHT_HUE_STEP
#        define RGBLIGHT_HUE_STEP   17
#    endif
#    ifndef RGBLIGHT_SAT_STEP
#        define RGBLIGHT_SAT_STEP   17
#    endif
#endif
#ifdef RGB_MATRIX_ENABLE
#    define RGB_MATRIX_SPLIT    { 33, 42 }
#endif

#ifndef OLED_FONT_H
#    define OLED_FONT_H "keyboards/reex/lib/logofont/logofont.c"
#    define OLED_FONT_START 32
#    define OLED_FONT_END 195
#endif

#if !defined(LAYER_STATE_8BIT) && !defined(LAYER_STATE_16BIT) && !defined(LAYER_STATE_32BIT)
#    define LAYER_STATE_8BIT
#endif

// To squeeze firmware size
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

#ifdef RGBLIGHT_ENABLE
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
#endif

#define TAP_CODE_DELAY 5

