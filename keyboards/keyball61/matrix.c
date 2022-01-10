#include "quantum.h"
#include "matrix.h"
#include "debounce.h"

#define PINNUM_ROW  MATRIX_ROWS
#define PINNUM_COL  (MATRIX_COLS / 2)

static pin_t row_pins[PINNUM_ROW] = MATRIX_ROW_PINS;
static pin_t col_pins[PINNUM_COL] = MATRIX_COL_PINS;

void matrix_init_custom(void) {
    // nothing to do...
}

static void set_pins_input(pin_t* pins, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        setPinInputHigh(pins[i]);
    }
}

static void set_pins_output(pin_t* pins, uint8_t n) {
    for (uint8_t i = 0; i < n; i++) {
        setPinOutput(pins[i]);
        writePinHigh(pins[i]);
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool changed = false;

    // scan column to row
    set_pins_input(col_pins, PINNUM_COL);
    set_pins_output(row_pins, PINNUM_ROW);
    for (uint8_t row = 0; row < PINNUM_ROW; row++) {
        writePinLow(row_pins[row]);
        matrix_output_select_delay();
        matrix_row_t next = current_matrix[row];
        for (uint8_t col = 0; col < PINNUM_COL; col++) {
            bool on = !readPin(col_pins[col]);
            if (on) {
                next |= 1 << col; 
            } else {
                next &= ~(1 << col);
            }
        }
        writePinHigh(row_pins[row]);
        if (current_matrix[row] != next) {
            current_matrix[row] = next;
            changed = true;
        }
        matrix_output_unselect_delay(row, next != 0);
    }

    // scan row to column.
    set_pins_input(row_pins, PINNUM_ROW);
    set_pins_output(col_pins, PINNUM_COL);
    for (uint8_t col = 0; col < PINNUM_COL; col++) {
        bool pressed = false;
        writePinLow(col_pins[col]);
        matrix_output_select_delay();
        matrix_row_t shifter = ((matrix_row_t)1) << (col + PINNUM_COL);
        for (uint8_t row = 0; row < PINNUM_ROW; row++) {
            bool on = !readPin(row_pins[row]);
            matrix_row_t prev = current_matrix[row];
            if (on) {
                current_matrix[row] |= shifter;
                pressed = true;
            } else {
                current_matrix[row] &= ~shifter;
            }
            if (current_matrix[row] != prev) {
                changed = true;
            }
        }
        writePinHigh(col_pins[col]);
        matrix_output_unselect_delay(col, pressed);
    }
    return changed;
}
