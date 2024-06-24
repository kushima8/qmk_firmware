#pragma once
/* Copyright 2018-2019 eswai <@eswai>
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

void nicola_type(void);
void nicola_clear(void);

void nicola_mode(uint16_t, keyrecord_t *);

void nicola_on(void);
void nicola_off(void);
bool nicola_state(void);

bool process_nicola(uint16_t, keyrecord_t *);
void set_nicola(uint8_t);

typedef enum nicola_keycodes {
  NI_TOP = SAFE_RANGE,
  NI_M_TOP = NI_TOP,
  NI_1 = NI_TOP,
  NI_2,
  NI_3,
  NI_4,
  NI_5,
  NI_6,
  NI_7,
  NI_8,
  NI_9,
  NI_0,
  NI_MINS,
  NI_EQL,
  NI_BSLS,

  NI_Q,
  NI_W,
  NI_E,
  NI_R,
  NI_T,
  NI_Y,
  NI_U,
  NI_I,
  NI_O,
  NI_P,
  NI_LBRC,
  NI_RBRC,

  NI_A,
  NI_S,
  NI_D,
  NI_F,
  NI_G,
  NI_H,
  NI_J,
  NI_K,
  NI_L,
  NI_SCLN,
  NI_QUOT,

  NI_Z,
  NI_X,
  NI_C,
  NI_V,
  NI_B,
  NI_N,
  NI_M,
  NI_COMM,
  NI_DOT,
  NI_SLSH,

  NI_M_BOTTOM = NI_SLSH,

  NI_SHFTL,
  NI_SHFTR,

  NI_BOTTOM = NI_SHFTR,
} NIKEYS;

#define NI_SAFE_RANGE (NI_BOTTOM+1)


