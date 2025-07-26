#include "quartz/ui/systems/style_setup.hpp"

void quartz::StyleSetupSystem::Update(bismuth::Registry& registry) {
    auto objectView = registry.GetView<GuiObjectComponent, GuiMeshComponent>();
    auto textView   = registry.GetView<GuiObjectComponent, TextMeshComponent>();

    auto& objectPool = registry.GetComponentPool<GuiObjectComponent>();

    auto objectBegin = objectPool.ComponentBegin();
    auto objectEnd   = objectPool.ComponentEnd();

    for(auto it = objectBegin; it != objectEnd; ++it) {
        UpdateLayout(*it, objectPool);
    }

    for(auto [entity, object, mesh] : objectView) {
        glm::vec4 color     = (object.style.Has(Properties::color)) ? object.style.Get<glm::vec4>(Properties::color) : glm::vec4(1.0f);
        
        glm::vec2 pos       = GetStyleValue<glm::vec2>(object.style, Properties::position, glm::vec2(0.0f));
        unsigned int width  = GetStyleValue<unsigned int>(object.style, Properties::width, 10);
        unsigned int height = GetStyleValue<unsigned int>(object.style, Properties::height, 10);

        auto paddingLeft   = GetStyleValue<unsigned int>(object.style, Properties::padding_left, 0);
        auto paddingRight  = GetStyleValue<unsigned int>(object.style, Properties::padding_right, 0);
        auto paddingTop    = GetStyleValue<unsigned int>(object.style, Properties::padding_top, 0);
        auto paddingBottom = GetStyleValue<unsigned int>(object.style, Properties::padding_bottom, 0);

        mesh.vertices = {
            glm::vec3(pos.x - paddingLeft,          pos.y - paddingBottom,       object.zLayer),
            glm::vec3(pos.x + width + paddingRight, pos.y - paddingBottom,       object.zLayer),
            glm::vec3(pos.x - paddingLeft,          pos.y + height + paddingTop, object.zLayer),
            glm::vec3(pos.x + width + paddingRight, pos.y + height + paddingTop, object.zLayer),
        };

        mesh.colors = {
            color, color, color, color
        };
    }

    for(auto [entity, object, mesh] : textView) {
        if(!mesh.fontAtlas) {
            glm::vec4 color = GetStyleValue<glm::vec4>(object.style, Properties::color, glm::vec4(1.0f));

            std::string fontPath  = GetStyleValue<std::string>(object.style, Properties::font_family, "assets/fonts/arial.ttf");
            unsigned int fontSize = GetStyleValue<unsigned int>(object.style, Properties::font_size, 24);

            try {
                mesh.fontAtlas = &mFontManager.GetFont(fontPath, fontSize);
            } catch (const std::exception& e) {
                std::cerr << "Font load error: " << e.what() << std::endl;
                continue;
            }

            glm::vec2 pos = GetStyleValue<glm::vec2>(object.style, Properties::position, glm::vec2(0.0f));
            float x = pos.x;

            for(char c : mesh.content) {
                const Character& ch = mesh.fontAtlas->characters.at(c);
                float y = pos.y - (ch.size.y - ch.bearing.y);
                
                mesh.vertices.push_back(glm::vec3(x,             y,             object.zLayer));
                mesh.vertices.push_back(glm::vec3(x + ch.size.x, y,             object.zLayer));
                mesh.vertices.push_back(glm::vec3(x,             y + ch.size.y, object.zLayer));
                mesh.vertices.push_back(glm::vec3(x + ch.size.x, y + ch.size.y, object.zLayer));

                mesh.uv.push_back(glm::vec2(ch.uvMin.x, ch.uvMin.y));
                mesh.uv.push_back(glm::vec2(ch.uvMax.x, ch.uvMin.y));
                mesh.uv.push_back(glm::vec2(ch.uvMin.x, ch.uvMax.y));
                mesh.uv.push_back(glm::vec2(ch.uvMax.x, ch.uvMax.y));

                for(int i = 0; i < 4; i++) {
                    mesh.colors.push_back(color);
                }
                
                x += (ch.advance >> 6);
            }
        }
    }
}

// Private
void quartz::StyleSetupSystem::UpdateLayout(GuiObjectComponent& guiObject, bismuth::ComponentPool<GuiObjectComponent>& guiPool) {
    auto layoutType = GetStyleValue<Layouts>(guiObject.style, Properties::layout, Layouts::vertical);

    auto& children = guiObject.childrenIDs;
    auto position  = GetStyleValue<glm::vec2>(guiObject.style, Properties::position, glm::vec2(0.0f));

    unsigned int spacing = 0;

    for(auto IDs : children) {
        if(layoutType == Layouts::vertical) {
            auto& childrenObject = guiPool.GetComponent(IDs);
            auto height        = GetStyleValue<unsigned int>(childrenObject.style, Properties::height, 10);

            auto paddingTop    = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_top, 0);
            auto paddingBottom = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_bottom, 0);
            auto marginTop     = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_top, 0);
            auto marginBottom  = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_bottom, 0);

            childrenObject.style.Set(Properties::position, glm::vec2(position.x, position.y + spacing + paddingBottom + marginBottom));
            childrenObject.style.Set(Properties::width, guiObject.style.Get<unsigned int>(Properties::width));

            spacing += height + paddingTop + paddingBottom + marginTop + marginBottom;
            
        } else if(layoutType == Layouts::horizontal) {
            auto& childrenObject = guiPool.GetComponent(IDs);
            auto width        = GetStyleValue<unsigned int>(childrenObject.style, Properties::width, 10);

            auto paddingLeft  = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_left, 0);
            auto paddingRight = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_right, 0);
            auto marginLeft   = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_left, 0);
            auto marginRight  = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_right, 0);

            childrenObject.style.Set(Properties::position, glm::vec2(position.x + spacing + paddingLeft + marginLeft, position.y));
            childrenObject.style.Set(Properties::height, guiObject.style.Get<unsigned int>(Properties::height));

            spacing += width + paddingLeft + paddingRight + marginLeft + marginRight;
        }
    }

}