#pragma once

#include "effect.hpp"

class Pixelate : public ImageEffect{
    private:
        size_t block_size;
    public:
        Pixelate(size_t size);
        void apply(const Image& src, Image& dest) const override;
};