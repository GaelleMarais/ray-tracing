#pragma once
#include "vec3.h"
#include "light.h"
#include <random>

struct LightCube{
    Vec3<float> position;
    float size;
    float intensity;
    Vec3<float> color;

    Light new_random_point_light(int sample_count)
    {
        Light light;
        light.intensity = intensity / (float) sample_count;
        light.color = color;

        Vec3<float> point;
        point.x = position.x + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        point.y = position.y + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        point.z = position.z + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        light.position = point;

        return light;
    }
};