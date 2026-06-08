#pragma once
#include <string>
#include "../audio.hpp"

class AudioEffect{
    protected:
        std::string effectName;
    public:
        AudioEffect(std::string name);
        virtual ~AudioEffect() = default;
        
        virtual void apply(const Audio& src, Audio& dest) const = 0;

        const std::string& getEffectName() const;
};