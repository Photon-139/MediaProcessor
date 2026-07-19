#pragma once
#include "effect.hpp"


struct DiffusionKernel{
    struct Target{int dr, dc, numerator;};
    std::vector<Target> targets;
    int denominator;
};

inline const DiffusionKernel FLOYD_STEINBERG = {
    {{0, 1, 7}, {1, -1, 3}, {1, 0, 5}, {1,1,1}},
    16
};

inline const DiffusionKernel ATKINSON = {
    {{0, 1, 1}, {0, 2, 1}, {1, -1, 1}, {1, 0, 1}, {1,1,1}, {2,0,1}},
    8
};

inline const DiffusionKernel JARVIS_JUDICE_NINKE = {
    {
        {0, 1, 7}, {0, 2, 5}, {1, -2, 3}, {1, -1, 5}, 
        {1, 0, 7}, {1, 1, 5}, {1, 2, 3}, {2, -2, 1},
        {2, -1, 3}, {2, 0, 5}, {2, 1, 3}, {2, 2, 1}
    },
    48
};

class FloydSteinberg : public ImageEffect{
private:
    DiffusionKernel kernel;
    int levels;
public:
    FloydSteinberg(DiffusionKernel kernel, int levels);
    void apply(const Image& src, Image& dst) const override;
};