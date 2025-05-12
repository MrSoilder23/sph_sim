#include "./quartz/graphics/instanced_renderer_system.hpp"
#include "quartz/core/components/instance_component.hpp"
#include "quartz/core/components/sphere_component.hpp"

void quartz::InstancedRendererSystem::Init(GLuint shaderProgram) {
    glGenVertexArrays(1, &mDummyVAO);
    glGenBuffers(1, &mInstanceVBO);

    mShaderProgram = shaderProgram;
}   

void quartz::InstancedRendererSystem::Update(bismuth::Registry& registry) {
    const auto cameraView = registry.GetView<CameraComponent, TransformComponent>();

    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    const auto& sphereData = spherePool.GetDenseComponents();

    glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(glm::vec4)*sphereData.size(),
        sphereData.data(),
        GL_STREAM_DRAW  
    );

    glUseProgram(mShaderProgram);
    glBindVertexArray(mDummyVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec4),
        (void*)0
    );
    glVertexAttribDivisor(0,1);

    GLint projectionMatrix = shader::FindUniformLocation(mShaderProgram, "uProjectionMatrix");
    GLint cameraPosition = shader::FindUniformLocation(mShaderProgram, "uCameraPosition");
    for(const auto& [entity, camera, transform] : cameraView) {
        glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, glm::value_ptr(camera.viewProjection));
        glUniformMatrix4fv(cameraPosition, 1, GL_FALSE, glm::value_ptr(transform.position));
    }

    glDrawArraysInstanced(GL_POINTS, 0, 1,sphereData.size());

}