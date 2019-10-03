#pragma once
#include "vec3.h"
#include "ray.h"

struct Triangle{
    Vec3<float> a;
    Vec3<float> b;
    Vec3<float> c;
    Vec3<float> color;
};

float intersection_triangle(Ray ray, Triangle triangle){
    const float epsilon = 0.0000001f;

    float a, f, u, v;
    Vec3<float> edge1, edge2, h, s, q;
    edge1 = triangle.b - triangle.a;
    edge2 = triangle.c - triangle.a;

    h = cross(ray.D, edge2);
    a = dot(edge1, h);
    if (a > -epsilon && a < epsilon) // This ray is parallel to this triangle
        return -1;
    f = 1.f/a;
    s = ray.P - triangle.a;
    u =  f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return -1;
    q = cross(s, edge1);
    v = f * dot(ray.D, q);
    if ( v < 0.0 || v + u > 1.0)
        return -1;
    
    // On calcule t pour savoir ou le point d'intersection se situe sur la ligne.
    float t = f * dot(edge2, q);
    if( t > epsilon)
        return t;

    return -1;
}

float nearest_intersection_triangle(Ray ray, Triangle triangles[], int nb_triangle, Triangle &triangle){
    float min_f = -1;
    for (int i=0; i<nb_triangle; i++){
        Triangle t = triangles[i];
        float f = intersection_triangle(ray, t);
        if (f > 0 && (min_f == -1 || f < min_f)){
            triangle=t;
            min_f=f;
        }
    }
    return min_f;
}

Vec3<float> normal_triangle(Triangle t){
    Vec3<float> edge1 = t.b - t.a;
    Vec3<float> edge2 = t.c - t.a;

    return normalise(cross(edge1, edge2));    
}