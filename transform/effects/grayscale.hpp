#pragma once

#include "effect.hpp"

class GrayScale : public ImageEffect{
    public:
        GrayScale();
        void apply(const Image& src, Image& dest) const override;
};