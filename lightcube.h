#pragma once
#include "vec3.h"
#include "light.h"
#include <random>

struct LightCube{
    Vec3<float> position;
    float size;
    float intensity;
    Vec3<float> color;

    Light new_random_point_light(std::mt19937_64& random, int sample_count)
    {
        Light light;
        light.intensity = intensity / (float) sample_count;
        light.color = color;

        float delta = size/2;
        std::uniform_real_distribution<> dist(-delta, +delta);
        light.position.x = dist(random);
        light.position.y = dist(random);
        light.position.z = dist(random);
        light.position = light.position + position;

        return light;
    }
};