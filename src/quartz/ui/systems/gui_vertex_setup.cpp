#include "quartz/ui/systems/gui_vertex_setup.hpp"

void quartz::GuiVertexSetupSystem::Update(bismuth::Registry& registry) {
    auto& guiMeshPool = registry.GetComponentPool<GuiMeshComponent>();

    const std::vector<GLuint> index = {
        2,0,1, 2,1,3
    };
    
    auto guiBegin = guiMeshPool.ComponentBegin();
    auto guiEnd   = guiMeshPool.ComponentEnd();
    
    for(auto mesh = guiBegin; mesh != guiEnd; ++mesh) {
        if(mesh->VAO != 0) {
            continue;
        }

        glGenVertexArrays(1, &mesh->VAO);
        glBindVertexArray(mesh->VAO);

        glGenBuffers(1, &mesh->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), mesh->vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &mesh->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

        glGenBuffers(1, &mesh->colorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->colorVBO);
        glBufferData(GL_ARRAY_BUFFER, mesh->colors.size() * sizeof(glm::vec4), mesh->colors.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindVertexArray(0);
    }
}