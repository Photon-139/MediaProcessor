#include "pixelate.hpp"
#include <algorithm>

Pixelate::Pixelate(size_t size): ImageEffect("Pixelation"), block_size(size){}

void Pixelate::apply(const Image& src, Image& dest) const{
    validateDimensions(src, dest);

    size_t width = src.width;
    size_t height = src.height;
    size_t channels = src.channels;

    for(size_t i = 0; i<(width+block_size-1)/block_size; ++i){
        for(size_t j = 0; j<(height+block_size-1)/block_size; ++j){

            size_t x0 = block_size*i;
            size_t y0 = block_size*j;
            std::vector<float> sum(3, 0);
            
            for(size_t y = y0; y<std::min((y0+block_size), height); ++y){
                for(size_t x = x0; x<std::min((x0+block_size), width); ++x){
                    
                    int index = (x+width*y)*channels;
                    sum[0]+=src.data[index];
                    sum[1]+=src.data[index+1];
                    sum[2]+=src.data[index+2];
                }
            }
            long long block_area = block_size*block_size;
            if(x0+block_size>=width || y0+block_size>=height){
                block_area = std::min(block_size, width-x0) * std::min(block_size, height-y0);
            }
            for(size_t x = x0; x<std::min((x0+block_size), width); ++x){
                for(size_t y = y0; y<std::min((y0+block_size), height); ++y){
                    
                    int index = (x+width*y)*channels;
                    dest.data[index] = sum[0]/block_area;
                    dest.data[index+1] = sum[1]/block_area;
                    dest.data[index+2] = sum[2]/block_area;

                }
            }

        }
    }
}