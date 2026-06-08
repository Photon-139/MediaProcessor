#pragma once

#include "effect.hpp"

class Sepia : public ImageEffect{
    public:
        Sepia();
        void apply(const Image& src, Image& dest) const override;
};