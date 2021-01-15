#pragma once
#include "layers.h"

// Should be called upon visualizer initialization
void initialize_my_animation_handler(Layers);

// should be called every time there is a state change.
void update_my_animation_handler(Layers);
