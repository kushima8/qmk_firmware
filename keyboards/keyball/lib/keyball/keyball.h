/*
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

//////////////////////////////////////////////////////////////////////////////

#ifndef KEYBALL_CPI_DEFAULT
#    define KEYBALL_CPI_DEFAULT 4  // 4: 500 CPI
#endif

#ifndef KEYBALL_SCROLL_DIV_DEFAULT
#    define KEYBALL_SCROLL_DIV_DEFAULT 4  // 4: 1/16 (1/2^n)
#endif

//////////////////////////////////////////////////////////////////////////////

#define KEYBALL_TX_GETINFO_INTERVAL 500
#define KEYBALL_TX_GETINFO_MAXTRY 10
#define KEYBALL_TX_GETMOTION_INTERVAL 5

//////////////////////////////////////////////////////////////////////////////

enum keyball_keycodes {
    KBC_RST = SAFE_RANGE,   // Keyball configuration: reset to default
    KBC_SAVE,               // Keyball configuration: save to EEPROM

    CPI_I100,               // CPI +100 CPI
    CPI_D100,               // CPI -100 CPI
    CPI_I1K,                // CPI +1000 CPI
    CPI_D1K,                // CPI -1000 CPI

    // In scroll mode, motion from primary trackball is treated as scroll
    // wheel.
    SCRL_TO,                // Toggle scroll mode
    SCRL_MO,                // Momentary scroll mode
    SCRL_DVI,               // Increment scroll divider
    SCRL_DVD,               // Decrement scroll divider

    KEYBALL_SAFE_RANGE,
};

typedef union {
    uint32_t raw;
    struct {
        uint8_t cpi : 7;
        uint8_t sdiv : 3;  // scroll divider
    };
} keyball_config_t;

typedef struct {
    uint16_t vid;
    uint16_t pid;
    uint8_t  ballcnt;  // count of balls: support only 0 or 1, for now
} keyball_info_t;

typedef uint8_t keyball_motion_id_t;

typedef struct {
    int16_t x;
    int16_t y;
} keyball_motion_t;

typedef uint8_t keyball_cpi_t;

typedef struct {
    bool this_have_ball;
    bool that_enable;
    bool that_have_ball;

    keyball_motion_t this_motion;
    keyball_motion_t that_motion;

    uint8_t cpi_value;
    bool    cpi_changed;

    bool    scroll_mode;
    uint8_t scroll_div;

    uint16_t       last_kc;
    keypos_t       last_pos;
    report_mouse_t last_mouse;
} keyball_t;

typedef enum {
    KEYBALL_ADJUST_PENDING   = 0,
    KEYBALL_ADJUST_PRIMARY   = 1,
    KEYBALL_ADJUST_SECONDARY = 2,
} keyball_adjust_t;

//////////////////////////////////////////////////////////////////////////////

extern keyball_t keyball;

//////////////////////////////////////////////////////////////////////////////

// keyball_oled_render_ballinfo renders ball information to OLED.
// It uses just 21 columns to show the info.
void keyball_oled_render_ballinfo(void);

// keyball_oled_render_keyinfo renders last processed key information to OLED.
// It shows column, row, key code, and key name (if available).
void keyball_oled_render_keyinfo(void);

// TODO: document
bool keyball_get_scroll_mode(void);

// TODO: document
void keyball_set_scroll_mode(bool mode);
