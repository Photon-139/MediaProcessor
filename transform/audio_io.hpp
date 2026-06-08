#pragma once
#include "audio.hpp"


namespace AudioIO{
    Audio decode_from_memory(const std::vector<unsigned char>& data);
    std::vector<unsigned char> encode_to_memory(const Audio& data, const int format_flag);
}