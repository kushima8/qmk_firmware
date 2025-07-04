// Copyright 2023 kushima8 (@kushima8)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// USB Device descriptor parameters
//#define VENDOR_ID           0x3938
//#define PRODUCT_ID          0x5235
//#define DEVICE_VER          0x0001
//#define MANUFACTURER        kushima8
//#define PRODUCT             Reex56mod

/* key matrix size */
#define MATRIX_ROWS         (6 * 2)  // split keyboard
#define MATRIX_COLS         (4 * 2)  // duplex matrix
#define MATRIX_ROW_PINS     { GP4, GP5, GP6, GP7, GP8, GP12 }
#define MATRIX_COL_PINS     { GP9, GP29, GP28, GP10 }
#define MATRIX_MASKED
#define DEBOUNCE            5

#define ENCODERS_PAD_A { GP19 }
#define ENCODERS_PAD_B { GP20 }
#define ENCODER_RESOLUTION 4
//#define ENCODERS_PAD_A_RIGHT { GP20 }
//#define ENCODERS_PAD_B_RIGHT { GP19 }
//#define ENCODER_RESOLUTIONS_RIGHT { 4 }
#define ENCODER_MAP_KEY_DELAY 10

#define DIP_SWITCH_PINS { GP18 }
//#define DIP_SWITCH_PINS_RIGHT { GP26 }

#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 3
#define AUTO_MOUSE_TIME 650
//#define AUTO_MOUSE_DELAY 200
//#define AUTO_MOUSE_DEBOUNCE 25

#define DYNAMIC_KEYMAP_LAYER_COUNT 4

// Split parameters
#define SERIAL_USART_TX_PIN GP1
#define SPLIT_HAND_MATRIX_GRID GP8, GP10
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_LEFT
#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT 4500
#define SPLIT_USB_TIMEOUT_POLL 25
#ifdef OLED_ENABLE
#    define SPLIT_OLED_ENABLE
#endif

// If your PC does not recognize Reex, try setting this macro. This macro
// increases the firmware size by 200 bytes, so it is disabled by default, but
// it has been reported to work well in such cases.
#define SPLIT_WATCHDOG_ENABLE

#define SPLIT_TRANSACTION_IDS_KB REEX_GET_INFO, REEX_GET_MOTION, REEX_SET_CPI, REEX_GET_EX_MOTION

// RGB LED settings
#define WS2812_DI_PIN GP0
#define WS2812_PIO_USE_PIO1
#ifdef RGBLIGHT_ENABLE
#    define RGBLED_NUM    64
#    define RGBLED_SPLIT    { 32, 32 }
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
#    define RGB_MATRIX_SPLIT    { 32, 32 }
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
#    define RGBLIGHT_EFFECT_BREATHING
#    define RGBLIGHT_EFFECT_RAINBOW_MOOD
#    define RGBLIGHT_EFFECT_RAINBOW_SWIRL
#    define RGBLIGHT_MODE_SNAKE
#    define RGBLIGHT_MODE_KNIGHT
#    define RGBLIGHT_MODE_CHRISTMAS
#    define RGBLIGHT_MODE_STATIC_GRADIENT
#    define RGBLIGHT_EFFECT_RGB_TEST
#    define RGBLIGHT_MODE_ALTERNATING
#    define RGBLIGHT_MODE_TWINKLE
#endif

#define TAP_CODE_DELAY 5

#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP18
#define SPI_MISO_PIN GP20
#define SPI_MOSI_PIN GP19
#define PMW3360_NCS_PINS {GP26,GP27}

//#define REEX_PMW3360_UPLOAD_SROM_ID 0x04
#define REEX_PMW3360_UPLOAD_SROM_ID 0x81

/* define RP2040 boot	 */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET // Activates the double-tap behavior
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U // Timeout window in ms in which the double tap can occur.

//#define SELECT_SOFT_SERIAL_SPEED 3
#define SERIAL_USART_SPEED 86400
