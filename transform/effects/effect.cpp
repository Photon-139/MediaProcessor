#include "effect.hpp"

#include <stdexcept>

Effect::Effect(const std::string& name): effectName(name) {}

const std::string& Effect::getEffectName() const {
    return effectName;
}

void Effect::validateDimensions(const Image& src, const Image& dest) const {
    if (src.width != dest.width || src.height != dest.height || src.channels != dest.channels) {
        throw std::runtime_error(
            "Source and destination image dimensions do not match"
        );
    }
}