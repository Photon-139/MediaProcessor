#include "normalize.hpp"
#include <algorithm>

Normalize::Normalize() : AudioEffect("Normalize"){}

void Normalize::apply(const Audio& src, Audio& dst) const{
    float max_val = 0.0f;
    for(const float sample : src.data){
        max_val = std::max(max_val, std::abs(sample)); 
    }
    float multiplier = 1.0f;
    if(max_val>0){
        multiplier = 1.0f/max_val;
    }
    for(size_t i = 0; i<src.data.size(); ++i){
        dst.data[i] = std::clamp(src.data[i]*multiplier, -1.0f, 1.0f);
    }
    
}