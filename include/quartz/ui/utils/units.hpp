#pragma once
// C++ standard libraries
#include <variant>

namespace quartz {

enum class Unit {
    Pixels, 
    Percent,
    Auto
};

struct Dimension {
    std::variant<float, unsigned int> value;
    Unit unit = Unit::Pixels;

    Dimension(unsigned int val, Unit u = Unit::Pixels) : value(val), unit(u) {}
    Dimension(float val, Unit u = Unit::Percent) : value(val), unit(u) {}

    unsigned int resolve(unsigned int parent) const {
        switch (unit) {
        case Unit::Pixels:
            return std::holds_alternative<unsigned int>(value)
                ? std::get<unsigned int>(value)
                : static_cast<unsigned int>(std::get<float>(value));

        case Unit::Percent:
            return std::holds_alternative<float>(value)
                ? (static_cast<unsigned int>(std::get<float>(value) * parent / 100.0f))
                : (static_cast<unsigned int>(std::get<unsigned int>(value)) * parent / 100.0f);

        case Unit::Auto:
            return parent;
            
        default:
            return 0u;
        }
    }
};

}