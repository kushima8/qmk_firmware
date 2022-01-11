/*
Copyright 2021 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

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

#include "config_common.h"

// USB Device descriptor parameters
#define VENDOR_ID 0x5957        // "YW" = Yowkees
#define PRODUCT_ID 0x0100       // dummy ID
#define DEVICE_VER 0x0001
#define MANUFACTURER Yowkees
#define PRODUCT Keyball61

// Key matrix parameters (Keyball61 is duplex matrix)
#define MATRIX_ROWS         (5 * 2)     // split keyboard
#define MATRIX_COLS         (4 * 2)     // duplex matrix
#define MATRIX_ROW_PINS     { D4, C6, D7, E6, B4 }
#define MATRIX_COL_PINS     { F4, F5, F6, F7 }
#define MATRIX_MASKED
#define DEBOUNCE            5

// Split parameters
#define SOFT_SERIAL_PIN         D2
#define SPLIT_HAND_MATRIX_GRID  F7, D7
#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT       500

// RGB LED settings
#define RGB_DI_PIN D3
