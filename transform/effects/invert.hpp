#pragma once

#include "effect.hpp"

class Invert : public ImageEffect{
    public:
        Invert();
        void apply(const Image& src, Image& dest) const override;
};