#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <iostream>

#include "timer.h"
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
#include "tree.h"
#include "parser.h"
#include "image.h"

#define USE_BVH
//#define DEBUG_BVH

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

#ifdef USE_BVH
    Hit_result hit = scene.root->intersection(surface_to_light);
    if (hit.distance >= 0 && hit.distance < dist_to_light)
        return color;
#else
    Sphere obstacle_sphere;
    float f_obstacle_sphere = nearest_intersection_sphere(surface_to_light, scene.spheres.data(), scene.spheres.size(), obstacle_sphere);
    //In shadow
    if (f_obstacle_sphere > 0 && f_obstacle_sphere < dist_to_light)
        return color;

    Triangle obstacle_triangle;
    float f_obstacle_triangle = nearest_intersection_triangle(surface_to_light, scene.triangles.data(), scene.triangles.size(), obstacle_triangle);
    //In shadow
    if (f_obstacle_triangle > 0 && f_obstacle_triangle < dist_to_light)
        return color;
#endif

    // case where the sphere is in light
    float angle = dot(surface_to_light.D, normal);
    float intensity = light.intensity * angle / (dist_to_light * dist_to_light);
    if (intensity < 0) intensity = 0;
    color = intensity * light.color;

    return color;
}

Vec3<float> get_illumination(Vec3<float> hit_point, const Vec3<float>& normal, Scene& scene)
{
    //Compute accumulated light intensity
    Vec3<float> illumination = {0, 0, 0};

    // For each cube_light in the scene
    for(int cl = 0; cl < scene.nb_lights; cl++) {
        
        LightCube light_cube = scene.light_cubes[cl];
        // For severals light_point in the light_cube
        for (int l = 0; l < light_samples; l++)
        {
            Light light_point = light_cube.new_random_point_light(light_samples);
            illumination = illumination + get_point_light_illumination(light_point, hit_point, normal, scene);
        }
    }
    return illumination;
}

Ray get_ray(const Vec3<float>& pixel, const Vec3<float>& camera){
    Ray ray;
    ray.P = camera;
    ray.D = normalise(pixel - camera);
    return ray;
}

Vec3<float> trace(const Ray& ray, Scene& scene)
{
    Vec3<float> color = {0, 0, 0};

    Hit_result hit = scene.root->intersection(ray);
    if (hit.distance >= 0)
    {
        //Compute color !
        Vec3<float> illumination = get_illumination(hit.hit_point, hit.normal, scene);
        color = hit.color * illumination;
    }

    return color;
}

Vec3<float> trace_ray(const Vec3<float>& hit_point, Vec3<float>& normal, Vec3<float>& col, Scene& scene){
    Vec3<float> illumination = {0, 0, 0};                
    Vec3<float> light_cube_illumination = {0, 0, 0};

    // For each cube_light in the scene
    for(int cl = 0; cl < scene.nb_lights; cl++){

    LightCube light_cube = scene.light_cubes[cl];
        // For severals light_point in the light_cube
        for (int l = 0; l < light_samples; l++)
        {
            Light light_point = light_cube.new_random_point_light(light_samples);
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

void draw_scene_ppm(PPM& ppm, Scene& scene){
    Timer timer;
    timer.reset();

#ifdef USE_BVH
    compute_bvh(scene);
#endif

#ifdef DEBUG_BVH
    Image depth(ppm.width, ppm.height);
    float max = 0;
    for (int x = 0; x < ppm.width; x++)
    {
        for (int y = 0; y < ppm.height; y++)
        {
            Ray ray = get_ray({(float) x, 0, (float) y}, scene.camera_cube.position);
            Hit_result hit = scene.root->intersection(ray);
            float d = hit.distance;
            if (max < d) max = d;
            depth.set(x, y, {d, d, d});
        }
    }
    std::cout << "max depth : " << max << std::endl;

    for (int x = 0; x < ppm.width; x++)
    {
        for (int y = 0; y < ppm.height; y++)
        {
            Vec3<float> pixel = depth.get(x, y);
            pixel.x = 1 - (pixel.x / max);
            pixel.y = 1 - (pixel.y / max);
            pixel.z = 1 - (pixel.z / max);
            depth.set(x, y, pixel);
        }
    }
    depth.save("imgs/depthtest.ppm");
#endif

    std::cout << "BVH Computed in " << timer.elapsed() << " seconds" << std::endl;
    timer.reset();
    std::cout << "Rendering Image" << std::endl;

    //New line for progress
    std::cout << std::endl;
    for(int i = 0; i < ppm.width; i++){
        std::cout << "\r Progress " << (i+1)  << "/" << ppm.width;
        
        #pragma omp parallel for    
        for(int j = 0; j < ppm.height; j++){

            Vec3<float> pixel = {(float)i, 0, (float)j};

            Vec3<float> total_color = {0, 0, 0};

            for(int c=0; c < camera_samples; c++){


                Vec3<float> camera_point =  scene.camera_cube.new_random_point();
                Ray r = get_ray(pixel, camera_point);

#ifdef USE_BVH
                Hit_result hit = scene.root->intersection(r);
               
                //Convert to RGB ranges
                total_color = trace(r, scene);
                //Normalize by clamping to [0;1] domain
                total_color.x = clamp(0, 1, total_color.x);
                total_color.y = clamp(0, 1, total_color.y);
                total_color.z = clamp(0, 1, total_color.z);
                //Apply gamma correction
                total_color.x = std::pow(total_color.x, 1/2.2f);
                total_color.y = std::pow(total_color.y, 1/2.2f);
                total_color.z = std::pow(total_color.z, 1/2.2f);
                //Map to [0;255]
                total_color.x = lerp(0, 255, total_color.x);
                total_color.y = lerp(0, 255, total_color.y);
                total_color.z = lerp(0, 255, total_color.z);
#else 
                Sphere s;
                float dist_sphere = nearest_intersection_sphere(r, scene.spheres.data(), scene.spheres.size(), s); 

                Triangle t;
                float dist_triangle = nearest_intersection_triangle(r, scene.triangles.data(), scene.triangles.size(), t);

                if(dist_triangle > -1 && dist_triangle < dist_sphere)
                    draw_triangle(r, dist_triangle, t, scene, total_color);

                else if(dist_sphere > -1 )
                    draw_sphere(r, dist_sphere, s, scene, total_color);
#endif
                ppmcolor color = easyppm_rgb(
                    (char) (total_color.x/ (float) camera_samples),
                    (char) (total_color.y/ (float) camera_samples),
                    (char) (total_color.z/ (float) camera_samples)
                    );
                easyppm_set(&ppm, i, j, color);

            }
        }
    }
    //New line to clean up after progress
    std::cout << std::endl;
    std::cout << " Image Rendered in " << timer.elapsed() << " seconds." << std::endl;
}



int main()
{
    std::string filename = "imgs/test.ppm";

    Scene scene;
    Node root;

    // WHITE FLOOR
    Sphere s3;
    s3.C = {300, 20600, 300};
    s3.R = 20000;
    s3.color={1,1,1};
    // scene.spheres.push_back(s3);

    // ORANGE SPHERE
    Sphere s6;
    s6.C = {300, 500, 000};
    s6.R = 170;
    s6.color={1.0f, 0.3f, 0.1f};
    scene.spheres.push_back(s6);

    // // BLUE SPHERE
    Sphere s7;
    s7.C = {100, 200, 200};
    s7.R = 80;
    s7.color={0,0.3f,1};
    scene.spheres[2]=s7;
    scene.spheres.push_back(s7);

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

    Vec3<float> pos = { 300,500,170 };
    for(int clone = 0; clone < 3; clone++)
    {
        pos.x += 100;
        std::vector<Triangle> mesh0 = read_file("./off/r2.off", 0.3f, pos);
        scene.triangles.insert(scene.triangles.end(), mesh0.begin(), mesh0.end());
    }

    draw_scene_ppm(ppm, scene);

    easyppm_write(&ppm, filename.c_str());
    std::cout << "Saved image in " << filename << std::endl;
    return 0;
}
