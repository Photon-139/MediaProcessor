#include "ordered_dithering.hpp"

OrderedDithering::OrderedDithering(int n) :ImageEffect("ordered_dithering"), n(n), bayer_matrix(build_bayer_matrix(n)) {}


OrderedDithering::ThresholdMatrix OrderedDithering::build_bayer_matrix(int n){
    if(n==2){
        return ThresholdMatrix{{{0, 2},{3, 1}}, 4};
    }
    auto sub_matrix = build_bayer_matrix(n/2);
    std::vector<std::vector<int>> res(n, std::vector<int>(n));
    for(auto i = 0; i<(n/2); ++i){
        for(auto j = 0; j<(n/2); ++j){
            res[i][j] = 4*sub_matrix.matrix[i][j];
            res[i][j+n/2] = 4*sub_matrix.matrix[i][j] + 2;
            res[i+n/2][j] = 4*sub_matrix.matrix[i][j] + 3;
            res[i+n/2][j+n/2] = 4*sub_matrix.matrix[i][j] + 1;
        }
    }
    return ThresholdMatrix{res, 4*sub_matrix.constant_factor};
}

void OrderedDithering::apply(const Image& src, Image& dst) const {
    auto pixel_count = src.width*src.height;
    auto channels = src.channels;
    for(size_t i = 0; i<pixel_count; ++i){
        auto r = i/src.width;
        auto c = i%src.width;
        auto mr = r%n;
        auto mc = c%n;

        auto matrix_value = bayer_matrix.matrix[mr][mc]*255;
        auto idx = i*channels;
        dst.data[idx] = (src.data[idx]*bayer_matrix.constant_factor>matrix_value ? 255 : 0);
        dst.data[idx+1] = (src.data[idx+1]*bayer_matrix.constant_factor>matrix_value ? 255 : 0);
        dst.data[idx+2] = (src.data[idx+2]*bayer_matrix.constant_factor>matrix_value ? 255 : 0);
        if(channels==4){
            dst.data[idx+3] = src.data[idx+3];
        }
    }
}