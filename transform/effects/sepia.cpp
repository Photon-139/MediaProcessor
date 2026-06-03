#include "sepia.hpp"
#include <algorithm>

Sepia::Sepia(): Effect("Sepia"){}

void Sepia::apply(const Image& src, Image& dest) const{
    validateDimensions(src, dest);

    size_t width = src.width;
    size_t height = src.height;
    size_t pixel_count = width*height;
    size_t channels = src.channels;
    size_t index;
    for(size_t i = 0; i<pixel_count; ++i){
        index = i*channels;
        double r = src.data[index];
        double g = src.data[index+1];
        double b = src.data[index+2];
        dest.data[index] = static_cast<unsigned char>(std::clamp(r*0.393+g*0.769+b*0.189, 0.0, 255.0));
        dest.data[index+1] = static_cast<unsigned char>(std::clamp(r*0.349+g*0.686+b*0.168, 0.0, 255.0));
        dest.data[index+2] = static_cast<unsigned char>(std::clamp(r*0.272+g*0.534+b*0.131, 0.0, 255.0));
        if(channels==4){
            dest.data[index+3] = src.data[index+3];
        }
    }
}