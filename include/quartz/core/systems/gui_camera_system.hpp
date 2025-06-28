#pragma once
// Third_party libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/core/components/gui_camera_component.hpp"

namespace quartz {

class GuiCameraSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}