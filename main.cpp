#include <omp.h>
#include <stdio.h>
#include <math.h>
#include "ray.h"
#include "sphere.h"
#include "lightcube.h"
#include "vec3.h"
#include "light.h"
#include "scene.h"
#include "easyppm.h"
#include "easyppm.c"
#include <iostream>

int image_width = 600;
int image_height = 600;
int light_intensity = 1e5;
int samples = 100;
Scene scene;

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

Vec3<float> get_point_light_illumination(const Light& light, const Vec3<float> hit_point, const Vec3<float>& normal, Scene scene)
{
    Vec3<float> color = {0, 0, 0};
    // Compute point light color

    Ray sphere_to_light;
    sphere_to_light.P = hit_point + 0.1f * normal; // move away from surface to avoid dead pixels
    sphere_to_light.D = normalise(light.position - hit_point);

    float dist_to_light = norm(light.position - hit_point);

    Sphere obstacle;
    // int obs = 0;
    float f_obstacle = nearest_intersection(sphere_to_light, scene.spheres, scene.nb_spheres, obstacle);

    if (f_obstacle == -1 || f_obstacle >= dist_to_light){
        // sphere is in light
        float angle = dot(sphere_to_light.D, normal);
        float intensity = light.intensity * angle / (dist_to_light * dist_to_light);
        if (intensity < 0) intensity = 0;
        color = intensity * light.color;
    }

    return color;
}

void draw_sphere_ppm(PPM ppm, Sphere s, Scene scene){



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
                
                //Can abstract to multiple light_cubes easily now ! :)
                Vec3<float> light_cube_illumination = {0, 0, 0};

                std::random_device random_device;
                std::mt19937_64 random(random_device());

                // For each cube_light in the scene
                for(int cl = 0; cl < scene.nb_lights; cl++){

                LightCube light_cube = scene.light_cubes[cl];
                    // For severals light_point in the light_cube
                    for (int l = 0; l < samples; l++)
                    {
                        Light light_point = light_cube.new_random_point_light(random, samples);
                        light_cube_illumination = light_cube_illumination + get_point_light_illumination(light_point, hit_point, normal, scene);
                    }
                    illumination = illumination + light_cube_illumination;

                }



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

    sscanf("imgs/scene.ppm","%s", filename);
    printf("Writing %s ... ", filename);

    // WHITE SPHERE
    Sphere s3;
    s3.C = {300, 300, 1500};
    s3.R = 300;
    s3.color={1,1,1};
    scene.spheres[0]=s3;

    // GREEN SPHERE
    Sphere s1;
    s1.C = {150, 250, 1000};
    s1.R = 80;
    s1.color={0,1,1};
    scene.spheres[1]=s1;

    // YELLOW SPHERE
    Sphere s2;
    s2.C = {400, 300, 1000};
    s2.R = 70;
    s2.color={1,1,0};
    scene.spheres[2]=s2;

    scene.nb_spheres=3;

    // YELLOW LIGHT
    LightCube l1;
    l1.color = {1, 1, 0};
    l1.intensity = light_intensity;
    l1.position = {300, 600, 0};
    l1.size = 100;
    scene.light_cubes[0] = l1;

    // RED LIGHT
    LightCube l2;
    l2.color = {1, 0, 0};
    l2.intensity = light_intensity *2;
    l2.position = {300, 300, 0};
    l2.size = 50;
    scene.light_cubes[1] = l2;
    
    scene.nb_lights = 2;


    PPM ppm = easyppm_create(image_width, image_height, IMAGETYPE_PPM);

    for(int i=0; i<scene.nb_spheres; i++){
        draw_sphere_ppm(ppm, scene.spheres[i], scene);
    }

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;

}
