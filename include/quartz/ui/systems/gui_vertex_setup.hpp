#pragma once
// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/ui/components/gui_mesh.hpp"
#include "quartz/ui/components/text_mesh.hpp"

namespace quartz {

class GuiVertexSetupSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}