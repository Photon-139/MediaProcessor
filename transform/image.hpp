#pragma once
#include <vector>

class Image{
    public:
    std::vector<unsigned char> data;
    size_t width;
    size_t height;
    size_t channels;
    Image(size_t w, size_t h, size_t c, unsigned char image[]): width(w), height(h), channels(c), data(image, image+w*h*c){}
    Image(size_t w, size_t h, size_t c) : width(w), height(h), channels(c), data(w*h*c){}
};