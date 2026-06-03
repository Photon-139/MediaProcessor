#include <iostream>
#include "pipeline.hpp"
#include "effects/grayscale.hpp"
#include <memory>
#include "../external/stb/stb_image.h"
#include "../external/stb/stb_image_write.h"

std::unique_ptr<Effect> make_effect(const std::string& name, const std::vector<std::string>& params){
    if(name=="grayscale"){
        return std::make_unique<GrayScale>();
    }else{
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
        effectPipeline.push_back(make_effect(effectName, effectParams));
        if(end==std::string::npos) break;
        start = end+1;
    }
}

std::vector<unsigned char> Pipeline::process(const std::vector<unsigned char>& raw_file_bytes){
    int width, height, channels;
    unsigned char* raw_pixels = stbi_load_from_memory(raw_file_bytes.data(), raw_file_bytes.size(), &width, &height, &channels, 0);
    
    if(!raw_pixels){
        throw std::runtime_error("Failed to load image");
    }
    
    Image bufferA = Image(width, height, channels, raw_pixels);
    stbi_image_free(raw_pixels);    
    Image bufferB = Image(width, height, channels);
     

    for(const auto& effect : effectPipeline){
        effect->apply(bufferA, bufferB);
        std::swap(bufferA, bufferB);        
    }

    return encode_image(bufferA);
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