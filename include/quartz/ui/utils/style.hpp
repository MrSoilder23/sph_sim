#pragma once
// C++ standard libraries
#include <variant>
#include <unordered_map>
#include <stdexcept>

// Third_party libraries
#include <glm/glm.hpp>

// Own libraries
#include "quartz/ui/utils/properties.hpp"

namespace quartz {

class Style {
    public:
        using StyleValue = std::variant<std::monostate, glm::vec2, unsigned int, int>;
    
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

            return std::get<Type>(it->second);
        }

    private:
        std::unordered_map<Properties, StyleValue> mData;
};

}