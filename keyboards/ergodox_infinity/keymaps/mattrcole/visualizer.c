/*
Copyright 2016 Fred Sundvik <fsundvik@gmail.com>

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

//Modified from the original visualizer.c found in the root of the
//ergodox_infinity keyboard folder. This displays custom images
//depending on the current layer.

#ifndef LCD_BACKLIGHT_ENABLE
#error This visualizer needs that LCD backlight is enabled
#endif

#ifndef LCD_ENABLE
#error This visualizer needs that LCD is enabled
#endif

// #include "print.h"

#include "resources/resources.h"
#include "visualizer.h"
#include "visualizer_keyframes.h"
#include "lcd_keyframes.h"
#include "lcd_backlight_keyframes.h"
#include "system/serial_link.h"
#include "default_animations.h"
#include "util.h"
#include "layers.h"
#include "animation-keyframes.h"

static bool first_state_update = true;

bool led_backlight_keyframe_fade_in_all(keyframe_animation_t* animation, visualizer_state_t* state);

static keyframe_animation_t color_animation = {
    .num_frames = 2,
    .loop = false,
    .frame_lengths = {gfxMillisecondsToTicks(200), gfxMillisecondsToTicks(500)},
    .frame_functions = {keyframe_no_operation, lcd_backlight_keyframe_animate_color},
};

/* static bool keyframe_fade_in(keyframe_animation_t* animation, visualizer_state_t* state) {
    bool ret = false;
    ret = ret || lcd_backlight_keyframe_animate_color(animation, state);
    ret = ret || led_backlight_keyframe_fade_in_all(animation, state);
    return ret;
} */

/* bool lcd_keyframe_draw_mac_layer_pic(keyframe_animation_t* animation, visualizer_state_t* state) {
    (void)state;
    (void)animation;

    gdispClear(Black);
    gdispGBlitArea(GDISP, 0, 0, 128, 32, 0, 0, LCD_WIDTH, (pixel_t*)resource_mac_layer);

    return false;
} */

/* static keyframe_animation_t mac_layer_animation = {
    .num_frames = 1,
    .loop = false,
    .frame_lengths = {
        gfxMillisecondsToTicks(0),
        gfxMillisecondsToTicks(5000)
    },
    .frame_functions = {
        lcd_keyframe_draw_mac_layer_pic,
        keyframe_fade_in
    },
}; */

/* bool lcd_keyframe_draw_win_layer_pic(keyframe_animation_t* animation, visualizer_state_t* state) {
    (void)state;
    (void)animation;
    gdispClear(Black);

    gdispGBlitArea(GDISP, 0, 0, 128, 32, 0, 0, LCD_WIDTH, (pixel_t*)resource_win_layer);

    return false;
} */

// static keyframe_animation_t win_layer_animation = {
//     .num_frames = 1,
//     .loop = false,
//     .frame_lengths = {gfxMillisecondsToTicks(0)},
//     .frame_functions = {lcd_keyframe_draw_win_layer_pic},
// };

static const uint32_t logo_background_color = LCD_COLOR(0x00, 0x00, 0xFF);
static const uint32_t initial_color = LCD_COLOR(0, 0, 0);


typedef enum {
    LCD_STATE_INITIAL,
    LCD_STATE_LAYER_BITMAP,
    LCD_STATE_BITMAP_AND_LEDS,
} lcd_state_t;

static lcd_state_t lcd_state = LCD_STATE_INITIAL;

typedef struct {
    uint8_t led_on;
    uint8_t led1;
    uint8_t led2;
    uint8_t led3;
} visualizer_user_data_t;

// Don't access from visualization function, use the visualizer state instead
static visualizer_user_data_t user_data_keyboard = {
    .led_on = 0,
    .led1 = LED_BRIGHTNESS_HI,
    .led2 = LED_BRIGHTNESS_HI,
    .led3 = LED_BRIGHTNESS_HI,
};

_Static_assert(sizeof(visualizer_user_data_t) <= VISUALIZER_USER_DATA_SIZE,
    "Please increase the VISUALIZER_USER_DATA_SIZE");

void initialize_user_visualizer(visualizer_state_t* state) {
    // The brightness will be dynamically adjustable in the future
    // But for now, change it here.
    // uprintf("Max frames: %d", MAX_VISUALIZER_KEY_FRAMES);
    lcd_backlight_brightness(130);
    state->current_lcd_color = initial_color;
    state->target_lcd_color = logo_background_color;
    lcd_state = LCD_STATE_INITIAL;
    start_keyframe_animation(&default_startup_animation);
}

static inline bool is_led_on(visualizer_user_data_t* user_data, uint8_t num) {
    return user_data->led_on & (1u << num);
}

void initialize_my_animation_handler(void);
void update_my_animation_handler(Layers);

void update_user_visualizer_state(visualizer_state_t* state, visualizer_keyboard_status_t* prev_status) {
    // const uint8_t OrangeYellow = 30;
    // const uint8_t GreenBlueGreen = 90;
    // const uint8_t Aqua = 120;
    // const uint8_t CoolBlue  = 150;
    // const uint8_t CoolBluePurple = 180;
    // const uint8_t Magenta = 210;

    const uint8_t _Red = 0;
    const uint8_t _OrangeAmber = 12;
    const uint8_t _Lime = 60;
    const uint8_t _Blue = 168;
    const uint8_t _Purple = 200;
    const uint8_t saturation = 255;

    uint8_t layer = biton32(state->status.layer);
    uint8_t previous_layer = biton32(prev_status->layer);

    if (layer == previous_layer && !first_state_update) return;

    if (first_state_update) {
        // print("Calling initializer");
        first_state_update = false;
        initialize_my_animation_handler();
    }
    else update_my_animation_handler((Layers)layer);

    switch(layer) {
        case WIN_LAYOUT:
            state->target_lcd_color = LCD_COLOR(_Blue, saturation, 0xFF);
            break;
        case MAC_LAYOUT:
            state->target_lcd_color = LCD_COLOR(_Lime, saturation, 0xFF);
            break;
        case GAM_LAYOUT:
            state->target_lcd_color = LCD_COLOR(_Red, saturation, 0xFF);
            break;
        case COD_LAYOUT:
            state->target_lcd_color = LCD_COLOR(_OrangeAmber, saturation, 0xFF);
            break;
        case FN_LAYER:
            state->target_lcd_color = LCD_COLOR(_Purple, saturation, 0xFF);
        default:
            state->target_lcd_color = LCD_COLOR(80, 0, 0xFF);
            break;
    }

    start_keyframe_animation(&color_animation);

}

void user_visualizer_suspend(visualizer_state_t* state) {
    state->layer_text = "Suspending...";
    uint8_t hue = LCD_HUE(state->current_lcd_color);
    uint8_t sat = LCD_SAT(state->current_lcd_color);
    state->target_lcd_color = LCD_COLOR(hue, sat, 0);
    start_keyframe_animation(&default_suspend_animation);
}

void user_visualizer_resume(visualizer_state_t* state) {
    state->current_lcd_color = initial_color;
    state->target_lcd_color = logo_background_color;
    lcd_state = LCD_STATE_INITIAL;
    start_keyframe_animation(&default_startup_animation);
}

void ergodox_board_led_on(void){
    // No board led support
}

void ergodox_right_led_1_on(void){
    user_data_keyboard.led_on |= (1u << 0);
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_2_on(void){
    user_data_keyboard.led_on |= (1u << 1);
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_3_on(void){
    user_data_keyboard.led_on |= (1u << 2);
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_board_led_off(void){
    // No board led support
}

void ergodox_right_led_1_off(void){
    user_data_keyboard.led_on &= ~(1u << 0);
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_2_off(void){
    user_data_keyboard.led_on &= ~(1u << 1);
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_3_off(void){
    user_data_keyboard.led_on &= ~(1u << 2);
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_1_set(uint8_t n) {
    user_data_keyboard.led1 = n;
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_2_set(uint8_t n) {
    user_data_keyboard.led2 = n;
    visualizer_set_user_data(&user_data_keyboard);
}

void ergodox_right_led_3_set(uint8_t n) {
    user_data_keyboard.led3 = n;
    visualizer_set_user_data(&user_data_keyboard);
}
