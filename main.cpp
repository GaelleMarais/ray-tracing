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

float lerp(float start, float end, float t)
{
    return start + t * (end - start);
}

Vec3<float> get_point_light_illumination(const Light& light, const Vec3<float> hit_point, const Vec3<float>& normal, Scene scene){
    Vec3<float> color = {0, 0, 0};
    // Compute point light color

    Ray sphere_to_light;
    sphere_to_light.P = hit_point + 0.1f * normal; // move away from surface to avoid dead pixels
    sphere_to_light.D = normalise(light.position - hit_point);

    float dist_to_light = norm(light.position - hit_point);

    Sphere obstacle;
    float f_obstacle = nearest_intersection(sphere_to_light, scene.spheres, scene.nb_spheres, obstacle);

    if (f_obstacle == -1 || f_obstacle >= dist_to_light){
        // case where the sphere is in light
        float angle = dot(sphere_to_light.D, normal);
        float intensity = light.intensity * angle / (dist_to_light * dist_to_light);
        if (intensity < 0) intensity = 0;
        color = intensity * light.color;
    }

    return color;
}

Ray get_ray(const Vec3<float> pixel, const Vec3<float> camera){
    Ray ray;
    ray.P = camera;
    ray.D = normalise(pixel - camera);
    return ray;
}

void draw_scene_ppm(PPM ppm, Scene scene){

    #pragma omp parallel for    
    for(int i = 0; i < ppm.width; i++){
        for(int j = 0; j < ppm.height; j++){

            Vec3<float> pixel = {(float)i, (float)j, 0};
            Ray r = get_ray(pixel, scene.camera);

            Sphere s;
            float f = nearest_intersection(r, scene.spheres, scene.nb_spheres, s);  

            if(f>0){ // case where the sphere is in the screen

                Vec3<float> hit_point = r.P + f * r.D;
                Vec3<float> normal = normalise(hit_point - s.C);
                Vec3<float> illumination = {0, 0, 0};
                
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

    // WHITE BACKGROUND
    Sphere s1;
    s1.C = {300, 300, 20600};
    s1.R = 20000;
    s1.color={1,1,1};
    scene.spheres[0]=s1;

    // WHITE CEILING
    Sphere s2;
    s2.C = {300, -20000, 300};
    s2.R = 20000;
    s2.color={1,1,1};
    scene.spheres[1]=s2;

    // WHITE FLOOR
    Sphere s3;
    s3.C = {300, 20600, 300};
    s3.R = 20000;
    s3.color={1,1,1};
    scene.spheres[2]=s3;

    // WHITE LEFT WALL
    Sphere s4;
    s4.C = {-20000, 300, 300};
    s4.R = 20000;
    s4.color={1,1,1};
    scene.spheres[3]=s4;

    // WHITE RIGHT WALL
    Sphere s5;
    s5.C = {20600, 300, 300};
    s5.R = 20000;
    s5.color={1,1,1};
    scene.spheres[4]=s5;

    // LIGHT BLUE SPHERE
    Sphere s6;
    s6.C = {400, 400, 400};
    s6.R = 120;
    s6.color={0,1,1};
    scene.spheres[5]=s6;

    // BLUE SPHERE
    Sphere s7;
    s7.C = {150, 250, 350};
    s7.R = 80;
    s7.color={0,0.3,1};
    scene.spheres[6]=s7;




    scene.nb_spheres=7;



    // YELLOW LIGHT
    LightCube l1;
    l1.color = {1, 1, 0};
    l1.intensity = light_intensity;
    l1.position = {300, 600, 300};
    l1.size = 300;
    scene.light_cubes[0] = l1;

    // WHITE LIGHT
    LightCube l2;
    l2.color = {1, 1, 1};
    l2.intensity = light_intensity;
    l2.position = {300, 100, 300};
    l2.size = 20;
    scene.light_cubes[1] = l2;
    
    scene.nb_lights = 2;

    scene.camera = {300,300,-2500};


    PPM ppm = easyppm_create(image_width, image_height, IMAGETYPE_PPM);

    draw_scene_ppm(ppm, scene);

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;

}
