#pragma once
// Third_party libraries
#include <glm/gtc/type_ptr.hpp>

// Own libraries
#include "./bismuth/registry.hpp"
#include "./quartz/graphics/shader.hpp"
#include "./quartz/core/components/camera_component.hpp"
#include "./quartz/core/components/transform_component.hpp"
#include "./quartz/core/components/instance_component.hpp"
#include "./quartz/core/components/sphere_component.hpp"

namespace quartz {

class InstancedRendererSystem {
    public:
        void Init(GLuint shaderProgram);

        void Update(bismuth::Registry& registry);
    private:
        GLuint mInstanceVBO;
        GLuint mDummyVAO;
        GLuint mShaderProgram;
};

}