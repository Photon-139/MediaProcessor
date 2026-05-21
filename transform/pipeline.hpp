#ifndef PIPELINE_H
#define PIPELINE_H

#include <effect.hpp>
#include <memory>

class Pipeline{
    private:
        std::vector<std::unique_ptr<Effect>> effectPipeline;
    public:
        Pipeline(std::string effectPipeline);
        void process(const Image& src, Image& dest);

        
};

#endif