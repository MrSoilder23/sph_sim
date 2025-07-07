#pragma once
// Own libraries
#include "./bismuth/registry.hpp"
#include "quartz/ui/components/gui_mesh.hpp"
#include "quartz/ui/components/gui_object_component.hpp"
#include "quartz/ui/utils/layouts.hpp"

namespace quartz {

class StyleSystem {
    public:
        void Update(bismuth::Registry& registry);

    private:
        template<typename Type>
        inline Type GetStyleValue(Style& style, Properties property, Type defaultValue) {
            return style.Has(property) ? style.Get<Type>(property) : defaultValue;
        }
};

}