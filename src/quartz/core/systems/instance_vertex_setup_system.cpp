#include "./quartz/core/systems/instance_vertex_setup_system.hpp"
#include "glad/glad.h"
#include "quartz/core/components/instance_component.hpp"
#include <cstddef>

void quartz::InstanceVertexSetupSystem::Update(bismuth::Registry& registry) {
    auto instanceView = registry.GetView<InstanceComponent, TransformComponent, MeshComponent>();

    for(auto [entity, instance, transform, mesh] : instanceView) {
        glGenVertexArrays(1, &mesh.VAO);
        glBindVertexArray(mesh.VAO);

        glGenBuffers(1, &mesh.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,4,GL_FLOAT,GL_FALSE,sizeof(InstanceComponent), (void*)0);
        glVertexAttribDivisor(3, 1);

    }
}