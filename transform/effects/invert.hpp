#pragma once

#include "effect.hpp"

class Invert : public Effect{
    public:
        Invert();
        void apply(const Image& src, Image& dest) const override;
};