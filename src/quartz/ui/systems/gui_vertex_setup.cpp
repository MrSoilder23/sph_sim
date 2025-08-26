#include "quartz/ui/systems/gui_vertex_setup.hpp"

void quartz::GuiVertexSetupSystem::Update(bismuth::Registry& registry) {
    auto& guiMeshPool  = registry.GetComponentPool<GuiMeshComponent>();
    auto& textMeshPool = registry.GetComponentPool<TextMeshComponent>();

    const std::vector<GLuint> index = {
        0,2,3, 0,3,1
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &mesh->VBOcolor);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VBOcolor);
        glBufferData(GL_ARRAY_BUFFER, mesh->colors.size() * sizeof(glm::vec4), mesh->colors.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindVertexArray(0);
    }

    auto textBegin = textMeshPool.ComponentBegin();
    auto textEnd   = textMeshPool.ComponentEnd();

    for(auto mesh = textBegin; mesh != textEnd; ++mesh) {
        if(mesh->VAO != 0) {
            continue;
        }
        
        std::vector<GLuint> tempIndex = {
            0,2,3, 0,3,1
        };
        std::vector<GLuint> textIndex;
        for(int i = 0; i < mesh->vertices.size()/4; i++) {
            textIndex.insert(textIndex.end(), tempIndex.begin(), tempIndex.end());
            for(int s = 0; s < tempIndex.size(); s++) {
                tempIndex[s] += 4;
            }
        }
        mesh->numIndices = textIndex.size();
        
        size_t vertexSize = mesh->vertices.size() * sizeof(glm::vec3);
        size_t uvSize = mesh->uv.size() * sizeof(glm::vec2);

        glGenVertexArrays(1, &mesh->VAO);
        glBindVertexArray(mesh->VAO);

        glGenBuffers(1, &mesh->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexSize + uvSize, nullptr, GL_DYNAMIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, mesh->vertices.data());
        glBufferSubData(GL_ARRAY_BUFFER, vertexSize, uvSize, mesh->uv.data());

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)vertexSize);

        glGenBuffers(1, &mesh->EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndex.size() * sizeof(GLuint), textIndex.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &mesh->VBOcolor);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VBOcolor);
        glBufferData(GL_ARRAY_BUFFER, mesh->colors.size() * sizeof(glm::vec4), mesh->colors.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindVertexArray(0);
    }
}