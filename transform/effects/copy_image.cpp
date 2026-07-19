#include "copy_image.hpp"

void CopyBuffer::apply(const Image& src, Image& dst) const {
    for(size_t i = 0; i<src.width*src.height; ++i){
        dst.data[i*src.channels] = dst.data[i*src.channels];
        dst.data[i*src.channels+1] = dst.data[i*src.channels];
        dst.data[i*src.channels+2] = dst.data[i*src.channels];
        if(src.channels==4){
            dst.data[i*src.channels+3] = dst.data[i*src.channels];
        }
    }
}