#pragma once 
#include "vec3.h"
#include "light.h"
#include "camera.h"
#include "sphere.h"
#include "tree.h"

#include <vector>


struct Scene{
    int nb_lights;
    LightCube light_cubes[10];
    std::vector<Triangle> triangles;
    std::vector<Sphere> spheres;
    CameraCube camera_cube;
    Tree* root = nullptr;
};


void compute_bvh(Scene& scene)
{
    //Delete if already exists
    if (scene.root)
        delete scene.root;

    //Build tree from scene data
    
    std::vector<Tree*> leaves;
    for (int i = 0; i < scene.spheres.size(); i++)
    {
        Sphere_leaf* s = new Sphere_leaf();
        s->sphere = &scene.spheres[i];
        leaves.push_back(s);
    }

    for (int i = 0; i < scene.triangles.size(); i++)
    {
        Triangle_leaf* t = new Triangle_leaf();
        t->triangle = &scene.triangles[i];
        leaves.push_back(t);
    }

    scene.root = build_tree(leaves);
}