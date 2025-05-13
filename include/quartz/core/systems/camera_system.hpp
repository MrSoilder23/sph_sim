#pragma once
// Third_party libraries
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

// Own libraries
#include "./bismuth/registry.hpp"
#include "./quartz/core/components/camera_component.hpp"
#include "./quartz/core/components/transform_component.hpp"

namespace quartz {

class CameraSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}