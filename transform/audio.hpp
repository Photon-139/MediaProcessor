#pragma once
#include <vector>
#include <sndfile.h>

class Audio{
public:
    size_t sample_rate;
    size_t channels;
    sf_count_t frames;
    std::vector<float> data;
    Audio(size_t sr, size_t ch, size_t fr) : sample_rate(sr), channels(ch), frames(fr), data(std::vector<float>(frames*channels, 0.0f)){};
};