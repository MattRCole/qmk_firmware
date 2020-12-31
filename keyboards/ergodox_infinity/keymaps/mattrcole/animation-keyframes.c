/**
 * Notes on this file:
 *
 * TL;DR Just call animate_elements(Layer layer) whenever
 * a layer change is detected in
 * update_user_visualizer_state()
 *
 * There's probably a better way of doing this with uGFX.
 *
 * This file divides the 128x32 lcd screen of the keyboard
 * into 4 sections.
 * 1, constant: this simply says "mode", although this
 *      can be changed to anything
 * 2, logo: A "logo" that fits the purpose of the activelayer
 * 3, name: written name of the active layer
 *      for example, if the layer is meant to be a generic
 *      Windows OS layout, the name would be "Windows", etc
 * 4, layer_symbol: if the layout is meant to be for typing,
 *      I've made a picture of a small keyboard. If the layer
 *      is meant for gaming, a little generic controller.
 *
 * Here's a picture to illustrate roughly what the division
 * boundaries are:
 * ,---------------------------------------------------------.
 * |              |                            |             |
 * |              |   3: name                  |             |
 * |              |                            |             |
 * |   2: logo    |----------------------------|  4:         |
 * |              |                            | layer_symbol|
 * |              |   1: constant (unanimated) |             |
 * |              |                            |             |
 * `---------------------------------------------------------'
 *
 * NOTE: If you want to modify these sections, it is entirely
 * possible to do so. You will just have to understand my
 * crappy code. This is an effort to document said crappy
 * code.
 *
 * ALSO NOTE: It's assumed that each element source (aka
 * picture) is the same width as the LCD screen. crappy
 * assumption, I know, but it works.
 *
 * An element, in this file only, means a section of a picture
 *  that corresponds with a portion of the screen.
 *      For example, you love playing dwarf fortress and have
 *      made a layout with macros designed for dwarf fortress
 *      The "logo" element that could correspond to that layer
 *      would be, for example, a picture of the game's symbol
 *      of a dwarf. Every time you switched to that layer, the
 *      dwarf symbol would slide into place (after whatever
 *      logo element that was previously there left the
 *      screen)
 * Each element must have a corresponding pictrue file in the
 * format discribed in
 * "<root>/quantum/visualizer/resources/lcd_logo.c".
 *
 * An element's picture must span the full 128x32 pixel area,
 * this means that you can have 1 picture of the final resting
 * state of the screen for a given layer, and re-use that
 * picture for the logo, name and layer_symbol elements of
 * that layer.
 *
 * This file will generate, at a maximum, a 15 frame animation
 * for any given element.
 *
 * This means that, if any 1 area needs
 * a complete element change (element x at rest to element y
 * at rest), a 31 frame keyframe_animation_t struct will be
 * generated:
 *      15 for element x removal,
 *      15 for element y insertion
 *      1  for the resting state of the screen
 *
 * Several elements can be moving at once during an animation
 *
 * Each element's movement is independent of all other
 * on-screen elements.
 *
 * Element animations will be fluid. No element will ever
 * magically "jump" into frame.
 *      For example, if a user toggels from layer A to layer
 *      B and then back to layer A, all in quick succession,
 *      The layer A elements will start moving off-screen,
 *      but then return to their resting position as soon as
 *      the user activates layer A.
 *
 * Animations are self-maintained, meaning that any call to
 * (start|stop)_animation will be handled in this file.
*/

// CODING HELP START =============================================================================================================

// these things should be toggled off during the build step. Honestly, this whole block should be removed before I upload this to
// github or something

// #define LCD_WIDTH 128
// #define LCD_HEIGHT 32
// #define LCD_ENABLE

// CODING HELP STOP ==============================================================================================================

#include "animation-keyframes.h"
#include "stdint.h"
#include "resources/resources.h"
#include "visualizer.h"
#include "layers.h"

#include "print.h"

// ENUM START ====================================================================================================================

typedef enum {
    EXPONENTIAL_31_PIXEL_MOVEMENT,
    LINEAR_16_PIXEL_MOVEMENT,
    TOTAL_ANIMATION_MOTION_CURVE_TYPES
} AnimationMotionCurveType;

typedef enum {
    X,
    Y
} Axis;

typedef enum {
    NEGATIVE,
    POSITIVE
} DirectionOfMovement;

typedef enum { LOGO, NAME, LAYER_SYMBOL, MODE, TOTAL_ELEMENTS } ElementList;

// ENUM STOP =====================================================================================================================

// STRUCT START ==================================================================================================================
typedef struct {
    uint8_t             frame;
    Layers              layer;
} ElementState;

typedef struct {
    ElementState logo, name, layer_symbol;
} State;

typedef struct {
    uint8_t upper_left_coordinates[2], lower_right_coordinates[2];
} BoundingBox;

/**
 * ElementProperties
 *
 * element: Enum value of element
 * axis_of_movement: animation will move Element
 * animation_motion_curve: How the element enters and exits the screen
 *      corresponds to AnimationMotionCurveReference where the index
 *      is ElementState.frame
 * direction_of_movement: direction of movement from rest position,
 *      negative if element moves left or up,
 *      positive if element moves right or down
 * bounding_box: zero-based coordinates of Element's total on-screen
 *      area.
*/
typedef struct {
    ElementList              element;
    Axis                     axis_of_movement;
    AnimationMotionCurveType animation_motion_curve;
    DirectionOfMovement      direction_of_movement;
    BoundingBox              bounding_box;
    ElementList              index;
} ElementProperties;

// STRUCT STOP ===================================================================================================================

// CONSTANTS START ===============================================================================================================

#define FRAME_TIME 200 // ms per frame
#define TOTAL_FRAMES 15
#define RESTING 0
#define PIXEL_PACKAGE_SIZE 8 // amount of pixels per byte. should be 8 unless you've drastically modified other things.

#define LARGEST_PIXEL_DELTA 8 // figure out the most pixels an animation could move in
#define LARGEST_FILL_DIMENSION 68 // find the element with the largest width or height

static const gColor ScreenEraseColor = White;

// static const uint16_t ScreenSizeInBytes = (LCD_WIDTH / PIXEL_PACKAGE_SIZE) * LCD_HEIGHT;

static const uint8_t AnimationMotionCurveReference[TOTAL_ANIMATION_MOTION_CURVE_TYPES][TOTAL_FRAMES + 1] = {
    { RESTING, 1, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 15, 18, 23, 31 },
//    ^original position,       sliding out of frame...       ^ only 1 pixel showing
    { RESTING, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }
};

static const uint8_t *const ElementResourceMap[TOTAL_LAYERS][TOTAL_ELEMENTS] = {
    { resource_logo_win, resource_name_win, resource_symbol_keyboard,   resource_mode_mode }, //WIN layer
    { resource_logo_mac, resource_name_mac, resource_symbol_keyboard,   resource_mode_mode }, //MAC layer
    { resource_logo_win, resource_name_gam, resource_symbol_controller, resource_mode_mode }, //GAM layer
    { resource_logo_cod, resource_name_cod, resource_symbol_controller, resource_mode_cod  }, //COD layer
    { NULL,              NULL,              resource_symbol_keyboard,   resource_mode_fn   }, //FN layer
};

static const ElementProperties Logo = {
    .axis_of_movement            = X,
    .animation_motion_curve      = EXPONENTIAL_31_PIXEL_MOVEMENT,
    .direction_of_movement       = NEGATIVE,
    .bounding_box = {
        .upper_left_coordinates  = { 0, 0 },
        .lower_right_coordinates = { 31, 31 }
    },
    .index                       = LOGO
};

static const ElementProperties Name = {
    .axis_of_movement            = Y,
    .animation_motion_curve      = LINEAR_16_PIXEL_MOVEMENT,
    .direction_of_movement       = NEGATIVE,
    .bounding_box = {
        .upper_left_coordinates  = { 32, 0 },
        .lower_right_coordinates = { 95, 15 }
    },
    .index                       = NAME
};

static const ElementProperties LayerSymbol = {
    .axis_of_movement            = X,
    .animation_motion_curve      = EXPONENTIAL_31_PIXEL_MOVEMENT,
    .direction_of_movement       = POSITIVE,
    .bounding_box = {
        .upper_left_coordinates  = { 96, 0 },
        .lower_right_coordinates = { 123, 31 }
    },
    .index                       = LAYER_SYMBOL
};

static const ElementProperties Mode = {
    .axis_of_movement            = Y,
    .animation_motion_curve      = LINEAR_16_PIXEL_MOVEMENT,
    .direction_of_movement       = POSITIVE,
    .bounding_box = {
        .upper_left_coordinates  = { 32, 16 },
        .lower_right_coordinates = { 95, 31 },
    },
    .index                       = MODE
};

// CONSTANTS STOP ================================================================================================================

// MUTABLE GLOBALS START =========================================================================================================

static keyframe_animation_t current_animation = {
    .loop = false
};

static ElementState current_state[TOTAL_ELEMENTS], goal_state[TOTAL_ELEMENTS];

static bool first_animation_call = true;

// MUTABLE GLOBAL STOP ==========================================================================================================

static ElementProperties get_element_properties(ElementList element) {
    switch (element) {
        case LOGO:
            return Logo;
        case NAME:
            return Name;
        case LAYER_SYMBOL:
            return LayerSymbol;
        case MODE:
        default:
            return Mode;
    }
}

// Checks to see if the current keyframe_animation will be invalidated by the layer change.
static bool animation_change_needed(Layers goal_layer) {
    for(ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        const uint8_t current_layer = current_state[element].layer;
        if (ElementResourceMap[goal_layer][element] == NULL) continue;
        if (ElementResourceMap[goal_layer][element] != ElementResourceMap[current_layer][element])
            return true;
    }
    return false;
}

// determines if the goal state for a given element has been met
static bool element_needs_animation_update(const ElementList element, const Layers goal_layer, const Layers current_layer, const uint8_t current_frame) {
    // if an element is not in its resting position, it will always need an animation update
    if (current_frame != RESTING) return true;

    // if no element resource is given for goal layer, then no update is needed
    // example: A "fn" layer may not need to change the logo element or the
    // layout symbol element.
    if (ElementResourceMap[goal_layer][element] == NULL) return false;

    //check if resources are the same, if so, no update is needed
    if (ElementResourceMap[goal_layer][element] != ElementResourceMap[current_layer][element]) return true;

    return false;
}

static uint8_t get_frame_offset_in_pixels(const AnimationMotionCurveType curve_type, const uint8_t frame) {
    return AnimationMotionCurveReference[curve_type][frame];
}

void get_target_start_position(int16_t *const output, const Axis movement_axis, const uint8_t frame_pixel_offset, const DirectionOfMovement direction, const BoundingBox *const boundaries) {
    const uint8_t *target_upper_left = boundaries->upper_left_coordinates;
    const Axis static_axis = movement_axis == X ? Y : X;

    output[static_axis] = target_upper_left[static_axis];
    output[movement_axis] = target_upper_left[movement_axis];

    output[movement_axis] += direction == POSITIVE ? frame_pixel_offset : -(int16_t)frame_pixel_offset;

    return;
}

void get_target_fill_area(uint8_t *const output, const Axis movement_axis, const uint8_t frame_pixel_offset, const DirectionOfMovement direction, const BoundingBox *const boundaries) {
    const uint8_t* target_lower_right = boundaries->lower_right_coordinates;
    const uint8_t* target_upper_left = boundaries->upper_left_coordinates;
/*     const Axis static_axis = movement_axis == X ? Y : X;

    output[static_axis] = target_lower_right[static_axis] - target_upper_left[static_axis];

    output[movement_axis] = direction == 1 + (NEGATIVE
        ? target_lower_right[movement_axis] - target_upper_left[movement_axis] - frame_pixel_offset
        : frame_pixel_offset); */

    output[X] = target_lower_right[X] - target_upper_left[X] + 1;
    output[Y] = target_lower_right[Y] - target_upper_left[Y] + 1;

    return;
}

/* void get_source_start_position(int16_t *const output, const Axis movement_axis, const uint8_t frame_pixel_offset, const DirectionOfMovement direction, const BoundingBox *const boundaries) {
    const uint8_t* source_upper_left = boundaries->upper_left_coordinates;
    const Axis static_axis = movement_axis == X ? Y : X;

    output[static_axis] = source_upper_left[static_axis];
    output[movement_axis] = source_upper_left[movement_axis];

    output[movement_axis] += direction == NEGATIVE ? frame_pixel_offset : 0;

    return;
} */

/**
 * @param[out] target_start_position[2] zero-based index of pixel where drawing should begin
 * @param[out] target_fill_area[2]      area to fill given as delta x, delta y coordinate pair
 * @param[out] source_start_position[2] zero-based index of pixel where element source picture
 *                                          should start to be drawn from
 * @param      element                  properties of element you are drawing
 * @param      frame_offset_in_pixels   the amount of pixels from the resting position of the
 *                                          element
*/
void get_draw_information(int16_t *const target_start_position, uint8_t *const target_fill_area, const ElementProperties *const properties, uint8_t frame_offset_in_pixels) {
    const DirectionOfMovement direction = properties->direction_of_movement;
    const BoundingBox boundaries = properties->bounding_box;
    const Axis movement_axis = properties->axis_of_movement;

    get_target_start_position(target_start_position, movement_axis, frame_offset_in_pixels, direction, &boundaries);
    get_target_fill_area(target_fill_area, movement_axis, frame_offset_in_pixels, direction, &boundaries);

    return;
}

void handle_slide_animation_common(const ElementList element, const uint8_t frame, const Layers active_layer) {
    const ElementProperties properties = get_element_properties(element);
    const uint8_t frame_offset_in_pixels = get_frame_offset_in_pixels(properties.animation_motion_curve, frame);
    const uint8_t *source = ElementResourceMap[active_layer][element];

    int16_t screen_start_position[2];
    uint8_t total_fill_area[2];

    get_draw_information(screen_start_position, total_fill_area, &properties, frame_offset_in_pixels);

    if (element == LOGO) {
        uprintf("Animating frame: %d\n\
            screen  x: %d\n\
            screen  y: %d\n\
            fill    x: %d\n\
            fill    y: %d\n",
            frame,
            screen_start_position[X],
            screen_start_position[Y],
            total_fill_area[X],
            total_fill_area[Y]
        );
    }

    gdispGBlitArea(GDISP, screen_start_position[X], screen_start_position[Y], total_fill_area[X], total_fill_area[Y], 0, 0, total_fill_area[X], (const gPixel*)source);
}

static uint8_t get_pixel_delta_between_frames(const uint8_t earlier_frame, const uint8_t later_frame, ElementList element) {
    AnimationMotionCurveType motion_curve_type = get_element_properties(element).animation_motion_curve;
    const uint8_t* motion_curve = AnimationMotionCurveReference[motion_curve_type];
    return motion_curve[later_frame] - motion_curve[earlier_frame];
}

void handle_slide_in_animation(const ElementList element) {
    const uint8_t next_frame = current_state[element].frame - 1;
    const uint8_t current_frame = current_state[element].frame;
    const uint8_t current_layer = current_state[element].layer;
    const uint8_t pixel_delta = get_pixel_delta_between_frames(next_frame, current_frame, element);

    if (pixel_delta != 0) handle_slide_animation_common(element, next_frame, current_layer);

    current_state[element].frame = next_frame;
}


/**
 * @param[out] erase_start_position[2] zero-based index of pixel where erasing should begin
 * @param[out] erase_fill_area[2]      area to erase given as delta x, delta y coordinate pair
 * @param      clean_up_frame          the most recently rendered frame
 * @param      element                 the element's enumeration index
*/
void get_erase_information(uint8_t *const erase_start_position, uint8_t *const erase_fill_area, const uint8_t clean_up_frame, ElementList element) {
/*     const ElementProperties properties = get_element_properties(element);
    const Axis movement_axis = properties.axis_of_movement;
    const Axis static_axis   = movement_axis == X ? Y : X;
    const DirectionOfMovement movement_direction = properties.direction_of_movement;
    const uint8_t clean_up_frame_pixel_offset = get_frame_offset_in_pixels(properties.animation_motion_curve, clean_up_frame);
    const uint8_t pixel_delta = get_pixel_delta_between_frames(clean_up_frame - 1, clean_up_frame, properties.animation_motion_curve);

    int16_t target_start_position[2];
    uint8_t target_fill_area[2];

    get_target_start_position(target_start_position, movement_axis, clean_up_frame_pixel_offset, movement_direction, &(properties.bounding_box));
    get_target_fill_area(target_fill_area, movement_axis, clean_up_frame_pixel_offset, movement_direction, &(properties.bounding_box));


    erase_start_position[static_axis] = target_start_position[static_axis];
    erase_start_position[movement_axis] = movement_direction == NEGATIVE
        ? target_start_position[movement_axis] + target_fill_area[movement_axis]
        : target_start_position[movement_axis] - pixel_delta;

    erase_fill_area[static_axis] = target_fill_area[static_axis];
    erase_fill_area[movement_axis] = pixel_delta; */
    const ElementProperties properties = get_element_properties(element);
    const uint8_t *element_upper_left = properties.bounding_box.upper_left_coordinates;
    const uint8_t *element_lower_right = properties.bounding_box.lower_right_coordinates;
    const Axis movement_axis = properties.axis_of_movement;
    const Axis static_axis   = movement_axis == X ? Y : X;
    const DirectionOfMovement movement_direction = properties.direction_of_movement;
    const uint8_t clean_up_frame_pixel_offset = get_frame_offset_in_pixels(properties.animation_motion_curve, clean_up_frame);
    const uint8_t pixel_delta = get_pixel_delta_between_frames(clean_up_frame - 1, clean_up_frame, properties.animation_motion_curve);

    erase_start_position[static_axis] = element_upper_left[static_axis];
    erase_start_position[movement_axis] = movement_direction == POSITIVE
        ? element_upper_left[movement_axis] + clean_up_frame_pixel_offset
        : element_lower_right[movement_axis] + clean_up_frame_pixel_offset;

    erase_fill_area[static_axis] = element_lower_right[static_axis] - element_upper_left[static_axis] + 1;
    erase_fill_area[movement_axis] = pixel_delta;

    return;
}

void handle_slide_out_clean_up(const ElementList element, const uint8_t clean_up_frame) {
    uint8_t erase_start_position[2], erase_fill_area[2];

    get_erase_information(erase_start_position, erase_fill_area, clean_up_frame, element);

    if(!erase_fill_area[X] || !erase_fill_area[Y]) return;

    gdispGFillArea(
        GDISP,
        erase_start_position[X],
        erase_start_position[Y],
        erase_fill_area[X],
        erase_fill_area[Y],
        ScreenEraseColor
    );
}

void handle_slide_out_animation(const ElementList element) {
    const uint8_t current_frame = current_state[element].frame;
    const uint8_t next_frame = current_frame + 1 > TOTAL_FRAMES
        ? current_frame
        : current_frame + 1;
    const uint8_t pixel_delta = get_pixel_delta_between_frames(current_frame, next_frame, element);

    if (next_frame == current_frame) {
        //technically done with slide-out animation, switching to a slide-in animation
        current_state[element].layer = goal_state[element].layer;
    }

    if (pixel_delta || next_frame == current_frame) handle_slide_animation_common(element, next_frame, current_state[element].layer);

    if (current_frame != next_frame) handle_slide_out_clean_up(element, next_frame);

    current_state[element].frame = next_frame;
}

bool animation_routine(keyframe_animation_t* animation, visualizer_state_t* state) {
    if (first_animation_call) {
        gdispGClear(GDISP, ScreenEraseColor);
        first_animation_call = false;
    }

    for(ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        if (!element_needs_animation_update(element, goal_state[element].layer, current_state[element].layer, current_state[element].frame)) continue;

        if (current_state[element].layer == goal_state[element].layer) handle_slide_in_animation(element);

        else handle_slide_out_animation(element);
    }
    return false;
}

void initialize_my_animation_handler(void) {
    print("intializing things\n");

    for (uint8_t i = 0; i < TOTAL_FRAMES * 2 + 1; i++) {
        current_animation.frame_functions[i] = &animation_routine;
        current_animation.frame_lengths[i] = gfxMillisecondsToTicks(FRAME_TIME);
    }

    Layers layer_to_animate = 0;
    uint8_t frames_needed = 0;

    for(Layers layer = 0; layer < TOTAL_LAYERS; layer++) {
        for(ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
            if (ElementResourceMap[layer][element]) {
                layer_to_animate = layer;
                frames_needed = TOTAL_FRAMES + 1;
                goto layer_found;
            }
        }
    }
layer_found:

    for(ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        goal_state[element].layer = current_state[element].layer = layer_to_animate;
        goal_state[element].frame = RESTING;

        current_state[element].frame = ElementResourceMap[layer_to_animate][element] == NULL
            ? RESTING
            : TOTAL_FRAMES;
    }

    current_animation.num_frames = frames_needed;

    start_keyframe_animation(&current_animation);
}

/* static uint8_t project_coords_onto_line(const uint8_t x, const uint8_t y, const uint8_t row_width) { return x + (y * row_width); }

static uint8_t get_bit(const uint8_t byte, const uint8_t bit_index) { return ((1 << bit_index) & byte) == 0 ? 0 : 1; }

static uint8_t insert_bit(const uint8_t byte, const uint8_t bit_index, const uint8_t bit_value) { return bit_value ? byte | (1 << bit_index) : byte & ~(1 << bit_index); }

static uint8_t get_pixel_offset_from_frame(const Axis offset_axis, const ElementProperties *element_properties, const uint8_t frame) {

}

static uint8_t get_source_pixel_offset(const Axis offset_axis, const ElementProperties *element_properties, const ElementState *element_state) {
    return offset_axis != element_properties->axis_of_movement
        ? element_properties->bounding_box.upper_left_coordinates[offset_axis]
        : get_pixel_offset_from_frame(offset_axis, element_properties, element_state->frame);
} */

/* void paste_image_on_image(
    const uint8_t *source,
    uint8_t *target,
    const ElementList element,
    const ElementState* element_state
    ) {
    const ElementProperties element_properties = get_element_properties(element);
    const BoundingBox *bounds = &(element_properties.bounding_box);

    const uint8_t source_x_offset = get_source_pixel_offset(X, &element_properties, element_state);
    const uint8_t source_y_offset = get_source_pixel_offset(Y, &element_properties, element_state);

    const uint8_t x_offset  = bounds->upper_left_coordinates[X];
    const uint8_t y_offset  = bounds->upper_left_coordinates[Y];

    const uint8_t x_range  = bounds->lower_right_coordinates[X] - source_x_offset;
    const uint8_t y_range  = bounds->lower_right_coordinates[Y] - source_y_offset;

    for (uint8_t j = 0; j <= y_range; j++) {
        for (uint8_t i = 0; i <= x_range; i++) {
            const uint8_t x = x_offset + i;
            const uint8_t y = y_offset + j;

            const uint8_t source_x = i + source_area.x0;
            const uint8_t source_y = j + source_area.y0;

            const uint8_t target_pixel     = project_coords_onto_line(x, y, target_row_len);
            const uint8_t target_index     = target_pixel / PIXEL_PACKAGE_SIZE;
            const uint8_t target_bit_index = target_pixel % PIXEL_PACKAGE_SIZE;
            const uint8_t target_byte      = target[target_index];

            const uint8_t source_pixel     = project_coords_onto_line(source_x, source_y, source_row_len);
            const uint8_t source_index     = source_pixel / PIXEL_PACKAGE_SIZE;
            const uint8_t source_bit_index = source_pixel % PIXEL_PACKAGE_SIZE;
            const uint8_t source_byte      = source[source_index];
            const uint8_t source_bit       = get_bit(source_byte, source_bit_index);

            const uint8_t byte_to_insert = insert_bit(target_byte, target_bit_index, source_bit);

            target[target_index] = byte_to_insert;
        }
    }
} */

void update_keyframe_animation(uint8_t needed_frames) {
    stop_keyframe_animation(&current_animation);

    current_animation.num_frames = needed_frames;

    start_keyframe_animation(&current_animation);
}

// this function will never return zero, only call if you know that an animation is needed
static uint8_t get_needed_element_frame_count(const ElementList element, const Layers current_layer, const Layers goal_layer, const uint8_t current_frame) {
    const uint8_t element_has_animation_on_current_layer = ElementResourceMap[current_layer][element] == NULL ? 0 : 1;
    const uint8_t resting_frame = 1;

    if (!element_has_animation_on_current_layer) {
        return TOTAL_FRAMES + resting_frame;
    }

    const bool need_to_move_off_screen = current_layer != goal_layer;
    const uint8_t move_off_of_screen_frames = need_to_move_off_screen ? TOTAL_FRAMES - current_frame : 0;
    const uint8_t move_on_screen_frames = need_to_move_off_screen ? TOTAL_FRAMES : current_frame;

    return move_off_of_screen_frames + move_on_screen_frames + resting_frame;
}

void update_goal_states(Layers new_layer) {
    for (ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        if (ElementResourceMap[new_layer][element] == NULL) continue;

        goal_state[element].layer = new_layer;
    }
}

/**
 *  update_lcd_animation
 *
 *  Should only be called on a layer change. Handles ALL lcd animations internally.
 *  (but not backlight)
*/
void update_my_animation_handler(Layers new_goal_layer) {
    print("updating animation....\n");
    if (!animation_change_needed(new_goal_layer)) return;

    uint8_t total_frames_needed = 0;

    for (ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        const uint8_t current_frame = current_state[element].frame;
        const uint8_t current_layer = current_state[element].layer;

        if (!element_needs_animation_update(element, new_goal_layer, current_layer, current_frame)) continue;

        const uint8_t element_frames_needed = get_needed_element_frame_count(element, current_layer, new_goal_layer, current_frame);

        if (element_frames_needed > total_frames_needed) total_frames_needed = element_frames_needed;
    }

    if (!total_frames_needed) return;

    update_keyframe_animation(total_frames_needed);
    update_goal_states(new_goal_layer);

}

#undef PIXEL_PACKAGE_SIZE
