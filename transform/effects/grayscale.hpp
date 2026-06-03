#pragma once

#include "effect.hpp"

class GrayScale : public Effect{
    public:
        GrayScale();
        void apply(const Image& src, Image& dest) const override;
};