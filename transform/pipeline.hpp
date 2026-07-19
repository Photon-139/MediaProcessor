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
        std::unique_ptr<ImageEffect> make_image_effect(const std::string& name, const std::vector<std::string>& params);
    public:
        Pipeline(std::string effectPipeline, const std::string& file_type, const std::string& file_format);
        std::vector<unsigned char> process(const std::vector<unsigned char>& raw_file_bytes);      
};