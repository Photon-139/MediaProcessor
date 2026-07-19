#pragma once
#include "effect.hpp"

class FloydSteinberg : public ImageEffect{
public:
    FloydSteinberg();
    void apply(const Image& src, Image& dst) const override;
};