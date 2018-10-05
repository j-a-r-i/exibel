/*
Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2018 Jari Ojanen

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

#include "ch.h"
#include "hal.h"

/*
 * scan matrix
 */
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"
#include "wait.h"

#ifndef DEBOUNCE
#   define DEBOUNCE 4
#endif
static uint8_t debouncing = DEBOUNCE;

/* matrix state(1:on, 0:off) */
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static matrix_row_t read_cols(void);
static void init_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000


inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

/* generic STM32F103C8T6 board */
#ifdef BOARD_GENERIC_STM32_F103
#define LED_ON()    do { palClearPad(GPIOC, GPIOC_LED) ;} while (0)
#define LED_OFF()   do { palSetPad(GPIOC, GPIOC_LED); } while (0)
#define LED_TGL()   do { palTogglePad(GPIOC, GPIOC_LED); } while (0)
#endif

void matrix_init(void)
{
    // initialize row and col
    unselect_rows();
    init_cols();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        matrix[i] = 0;
        matrix_debouncing[i] = 0;
    }

    //debug
    debug_enable = false;
    debug_matrix = false;
    LED_ON();
    wait_ms(500);
    LED_OFF();
}

uint8_t matrix_scan(void)
{
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        select_row(i);
        wait_us(30);  // without this wait read unstable value.
        matrix_row_t cols = read_cols();
        if (matrix_debouncing[i] != cols) {
            matrix_debouncing[i] = cols;
            if (debouncing) {
                debug("bounce!: "); debug_hex(debouncing); debug("\n");
            }
            debouncing = DEBOUNCE;
        }
        unselect_rows();
    }

    if (debouncing) {
        if (--debouncing) {
            wait_ms(1);
        } else {
            for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
                matrix[i] = matrix_debouncing[i];
            }
        }
    }

    return 1;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row); print(": ");
        pbin_reverse16(matrix_get_row(row));
        print("\n");
    }
}

/* Column pin configuration
 */
static void  init_cols(void)
{
    palSetGroupMode(GPIOB,
		    BIT0 |
		    BIT1 |
		    BIT3 |
		    BIT4 |
		    BIT5 |
		    BIT6 |
		    BIT7 |
		    BIT8 |
		    BIT9 |
		    BIT10 |
		    BIT11 |
		    BIT12 |
		    BIT13 |
		    BIT14 |
		    BIT15, 0, PAL_MODE_INPUT_PULLUP);
    palSetGroupMode(GPIOA,
                    BIT7 |
		    BIT8 |
		    BIT15, 0, PAL_MODE_INPUT_PULLUP);
}

/* Returns status of switches(1:on, 0:off) */
static matrix_row_t read_cols(void)
{
    uint32_t porta = palReadPort(GPIOA);
    uint32_t portb = palReadPort(GPIOB);
    
    return (((portb & BIT0 ) == BIT0 ) ? 0 : (1<<0)) |
	   (((portb & BIT1 ) == BIT1 ) ? 0 : (1<<1)) |
	   (((portb & BIT3 ) == BIT3 ) ? 0 : (1<<2)) |
           (((portb & BIT4 ) == BIT4 ) ? 0 : (1<<3)) |
	   (((portb & BIT5 ) == BIT5 ) ? 0 : (1<<4)) |
	   (((portb & BIT6 ) == BIT6 ) ? 0 : (1<<5)) |
	   (((portb & BIT7 ) == BIT7 ) ? 0 : (1<<6)) |
	   (((portb & BIT8 ) == BIT8 ) ? 0 : (1<<7)) |
	   (((portb & BIT9 ) == BIT9 ) ? 0 : (1<<8)) |
	   (((portb & BIT10) == BIT10) ? 0 : (1<<9)) |
	   (((portb & BIT11) == BIT11) ? 0 : (1<<10)) |
	   (((portb & BIT12) == BIT12) ? 0 : (1<<11)) |
	   (((portb & BIT13) == BIT13) ? 0 : (1<<12)) |
	   (((portb & BIT14) == BIT14) ? 0 : (1<<13)) |
	   (((portb & BIT15) == BIT15) ? 0 : (1<<14)) |
	   (((porta & BIT7 ) == BIT7 ) ? 0 : (1UL<<15)) |
	   (((porta & BIT8 ) == BIT8 ) ? 0 : (1UL<<16)) |
	   (((porta & BIT15) == BIT15) ? 0 : (1UL<<17));
}

/* Row pin configuration
 */
static void unselect_rows()
{
    palSetGroupMode(GPIOA,
		    BIT0 |
		    BIT1 |
		    BIT2 |
		    BIT3 |
		    BIT4 |
		    BIT5 |
		    BIT6, 0, PAL_MODE_INPUT);
}

static void select_row(uint8_t row)
{
    if (row < MATRIX_ROWS) {
	palSetPadMode(GPIOA, row, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(  GPIOA, row);
    }
}
