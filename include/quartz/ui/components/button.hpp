#pragma once
// C++ standard libraries
#include <functional>

// Own libraries
#include "quartz/ui/utils/button_state.hpp"

struct ButtonComponent {
    std::function<void()> onClick;
    quartz::ButtonState state = quartz::ButtonState::normal;

    bool isDirty = true;
};