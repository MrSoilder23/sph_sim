#pragma once
// C++ standard libraries
#include <vector>

// Own libraries
#include "quartz/ui/utils/style.hpp"
#include "bismuth/registry.hpp"

struct GuiObjectComponent {
    quartz::Style style;
    float zLayer = -1.0f;

    bismuth::EntityID parentID = bismuth::INVALID_INDEX;
};