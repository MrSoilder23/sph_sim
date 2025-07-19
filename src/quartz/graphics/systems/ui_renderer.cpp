#include "quartz/graphics/systems/ui_renderer.hpp"

quartz::UiRendererSystem::UiRendererSystem() {
    mGuiProgram = shader::CreateGraphicsPipeline("./shaders/ui/gui_vert.glsl", "./shaders/ui/gui_frag.glsl");
    mUniformProjectionMatrix = shader::FindUniformLocation(mGuiProgram, "uProjectionMatrix");

    mTextProgram = shader::CreateGraphicsPipeline("./shaders/ui/text_vert.glsl", "./shaders/ui/text_frag.glsl");
    mUniformProjectionMatrix2 = shader::FindUniformLocation(mTextProgram, "uProjectionMatrix");
    mUniformFontAtlas         = shader::FindUniformLocation(mTextProgram, "uFontAtlas");
}

void quartz::UiRendererSystem::Update(bismuth::Registry& registry) {
    auto& guiCameraPool = registry.GetComponentPool<GuiCameraComponent>();
    auto& guiMeshPool   = registry.GetComponentPool<GuiMeshComponent>();
    auto& textMeshPool  = registry.GetComponentPool<TextMeshComponent>();
    
    auto& camera = guiCameraPool.GetDenseComponents()[0];

    glUseProgram(mGuiProgram);
    glUniformMatrix4fv(mUniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(camera.projectionMatrix));

    auto guiBegin = guiMeshPool.ComponentBegin();
    auto guiEnd   = guiMeshPool.ComponentEnd();

    for(auto mesh = guiBegin; mesh != guiEnd; ++mesh) {
        glBindVertexArray(mesh->VAO);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(mTextProgram);
    glUniformMatrix4fv(mUniformProjectionMatrix2, 1, GL_FALSE, glm::value_ptr(camera.projectionMatrix));

    glUniform1i(mUniformFontAtlas, 0); 

    auto textBegin = textMeshPool.ComponentBegin();
    auto textEnd   = textMeshPool.ComponentEnd();

    for(auto mesh = textBegin; mesh != textEnd; ++mesh) {
        glBindVertexArray(mesh->VAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->fontAtlas->textureID);

        glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_INT, (void*)0);
    }

}