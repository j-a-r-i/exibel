/*
Copyright 2018      Jari Ojanen

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
#include "keycode.h"
#include "action.h"
#include "action_macro.h"
#include "action_util.h"
#include "report.h"
#include "host.h"
#include "keymap.h"

/* Layer names:
 */
#define LAYER_BASE 0
#define LAYER_LGUI 1
#define LAYER_RALT 2


/* Maps physical keyboard layout to hardware matrix
 */
#define KMAP( \
    K01,   K02, K03, K04, K05, K06, K07, K08, K09, K0A, K0B, K0C, K0D, K0E, \
    K11,    K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, K1E, \
    K21,     K22, K23, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C, K2D,      \
    K31, K32, K33, K34, K35, K36, K37, K38, K39, K3A, K3B, K3C, K3D,      \
         K41, K42,                K43,                K44, K45  \
) {  \
{KC_##K12, KC_##K13, KC_##K14, KC_##K15, KC_##K16, KC_##K11, KC_##K25, KC_##K43, KC_##K37, KC_##K22, KC_##K23, KC_##K24, KC_##K21, KC_##K31, KC_##K33, KC_##K34, KC_##K35, KC_##K36 }, \
{KC_##K28, KC_##K0E, KC_##K2A, KC_##K07, KC_NO,    KC_##K01, KC_##K29, KC_##K19, KC_##K26, KC_NO,    KC_##K0C, KC_##K0D, KC_NO,    KC_##K18, KC_##K02, KC_##K03, KC_##K04, KC_##K06 }, \
{KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_##K17, KC_##K1B, KC_##K41, KC_NO,    KC_##K1A, KC_##K05, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO    }, \
{KC_##K0B, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_##K08, KC_NO,    KC_NO,    KC_##K39, KC_NO,    KC_##K09, KC_##K0A, KC_NO,    KC_##K1E, KC_NO,    KC_##K3D, KC_NO,    KC_NO,   }, \
{KC_NO,    KC_##K27, KC_##K2D, KC_NO,    KC_NO,    KC_##K2B, KC_NO,    KC_NO,    KC_##K42, KC_NO,    KC_NO,    KC_##K2C, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,   }, \
{KC_##K3A, KC_##K3B, KC_##K3C, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_##K38, KC_##K45, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,   }, \
{KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_##K32, KC_##K1C, KC_##K1D, KC_NO,    KC_NO,    KC_##K44, KC_NO,    KC_NO,    KC_NO,   }}
//1         2         3         4         5         6         7         8         9         10        11        12        13       14         15        16        17        18

/*
[LAYER_BASE] = 
      KMAP(ESC,     1,    2,    3,    4,    5,    6,    7,    8,    9,    0,    MINS, EQL,  BSPACE,
  	   TAB,      Q,    W,    E,    R,    T,    Y,    U,    I,    O,    P,    FN10, FN11, ENT,
	   LCTRL,     A,    S,    D,    F,    G,    H,    J,    K,    L,    SCLN, QUOT, BSLASH,
	   LSFT, NUBS, Z,    X,    C,    V,    B,    N,    M,    COMM, DOT,  SLASH, RSFT,
                 FN0,  LALT,                 FN1,                        FN2 , RCTRL),
*/
static const uint8_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[LAYER_BASE] = 
      KMAP(ESC,     1,    2,    3,    4,    5,    6,    7,    8,    9,    0,    MINS, FN14, BSPACE,
  	   TAB,      Q,    W,    E,    R,    T,    Y,    U,    I,    O,    P,    FN10, FN11, ENT,
	   CAPS,      A,    S,    D,    F,    G,    H,    J,    K,    L,    SCLN, QUOT, BSLASH,
	   LSFT, NUBS, Z,    X,    C,    V,    B,    N,    M,    COMM, DOT,  SLASH, RSFT,
                 FN0,  LALT,                SPACE,                        FN2 , RCTRL),

[LAYER_LGUI] =
      KMAP(TRNS,    F1,   F2,   F3,   F4,   F5,   F6,   F7,   F8,   F9,   F10,  F11,  F12,  DEL,
  	   TRNS,     TRNS, TRNS, FN5,  FN4,  FN3,  TRNS, PGUP, UP,   PGDN, HOME, TRNS, TRNS, TRNS,
  	   TRNS,      TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, LEFT, DOWN, RGHT, END,  TRNS, INS,
  	   TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
                 TRNS, TRNS,                 TRNS,                     TRNS, TRNS),

[LAYER_RALT] =
      KMAP(TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
	   TRNS,     TRNS, TRNS, FN10, FN11, FN8,  TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
	   TRNS,      FN6,  FN7,  FN9,  FN14, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
	   TRNS, TRNS, TRNS, TRNS, FN12, FN13, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
                 TRNS, TRNS,                 TRNS,                     TRNS, TRNS)
};

/** Action map
 */
static const action_t fn_actions[] = {
    [ 0] = ACTION_LAYER_TAP_KEY(1, KC_LGUI),
    [ 1] = ACTION_LAYER_TAP_KEY(1, KC_SPACE),
    [ 2] = ACTION_LAYER_MOMENTARY(2), // layer switch for AltGr
    [ 3] = ACTION_MODS_KEY(MOD_LALT | MOD_LCTL, KC_DEL), // Ctrl + Alt + Del
    [ 4] = ACTION_MODS_KEY(MOD_LGUI, KC_R),
    [ 5] = ACTION_MODS_KEY(MOD_LGUI, KC_L),
    [ 6] = ACTION_MODS_KEY(MOD_RALT, KC_2),   // @
    [ 7] = ACTION_MODS_KEY(MOD_RALT, KC_4),   // $
    [ 8] = ACTION_MODS_KEY(MOD_RALT, KC_RBRACKET),  // ~
    [ 9] = ACTION_MODS_KEY(MOD_RALT, KC_NONUS_BSLASH), // |
    [10] = ACTION_MODS_KEY(MOD_RALT, KC_7),   // {
    [11] = ACTION_MODS_KEY(MOD_RALT, KC_0),   // }
    [12] = ACTION_MODS_KEY(MOD_RALT, KC_8),   // [
    [13] = ACTION_MODS_KEY(MOD_RALT, KC_9),   // ]
    [14] = ACTION_MODS_KEY(MOD_RALT, KC_MINS), // backflash
    //   [15] = ACTION_LAYER_TOGGLE(1)
};

//------------------------------------------------------------------------------
uint8_t keymap_key_to_keycode(uint8_t layer, keypos_t key)
{
    uint8_t curKey = keymaps[layer][key.row][key.col];
    
    if (layer == LAYER_RALT) {
	uint8_t base = keymaps[LAYER_BASE][key.row][key.col];

	if ((curKey == KC_TRNS) && (base != KC_NO)) {
	    // sends LALT+key
	    add_weak_mods(MOD_BIT(KC_LALT));
	    add_key(base);
	    send_keyboard_report();
	    del_weak_mods(MOD_BIT(KC_LALT));
	    del_key(base);
	    send_keyboard_report();
	    
	    return KC_NO;
	}
    } else if (layer == LAYER_LGUI) {
	uint8_t base = keymaps[LAYER_BASE][key.row][key.col];

	if ((curKey == KC_TRNS) && (base != KC_NO)) {
	    // sends LGUI+key
	    add_weak_mods(MOD_BIT(KC_LGUI));
	    add_key(base);
	    send_keyboard_report();
	    del_weak_mods(MOD_BIT(KC_LGUI));
	    del_key(base);
	    send_keyboard_report();
	    
	    return KC_NO;
	}
    }
    return curKey;
}

//------------------------------------------------------------------------------
action_t keymap_fn_to_action(uint8_t keycode)
{
    return fn_actions[FN_INDEX(keycode)];
}
