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

#include "yuiop50d.h"

#ifdef RGB_MATRIX_ENABLE
led_config_t g_led_config = {
    // Key Matrix to LED Index.
    {
        {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,    NO_LED },
        { 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10 },
        { 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
        { 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,    NO_LED },
        { 42, 43, 44, 45, 46, 47, 48, 49,            NO_LED, NO_LED, NO_LED }
    },
    // LED Index to Physical Position
    {
            { 11, 0}, { 33, 0}, { 55, 0}, { 77, 0}, { 99, 0}, {121, 0}, {143, 0}, {165, 0}, {187, 0}, {209, 0},
        {220,16}, {198,16}, {176,16}, {154,16}, {132,16}, {110,16}, { 88,16}, { 66,16}, { 44,16}, { 22,16}, {  0,16},
          {  4,32}, { 26,32}, { 48,32}, { 70,32}, { 92,32}, {114,32}, {136,32}, {158,32}, {180,32}, {202,32}, {224,32},
              {214,48}, {192,48}, {170,48}, {148,48}, {126,48}, {104,48}, { 82,48}, { 60,48}, { 38,48}, { 16,48},
        {  4,64},           { 44,64}, { 66,64}, { 88,64},     {121,64}, {143,64}, {165,64},                   {224,64}
    },
    // LED Index to Flag
    {
         4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
         4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
          4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
         4,   1, 1, 1,  1, 1, 1,       4
    }
};
#endif // RGB_MATRIX_ENABLE
