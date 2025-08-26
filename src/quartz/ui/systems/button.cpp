#include "quartz/ui/systems/button.hpp"

void quartz::ButtonSystem::Update(bismuth::Registry& registry) {
    auto buttonView = registry.GetView<GuiObjectComponent, ButtonComponent>();
    auto& mouse     = registry.GetSingleton<MouseStateComponent>();
    
    for(auto [entity, guiObject, button] : buttonView) {
        bool isHovered = CheckAABB(mouse.position, guiObject);
        
        if(isHovered && mouse.leftPressed) {
            if(button.onClick) {
                button.onClick();
            }
        }
    }
}

// Private
bool quartz::ButtonSystem::CheckAABB(const glm::vec2& point, GuiObjectComponent& object) {
    auto position = object.style.Get<DimensionVec2>(Properties::position);
    auto width    = object.style.Get<Dimension>(Properties::width).resolve(0u);
    auto height   = object.style.Get<Dimension>(Properties::height).resolve(0u);

    auto posX = position.x.resolve(0u);
    auto posY = position.y.resolve(0u);

    return point.x >= posX &&
           point.x <= posX + width &&
           point.y >= posY &&
           point.y <= posY + height;
}