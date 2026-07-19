#pragma once
#include "effect.hpp"
#include <unordered_map>

class OrderedDithering : public ImageEffect{
    private:
        struct ThresholdMatrix{
            std::vector<std::vector<int>> matrix;
            int constant_factor;
        };
        int n;
        ThresholdMatrix bayer_matrix;
        static ThresholdMatrix build_bayer_matrix(int n);
    public:
        OrderedDithering(int n);
        void apply(const Image& src, Image& dst) const override;
};