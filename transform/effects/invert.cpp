#include "invert.hpp"
#include <algorithm>

Invert::Invert(): ImageEffect("GrayScale"){}

void Invert::apply(const Image& src, Image& dest) const{
    validateDimensions(src, dest);

    size_t width = src.width;
    size_t height = src.height;
    size_t pixel_count = width*height;
    size_t channels = src.channels;
    size_t index;
    for(size_t i = 0; i<pixel_count; ++i){
        index = i*channels;
        dest.data[index] = 255-src.data[index];
        dest.data[index+1] = 255-src.data[index+1];
        dest.data[index+2] = 255-src.data[index+2];

        if(channels==4){
            dest.data[index+3] = src.data[index+3];
        }
    }
}