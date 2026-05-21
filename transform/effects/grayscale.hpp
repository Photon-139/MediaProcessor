#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include "effect.hpp"

class GrayScale : Effect{
    public:
        GrayScale();
        void apply(const Image& src, Image& dest) const override;
};

#endif