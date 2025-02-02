#include QMK_KEYBOARD_H
#include "version.h"

#define MEDIA      5 // Media layer
#define WIN_LAYOUT 0 // windows
#define MAC_LAYOUT 1 // macOs
#define GAM_LAYOUT 2 // Default Game Layout
#define COD_LAYOUT 3 // Call Of Duty
#define FN_LAYER   4 // Layer Switcher Layer

enum custom_keycodes {
  PLACEHOLDER = SAFE_RANGE, // can always be here
  EPRM,
  VRSN,
  RGB_SLD
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/* Keymap 0: Windows layer
 *
 * ,--------------------------------------------------.           ,---------------------------------------------------.
 * |   N/A  |   1  |   2  |   3   |   4   |   5  |   6  |           |   7  |   8  |   9  |   0  |   -  |   =   |  N/A   |
 * |--------+------+------+-------+-------+-------------|           |------+------+------+------+------+-------+--------|
 * |   N/A  |   Q  |   W  |   E   |   R   |   T  | Media|           |LG+<- |   Y  |   U  |   I  |   O  |   P   |   \    |
 * |--------+------+------+-------+-------+------|      |           |      |------+------+------+------+-------+--------|
 * | ~      |   A  |   S  |   D   |   F   |   G  |------|           |------|   H  |   J  |   K  |   L  |   ;   |    '   |
 * |--------+------+------+-------+-------+------|  FN  |           |LG+-> |------+------+------+------+-------+--------|
 * | LShift |   Z  |   X  |   C   |   V   |   B  |      |           |      |   N  |   M  |   ,  |   .  |   /   |   N/A  |
 * `--------+------+------+-------+-------+-------------'           `-------------+------+------+------+-------+--------'
 *   | LCtl |  LG  | LAlt |LALT+<-|LALT+->|                                       |  [   |  ]   |LCtl+.|LALT+<-|LALT+-> |
 *   `------------------------------------'                                       `-------------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        | Copy | Cut  |       | Home |  End   |
 *                                 ,------|------|------|       |------+--------+------.
 *                                 |      |      | Paste|       |Insert|        |      |
 *                                 | BkSp |  Tab |------|       |------|  Enter |Space |
 *                                 |      |      |  Esc |       |Delete|        |      |
 *                                 `--------------------'       `----------------------'
 */
// If it accepts an argument (i.e, is a function), it doesn't need KC_.
// Otherwise, it needs KC_*
[WIN_LAYOUT] = LAYOUT_ergodox(  // layer 0 : windows layout
        // left hand
        KC_TRNS, KC_1,    KC_2,    KC_3,          KC_4,          KC_5,       KC_6,
        KC_TRNS, KC_Q,    KC_W,    KC_E,          KC_R,          KC_T,       KC_TRNS,//MO(FN_LAYER),
        KC_GRV,  KC_A,    KC_S,    KC_D,          KC_F,          KC_G,
        KC_LSFT, KC_Z,    KC_X,    KC_C,          KC_V,          KC_B,       MO(FN_LAYER),
        KC_LCTL, KC_LGUI, KC_LALT, LALT(KC_LEFT), LALT(KC_RGHT),
                                                                 LCTL(KC_C), LCTL(KC_X),
                                                                             LCTL(KC_V),
                                                        KC_BSPC,     KC_TAB, KC_ESC,
        // right hand
             KC_7,          KC_8,   KC_9,    KC_0,    KC_MINS,      KC_EQL,        KC_TRNS,
             LGUI(KC_LEFT), KC_Y,   KC_U,    KC_I,    KC_O,         KC_P,          KC_BSLS,
                            KC_H,   KC_J,    KC_K,    KC_L,         KC_SCLN,       KC_QUOT,
             LGUI(KC_RGHT), KC_N,   KC_M,    KC_COMM, KC_DOT,       KC_SLSH,       KC_TRNS,
                                    KC_LBRC, KC_RBRC, LCTL(KC_DOT), LALT(KC_LEFT), LALT(KC_RGHT),
             KC_LALT, KC_END,
             KC_INS,
             KC_DEL,  KC_ENT, KC_SPC
    ),
/* Keymap 1: Mac Layer
 *
 * ,-----------------------------------------------------.           ,---------------------------------------------------.
 * |         |      |      |       |       |      |      |           |      |      |      |      |      |       |        |
 * |---------+------+------+-------+-------+------+------|           |------+------+------+------+------+-------+--------|
 * |         |      |      |       |       |      |      |           |      |      |      |      |      |       |        |
 * |---------+------+------+-------+-------+------|      |           |      |------+------+------+------+-------+--------|
 * |         |      |      |       |       |      |------|           |------|      |      |      |      |       |        |
 * |---------+------+------+-------+-------+------|      |           |      |------+------+------+------+-------+--------|
 * |         |      |      |       |       |      |      |           |      |      |      |      |      |       |        |
 * `---------+------+------+-------+-------+-------------'           `-------------+------+------+------+-------+--------'
 *   | LCTL  | LALT | LGUI |LCTL+<-|LCTL+->|                                       |      |      |LGUI+.|LCTL+<-|LCTL+->|
 *   `-------------------------------------'                                       `------------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        | Copy | Cut  |       |      |      |
 *                                 ,------|------|------|       |------+------+------.
 *                                 |      |      | Paste|       |      |      |      |
 *                                 |      |      |------|       |------|      |      |
 *                                 |      |      |      |       |      |      |      |
 *                                 `--------------------'       `--------------------'
 */
// MAC LAYOUT
[MAC_LAYOUT] = LAYOUT_ergodox(
       // left hand
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,       KC_TRNS,       KC_TRNS,    KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,       KC_TRNS,       KC_TRNS,    KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,       KC_TRNS,       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,       KC_TRNS,       KC_TRNS,    KC_TRNS,
       KC_LCTL, KC_LALT, KC_LGUI, LCTL(KC_LEFT), LCTL(KC_RGHT),
                                                                LGUI(KC_C), LGUI(KC_X),
                                                                            LGUI(KC_V),
                                                       KC_TRNS, KC_TRNS,    KC_TRNS,
       // right hand
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS,       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS,       KC_TRNS,
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS,       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,      KC_TRNS,       KC_TRNS,
                         KC_TRNS, KC_TRNS, LGUI(KC_DOT), LCTL(KC_LEFT), LCTL(KC_RGHT),
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS
),
/* Keymap 2: Generic Game Layout
 *
 * ,--------------------------------------------------.           ,--------------------------------------------------.
 * |  ESC   |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
 * |  TAB   |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |  LSHFT |      |      |      |      |      |------|           |------|      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |  LCTRL |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *   | LALT | N/A  | N/A  | N/A  | N/A  |                                       |      |      |      |      |      |
 *   `----------------------------------'                                       `----------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        |      |      |       |      |      |
 *                                 ,------|------|------|       |------+------+------.
 *                                 |      |      |      |       |      |      |      |
 *                                 | SPACE| ENTER|------|       |------|      |      |
 *                                 |      |      |      |       |      |      |      |
 *                                 `--------------------'       `--------------------'
 */
// Generic Game Layout
[GAM_LAYOUT] = LAYOUT_ergodox(
       KC_ESC,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TAB,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_LSFT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_LCTL, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_LALT, KC_NO,   KC_NO,   KC_NO,   KC_NO,
                                                    KC_TRNS, KC_TRNS,
                                                             KC_TRNS,
                                           KC_SPC,  KC_ENT,  KC_TRNS,
    // right hand
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                         KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS
),
/* Keymap 3: Call of Duty Layout
 *
 * ,--------------------------------------------------.           ,--------------------------------------------------.
 * |  ESC   |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
 * |  TAB   |  M   |  Q   |  W   |  E   |  R   |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |  LSFT  | LSFT |  A   |  S   |  D   |  F   |------|           |------|      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |  LCTL  |      |  Z   | LCTL |  X   |      |      |           |      |      |      |      |      |      |        |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *   | LALT |  N/A |  N/A |  N/A |  N/A |                                       |      |      |      |      |      |
 *   `----------------------------------'                                       `----------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        |  N/A |  M   |       |      |      |
 *                                 ,------|------|------|       |------+------+------.
 *                                 |      |      |  N/A |       |      |      |      |
 *                                 | SPACE|   C  |------|       |------|      |      |
 *                                 |      |      | LALT |       |      |      |      |
 *                                 `--------------------'       `--------------------'
 */
// Call of Duty Layout
[COD_LAYOUT] = LAYOUT_ergodox(
       KC_ESC,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TAB,  KC_M,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_TRNS,
       KC_LSFT, KC_LSFT, KC_A,    KC_S,    KC_D,    KC_F,
       KC_LCTL, KC_TRNS, KC_Z,    KC_LCTL, KC_X,    KC_TRNS, KC_TRNS,
       KC_LALT, KC_NO,   KC_NO,   KC_NO,   KC_NO,
                                                    KC_NO,   KC_M,
                                                             KC_NO,
                                           KC_SPC,  KC_C,    KC_LALT,
    // right hand
       KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                 KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS
),
/* Keymap 4: Fn Layer
 *
 * ,--------------------------------------------------.           ,--------------------------------------------------.
 * |        |  F1  |  F2  |  F3  |  F4  |  F5  |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
 * |        |  F6  |  F7  |  F8  |  F9  |  F10 |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |        |  F11 |  F12 |      |      |      |------|           |------|  <-  |  v   |  ^   |  ->  |      |        |
 * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |  N/A |           |      |      |      |      |      |      |        |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *   |      |      |      |      |      |                                       |      |      |      |      |      |
 *   `----------------------------------'                                       `----------------------------------'
 *                                        ,-------------.       ,-------------.
 *                                        |      |      |       |      |      |
 *                                 ,------|------|------|       |------+------+------.
 *                                 |      |      | COD  |       |      |      |      |
 *                                 | WIN  | MAC  |------|       |------|      |      |
 *                                 | LAYER| LAYER| GAM  |       |      |      |      |
 *                                 `--------------------'       `--------------------'
 */
// Layer Switching Help Layer
[FN_LAYER] = LAYOUT_ergodox(
       KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_TRNS,
       KC_TRNS, KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_TRNS,
       KC_TRNS, KC_F11,  KC_F12,  KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                                    KC_TRNS, KC_TRNS,
                                                             TO(COD_LAYOUT),
                             TO(WIN_LAYOUT), TO(MAC_LAYOUT), TO(GAM_LAYOUT),
    // right hand
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                         KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
       KC_TRNS, KC_TRNS,
       KC_TRNS,
       KC_TRNS, KC_TRNS, KC_TRNS
),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    // dynamically generate these.
    case EPRM:
      if (record->event.pressed) {
        eeconfig_init();
      }
      return false;
      break;
    case VRSN:
      if (record->event.pressed) {
        SEND_STRING (QMK_KEYBOARD "/" QMK_KEYMAP " @ " QMK_VERSION);
      }
      return false;
      break;
    case RGB_SLD:
      if (record->event.pressed) {
        #ifdef RGBLIGHT_ENABLE
          rgblight_mode(1);
        #endif
      }
      return false;
      break;
  }
  return true;
}

// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {

};


// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {

    uint8_t layer = biton32(layer_state);

    ergodox_board_led_off();
    ergodox_right_led_1_off();
    ergodox_right_led_2_off();
    ergodox_right_led_3_off();
    switch (layer) {
      // TODO: Make this relevant to the ErgoDox EZ.
        case 1:
            ergodox_right_led_1_on();
            break;
        case 2:
            ergodox_right_led_2_on();
            break;
        default:
            // none
            break;
    }

};
