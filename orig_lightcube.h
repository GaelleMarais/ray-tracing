#pragma once
#include "vec3.h"
#include "light.h"
#include <random>

struct LightCube{
    Vec3<float> position, color;
    float size, intensity;

    Light new_random_light(int sample_count){

        Light light;
        Vec3<float> point;
        light.intensity = intensity / (float) sample_count;
        light.color = color;

        point.x = position.x + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        point.y = position.y + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        point.z = position.z + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        
        light.position = point;

        return light;
    }

    Light new_random_point_light(std::mt19937_64& random, int sample_count)
    {
        Light light;
        light.intensity = intensity / (float) sample_count;
        light.color = color;

        float delta = size/2;
        std::uniform_real_distribution<> dist(-delta, +delta);
        light.position.x = (float) dist(random);
        light.position.y = (float) dist(random);
        light.position.z = (float) dist(random);
        light.position = light.position + position;

        return light;
    }
};