#include "gaussian_blur.hpp"
#include <cmath>
#include <algorithm>

GaussianBlur::GaussianBlur(int rad):ImageEffect("GaussianBlur"), radius(rad){}

void GaussianBlur::apply(const Image& src, Image& dst) const{
    float sigma = radius/3.0f;
    int kernel_size = 2*radius+1;

    std::vector<float> kernel(kernel_size);
    float sum = 0;
    for(int x = -radius; x<=radius; ++x){
        kernel[x+radius] = std::exp(-(x*x)/(2*sigma*sigma));
        sum+=kernel[x+radius];
    }
    for(float& i : kernel){
        i/=sum;
    }
    const int ROWS = src.height;
    const int COLS = src.width;
    const int CHANNELS = src.channels;
    std::vector<float> temp(ROWS*COLS*CHANNELS);
    for(int row = 0; row<ROWS; ++row){
        for(int col = 0; col<COLS; ++col){
            for(int c = 0; c<CHANNELS; ++c){
                float acc = 0;
                for(int k = -radius; k<=radius; ++k){
                    int index = std::clamp(col+k, 0, COLS-1);
                    float weight = kernel[radius+k];
                    acc+= src.data[(row*COLS+index)*CHANNELS+c]*weight;
                }
                temp[(row*COLS+col)*CHANNELS+c] = acc;
            }
        }
    }
    for(int row = 0; row<ROWS; ++row){
        for(int col = 0; col<COLS; ++col){
            for(int c = 0; c<CHANNELS; ++c){
                float acc = 0;
                for(int k = -radius; k<=radius; ++k){
                    int index = std::clamp(row+k, 0, ROWS-1);
                    float weight = kernel[radius+k];
                    acc+= temp[(index*COLS+col)*CHANNELS+c]*weight;
                }
                dst.data[(row*COLS + col)*CHANNELS+c] = static_cast<unsigned char>(std::clamp(acc, 0.0f, 255.0f));
            }
        }
    }
}