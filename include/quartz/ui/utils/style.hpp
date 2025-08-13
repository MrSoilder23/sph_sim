#pragma once
// C++ standard libraries
#include <variant>
#include <unordered_map>
#include <stdexcept>
#include <string>

// Third_party libraries
#include <glm/glm.hpp>

// Own libraries
#include "quartz/ui/utils/properties.hpp"
#include "quartz/ui/utils/layouts.hpp"
#include "quartz/ui/utils/units.hpp"

namespace quartz {

class Style {
    public:
        using StyleValue = std::variant<std::monostate, Layouts, std::string, glm::vec4, glm::vec2, Dimension>;
    
        void Set(Properties property, StyleValue value) {
            mData[property] = value;
        }
        
        inline bool Has(Properties property) const {
            auto it = mData.find(property);
            if(it == mData.end()) {
                return false;
            }

            return !std::holds_alternative<std::monostate>(it->second);
        }

        template<typename Type>
        Type Get(Properties property) {
            auto it = mData.find(property);
            if(it == mData.end()) {
                throw std::out_of_range("Property not found");
            }

            assert(std::holds_alternative<Type>(it->second) && "Wrong property variable type");

            return std::get<Type>(it->second);
        }

    private:
        std::unordered_map<Properties, StyleValue> mData;
};

}