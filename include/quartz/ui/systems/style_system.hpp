#pragma once
// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/core/components/mesh_component2.hpp"
#include "quartz/ui/components/gui_object_component.hpp"

namespace quartz {

class StyleSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}