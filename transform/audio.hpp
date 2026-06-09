#pragma once
#include <vector>
#include <sndfile.h>

class Audio{
public:
    size_t sample_rate;
    size_t channels;
    sf_count_t frames;
    int format;
    std::vector<float> data;
    Audio(size_t sr, size_t ch, size_t fr, int format_) : sample_rate(sr), channels(ch), frames(fr), format(format_), data(std::vector<float>(frames*channels, 0.0f)){};
};