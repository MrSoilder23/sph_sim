#pragma once
// Own libraries
#include "./bismuth/registry.hpp"
#include "./quartz/core/camera_component.hpp"
#include "./quartz/core/instance_component.hpp"
#include "./quartz/core/mesh_component.hpp"
#include "./quartz/core/transform_component.hpp"

namespace quartz {

class InstancedRendererSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}