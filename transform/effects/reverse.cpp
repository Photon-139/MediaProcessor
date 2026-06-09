#include "reverse.hpp"

Reverse::Reverse(): AudioEffect("Reverse"){}

void Reverse::apply(const Audio& src, Audio& dst) const{
    size_t index, end_index;
    size_t frames{src.frames};
    size_t channels{src.channels};
    for(size_t i = 0; i<frames/2; ++i){
        index = i*channels;
        end_index = (frames - 1 - i)*channels;
        for(size_t j{}; j<channels; ++j){
            dst.data[end_index+j] = src.data[index+j];
            dst.data[index+j] = src.data[end_index+j];
        }
    }
    if(frames%2!=0){
        index = (frames*channels)/2-1;
        for(size_t i = 0; i<channels; ++i){
            dst.data[index+i] = src.data[index+i];
        }
    }
}