#pragma once
// Own libaries
#include "./bismuth/registry.hpp"
#include "quartz/core/components/sphere_component.hpp"
#include "./quartz/core/components/instance_component.hpp"

class TestSystem {
    public:
        void Update(bismuth::Registry& registry);
};