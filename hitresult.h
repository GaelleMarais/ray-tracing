#pragma once

#include "vec3.h"

struct Hit_result
{
    float       distance = -1;
    Vec3<float> hit_point;
    Vec3<float> normal;
    Vec3<float> color;
};