#pragma once
#include "effects/effect.hpp"
#include <memory>

class Pipeline{
    private:
        std::vector<std::unique_ptr<Effect>> effectPipeline;
        std::string file_type;
        const std::string file_format;
        std::vector<unsigned char> encode_image(Image& img);
    public:
        Pipeline(std::string effectPipeline, const std::string& file_type, const std::string& file_format);
        std::vector<unsigned char> process(const std::vector<unsigned char>& raw_file_bytes);      
};