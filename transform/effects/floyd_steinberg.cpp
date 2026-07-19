#include "floyd_steinberg.hpp"
#include <algorithm>

FloydSteinberg::FloydSteinberg(DiffusionKernel kernel, int levels):ImageEffect("FloydSteinberg"), kernel(std::move(kernel)), levels(levels){};

void FloydSteinberg::apply(const Image& src, Image& dst) const{
    auto index = [COLS=src.width, CH=src.channels](size_t r, size_t c){
        return (COLS*r + c)*CH;
    };

    std::vector<int> buffer(src.data.data(), src.data.data()+src.width*src.height*src.channels);
    int safe_steps = std::max(2, levels);
    int step = 255/(safe_steps-1);
    for(size_t r = 0; r<src.height; ++r){
        for(size_t c = 0; c<src.width; ++c){
            size_t pixel_index = index(r, c);
            for(size_t ch = 0; ch<3; ++ch){
                int old_val = std::clamp(buffer[pixel_index+ch], 0, 255);
                int new_val = std::clamp(((old_val)/step)*step, 0, 255);

                dst.data[pixel_index+ch] = static_cast<unsigned char>(new_val);
                int error = old_val - new_val;


                for(const auto& target : kernel.targets){
                    long long nr = static_cast<long long>(r) + target.dr;
                    long long nc = static_cast<long long>(c) + target.dc;
                    if(nr<0 || nc<0 || nr>=static_cast<long long>(src.height) || nc>=static_cast<long long>(src.width)) continue;
                    buffer[index(nr, nc)+ch]+=error*target.numerator/kernel.denominator;
                }
            }
            if(src.channels==4){
                dst.data[pixel_index+3] = src.data[pixel_index+3];
            }
        }
    }
    
}