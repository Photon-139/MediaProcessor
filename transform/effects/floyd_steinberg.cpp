#include "floyd_steinberg.hpp"
#include <algorithm>

FloydSteinberg::FloydSteinberg():ImageEffect("FloydSteinberg"){};

void FloydSteinberg::apply(const Image& src, Image& dst) const{
    auto index = [COLS=src.width, CH=src.channels](size_t r, size_t c){
        return (COLS*r + c)*CH;
    };

    std::vector<int> buffer(src.data.data(), src.data.data()+src.width*src.height*src.channels);
    for(size_t r = 0; r<src.height; ++r){
        for(size_t c = 0; c<src.width; ++c){
            size_t pixel_index = index(r, c);
            bool has_right = c+1<src.width;
            bool has_down = r+1<src.height;
            bool has_left = c>0;
            for(size_t ch = 0; ch<3; ++ch){
                int old_val = std::clamp(buffer[pixel_index+ch], 0, 255);
                int new_val = old_val>127 ? 255 : 0;

                dst.data[pixel_index+ch] = static_cast<unsigned char>(new_val);
                int error = old_val - new_val;


                if(has_right) buffer[index(r, c+1)+ch]+=error*7/16;

                if(has_down){
                    buffer[index(r+1, c)+ch]+=error*5/16;
                    if(has_left) buffer[index(r+1, c-1)+ch]+=error*3/16;
                    if(has_right) buffer[index(r+1, c+1)+ch]+=error*1/16;
                }
            }
            if(src.channels==4){
                dst.data[pixel_index+3] = src.data[pixel_index+3];
            }
        }
    }
    
}