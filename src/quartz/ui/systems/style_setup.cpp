#include "quartz/ui/systems/style_setup.hpp"

void quartz::StyleSetupSystem::Update(bismuth::Registry& registry) {
    auto objectView = registry.GetView<GuiObjectComponent, GuiMeshComponent>();
    auto textView   = registry.GetView<GuiObjectComponent, TextMeshComponent>();

    auto& objectPool = registry.GetComponentPool<GuiObjectComponent>();

    std::unordered_map<bismuth::EntityID, std::vector<bismuth::EntityID>> childrenMap;
    auto& denseIDs = objectPool.GetDenseEntities();
    for(bismuth::EntityID id : denseIDs) {
        auto& currentComponent = objectPool.GetComponent(id);

        if(currentComponent.parentID != bismuth::INVALID_INDEX) {
            childrenMap[currentComponent.parentID].push_back(id);
        }
    }

    auto beginIt = objectPool.ComponentBegin();
    auto endIt   = objectPool.ComponentEnd();
    for(auto it = beginIt; it != endIt; ++it) {
        ComputeSizes(objectPool, *it);
    }

    for(auto& [parentID, children] : childrenMap) {
        ComputeLayout(objectPool, children, parentID);
    }

    for(auto [entity, object, mesh] : objectView) {
        glm::vec4 color     = GetStyleValue<glm::vec4>(object.style, Properties::background_color, glm::vec4(1.0f));
        
        glm::vec2 pos       = GetStyleValue<glm::vec2>(object.style, Properties::position, glm::vec2(0.0f));
        unsigned int width  = GetStyleValue<Dimension>(object.style, Properties::width,  Dimension{10u, Unit::Pixels}).resolve(0);
        unsigned int height = GetStyleValue<Dimension>(object.style, Properties::height, Dimension{10u, Unit::Pixels}).resolve(0);

        auto paddingLeft   = GetStyleValue<Dimension>(object.style, Properties::padding_left,   Dimension{0u, Unit::Pixels}).resolve(0);
        auto paddingRight  = GetStyleValue<Dimension>(object.style, Properties::padding_right,  Dimension{0u, Unit::Pixels}).resolve(0);
        auto paddingTop    = GetStyleValue<Dimension>(object.style, Properties::padding_top,    Dimension{0u, Unit::Pixels}).resolve(0);
        auto paddingBottom = GetStyleValue<Dimension>(object.style, Properties::padding_bottom, Dimension{0u, Unit::Pixels}).resolve(0);

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
            unsigned int fontSize = GetStyleValue<Dimension>(object.style, Properties::font_size, Dimension{24u, Unit::Pixels}).resolve(0);

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
                float y = pos.y - ch.bearing.y + mesh.fontAtlas->height;
                
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
void quartz::StyleSetupSystem::ComputeSizes(
    bismuth::ComponentPool<GuiObjectComponent>& guiPool, 
    GuiObjectComponent                        & currentObject
) {
    unsigned int defaultHeight = 0;
    unsigned int defaultWidth  = 0;

    Layouts layoutType = Layouts::absolute;
    
    if(currentObject.parentID != bismuth::INVALID_INDEX) {
        auto& parentObject = guiPool.GetComponent(currentObject.parentID);

        layoutType    = GetStyleValue<Layouts>(parentObject.style, Properties::layout, Layouts::vertical);
        
        defaultHeight = GetStyleValue<Dimension>(parentObject.style, Properties::height, Dimension{10u}).resolve(0);
        defaultWidth  = GetStyleValue<Dimension>(parentObject.style, Properties::width,  Dimension{10u}).resolve(0);
    }

    auto height   = GetStyleValue<Dimension>(currentObject.style, Properties::height,         Dimension{10u}).resolve(defaultHeight);
    auto width    = GetStyleValue<Dimension>(currentObject.style, Properties::width,          Dimension{10u}).resolve(defaultWidth);
    
    auto paddingR = GetStyleValue<Dimension>(currentObject.style, Properties::padding_right,  Dimension{0u}).resolve(defaultWidth);
    auto paddingL = GetStyleValue<Dimension>(currentObject.style, Properties::padding_left,   Dimension{0u}).resolve(defaultWidth);
    auto paddingT = GetStyleValue<Dimension>(currentObject.style, Properties::padding_top,    Dimension{0u}).resolve(defaultHeight);
    auto paddingB = GetStyleValue<Dimension>(currentObject.style, Properties::padding_bottom, Dimension{0u}).resolve(defaultHeight);

    auto marginR  = GetStyleValue<Dimension>(currentObject.style, Properties::margin_right,   Dimension{0u}).resolve(defaultWidth);
    auto marginL  = GetStyleValue<Dimension>(currentObject.style, Properties::margin_left,    Dimension{0u}).resolve(defaultWidth);
    auto marginT  = GetStyleValue<Dimension>(currentObject.style, Properties::margin_top,     Dimension{0u}).resolve(defaultHeight);
    auto marginB  = GetStyleValue<Dimension>(currentObject.style, Properties::margin_bottom,  Dimension{0u}).resolve(defaultHeight);

    if(layoutType == Layouts::vertical) {
        width = defaultWidth - paddingL;
    } else if(layoutType == Layouts::horizontal) {
        height = defaultHeight - paddingB;
    }

    currentObject.style.Set(Properties::height, Dimension{height});
    currentObject.style.Set(Properties::width,  Dimension{width});

    // Setting values to pixels, because they might be in different unit which requires parent.
    currentObject.style.Set(Properties::padding_right,  Dimension{paddingR});
    currentObject.style.Set(Properties::padding_left,   Dimension{paddingL});
    currentObject.style.Set(Properties::padding_top,    Dimension{paddingT});
    currentObject.style.Set(Properties::padding_bottom, Dimension{paddingB});

    currentObject.style.Set(Properties::margin_right,   Dimension{marginR});
    currentObject.style.Set(Properties::margin_left,    Dimension{marginL});
    currentObject.style.Set(Properties::margin_top,     Dimension{marginT});
    currentObject.style.Set(Properties::margin_bottom,  Dimension{marginB});
}

void quartz::StyleSetupSystem::ComputeLayout(
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
            auto height        = GetStyleValue<Dimension>(childrenObject.style, Properties::height,         Dimension{10u, Unit::Pixels}).resolve(0);

            auto paddingTop    = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_top,    Dimension{0u, Unit::Pixels}).resolve(0);
            auto paddingBottom = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_bottom, Dimension{0u, Unit::Pixels}).resolve(0);
            auto marginTop     = GetStyleValue<Dimension>(childrenObject.style, Properties::margin_top,     Dimension{0u, Unit::Pixels}).resolve(0);
            auto marginBottom  = GetStyleValue<Dimension>(childrenObject.style, Properties::margin_bottom,  Dimension{0u, Unit::Pixels}).resolve(0);

            auto paddingLeft = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_left,     Dimension{0u, Unit::Pixels}).resolve(0);

            childrenObject.style.Set(Properties::position, glm::vec2(position.x + paddingLeft, position.y + spacing + paddingBottom + marginBottom));

            spacing += height + paddingTop + paddingBottom + marginTop + marginBottom;
            
        } else if(layoutType == Layouts::horizontal) {
            auto& childrenObject = guiPool.GetComponent(IDs);
            auto width        = GetStyleValue<Dimension>(childrenObject.style, Properties::width,         Dimension{10u, Unit::Pixels}).resolve(0);

            auto paddingLeft  = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_left,  Dimension{0u, Unit::Pixels}).resolve(0);
            auto paddingRight = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_right, Dimension{0u, Unit::Pixels}).resolve(0);
            auto marginLeft   = GetStyleValue<Dimension>(childrenObject.style, Properties::margin_left,   Dimension{0u, Unit::Pixels}).resolve(0);
            auto marginRight  = GetStyleValue<Dimension>(childrenObject.style, Properties::margin_right,  Dimension{0u, Unit::Pixels}).resolve(0);

            auto paddingBottom = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_bottom, Dimension{0u, Unit::Pixels}).resolve(0);

            childrenObject.style.Set(Properties::position, glm::vec2(position.x + spacing + paddingLeft + marginLeft, position.y + paddingBottom));

            spacing += width + paddingLeft + paddingRight + marginLeft + marginRight;
        }
    }

}