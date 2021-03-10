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
#define PRODUCT_ID      0x0004
#define DEVICE_VER      0x0001
#define MANUFACTURER    koron

#define PRODUCT         YUIOP22RS Rev.2
#define DESCRIPTION     Experimental Keyboard

// Key matrix size and pins
#define MATRIX_ROWS 6
#define MATRIX_COLS 5
#define MATRIX_ROW_PINS { B1, F7, F6, F5, F4, B6 }
#define MATRIX_COL_PINS { D4, C6, D7, E6, B4 }
#define DIODE_DIRECTION COL2ROW

// For input stability
#define DEBOUNCE 5

// List of row and col pairs
#define DIP_SWITCH_MATRIX_GRID { {5,0}, {5,1}, {5,2}, {5,3} }
#define MATRIX_MASKED

// Not implemented yet.
//// Splitting
//#define USE_SERIAL
//#define SOFT_SERIAL_PIN D2
//#define SPLIT_USB_DETECT
