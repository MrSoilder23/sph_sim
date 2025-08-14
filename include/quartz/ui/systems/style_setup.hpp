#pragma once
// C++ standard libraries
#include <unordered_map>

// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/ui/components/gui_mesh.hpp"
#include "quartz/ui/components/gui_object_component.hpp"
#include "quartz/ui/components/text_mesh.hpp"
#include "quartz/ui/utils/layouts.hpp"
#include "quartz/ui/utils/font_manager.hpp"

namespace quartz {

class StyleSetupSystem {
    public:
        StyleSetupSystem(
            FontManager& fontManager,
            unsigned int screenWidth,
            unsigned int screenHeight
        ) : mFontManager(fontManager), mScreenWidth(screenWidth), mScreenHeight(screenHeight) {}

        void Update(bismuth::Registry& registry);

    private:
        template<typename Type>
        inline Type GetStyleValue(Style& style, Properties property, Type defaultValue) {
            return style.Has(property) ? style.Get<Type>(property) : defaultValue;
        }

        void ComputeSizes(
            bismuth::ComponentPool<GuiObjectComponent>& guiPool, 
            GuiObjectComponent                        & currentObject
        );

        void ComputeLayout(
            bismuth::ComponentPool<GuiObjectComponent>& guiPool,
            std::vector<bismuth::EntityID> const&       childrenIDs,
            bismuth::EntityID              const&       currentID
        );

    private:
        FontManager& mFontManager;

        unsigned int mScreenWidth;
        unsigned int mScreenHeight;
};

}