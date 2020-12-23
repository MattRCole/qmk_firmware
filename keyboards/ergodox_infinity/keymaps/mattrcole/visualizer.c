/*
Copyright 2017 Fred Sundvik

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

#include "simple_visualizer.h"
#include "util.h"

// Copied from keymap.c
enum ergodox_layers {
  WIN_LAYOUT,
  MAC_LAYOUT,
  GAM_LAYOUT,
  COD_LAYOUT,
  FN_LAYER
};
// This function should be implemented by the keymap visualizer
// Don't change anything else than state->target_lcd_color and state->layer_text as that's the only thing
// that the simple_visualizer assumes that you are updating
// Also make sure that the buffer passed to state->layer_text remains valid until the previous animation is
// stopped. This can be done by either double buffering it or by using constant strings
static void get_visualizer_layer_and_color(visualizer_state_t* state) {
    const uint8_t _Red = 0;
    const uint8_t _OrangeAmber = 12;
    // const uint8_t OrangeYellow = 30;
    const uint8_t _Lime = 60;
    // const uint8_t GreenBlueGreen = 90;
    // const uint8_t Aqua = 120;
    // const uint8_t CoolBlue  = 150;
    const uint8_t _Blue = 168;
    // const uint8_t CoolBluePurple = 180;
    const uint8_t _Purple = 200;
    // const uint8_t Magenta = 210;

    uint8_t saturation = 255;

    uint8_t layer = biton32(state->status.layer);
    state->target_lcd_color = LCD_COLOR(layer << 2, saturation, 0xFF);

    switch(layer) {
        case WIN_LAYOUT:
            state->layer_text = "Windows";
            state->target_lcd_color = LCD_COLOR(_Blue, saturation, 0xFF);
            break;
        case MAC_LAYOUT:
            state->layer_text = "MacOs";
            state->target_lcd_color = LCD_COLOR(_Lime, saturation, 0xFF);
            break;
        case GAM_LAYOUT:
            state->layer_text = "Gaming";
            state->target_lcd_color = LCD_COLOR(_Red, saturation, 0xFF);
            break;
        case COD_LAYOUT:
            state->layer_text = "Call Of Duty";
            state->target_lcd_color = LCD_COLOR(_OrangeAmber, saturation, 0xFF);
            break;
        case FN_LAYER:
            state->layer_text = "Fn Mode";
            state->target_lcd_color = LCD_COLOR(_Purple, saturation, 0xFF);
        default:
            state->layer_text = "None";
            state->target_lcd_color = LCD_COLOR(80, 0, 0xFF);
            break;
    }
}
