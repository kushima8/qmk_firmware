// Copyright 2023 kushima8 (@kushima8)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include "config_common.h"

// USB Device descriptor parameters
#define VENDOR_ID           0x3938
#define PRODUCT_ID          0x3034
#define DEVICE_VER          0x0001
#define MANUFACTURER        kushima8
#define PRODUCT             tks4

/* key matrix size */
#define MATRIX_ROWS         1
#define MATRIX_COLS         4
#define MATRIX_ROW_PINS     { D4 }
#define MATRIX_COL_PINS     { B5, F4, F5, B6 }
#define DEBOUNCE            5

#define DIODE_DIRECTION COL2ROW

// VIA config
#define VIA_CUSTOM_LIGHTING_ENABLE
#define VIA_RGBLIGHT_USER_ADDR (EECONFIG_SIZE)
#define VIA_EEPROM_MAGIC_ADDR (VIA_RGBLIGHT_USER_ADDR + DYNAMIC_KEYMAP_LAYER_COUNT * 4)  // Layer * 4bytes(RGB Light config)


#define SPLIT_TRANSACTION_IDS_KB REEX_GET_INFO, REEX_GET_MOTION, REEX_SET_CPI

// RGB LED settings
#define RGB_DI_PIN          D3
#ifdef RGBLIGHT_ENABLE
#    define RGBLED_NUM      4
#    ifndef RGBLIGHT_LIMIT_VAL
#        define RGBLIGHT_LIMIT_VAL  100 // limitated for power consumption
#    endif
#    ifndef RGBLIGHT_VAL_STEP
#        define RGBLIGHT_VAL_STEP   12
#    endif
#    ifndef RGBLIGHT_HUE_STEP
#        define RGBLIGHT_HUE_STEP   17
#    endif
#    ifndef RGBLIGHT_SAT_STEP
#        define RGBLIGHT_SAT_STEP   17
#    endif
#endif


#if !defined(LAYER_STATE_8BIT) && !defined(LAYER_STATE_16BIT) && !defined(LAYER_STATE_32BIT)
#    define LAYER_STATE_8BIT
#endif

// To squeeze firmware size
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

#ifdef RGBLIGHT_ENABLE
#    define RGBLIGHT_EFFECT_BREATHING
#    define RGBLIGHT_EFFECT_RGB_TEST
#endif

#define TAP_CODE_DELAY 5

