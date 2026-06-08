#include "audio_effect.hpp"

AudioEffect::AudioEffect(std::string name) : effectName(name){}

const std::string& AudioEffect::getEffectName() const{
    return effectName;
}