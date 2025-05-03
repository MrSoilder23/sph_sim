#include "./quartz/graphics/instanced_renderer_system.hpp"

void quartz::InstancedRendererSystem::Init(GLuint shaderProgram) {
    glGenVertexArrays(1, &mDummyVAO);
    glGenBuffers(1, &mInstanceVBO);

    mShaderProgram = shaderProgram;
}   

void quartz::InstancedRendererSystem::Update(bismuth::Registry& registry) {
    const auto cameraView = registry.GetView<CameraComponent, TransformComponent>();
    auto modelView = registry.GetView<InstanceComponent, SphereComponent>();

    float sphereRadius;
    std::vector<glm::vec4> instances;
    for(auto [entities, _, sphere] : modelView) {
        instances.push_back(sphere.positionAndRadius);
        sphereRadius = sphere.positionAndRadius.w;
    }

    if(instances.empty()) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(glm::vec4)*instances.size(),
        instances.data(),
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
    for(auto [entity, camera, transform] : cameraView) {
        glUniformMatrix4fv(projectionMatrix, 1, GL_FALSE, glm::value_ptr(camera.viewProjection));
        glUniformMatrix4fv(cameraPosition, 1, GL_FALSE, glm::value_ptr(transform.position));
    }

    glDrawArraysInstanced(GL_POINTS, 0, 32*32,instances.size());

}