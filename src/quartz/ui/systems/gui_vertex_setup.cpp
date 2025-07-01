#include "quartz/ui/systems/gui_vertex_setup.hpp"

void quartz::GuiVertexSetupSystem::Update(bismuth::Registry& registry) {
    auto styleView = registry.GetView<GuiObjectComponent, MeshComponent2>();

    const std::vector<GLuint> index = {
        2,0,1, 2,1,3
    };

    for(auto [entity, object, mesh] : styleView) {
        glGenVertexArrays(1, &mesh.VAO);
        glBindVertexArray(mesh.VAO);

        glGenBuffers(1, &mesh.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(glm::vec3), mesh.vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);

        glGenBuffers(1, &mesh.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
        glDisableVertexAttribArray(0);
    }
}