#include "quartz/ui/systems/style_system.hpp"

void quartz::StyleSystem::Update(bismuth::Registry& registry) {
    auto styleView = registry.GetView<GuiObjectComponent, GuiMeshComponent>();

    auto& objectPool = registry.GetComponentPool<GuiObjectComponent>();

    for(auto [entity, object, mesh] : styleView) {
        auto layoutType = (object.style.Has(Properties::layout)) ? object.style.Get<Layouts>(Properties::layout) : Layouts::vertical;

        if(layoutType == Layouts::vertical) {
            auto& children = object.childrenIDs;
            auto position  = GetStyleValue<glm::vec2>(object.style, Properties::position, glm::vec2(0.0f));

            unsigned int spacing = 0;

            for(auto IDs : children) {
                auto& childrenObject = objectPool.GetComponent(IDs);
                auto height        = childrenObject.style.Get<unsigned int>(Properties::height);

                auto paddingTop    = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_top, 0);
                auto paddingBottom = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_bottom, 0);
                auto marginTop     = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_top, 0);
                auto marginBottom  = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_bottom, 0);

                childrenObject.style.Set(Properties::position, glm::vec2(position.x, position.y + spacing + paddingBottom + marginBottom));
                spacing += height + paddingTop + paddingBottom + marginTop + marginBottom;
            }
        } else if(layoutType == Layouts::horizontal) {
            auto& children = object.childrenIDs;
            auto position  = GetStyleValue<glm::vec2>(object.style, Properties::position, glm::vec2(0.0f));

            unsigned int spacing = 0;

            for(auto IDs : children) {
                auto& childrenObject = objectPool.GetComponent(IDs);
                auto width        = childrenObject.style.Get<unsigned int>(Properties::width);

                auto paddingLeft  = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_left, 0);
                auto paddingRight = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_right, 0);
                auto marginLeft   = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_left, 0);
                auto marginRight  = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_right, 0);

                childrenObject.style.Set(Properties::position, glm::vec2(position.x + spacing + paddingLeft + marginLeft, position.y));
                spacing += width + paddingLeft + paddingRight + marginLeft + marginRight;
            }
        }
    }

    for(auto [entity, object, mesh] : styleView) {
        glm::vec4 color     = (object.style.Has(Properties::color)) ? object.style.Get<glm::vec4>(Properties::color) : glm::vec4(1.0f);
        
        glm::vec2 pos       = object.style.Get<glm::vec2>(Properties::position);
        unsigned int width  = object.style.Get<unsigned int>(Properties::width);
        unsigned int height = object.style.Get<unsigned int>(Properties::height);

        mesh.vertices = {
            glm::vec3(pos.x,         pos.y,          object.zLayer),
            glm::vec3(pos.x + width, pos.y,          object.zLayer),
            glm::vec3(pos.x,         pos.y + height, object.zLayer),
            glm::vec3(pos.x + width, pos.y + height, object.zLayer),
        };

        mesh.colors = {
            color, color, color, color, color, color
        };
    }
}