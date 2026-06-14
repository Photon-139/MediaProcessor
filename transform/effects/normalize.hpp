#pragma once
#include "audio_effect.hpp"

class Normalize : public AudioEffect{
    public:
    Normalize();
    void apply(const Audio& src, Audio& dst) const override;
};