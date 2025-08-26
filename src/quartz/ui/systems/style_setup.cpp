#include "quartz/ui/systems/style_setup.hpp"

void quartz::StyleSetupSystem::Update(bismuth::Registry& registry) {
    auto objectView = registry.GetView<GuiObjectComponent, GuiMeshComponent>();
    auto textView   = registry.GetView<GuiObjectComponent, TextMeshComponent>();

    auto& objectPool = registry.GetComponentPool<GuiObjectComponent>();

    std::unordered_map<bismuth::EntityID, std::vector<bismuth::EntityID>> childrenMap;
    std::vector<bismuth::EntityID> rootEntities;

    auto& denseIDs = objectPool.GetDenseEntities();
    for(bismuth::EntityID id : denseIDs) {
        auto& currentComponent = objectPool.GetComponent(id);

        if(currentComponent.parentID != bismuth::INVALID_INDEX) {
            childrenMap[currentComponent.parentID].push_back(id);
        } else {
            rootEntities.push_back(id);
        }
    }

    std::queue<bismuth::EntityID> processingQueue;
    for(auto rootID : rootEntities) {
        processingQueue.push(rootID);
    }

    while(!processingQueue.empty()) {
        bismuth::EntityID currentID = processingQueue.front();
        processingQueue.pop();

        auto& currentComponent = objectPool.GetComponent(currentID);
        ComputeSizes(objectPool, currentComponent);

        // Add children to the queue for processing
        if (childrenMap.find(currentID) != childrenMap.end()) {
            for (auto childID : childrenMap[currentID]) {
                processingQueue.push(childID);
            }
        }
    }

    for (auto rootID : rootEntities) {
        processingQueue.push(rootID);
    }

    while (!processingQueue.empty()) {
        bismuth::EntityID currentID = processingQueue.front();
        processingQueue.pop();

        if (childrenMap.find(currentID) != childrenMap.end()) {
            ComputeLayout(objectPool, childrenMap[currentID], currentID);
            
            // Add children to the queue for processing
            for (auto childID : childrenMap[currentID]) {
                processingQueue.push(childID);
            }
        }
    }
    for(auto [entity, object, mesh] : objectView) {
        glm::vec4 color     = GetStyleValue<glm::vec4>(object.style, Properties::background_color, glm::vec4(1.0f));
        
        auto pos            = GetStyleValue<DimensionVec2>(object.style, Properties::position, DimensionVec2{0u, 0u});
        unsigned int width  = GetStyleValue<Dimension>(object.style, Properties::width,  Dimension{10u, Unit::Pixels}).resolve(0);
        unsigned int height = GetStyleValue<Dimension>(object.style, Properties::height, Dimension{10u, Unit::Pixels}).resolve(0);

        auto paddingLeft   = GetStyleValue<Dimension>(object.style, Properties::padding_left,   Dimension{0u, Unit::Pixels}).resolve(0);
        auto paddingRight  = GetStyleValue<Dimension>(object.style, Properties::padding_right,  Dimension{0u, Unit::Pixels}).resolve(0);
        auto paddingTop    = GetStyleValue<Dimension>(object.style, Properties::padding_top,    Dimension{0u, Unit::Pixels}).resolve(0);
        auto paddingBottom = GetStyleValue<Dimension>(object.style, Properties::padding_bottom, Dimension{0u, Unit::Pixels}).resolve(0);

        auto posX = pos.x.resolve(0);
        auto posY = pos.y.resolve(0);

        mesh.vertices = {
            glm::vec3(posX - paddingLeft,          posY - paddingTop,             object.zLayer),
            glm::vec3(posX + width + paddingRight, posY - paddingTop,             object.zLayer),
            glm::vec3(posX - paddingLeft,          posY + height + paddingBottom, object.zLayer),
            glm::vec3(posX + width + paddingRight, posY + height + paddingBottom, object.zLayer),
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

            auto pos = GetStyleValue<DimensionVec2>(object.style, Properties::position, DimensionVec2{0u, 0u});
            float xPos = pos.x.resolve(0);
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
                float y = pos.y.resolve(0) - ch.bearing.y + mesh.fontAtlas->height;
                
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
    unsigned int defaultHeight = mScreenHeight;
    unsigned int defaultWidth  = mScreenWidth;

    Layouts layoutType = Layouts::absolute;
    
    if(currentObject.parentID != bismuth::INVALID_INDEX) {
        auto& parentObject = guiPool.GetComponent(currentObject.parentID);

        layoutType    = GetStyleValue<Layouts>(parentObject.style, Properties::layout, Layouts::vertical);
        
        defaultHeight = GetStyleValue<Dimension>(parentObject.style, Properties::height, Dimension{10u}).resolve(0);
        defaultWidth  = GetStyleValue<Dimension>(parentObject.style, Properties::width,  Dimension{10u}).resolve(0);
    }

    //                                                            Properties                  Default values (Pixels)
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

    auto fontSize = GetStyleValue<Dimension>(currentObject.style, Properties::font_size,      Dimension{24u}).resolve(defaultHeight);

    if(layoutType == Layouts::vertical) {
        width = defaultWidth - paddingL;
    } else if(layoutType == Layouts::horizontal) {
        height = defaultHeight - paddingT;
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
    
    currentObject.style.Set(Properties::font_size,      Dimension{fontSize});
}

void quartz::StyleSetupSystem::ComputeLayout(
    bismuth::ComponentPool<GuiObjectComponent>& guiPool,
    std::vector<bismuth::EntityID> const&       childrenIDs,
    bismuth::EntityID              const&       currentID
) {
    unsigned int defaultHeight = mScreenHeight;
    unsigned int defaultWidth  = mScreenWidth;

    auto& parentObject = guiPool.GetComponent(currentID);

    if(parentObject.parentID != bismuth::INVALID_INDEX) {
        auto& parentObject2 = guiPool.GetComponent(parentObject.parentID);

        auto height = GetStyleValue<Dimension>(parentObject2.style, Properties::height, Dimension{10u, Unit::Pixels}).resolve(0);
        auto width  = GetStyleValue<Dimension>(parentObject2.style, Properties::width,  Dimension{10u, Unit::Pixels}).resolve(0);

        defaultHeight = height;
        defaultWidth  = width;
    }

    auto layoutType = GetStyleValue<Layouts>(parentObject.style, Properties::layout, Layouts::vertical);
    auto position   = GetStyleValue<DimensionVec2>(parentObject.style, Properties::position, DimensionVec2{0u, 0u});

    unsigned int posX = position.x.resolve(defaultWidth);
    unsigned int posY = position.y.resolve(defaultHeight);

    parentObject.style.Set(Properties::position, DimensionVec2{posX, posY});

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

            childrenObject.style.Set(Properties::position, 
                DimensionVec2{posX + paddingLeft, 
                              posY + spacing + paddingTop + marginTop});

            spacing += height + paddingTop + paddingBottom + marginTop + marginBottom;
            
        } else if(layoutType == Layouts::horizontal) {
            auto& childrenObject = guiPool.GetComponent(IDs);
            auto width        = GetStyleValue<Dimension>(childrenObject.style, Properties::width,         Dimension{10u, Unit::Pixels}).resolve(0);

            auto paddingLeft  = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_left,  Dimension{0u, Unit::Pixels}).resolve(0);
            auto paddingRight = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_right, Dimension{0u, Unit::Pixels}).resolve(0);
            auto marginLeft   = GetStyleValue<Dimension>(childrenObject.style, Properties::margin_left,   Dimension{0u, Unit::Pixels}).resolve(0);
            auto marginRight  = GetStyleValue<Dimension>(childrenObject.style, Properties::margin_right,  Dimension{0u, Unit::Pixels}).resolve(0);

            auto paddingTop = GetStyleValue<Dimension>(childrenObject.style, Properties::padding_top, Dimension{0u, Unit::Pixels}).resolve(0);

            childrenObject.style.Set(Properties::position, 
                DimensionVec2{posX + spacing + paddingLeft + marginLeft, 
                              posY + paddingTop});

            spacing += width + paddingLeft + paddingRight + marginLeft + marginRight;
        }
    }

}