#pragma once
#include "audio_effect.hpp"

class Reverse : public AudioEffect{
    public:
        Reverse();
        void apply(const Audio& src, Audio& dst) const override;
};