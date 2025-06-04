#include "./quartz/graphics/instanced_renderer_system.hpp"
#include "quartz/core/components/instance_component.hpp"
#include "quartz/core/components/sphere_component.hpp"

void quartz::InstancedRendererSystem::Init(GLuint shaderProgram) {
    glGenVertexArrays(1, &mDummyVAO);
    glGenBuffers(1, &mInstanceVBO);

    mShaderProgram = shaderProgram;

    mUniformProjectionMatrix = shader::FindUniformLocation(mShaderProgram, "uProjectionMatrix");
    mUniformCameraPosition   = shader::FindUniformLocation(mShaderProgram, "uCameraPosition");
}   

void quartz::InstancedRendererSystem::Update(bismuth::Registry& registry) {
    const auto cameraView = registry.GetView<CameraComponent, TransformComponent>();

    auto& spherePool = registry.GetComponentPool<SphereComponent>();
    const auto& sphereData = spherePool.GetDenseComponents();
    const size_t& sphereSize = sphereData.size();

    glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
    if(mDataSize != sphereSize) {
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(glm::vec4)*sphereSize,
            nullptr,
            GL_STREAM_DRAW  
        );

        mDataSize = sphereSize;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4)*sphereSize, sphereData.data());


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

    for(const auto& [entity, camera, transform] : cameraView) {
        glUniformMatrix4fv(mUniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(camera.viewProjection));
        glUniformMatrix4fv(mUniformCameraPosition, 1, GL_FALSE, glm::value_ptr(transform.position));
    }

    glDrawArraysInstanced(GL_POINTS, 0, 1,sphereData.size());

}