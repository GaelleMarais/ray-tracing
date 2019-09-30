#pragma once
#include "vec3.h"


struct Light{
    Vec3<float> position;
    Vec3<float> color;
    float intensity;
};