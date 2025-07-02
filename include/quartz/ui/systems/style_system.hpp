#pragma once
// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/ui/components/gui_mesh.hpp"
#include "quartz/ui/components/gui_object_component.hpp"

namespace quartz {

class StyleSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}