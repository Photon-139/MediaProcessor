#include "grayscale.hpp"
#include <algorithm>

GrayScale::GrayScale(): Effect("GrayScale"){}

void GrayScale::apply(const Image& src, Image& dest) const{
    validateDimensions(src, dest);

    size_t width = src.width;
    size_t height = src.height;
    size_t pixel_count = width*height;
    size_t channels = src.channels;
    size_t index;
    for(size_t i = 0; i<pixel_count; ++i){
        index = i*channels;
        unsigned char r = src.data[index];
        unsigned char g = src.data[index+1];
        unsigned char b = src.data[index+2];
        unsigned char gray = static_cast<unsigned char>(0.299f*r + 0.587f*g + 0.114f*b);
        
        dest.data[index] = gray;
        dest.data[index+1] = gray;
        dest.data[index+2] = gray;
        if(channels==4){
            dest.data[index+3] = src.data[index+3];
        }
    }
}