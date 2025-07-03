#include "quartz/ui/systems/style_system.hpp"

void quartz::StyleSystem::Update(bismuth::Registry& registry) {
    auto styleView = registry.GetView<GuiObjectComponent, GuiMeshComponent>();

    for(auto [entity, object, mesh] : styleView) {
        glm::vec2 pos      = object.style.Get<glm::vec2>(Properties::position);
        unsigned int width = object.style.Get<unsigned int>(Properties::width);
        unsigned int height= object.style.Get<unsigned int>(Properties::height);

        mesh.vertices = {
            glm::vec3(pos.x,         pos.y,          object.zLayer),
            glm::vec3(pos.x + width, pos.y,          object.zLayer),
            glm::vec3(pos.x,         pos.y + height, object.zLayer),
            glm::vec3(pos.x + width, pos.y + height, object.zLayer),
        };
    }
}