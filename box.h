#pragma once
#include "vec3.h"
#include "sphere.h"
#include "ray.h"

struct Box{
    Vec3<float> min,max;
};

Box create_box_from_sphere(Sphere sphere){
    Box box;
    box.min = {sphere.C - sphere.R};
    box.max = {sphere.R + sphere.C};

    return box;
}

Box create_box_from_boxes(Box b1, Box b2){
    Box box;
    box.min = {std::min(b1.min.x, b2.min.x),
               std::min(b1.min.y, b2.min.y),
               std::min(b1.min.z, b2.min.z)};
    box.max = {std::max(b1.max.x, b2.max.x),
               std::max(b1.max.y, b2.max.y),
               std::max(b1.max.z, b2.max.z)};
    return box;
}

Box create_box_from_triangle(Triangle t){
    Box box;
    box.min = {std::min(std::min(t.a.x, t.b.x),t.c.x),
               std::min(std::min(t.a.y, t.b.y),t.c.y),
               std::min(std::min(t.a.z, t.b.z),t.c.z)};
    box.max = {std::max(std::max(t.a.x, t.b.x),t.c.x),
               std::max(std::max(t.a.y, t.b.y),t.c.y),
               std::max(std::max(t.a.z, t.b.z),t.c.z)};
    return box;
}

float intersection_box(Ray ray, Box box){
    Vec3<float> dirfrac;
    // r.dir is unit direction vector of ray
    dirfrac.x = 1.0f / ray.D.x;
    dirfrac.y = 1.0f / ray.D.y;
    dirfrac.z = 1.0f / ray.D.z;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    float t1 = (box.min.x - ray.P.x)*dirfrac.x;
    float t2 = (box.max.x - ray.P.x)*dirfrac.x;
    float t3 = (box.min.y - ray.P.y)*dirfrac.y;
    float t4 = (box.max.y - ray.P.y)*dirfrac.y;
    float t5 = (box.min.z - ray.P.z)*dirfrac.z;
    float t6 = (box.max.z - ray.P.z)*dirfrac.z;

    float tmin = std::max(std::max(std::min(t1, t2),std:: min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    // float t;

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0)
    {        
        return -1;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
    {
        return -1;
    }
    return tmin;
}

float nearest_intersection_box(Ray ray, Box boxes[], int nb_box, Box &box){
    float min_f = -1;
    for (int i=0; i<nb_box; i++){
        Box b = boxes[i];
        float f = intersection_box(ray, b);
        if (f > 0 && (min_f == -1 || f < min_f)){
            box = b;
            min_f = f;
        }
    }
    return min_f;
}