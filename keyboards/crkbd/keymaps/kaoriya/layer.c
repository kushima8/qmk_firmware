#include QMK_KEYBOARD_H

#include "print.h"

layer_state_t layer_state_set_user(layer_state_t state) {
    if ((state & 0b0000110) == 0b0000110) {
        state |= 0b0001000;
    } else {
        state &= ~0b0001000;
    }
    if ((state & 0b0110000) == 0b0110000) {
        state |= 0b1000000;
    } else {
        state &= ~0b1000000;
    }
#ifdef CONSOLE_ENABLE
    uprintf("layer: %04X\n", state);
#endif
    return state;
}

#ifdef OLED_DRIVER_ENABLE

void oled_render_layer_state(void) {
    oled_write_P(PSTR("Layer: "), false);
    if ((layer_state & 0b0001000) != 0) {
        oled_write_ln_P(PSTR("Functions"), false);
    } else if ((layer_state & 0b0000110) != 0) {
        oled_write_ln_P(PSTR("Numbers"), false);
    } else if ((layer_state & 0b1000000) != 0) {
        oled_write_ln_P(PSTR("Systems"), false);
    } else if ((layer_state & 0b0110000) != 0) {
        oled_write_ln_P(PSTR("Shifts"), false);
    } else {
        oled_write_ln_P(PSTR("Default"), false);
    }
}

#endif // OLED_DRIVER_ENABLE
