#pragma once
#include <effect.hpp>
#include <memory>

class Pipeline{
    private:
        std::vector<std::unique_ptr<Effect>> effectPipeline;
    public:
        Pipeline(std::string effectPipeline);
        void process(Image& bufferA, Image& bufferB);

        
};