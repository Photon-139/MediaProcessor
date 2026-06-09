#pragma once
#include "effects/effect.hpp"
#include <memory>
#include "effects/audio_effect.hpp"

class Pipeline{
    private:
        std::vector<std::unique_ptr<ImageEffect>> imagePipeline;
        std::vector<std::unique_ptr<AudioEffect>> audioPipeline;
        std::string file_type;
        const std::string file_format;
        std::vector<unsigned char> encode_image(Image& img);
    public:
        Pipeline(std::string effectPipeline, const std::string& file_type, const std::string& file_format);
        std::vector<unsigned char> process(const std::vector<unsigned char>& raw_file_bytes);      
};