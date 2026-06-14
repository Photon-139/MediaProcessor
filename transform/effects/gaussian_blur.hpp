#pragma once

#include "effect.hpp"

class GaussianBlur : public ImageEffect{
    private:
        int radius;
    public:
        GaussianBlur(int rad);
        void apply(const Image& src, Image& dest) const override;
};