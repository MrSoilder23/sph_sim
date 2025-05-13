#pragma once
// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/core/components/instance_component.hpp"
#include "quartz/core/components/mesh_component.hpp"
#include "quartz/core/components/transform_component.hpp"

namespace quartz {

class InstanceVertexSetupSystem {
    public:
        void Update(bismuth::Registry& registry);
};

}