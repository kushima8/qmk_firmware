/*
Copyright 2021 MURAOKA Taro (aka KoRoN, @kaoriya)

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

// USB Device descriptor parameter
#define VENDOR_ID       0x7890
#define PRODUCT_ID      0x0005
#define DEVICE_VER      0x0001
#define MANUFACTURER    koron

#define PRODUCT         YUIOP63HHS Rev.1
#define DESCRIPTION     Experimental Keyboard

// Key matrix size and pins
#define MATRIX_ROWS 10
#define MATRIX_COLS 8
#define MATRIX_ROW_PINS { C6, D7, E6, B4, B5 }
#define MATRIX_COL_PINS { F4, F5, F6, F7, B1, B3, B2, B6 }
#define DIODE_DIRECTION COL2ROW

// For input stability
#define DEBOUNCE 5

// Splitting
#define USE_SERIAL
#define SOFT_SERIAL_PIN D2
#define SPLIT_USB_DETECT
#define SPLIT_HAND_MATRIX_GRID B5, B6
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_RIGHT
