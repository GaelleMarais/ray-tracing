#pragma once
#include "vec3.h"
#include <random>

struct CameraCube{
    Vec3<float> position;
    int size;

    Vec3<float> new_random_point()
    {
        Vec3<float> point;

        point.x = position.x + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        point.y = position.y + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(size)));
        point.z = position.z;
        
        return point;
    }
};