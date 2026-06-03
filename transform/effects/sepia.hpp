#pragma once

#include "effect.hpp"

class Sepia : public Effect{
    public:
        Sepia();
        void apply(const Image& src, Image& dest) const override;
};