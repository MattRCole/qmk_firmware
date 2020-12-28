#include "animation-keyframes.h"
#include "stdint.h"

#define PIXEL_PACKAGE_SIZE 8


static uint8_t logo_x_skips[] = {
    0, 1, 2, 3, 4, 6, 8, 9, 11, 14, 17, 21, 29
};//^origin,      sliding out of frame      ^ only 1 pixel showing

// dynamically make slide-in/slide-out animations

typedef enum {
    WIN_LAYOUT,
    MAC_LAYOUT,
    GAM_LAYOUT,
    COD_LAYOUT,
    FN_LAYER
} Layers;

typedef enum {
    X,
    Y
} AxisOfMovement;

typedef struct {
    Layers         layer;
    uint8_t        origin_offset;
    AxisOfMovement axis_of_movement;
} Element;

typedef struct {
    Element logo;
    Element name;
    Element layout_purpose_symbol;
} State;

typedef struct {
    uint8_t x0, x1, y0, y1;
} Area;

inline uint8_t project_coords_onto_line(const uint8_t x, const uint8_t y, const uint8_t row_width) { return x + (y * row_width); }

inline uint8_t get_bit(const uint8_t byte, const uint8_t bit_index) { return ((1 << bit_index) & byte) == 0 ? 0 : 1; }

inline uint8_t insert_bit(const uint8_t byte, const uint8_t bit_index, const uint8_t bit_value) {
    return bit_value ? byte | (1 << bit_index) : byte & ~(1 << bit_index);
}

void paste_image_on_image(
    const uint8_t *source,
    uint8_t *target,
    const uint8_t source_row_len,
    const uint8_t target_row_len,
    const uint8_t target_array_size,
    const uint8_t source_array_size,
    const Area source_area,
    const uint8_t target_x,
    const uint8_t target_y) {

    const uint8_t x_offset = target_x;
    const uint8_t y_offset = target_y;
    const uint8_t x_range = source_area.x1 - source_area.x0;
    const uint8_t y_range = source_area.y1 - source_area.y0;

    for(uint8_t j = 0; j <= y_range; j++) {
        for(uint8_t i = 0; i <= x_range; i++) {
            const uint8_t x = x_offset + i;
            const uint8_t y = y_offset + j;

            const uint8_t source_x = i + source_area.x0;
            const uint8_t source_y = j + source_area.y0;

            const uint8_t target_pixel = project_coords_onto_line(x, y, target_row_len);
            const uint8_t target_index = target_pixel / PIXEL_PACKAGE_SIZE;
            const uint8_t target_bit_index = target_pixel % PIXEL_PACKAGE_SIZE;
            const uint8_t target_byte = target[target_index];

            const uint8_t source_pixel = project_coords_onto_line(source_x, source_y, source_row_len);
            const uint8_t source_index = source_pixel / PIXEL_PACKAGE_SIZE;
            const uint8_t source_bit_index = source_pixel % PIXEL_PACKAGE_SIZE;
            const uint8_t source_byte = source[source_index];
            const uint8_t source_bit = get_bit(source_byte, source_bit_index);

            const uint8_t byte_to_insert = insert_bit(target_byte, target_bit_index, source_bit);

            target[target_index] = byte_to_insert;
        }
    }
}

#undef PIXEL_PACKAGE_SIZE
