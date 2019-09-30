#include "vec3.h"

struct Sphere{
    Vec3<float> C; //center
    int R;  //radius
    Vec3<float> color;
};

bool operator==(Sphere a, Sphere b){
    return (a.C==b.C && a.R==b.R);
}


// Intersection between the ray and the sphere's surface
// Indicates where to draw the sphere in the screen
float intersection(Ray ray, Sphere sphere){
    float a = 1;
    float b = 2*(dot(ray.P, ray.D) - dot(sphere.C, ray.D));
    float c = dot(ray.P,ray.P)
             +dot(sphere.C,sphere.C)
             -(dot(sphere.C,ray.P)*2)
             -(sphere.R*sphere.R);

    float delta = (b*b) - (4*a*c);
    float solution;

    if(delta>0){
        float s1 = (-b - std::sqrt(delta))/ (2*a);
        float s2 = (-b + std::sqrt(delta))/ (2*a);
        if (s1>0)
            solution = s1;
        else if (s2>0)
            solution = s2;
        else 
            return -1;
    }else{
        return -1;
    }
    //DEBUG
    //printf("%f", solution);
    return solution;
}

float nearest_intersection(Ray ray, Sphere spheres[], int nb_sphere, Sphere &sphere){
    float min_f = -1;
    for (int i=0; i<nb_sphere; i++){
        Sphere s = spheres[i];
        float f = intersection(ray, s);
        if (f > 0 && (min_f == -1 || f < min_f)){
            sphere=s;
            min_f=f;
        }
    }
    return min_f;
}

float intersection_max(Ray ray, Sphere sphere){
    float a = 1;
    float b = 2*(dot(ray.P, ray.D) - dot(sphere.C, ray.D));
    float c = dot(ray.P,ray.P)
             +dot(sphere.C,sphere.C)
             -(dot(sphere.C,ray.P)*2)
             -(sphere.R*sphere.R);

    float delta = (b*b) - (4*a*c);
    float solution;

    if(delta>0){
        float s1 = (-b - std::sqrt(delta))/ (2*a);
        float s2 = (-b + std::sqrt(delta))/ (2*a);
        if (s1>0)
            solution = s2;
        else if (s2>0)
            solution = s2;
        else 
            return 0;
    }else{
        return 0;
    }
    //DEBUG
    //printf("%f", solution);
    return solution;
}