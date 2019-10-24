#pragma once

#include <math.h>
#include <string>

#include "vec3.h"
#include "easyppm.h"

class Image
{
    unsigned int width;
    unsigned int height;
    Vec3<float>* pixels;

public:
    Image(unsigned int width, unsigned int height) : width(width), height(height) {
        pixels = new Vec3<float>[width * height];
    }

    unsigned int getWidth() {
        return width;
    }

    unsigned int getHeight() {
        return height;
    }

    Vec3<float> get(unsigned int x, unsigned int y) {
        if (x >= width) return {0, 0, 0};
        if (y >= height) return {0, 0, 0};
        return pixels[x + y * width];
    }
    
    void set(unsigned int x, unsigned int y, Vec3<float> value) {
        if (x >= width) return;
        if (y >= height) return;
        pixels[x + y * width] = value;
    }

    void save(const std::string& filename)
    {
        PPM ppm = easyppm_create(width, height, IMAGETYPE_PPM);    
        for (unsigned int x = 0; x < width; x++)
        {
            for (unsigned int y = 0; y < height; y++)
            {
                //Copy data
                Vec3<float> color = pixels[x + y * width];
                //Clamp
                if (color.x < 0) color.x = 0; if (color.x > 1) color.x = 1;
                if (color.y < 0) color.y = 0; if (color.z > 1) color.z = 1;
                if (color.z < 0) color.z = 0; if (color.y > 1) color.y = 1;
                //Gamma correction
                color.x = std::pow(color.x, 1 / 2.2f);
                color.y = std::pow(color.y, 1 / 2.2f);
                color.z = std::pow(color.z, 1 / 2.2f);
                //Lerp
                color.x *= 255;
                color.y *= 255;
                color.z *= 255;

                ppmcolor data = easyppm_rgb(
                    (unsigned char) color.x,
                    (unsigned char) color.y,
                    (unsigned char) color.z);

                easyppm_set(&ppm, x, y, data);
            }
        }
        easyppm_write(&ppm, filename.c_str());
        easyppm_destroy(&ppm);
    }
};