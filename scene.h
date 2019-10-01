#pragma once 
#include "vec3.h"
#include "light.h"
#include "sphere.h"

struct Scene{
    int nb_spheres;
    Sphere spheres[10];
    int nb_lights;
    LightCube light_cubes[10];
};