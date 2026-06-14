#include <iostream>
#include "pipeline.hpp"
#include "effects/grayscale.hpp"
#include <memory>
#include "../external/stb/stb_image.h"
#include "../external/stb/stb_image_write.h"
#include "effects/sepia.hpp"
#include "effects/pixelate.hpp"
#include "effects/invert.hpp"
#include "audio_io.hpp"
#include "effects/audio_effect.hpp"
#include "effects/reverse.hpp"
#include "effects/normalize.hpp"

std::unique_ptr<ImageEffect> make_image_effect(const std::string& name, const std::vector<std::string>& params){
    if(name=="grayscale"){
        return std::make_unique<GrayScale>();
    }else if(name=="sepia"){
        return std::make_unique<Sepia>();
    }else if(name=="pixelate"){
        return std::make_unique<Pixelate>(std::stoi(params[0]));
    }else if(name=="invert"){
        return std::make_unique<Invert>();
    }
    else{
        throw std::runtime_error("Unknown effect: "+name);
    }
}
std::unique_ptr<AudioEffect> make_audio_effect(const std::string& name, const std::vector<std::string>& params){
    if(name=="reverse"){
        return std::make_unique<Reverse>();
    }else if(name=="normalize"){
        return std::make_unique<Normalize>();
    }
    else{
        throw std::runtime_error("Unknown effect: "+name);
    }
}

Pipeline::Pipeline(std::string pipeline_str, const std::string& file_type_, const std::string& file_format_):file_type(file_type_), file_format(file_format_){
    if(pipeline_str.empty()){
        throw std::runtime_error("Pipeline string is empty, cannot transform");
    }
    size_t start = 0;
    size_t end = 0;
    while(start<pipeline_str.size()){
        end = pipeline_str.find(";", start);
        size_t len = end!=std::string::npos ? (end-start) : std::string::npos;
        std::string segment = pipeline_str.substr(start, len);
        std::string effectName;
        std::vector<std::string> effectParams;
        if(segment.size()>0){
            size_t delimiter = segment.find(":");
            effectName = segment.substr(0, delimiter);
            if(delimiter!=std::string::npos){ 
                std::string params = segment.substr(delimiter+1);
                size_t pStart = 0, pEnd = 0;
                while(pStart<params.size()){
                    pEnd = params.find(",", pStart);
                    size_t pLen = pEnd != std::string::npos ? (pEnd - pStart) : std::string::npos;
                    std::string param = params.substr(pStart, pLen);
                    if(!param.empty()){
                        effectParams.push_back(param);
                    } 
                    if(pEnd==std::string::npos) break;
                    pStart = pEnd + 1;
                }
            }
        }
        if(file_type=="image"){
            imagePipeline.push_back(make_image_effect(effectName, effectParams));
        }else if(file_type=="audio"){
            audioPipeline.push_back(make_audio_effect(effectName, effectParams));
        }
        if(end==std::string::npos) break;
        start = end+1;
    }
}

std::vector<unsigned char> Pipeline::process(const std::vector<unsigned char>& raw_file_bytes){
    if(file_type=="image"){
        int width, height, channels;
        unsigned char* raw_pixels = stbi_load_from_memory(raw_file_bytes.data(), raw_file_bytes.size(), &width, &height, &channels, 0);
        
        if(!raw_pixels){
            throw std::runtime_error("Failed to load image");
        }
        
        Image bufferA = Image(width, height, channels, raw_pixels);
        stbi_image_free(raw_pixels);    
        Image bufferB = Image(width, height, channels);
         
    
        for(const auto& effect : imagePipeline){
            effect->apply(bufferA, bufferB);
            std::swap(bufferA, bufferB);        
        }
    
        return encode_image(bufferA);
    }else if(file_type=="audio"){
        Audio buffA = AudioIO::decode_from_memory(raw_file_bytes);
        Audio buffB(buffA.sample_rate, buffA.channels, buffA.frames, buffA.format);
        for(const auto& effect : audioPipeline){
            effect->apply(buffA, buffB);
            std::swap(buffA, buffB);
        }

        return AudioIO::encode_to_memory(buffA);
    }

    throw std::runtime_error("Unknown file type: "+file_type);
}

std::vector<unsigned char> Pipeline::encode_image(Image& img){
    std::vector<unsigned char> output;
    auto write_callback = [](void* context, void* data, int size){
        auto* out = static_cast<std::vector<unsigned char>*>(context);
        unsigned char* bytes = static_cast<unsigned char*>(data);
        out->insert(out->end(), bytes, bytes+size);
    };
    int res;
    if(file_format=="jpg" || file_format=="jpeg"){
        res = stbi_write_jpg_to_func(write_callback, &output, img.width, img.height, img.channels, img.data.data(), 100);
    }
    if(!res){
        throw std::runtime_error("Failed to encode image");
    }
    return output;
}