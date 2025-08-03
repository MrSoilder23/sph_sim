#pragma once
// C++ standard libraries
#include <vector>

// Own libraries
#include "quartz/ui/utils/style.hpp"

struct GuiObjectComponent {
    quartz::Style style;
    float zLayer = -1.0f;

    uint32_t parentID;
};