#include <iostream>
#include "pipeline.hpp"

Pipeline::Pipeline(std::string effectPipeline){
    if(effectPipeline.empty()){
        std::cout << "Pipeline Empty, cannot transform" << std::endl;
        exit(1);
    }
    size_t start = 0;
    size_t end = 0;
    while(start<effectPipeline.size()){
        end = effectPipeline.find(";", start);
        size_t len = end!=std::string::npos ? (end-start) : std::string::npos;
        std::string segment = effectPipeline.substr(start, len);
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
        std::cout << "Effect name: " << effectName << std::endl;
        for(const auto& s : effectParams){
            std::cout << "Parameter: " << s << std::endl;
        }
        if(end==std::string::npos) break;
        start = end+1;
    }
}

void Pipeline::process(Image& bufferA, Image& bufferB){
    bool flip = false;

    for(const auto& effect : effectPipeline){
        if(!flip){
            effect->apply(bufferA, bufferB);
            flip = true;
        }else{
            effect->apply(bufferB, bufferA);
            flip=false;
        }
    }
    if(flip){
        std::swap(bufferA, bufferB);
    }
}