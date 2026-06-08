#include "effect.hpp"

#include <stdexcept>

ImageEffect::ImageEffect(const std::string& name): effectName(name) {}

const std::string& ImageEffect::getEffectName() const {
    return effectName;
}

void ImageEffect::validateDimensions(const Image& src, const Image& dest) const {
    if (src.width != dest.width || src.height != dest.height || src.channels != dest.channels) {
        throw std::runtime_error(
            "Source and destination image dimensions do not match"
        );
    }
}