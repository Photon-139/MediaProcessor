#pragma once
#include <string>
#include "../image.hpp"

class ImageEffect{
    protected:
        std::string effectName;
        void validateDimensions(const Image& src, const Image& dest) const;
    public:
        ImageEffect(const std::string& name);

        virtual ~ImageEffect() = default;
        
        virtual void apply(const Image& src, Image& dest) const = 0;

        const std::string& getEffectName() const;
};