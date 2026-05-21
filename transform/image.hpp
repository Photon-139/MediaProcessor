#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

struct Image{
    std::vector<unsigned char> data;
    size_t width;
    size_t height;
    size_t channels;
};

#endif