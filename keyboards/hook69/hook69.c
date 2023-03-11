/* Copyright 2022 kushima8
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

#include "hook69.h"
#include "eeprom.h"
#include "eeconfig.h"

#if defined(RGBLIGHT_ENABLE)
#define LIGHTING_GET_VAL rgblight_get_val
#define LIGHTING_GET_MODE rgblight_get_mode
#define LIGHTING_GET_SPEED rgblight_get_speed
#define LIGHTING_GET_HUE rgblight_get_hue
#define LIGHTING_GET_SAT rgblight_get_sat
#define LIGHTING_SETHSV_NOEEPROM(h, s, v) rgblight_sethsv_noeeprom(h, s, v)
#define LIGHTING_SETMODE_NOEEPROM(mode) rgblight_mode_noeeprom(mode)
#define LIGHTING_DISABLE_NOEEPROM(b) rgblight_disable_noeeprom(b)
#define LIGHTING_ENABLE_NOEEPROM(b) rgblight_enable_noeeprom(b)
#define LIGHTING_SETSPEED_NOEEPROM(speed) rgblight_set_speed_noeeprom(speed)
#define LIGHTING_UPDATE_EECONFIG() eeconfig_update_rgblight_current()
#define LIGHTING_CONFIG rgblight_config
extern rgblight_config_t rgblight_config;
#elif defined(RGB_MATRIX_ENABLE)
#define LIGHTING_GET_VAL rgb_matrix_get_val
#define LIGHTING_GET_MODE rgb_matrix_get_mode
#define LIGHTING_GET_SPEED rgb_matrix_get_speed
#define LIGHTING_GET_HUE rgb_matrix_get_hue
#define LIGHTING_GET_SAT rgb_matrix_get_sat
#define LIGHTING_SETHSV_NOEEPROM(h, s, v) rgb_matrix_sethsv_noeeprom(h, s, v)
#define LIGHTING_SETMODE_NOEEPROM(mode) rgb_matrix_mode_noeeprom(mode)
#define LIGHTING_DISABLE_NOEEPROM(b) rgb_matrix_disable_noeeprom(b)
#define LIGHTING_ENABLE_NOEEPROM(b) rgb_matrix_enable_noeeprom(b)
#define LIGHTING_SETSPEED_NOEEPROM(speed) rgb_matrix_set_speed_noeeprom(speed)
#define LIGHTING_UPDATE_EECONFIG() eeconfig_update_rgb_matrix()
#define LIGHTING_CONFIG rgb_matrix_config
#else
#define LIGHTING_GET_VAL(...) 0
#define LIGHTING_GET_MODE(...) 0
#define LIGHTING_GET_SPEED(...) 0
#define LIGHTING_GET_HUE(...) 0
#define LIGHTING_GET_SAT(...) 0
#define LIGHTING_SETHSV_NOEEPROM(h, s, v)
#define LIGHTING_SETMODE_NOEEPROM(mode)
#define LIGHTING_DISABLE_NOEEPROM(b)
#define LIGHTING_ENABLE_NOEEPROM(b)
#define LIGHTING_SETSPEED_NOEEPROM(b)
#define LIGHTING_UPDATE_EECONFIG()
#endif

static void via_custom_lighting_get_value(uint8_t *data) {
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);
    switch (*value_id) {
        case id_qmk_rgblight_brightness: {
            value_data[0] = LIGHTING_GET_VAL();
            break;
        }
        case id_qmk_rgblight_effect: {
            value_data[0] = LIGHTING_GET_MODE();
            break;
        }
        case id_qmk_rgblight_effect_speed: {
            value_data[0] = LIGHTING_GET_SPEED();
            break;
        }
        case id_qmk_rgblight_color: {
            value_data[0] = LIGHTING_GET_HUE();
            value_data[1] = LIGHTING_GET_SAT();
            break;
        }
    }
}

static void via_custom_lighting_set_value(uint8_t *data) {
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);
    switch (*value_id) {
        case id_qmk_rgblight_brightness: {
            LIGHTING_SETHSV_NOEEPROM(LIGHTING_GET_HUE(), LIGHTING_GET_SAT(), value_data[0]);
            break;
        }
        case id_qmk_rgblight_effect: {
            LIGHTING_SETMODE_NOEEPROM(value_data[0]);
            if (value_data[0] == 0) {
                LIGHTING_DISABLE_NOEEPROM();
            } else {
                LIGHTING_ENABLE_NOEEPROM();
            }
            break;
        }
        case id_qmk_rgblight_effect_speed: {
            LIGHTING_SETSPEED_NOEEPROM(value_data[0]);
            break;
        }
        case id_qmk_rgblight_color: {
            LIGHTING_SETHSV_NOEEPROM(value_data[0], value_data[1],
                                     LIGHTING_GET_VAL());
            break;
        }
    }
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    uint8_t *command_id = &(data[0]);
    uint8_t *value_data = &(data[1]);
    uint8_t  layer      = get_highest_layer(layer_state);

    switch (*command_id) {
        case id_lighting_set_value:
            via_custom_lighting_set_value(value_data);
            break;

        case id_lighting_get_value:
            via_custom_lighting_get_value(value_data);
            break;

        case id_lighting_save:
            // Save rgblight config per layer
            eeprom_update_dword((uint32_t *)(VIA_RGBLIGHT_USER_ADDR + 4 * layer), LIGHTING_CONFIG.raw);
            LIGHTING_UPDATE_EECONFIG();
            break;

        default:
            break;
    }
}
