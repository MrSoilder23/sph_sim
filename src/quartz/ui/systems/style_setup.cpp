#include "quartz/ui/systems/style_setup.hpp"

void quartz::StyleSetupSystem::Update(bismuth::Registry& registry) {
    auto objectView = registry.GetView<GuiObjectComponent, GuiMeshComponent>();
    auto textView   = registry.GetView<GuiObjectComponent, TextMeshComponent>();

    auto& objectPool = registry.GetComponentPool<GuiObjectComponent>();

    std::unordered_map<bismuth::EntityID, std::vector<bismuth::EntityID>> childrenMap;
    auto& denseIDs = objectPool.GetDenseEntities();
    for(int i = 0; i < denseIDs.size(); i++) {
        auto& currentID = denseIDs[i];
        auto& currentComponent = objectPool.GetComponent(currentID);

        if(currentComponent.parentID != bismuth::INVALID_INDEX) {
            childrenMap[currentComponent.parentID].push_back(currentID);
        }
    }

    for(auto& [parentID, children] : childrenMap) {
        UpdateLayout(objectPool, children, parentID);
    }

    for(auto [entity, object, mesh] : objectView) {
        glm::vec4 color     = GetStyleValue<glm::vec4>(object.style, Properties::background_color, glm::vec4(1.0f));
        
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
            float xPos = pos.x;
            FT_UInt prevGlyphIndex = 0;

            for(char c : mesh.content) {
                if(c == ' ') {
                    xPos += fontSize * 0.3f;
                    continue;
                }

                const Character& ch = mesh.fontAtlas->characters.at(c);
                if(prevGlyphIndex) {
                    FT_Vector kerning;
                    FT_Get_Kerning(
                        mesh.fontAtlas->ftFace, 
                        prevGlyphIndex,
                        ch.glyphIndex,
                        FT_KERNING_DEFAULT,
                        &kerning
                    );
                    xPos += (kerning.x >> 6);
                }

                float x = xPos + ch.bearing.x;
                float y = pos.y - (ch.size.y - ch.bearing.y) + (mesh.fontAtlas->height * 0.3f);
                
                mesh.vertices.push_back(glm::vec3(x,             y,             object.zLayer + 0.001));
                mesh.vertices.push_back(glm::vec3(x + ch.size.x, y,             object.zLayer + 0.001));
                mesh.vertices.push_back(glm::vec3(x,             y + ch.size.y, object.zLayer + 0.001));
                mesh.vertices.push_back(glm::vec3(x + ch.size.x, y + ch.size.y, object.zLayer + 0.001));

                mesh.uv.push_back(glm::vec2(ch.uvMin.x, ch.uvMin.y));
                mesh.uv.push_back(glm::vec2(ch.uvMax.x, ch.uvMin.y));
                mesh.uv.push_back(glm::vec2(ch.uvMin.x, ch.uvMax.y));
                mesh.uv.push_back(glm::vec2(ch.uvMax.x, ch.uvMax.y));

                for(int i = 0; i < 4; i++) {
                    mesh.colors.push_back(color);
                }
                
                xPos += (ch.advance >> 6);
                prevGlyphIndex = ch.glyphIndex;
            }
        }
    }
}

// Private
void quartz::StyleSetupSystem::UpdateLayout(
    bismuth::ComponentPool<GuiObjectComponent>& guiPool,
    std::vector<bismuth::EntityID> const&       childrenIDs,
    bismuth::EntityID              const&       currentID
) {
    auto& parentObject = guiPool.GetComponent(currentID);

    auto layoutType = GetStyleValue<Layouts>(parentObject.style, Properties::layout, Layouts::vertical);
    auto position   = GetStyleValue<glm::vec2>(parentObject.style, Properties::position, glm::vec2(0.0f));

    unsigned int spacing = 0;

    for(auto IDs : childrenIDs) {
        if(layoutType == Layouts::vertical) {
            auto& childrenObject = guiPool.GetComponent(IDs);
            auto height        = GetStyleValue<unsigned int>(childrenObject.style, Properties::height, 10);

            auto paddingTop    = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_top, 0);
            auto paddingBottom = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_bottom, 0);
            auto marginTop     = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_top, 0);
            auto marginBottom  = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_bottom, 0);

            auto paddingLeft = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_left, 0);

            childrenObject.style.Set(Properties::position, glm::vec2(position.x + paddingLeft, position.y + spacing + paddingBottom + marginBottom));
            childrenObject.style.Set(Properties::width, parentObject.style.Get<unsigned int>(Properties::width) - paddingLeft);

            spacing += height + paddingTop + paddingBottom + marginTop + marginBottom;
            
        } else if(layoutType == Layouts::horizontal) {
            auto& childrenObject = guiPool.GetComponent(IDs);
            auto width        = GetStyleValue<unsigned int>(childrenObject.style, Properties::width, 10);

            auto paddingLeft  = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_left, 0);
            auto paddingRight = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_right, 0);
            auto marginLeft   = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_left, 0);
            auto marginRight  = GetStyleValue<unsigned int>(childrenObject.style, Properties::margin_right, 0);

            auto paddingBottom = GetStyleValue<unsigned int>(childrenObject.style, Properties::padding_bottom, 0);

            childrenObject.style.Set(Properties::position, glm::vec2(position.x + spacing + paddingLeft + marginLeft, position.y + paddingBottom));
            childrenObject.style.Set(Properties::height, parentObject.style.Get<unsigned int>(Properties::height) - paddingBottom);

            spacing += width + paddingLeft + paddingRight + marginLeft + marginRight;
        }
    }

}