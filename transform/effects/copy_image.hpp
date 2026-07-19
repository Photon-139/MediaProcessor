#pragma once
#include "effect.hpp"

class CopyBuffer : public ImageEffect{
public:
    void apply(const Image& src, Image& dst) const override;
};