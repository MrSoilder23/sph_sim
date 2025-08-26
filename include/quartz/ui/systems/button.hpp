#pragma once
// Own libraries
#include "bismuth/registry.hpp"
#include "quartz/ui/components/button.hpp"
#include "quartz/ui/components/text_mesh.hpp"
#include "quartz/ui/components/gui_object_component.hpp"
#include "quartz/core/components/mouse_state.hpp"

namespace quartz {

class ButtonSystem {
    public:
        void Update(bismuth::Registry& registry);

    private:
        bool CheckAABB(const glm::vec2& point, GuiObjectComponent& object);
};

}