/**
 * Notes on this file:
 *
 * TL;DR Just call update_my_animation_handler(Layer layer)
 * whenever a layer change is detected in
 * update_user_visualizer_state()
 *
 * Most of the following is customizable, but is used as an
 * example
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
 *
 * NOTE: It's assumed that each element source (aka
 * picture) spans only the area that it occupies on-screen
 *
 * An element, for our purposes, means a list of specifications
 *  that defines how a portion of the screen will be animated.
 *      For example, you love playing dwarf fortress and have
 *      made a keyboard layer (layout) with macros designed for
 *      dwarf fortress. Assuming that you're using the
 *      defaults, you could create a bitmap of the dwarf symbol
 *      from Dwarf Fortress and assign that bitmap to the logo
 *      element's resource for that keyboard layer. Every time
 *      the Dwarf Fortress layer was active, the dwarf Bitmap
 *      would slide onto screen.
 * Each element will have a list of bitmap references that
 *  correspond to every given keyboard layer. If the bitmap
 *  reference is NULL, this means that activating that layer
 *  will not animate that specific element, rather, it will
 *  leave the previous element bitmap on screen.
 * Element bitmaps must follow the format discribed in
 * "<root>/quantum/visualizer/resources/lcd_logo.c".
 *
 * Several elements can be moving at once during a layer
 *  change. with each element's movement being independent of
 *  all other on-screen elements.
 *
 * Animations will only be triggered on a layer change.
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

#include "animation-keyframes.h"
#include "stdint.h"
#include "resources/resources.h"
#include "visualizer.h"
#include "layers.h"

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
    uint8_t upper_left_coordinates[2], lower_right_coordinates[2];
} BoundingBox;

/**
 * ElementProperties
 *
 * element: corresponding ElementList enum value of element
 * axis_of_movement: Either X or Y. If the animation is up or down,
 *      the value would be Y, left or right would be X
 * animation_motion_curve: the AnimationMotionCurveReference that
 *      the animation of the element should follow (read
 *      AnimationMotionCurveReference comments for more details)
 * direction_of_movement: direction of movement from rest position,
 *      negative if element moves left or up,
 *      positive if element moves right or down
 * bounding_box: zero-based coordinates of Element's boundary
 *      corners on the LCD screen.
 *
*/
typedef struct {
    ElementList              element;
    Axis                     axis_of_movement;
    AnimationMotionCurveType animation_motion_curve;
    DirectionOfMovement      direction_of_movement;
    BoundingBox              bounding_box;
} ElementProperties;

// STRUCT STOP ===================================================================================================================

// CONSTANTS START ===============================================================================================================

#define FRAME_TIME 20 // ms per frame
#define TOTAL_FRAMES 15
#define RESTING 0
#define PIXEL_PACKAGE_SIZE 8 // amount of pixels per byte. should be 8 unless you've drastically modified other things.

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
    { NULL,              NULL,              resource_symbol_keyboard,   resource_mode_fn   }  //FN layer
};

static const ElementProperties ElementPropertiesList[] = {
    {
        .axis_of_movement            = X,
        .animation_motion_curve      = EXPONENTIAL_31_PIXEL_MOVEMENT,
        .direction_of_movement       = NEGATIVE,
        .bounding_box = {
            .upper_left_coordinates  = { 0, 0 },
            .lower_right_coordinates = { 31, 31 }
        },
        .element                       = LOGO
    },
    {
        .axis_of_movement            = Y,
        .animation_motion_curve      = LINEAR_16_PIXEL_MOVEMENT,
        .direction_of_movement       = NEGATIVE,
        .bounding_box = {
            .upper_left_coordinates  = { 32, 0 },
            .lower_right_coordinates = { 95, 15 }
        },
        .element                       = NAME
    },
    {
        .axis_of_movement            = X,
        .animation_motion_curve      = EXPONENTIAL_31_PIXEL_MOVEMENT,
        .direction_of_movement       = POSITIVE,
        .bounding_box = {
            .upper_left_coordinates  = { 96, 0 },
            .lower_right_coordinates = { 127, 31 }
        },
        .element                       = LAYER_SYMBOL
    },
    {
        .axis_of_movement            = Y,
        .animation_motion_curve      = LINEAR_16_PIXEL_MOVEMENT,
        .direction_of_movement       = POSITIVE,
        .bounding_box = {
            .upper_left_coordinates  = { 32, 16 },
            .lower_right_coordinates = { 95, 31 },
        },
        .element                       = MODE
    }
};

// CONSTANTS STOP ================================================================================================================

// MUTABLE GLOBALS START =========================================================================================================

static keyframe_animation_t current_animation = {
    .loop = false
};

static ElementState current_state[TOTAL_ELEMENTS], goal_state[TOTAL_ELEMENTS];

// MUTABLE GLOBAL STOP ==========================================================================================================

static ElementProperties get_element_properties(ElementList element) {
    for(uint8_t i = 0; i < TOTAL_ELEMENTS; i++) {
        if (ElementPropertiesList[i].element == element) return ElementPropertiesList[i];
    }
    return (ElementProperties){};
}

/* static ElementProperties get_element_properties(ElementList element) {
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
} */

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

void get_target_start_position(uint8_t *const output, const Axis movement_axis, const uint8_t frame_pixel_offset, const DirectionOfMovement direction, const BoundingBox *const boundaries) {
    output[X] = boundaries->upper_left_coordinates[X];
    output[Y] = boundaries->upper_left_coordinates[Y];

    if (direction == NEGATIVE) return;

    output[movement_axis] += frame_pixel_offset;

    return;
}

void get_source_start_position(uint8_t *const output, const Axis movement_axis, const uint8_t frame_pixel_offset, const DirectionOfMovement direction, const BoundingBox *const boundaries) {
    output[X] = 0;
    output[Y] = 0;

    if (direction == POSITIVE) return;

    output[movement_axis] = frame_pixel_offset;

    return;
}

void get_target_fill_area(uint8_t *const output, const Axis movement_axis, const uint8_t frame_pixel_offset, const BoundingBox *const boundaries) {
    const Axis static_axis = movement_axis == X ? Y : X;

    const uint8_t source_dimensions[2] = { // Remember: fill area is not zero based, it is total pixles
      boundaries->lower_right_coordinates[X] - boundaries->upper_left_coordinates[X] + 1,
      boundaries->lower_right_coordinates[Y] - boundaries->upper_left_coordinates[Y] + 1
    };

    output[static_axis] = source_dimensions[static_axis];

    output[movement_axis] = source_dimensions[movement_axis] - frame_pixel_offset;

    return;
}

/**
 * @param[out] target_start_position[2] zero-based index of pixel where drawing should begin
 * @param[out] target_fill_area[2]      area to fill given as delta x, delta y coordinate pair
 * @param[out] source_start_position[2] zero-based index of pixel where element source picture
 *                                          should start to be drawn from
 * @param      properties                   properties of element you are drawing
 * @param      frame_offset_in_pixels   the amount of pixels from the resting position of the
 *                                          element
*/
uint8_t get_draw_information(uint8_t *const target_start_position, uint8_t *const target_fill_area, uint8_t *const source_start_position, const ElementProperties *const properties, uint8_t frame_offset_in_pixels) {
    const DirectionOfMovement direction = properties->direction_of_movement;
    const BoundingBox boundaries = properties->bounding_box;
    const Axis movement_axis = properties->axis_of_movement;

    get_target_start_position(target_start_position, movement_axis, frame_offset_in_pixels, direction, &boundaries);
    get_target_fill_area(target_fill_area, movement_axis, frame_offset_in_pixels, &boundaries);
    get_source_start_position(source_start_position, movement_axis, frame_offset_in_pixels, direction, &boundaries);

    return boundaries.lower_right_coordinates[X] - boundaries.upper_left_coordinates[X] + 1;
}

void handle_slide_animation_common(const ElementList element, const uint8_t frame, const Layers active_layer) {
    const ElementProperties properties = get_element_properties(element);
    const uint8_t frame_offset_in_pixels = get_frame_offset_in_pixels(properties.animation_motion_curve, frame);
    const uint8_t *source = ElementResourceMap[active_layer][element];

    uint8_t screen_start_position[2];
    uint8_t total_fill_area[2];
    uint8_t bitmap_start_position[2];

    const uint8_t bitmap_row_length =
      get_draw_information(screen_start_position, total_fill_area, bitmap_start_position, &properties, frame_offset_in_pixels);


    gdispGBlitArea(GDISP,\
        screen_start_position[X],\
        screen_start_position[Y],\
        total_fill_area[X],\
        total_fill_area[Y],\
        bitmap_start_position[X],\
        bitmap_start_position[Y],\
        bitmap_row_length,\
        (const gPixel*)source);
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
        ? element_upper_left[movement_axis]
        : element_lower_right[movement_axis] - clean_up_frame_pixel_offset;

    erase_fill_area[static_axis] = element_lower_right[static_axis] - element_upper_left[static_axis] + 1;
    erase_fill_area[movement_axis] = pixel_delta + 1;

    return;
}

void handle_slide_out_clean_up(const ElementList element, const uint8_t clean_up_frame) {
    uint8_t erase_start_position[2], erase_fill_area[2];

    get_erase_information(erase_start_position, erase_fill_area, clean_up_frame, element);

    if(erase_fill_area[X] == 0 || erase_fill_area[Y] == 0) return;

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
        //technically done with the slide-out animation, switching to a slide-in animation
        current_state[element].layer = goal_state[element].layer;
    }

    if (pixel_delta || next_frame == current_frame) handle_slide_animation_common(element, next_frame, current_state[element].layer);

    if (current_frame != next_frame) handle_slide_out_clean_up(element, next_frame);

    current_state[element].frame = next_frame;
}

bool animation_routine(keyframe_animation_t* animation, visualizer_state_t* state) {
    for(ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        if (!element_needs_animation_update(element, goal_state[element].layer, current_state[element].layer, current_state[element].frame)) continue;

        if (current_state[element].layer == goal_state[element].layer) handle_slide_in_animation(element);

        else handle_slide_out_animation(element);
    }
    return false;
}

void initialize_my_animation_handler(Layers layer) {
    for (uint8_t i = 0; i < TOTAL_FRAMES * 2 + 1; i++) {
        current_animation.frame_functions[i] = &animation_routine;
        current_animation.frame_lengths[i] = gfxMillisecondsToTicks(FRAME_TIME);
    }

    const uint8_t frames_needed = TOTAL_FRAMES + 1;
    bool animation_needed = false;

    for(ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        goal_state[element].layer = current_state[element].layer = layer;
        goal_state[element].frame = RESTING;

        current_state[element].frame = RESTING;

        if (ElementResourceMap[layer][element] != NULL ) {
            current_state[element].frame = TOTAL_FRAMES;
            animation_needed = true;
        }

    }

    gdispGClear(GDISP, ScreenEraseColor);

    if (animation_needed) {
        current_animation.num_frames = frames_needed;

        start_keyframe_animation(&current_animation);
    }
}

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

void update_element_layer_states(const ElementList element, const Layers new_layer) {
    ElementState *const current = &current_state[element];
    ElementState *const goal = &goal_state[element];
    const uint8_t *const new_layer_resource = ElementResourceMap[new_layer][element];
    const uint8_t *const current_resource = ElementResourceMap[current->layer][element];

    if (current->layer == new_layer && goal->layer == new_layer) return;

    if (new_layer_resource == NULL) {
        goal->layer = current->layer;
        return;
    }

    goal->layer = new_layer;

    if (current_resource == new_layer_resource) {
        current->layer = new_layer;
    }

}

/**
 *  update_lcd_animation
 *
 *  Should only be called on a layer change. Handles ALL lcd animations internally.
 *  (but not backlight)
*/
void update_my_animation_handler(Layers new_goal_layer) {
    uint8_t total_frames_needed = 0;

    for (ElementList element = 0; element < TOTAL_ELEMENTS; element++) {
        const uint8_t current_frame = current_state[element].frame;
        const uint8_t current_layer = current_state[element].layer;

        if (element_needs_animation_update(element, new_goal_layer, current_layer, current_frame)) {
            const uint8_t element_frames_needed = get_needed_element_frame_count(element, current_layer, new_goal_layer, current_frame);

            if (element_frames_needed > total_frames_needed) total_frames_needed = element_frames_needed;
        }

        update_element_layer_states(element, new_goal_layer);
    }

    if (total_frames_needed) update_keyframe_animation(total_frames_needed);
}

#undef PIXEL_PACKAGE_SIZE
