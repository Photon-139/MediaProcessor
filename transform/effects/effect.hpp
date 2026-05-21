#ifndef EFFECT_H
#define EFFECT_H

#include <string>
#include "../image.hpp"

class Effect{
    protected:
        std::string effectName;
        void validateDimensions(const Image& src, const Image& dest) const;
    public:
        Effect(const std::string& name);

        virtual ~Effect() = default;
        
        virtual void apply(const Image& src, Image& dest) const = 0;

        const std::string& getEffectName() const;
};

#endif