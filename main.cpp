#include <omp.h>
#include <stdio.h>
#include <math.h>
#include "ray.h"
#include "sphere.h"
#include "cube.h"
#include "vec3.h"
#include "light.h"
#include "easyppm.h"
#include "easyppm.c"
#include <iostream>

int image_width = 600;
int image_height = 600;
int INTENS = 1e5;
int samples = 100;

float clamp(float min, float max, float value)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

//t e [0; 1]
float lerp(float start, float end, float t)
{
    return start + t * (end - start);
}

Vec3<float> get_point_light_illumination(const Light& light, const Vec3<float> hit_point, const Vec3<float>& normal, Sphere spheres[], int nb_spheres)
{
    Vec3<float> color = {0, 0, 0};
    // Compute point light color

    Ray sphere_to_light;
    sphere_to_light.P = hit_point + 0.1f * normal; // move away from surface to avoid dead pixels
    sphere_to_light.D = normalise(light.position - hit_point);

    float dist_to_light = norm(light.position - hit_point);

    Sphere obstacle;
    // int obs = 0;
    float f_obstacle = nearest_intersection(sphere_to_light, spheres, nb_spheres, obstacle);

    if (f_obstacle == -1 || f_obstacle >= dist_to_light){
        // sphere is in light
        float angle = dot(sphere_to_light.D, normal);
        float intensity = light.intensity * angle / (dist_to_light * dist_to_light);
        if (intensity < 0) intensity = 0;
        color = intensity * light.color;
    }

    return color;
}

void draw_sphere_ppm(PPM ppm, Sphere s, Sphere spheres[], int nb_spheres){

    CubeLight cube_light;
    cube_light.color = {1, 1, 1};
    cube_light.intensity = INTENS;
    cube_light.position = {300, 600, 300};
    cube_light.size = 100;

    #pragma omp parallel for    
    for(int i = 0; i < ppm.width; i++){
        for(int j = 0; j < ppm.height; j++){

            Ray r;
            r.P = {(float)i,(float)j,0};
            r.D = {0,0,1};

            float f = intersection(r,s);             

            if(f>0){ // case where the sphere is in the screen

                Vec3<float> hit_point = r.P + f * r.D;
                Vec3<float> normal = normalise(hit_point - s.C);
                Vec3<float> illumination = {0, 0, 0};
                
                //Can abstract to multiple cube_lights easily now ! :)
                Vec3<float> cube_light_illumination = {0, 0, 0};

                std::random_device random_device;
                std::mt19937_64 random(random_device());

                //Surface light illumination computing
                for (int l = 0; l < samples; l++)
                {
                    Light light_point = cube_light.new_random_point_light(random, samples);
                    cube_light_illumination = cube_light_illumination + get_point_light_illumination(light_point, hit_point, normal, spheres, nb_spheres);
                }
                illumination = illumination + cube_light_illumination;

                //Convert to RGB ranges
                Vec3<float> color = s.color * illumination;
                //Normalize by clamping to [0;1] domain
                color.x = clamp(0, 1, color.x);
                color.y = clamp(0, 1, color.y);
                color.z = clamp(0, 1, color.z);
                //Apply the weird shit (gamma correction)
                color.x = std::pow(color.x, 1/2.2f);
                color.y = std::pow(color.y, 1/2.2f);
                color.z = std::pow(color.z, 1/2.2f);
                //Map to [0;255]
                color.x = lerp(0, 255, color.x);
                color.y = lerp(0, 255, color.y);
                color.z = lerp(0, 255, color.z);
                //Write to framebuffer
                easyppm_set(&ppm, i, j, easyppm_rgb(color.x, color.y, color.z));
            }
        }
    }
}



int main(int argc, char* argv[])
{
    char filename[250];
    Sphere spheres[10];

    sscanf("imgs/light.ppm","%s", filename);
    printf("Writing %s ... ", filename);

    // VERT
    Sphere s1;
    s1.C = {150, 250, 600};
    s1.R = 80;
    s1.color={0,1,1};
    spheres[0]=s1;

    // JAUNE
    Sphere s2;
    s2.C = {400, 300, 600};
    s2.R = 70;
    s2.color={1,1,0};
    spheres[1]=s2;

    // ROUGE
    Sphere s3;
    s3.C = {300, 50, 900};
    s3.R = 300;
    s3.color={1,0,0};
    spheres[2]=s3;


    int nb_sphere=3;

    PPM ppm = easyppm_create(image_width, image_height, IMAGETYPE_PPM);

    draw_sphere_ppm(ppm,s3,spheres,nb_sphere);
    draw_sphere_ppm(ppm,s2,spheres,nb_sphere);
    draw_sphere_ppm(ppm,s1,spheres,nb_sphere);

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;

}
