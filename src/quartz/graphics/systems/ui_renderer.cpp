#include "quartz/graphics/systems/ui_renderer.hpp"

quartz::UiRendererSystem::UiRendererSystem() {
    mProgram = shader::CreateGraphicsPipeline("./shaders/gui_vert.glsl", "./shaders/gui_frag.glsl");
    mUniformProjectionMatrix = shader::FindUniformLocation(mProgram, "uProjectionMatrix");
}

void quartz::UiRendererSystem::Update(bismuth::Registry& registry) {
    auto& guiCameraPool = registry.GetComponentPool<GuiCameraComponent>();
    auto& guiMeshPool   = registry.GetComponentPool<GuiMeshComponent>();
    
    auto& camera = guiCameraPool.GetDenseComponents()[0];

    glUseProgram(mProgram);
    glUniformMatrix4fv(mUniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(camera.projectionMatrix));

    auto guiBegin = guiMeshPool.ComponentBegin();
    auto guiEnd   = guiMeshPool.ComponentEnd();

    for(auto mesh = guiBegin; mesh != guiEnd; ++mesh) {
        glBindVertexArray(mesh->VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    }
}