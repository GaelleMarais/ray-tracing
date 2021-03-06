#include <omp.h>
#include <stdio.h>
#include <math.h>
#include "ray.h"
#include "sphere.h"
#include "triangle.h"
#include "lightcube.h"
#include "vec3.h"
#include "light.h"
#include "scene.h"
#include "camera.h"
#include "easyppm.h"
#include "easyppm.c"
#include "box.h"
#include "parser.h"
#include <iostream>

int image_width = 600;
int image_height = 600;
float light_intensity = 5e4;
int light_samples = 1;
int camera_samples = 1;

float clamp(float min, float max, float value){
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float lerp(float start, float end, float t){
    return start + t * (end - start);
}

Vec3<float> get_point_light_illumination(const Light& light, const Vec3<float>& hit_point, const Vec3<float>& normal, Scene& scene){
    Vec3<float> color = {0, 0, 0};
    // Compute point light color

    Ray surface_to_light;
    surface_to_light.P = hit_point + 0.1f * normal; // move away from surface to avoid dead pixels
    surface_to_light.D = normalise(light.position - hit_point);

    float dist_to_light = norm(light.position - hit_point);

    Sphere obstacle_sphere;
    float f_obstacle_sphere = nearest_intersection_sphere(surface_to_light, scene.spheres, scene.nb_spheres, obstacle_sphere);

    Triangle obstacle_triangle;
    float f_obstacle_triangle = nearest_intersection_triangle(surface_to_light, scene.triangles, scene.nb_triangles, obstacle_triangle);

    if ((f_obstacle_sphere == -1 || f_obstacle_sphere >= dist_to_light) && (f_obstacle_triangle == -1 || f_obstacle_triangle >= dist_to_light)){
        // case where the sphere is in light
        float angle = dot(surface_to_light.D, normal);
        float intensity = light.intensity * angle / (dist_to_light * dist_to_light);
        if (intensity < 0) intensity = 0;
        color = intensity * light.color;
    }

    return color;
}

Ray get_ray(const Vec3<float>& pixel, const Vec3<float>& camera){
    Ray ray;
    ray.P = camera;
    ray.D = normalise(pixel - camera);
    return ray;
}

Vec3<float> trace_ray(const Vec3<float>& hit_point, Vec3<float>& normal, Vec3<float>& col, Scene& scene){
    Vec3<float> illumination = {0, 0, 0};                
    Vec3<float> light_cube_illumination = {0, 0, 0};

    std::random_device random_device;
    std::mt19937_64 random(random_device());

    // For each cube_light in the scene
    for(int cl = 0; cl < scene.nb_lights; cl++){

    LightCube light_cube = scene.light_cubes[cl];
        // For severals light_point in the light_cube
        for (int l = 0; l < light_samples; l++)
        {
            Light light_point = light_cube.new_random_point_light(random, light_samples);
            light_cube_illumination = light_cube_illumination + get_point_light_illumination(light_point, hit_point, normal, scene);
        }
        illumination = illumination + light_cube_illumination;

    }

    //Convert to RGB ranges
    Vec3<float> color = col * illumination;
    //Normalize by clamping to [0;1] domain
    color.x = clamp(0, 1, color.x);
    color.y = clamp(0, 1, color.y);
    color.z = clamp(0, 1, color.z);
    //Apply gamma correction
    color.x = std::pow(color.x, 1/2.2f);
    color.y = std::pow(color.y, 1/2.2f);
    color.z = std::pow(color.z, 1/2.2f);
    //Map to [0;255]
    color.x = lerp(0, 255, color.x);
    color.y = lerp(0, 255, color.y);
    color.z = lerp(0, 255, color.z);

    return color;

}

void draw_triangle(Ray& r, float dist_surface, Triangle& t, Scene& scene, Vec3<float>& total_color){
    Vec3<float> hit_point = r.P + dist_surface * r.D;
    Vec3<float> normal = normal_triangle(t);
    Vec3<float> camera_point_color = trace_ray(hit_point, normal, t.color, scene);

    total_color = total_color + camera_point_color;
}

void draw_sphere(Ray& r, float dist_surface, Sphere& s, Scene& scene, Vec3<float>& total_color){
    Vec3<float> hit_point = r.P + dist_surface * r.D;
    Vec3<float> normal = normalise(hit_point - s.C);
    Vec3<float> camera_point_color = trace_ray(hit_point, normal, s.color, scene);

    total_color = total_color + camera_point_color;
}

void draw_scene_ppm(PPM ppm, Scene& scene){

    Box root;

    #pragma omp parallel for    
    for(int i = 0; i < ppm.width; i++){
        for(int j = 0; j < ppm.height; j++){

            Vec3<float> pixel = {(float)i, 0, (float)j};

            Vec3<float> total_color = {0, 0, 0};

            for(int c=0; c < camera_samples; c++){

                std::random_device random_device;
                std::mt19937_64 random(random_device());

                Vec3<float> camera_point =  scene.camera_cube.new_random_point();
                Ray r = get_ray(pixel, camera_point);

                Sphere s;
                float f_sphere = nearest_intersection_sphere(r, scene.spheres, scene.nb_spheres, s); 

                Triangle t;
                float f_triangle = nearest_intersection_triangle(r, scene.triangles, scene.nb_triangles, t);

                if(f_triangle > -1 && f_triangle < f_sphere)
                    draw_triangle(r, f_triangle, t, scene, total_color);

                else if(f_sphere > -1 )
                    draw_sphere(r, f_sphere, s, scene, total_color);


            ppmcolor color = easyppm_rgb(
                (char) (total_color.x/ (float) camera_samples),
                (char) (total_color.y/ (float) camera_samples),
                (char) (total_color.z/ (float) camera_samples)
                );
            easyppm_set(&ppm, i, j, color);

            }
        }
    }
}



int main()
{
    char filename[250];

    sscanf("imgs/mesh3.ppm","%s", filename);
    printf("Writing %s ... ", filename);

    Scene scene;

    // WHITE FLOOR
    Sphere s3;
    s3.C = {300, 20600, 300};
    s3.R = 20000;
    s3.color={1,1,1};
    scene.spheres[0]=s3;

    // ORANGE SPHERE
    Sphere s6;
    s6.C = {300, 500, 000};
    s6.R = 170;
    s6.color={1.0f, 0.3f, 0.1f};
    scene.spheres[1]=s6;

    // // BLUE SPHERE
    Sphere s7;
    s7.C = {100, 200, 200};
    s7.R = 80;
    s7.color={0,0.3f,1};
    scene.spheres[2]=s7;

    scene.nb_spheres=3;

    // YELLOW LIGHT
    LightCube l1;
    l1.color = {1, 1, 0};
    l1.intensity = light_intensity;
    l1.position = {200, 100, 400};
    l1.size = 30;
    scene.light_cubes[0] = l1;

    // WHITE LIGHT
    LightCube l2;
    l2.color = {1, 1, 1};
    l2.intensity = light_intensity;
    l2.position = {000, 100, 000};
    l2.size = 20;
    scene.light_cubes[1] = l2;

    // CYAN LIGHT
    LightCube l3;
    l3.color = {0, 0, 1};
    l3.intensity = light_intensity;
    l3.position = {000, 300, 600};
    l3.size = 20;
    scene.light_cubes[2] = l3;
    
    scene.nb_lights = 3;

    CameraCube camera;
    camera.position = {300,-1500,300};
    camera.size=1;
    scene.camera_cube = camera;


    PPM ppm = easyppm_create(image_width, image_height, IMAGETYPE_PPM);
    scene.nb_triangles = 616;
    Triangle tr[scene.nb_triangles];
    Vec3<float> pos = { 300,500,170};
    read_file("./off/r2.off", 0.3f, pos, tr);

    for(int i = 0; i < scene.nb_triangles; i++){
        scene.triangles[i] = tr[i];
    }

    // DEBUG
    // for (int i = 0; i < scene.nb_triangles; i++){
    //     std::cout << tr[i] << std::endl;
    // }


    draw_scene_ppm(ppm, scene);

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;
}
