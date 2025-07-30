#pragma once
// C++ standard libraries
#include <variant>

namespace quartz {

enum class Unit {
    Pixels, 
    Percent,
    Auto
};

struct Value {
    std::variant<float, unsigned int> value;
    Unit unit = Unit::Pixels;

    Value(unsigned int value = 0, Unit unit = Unit::Pixels) : value(value), unit(unit) {}
    Value(float value, Unit unit = Unit::Percent) : value(value), unit(unit) {}
};
}