#pragma once
#include "ray.h"
#include "sphere.h"
#include "box.h"
#include "hitresult.h"
#include <vector>
#include <algorithm>

struct Tree {
    virtual ~Tree() = default;
    virtual Box get_bounding_box() const = 0;
    virtual Hit_result intersection (Ray ray) = 0;

    static bool compare_x_box(Tree* lhs, Tree* rhs)
    {
        Box lhsbox = lhs->get_bounding_box();
        Box rhsbox = rhs->get_bounding_box();
        Vec3<float> lhscenter = lhsbox.max - lhsbox.min;
        Vec3<float> rhscenter = rhsbox.max - rhsbox.min;
        return lhscenter.x < rhscenter.x;
    }

    static bool compare_y_box(Tree* lhs, Tree* rhs)
    {
        Box lhsbox = lhs->get_bounding_box();
        Box rhsbox = rhs->get_bounding_box();
        Vec3<float> lhscenter = lhsbox.max - lhsbox.min;
        Vec3<float> rhscenter = rhsbox.max - rhsbox.min;
        return lhscenter.y < rhscenter.y;
    }

    static bool compare_z_box(Tree* lhs, Tree* rhs)
    {
        Box lhsbox = lhs->get_bounding_box();
        Box rhsbox = rhs->get_bounding_box();
        Vec3<float> lhscenter = lhsbox.max - lhsbox.min;
        Vec3<float> rhscenter = rhsbox.max - rhsbox.min;
        return lhscenter.z < rhscenter.z;
    }

};


struct Sphere_leaf : public Tree{
    Sphere *sphere;

    virtual ~Sphere_leaf() {}

    virtual Box get_bounding_box() const {
        return create_box_from_sphere(*sphere);
    }

    Hit_result intersection(Ray ray) override{
        Hit_result result;
        result.distance = intersection_sphere(ray, *sphere);
        if (result.distance >= 0)
        {
            result.hit_point = ray.P + result.distance * ray.D;
            result.normal = normalise(result.hit_point - sphere->C);
            result.color = sphere->color;
        }
        return result;
    }
};

struct Triangle_leaf : public Tree{
    Triangle *triangle;

    virtual ~Triangle_leaf() {}

    virtual Box get_bounding_box() const {
        return create_box_from_triangle(*triangle);
    }


    Hit_result intersection(Ray ray) override {
        Hit_result result;
        result.distance = intersection_triangle(ray, *triangle);
        if (result.distance >= 0)
        {
            result.hit_point = ray.P + result.distance * ray.D;
            result.normal = normalise(cross(triangle->b - triangle->a, triangle->c - triangle->a));
            result.color = triangle->color;
        }
        return result;
    }
};

struct Node : public Tree {
    Tree *left, *right;
    Box box;

    virtual ~Node() {
        if (left) delete left;
        if (right) delete right;
    }
    
    virtual Box get_bounding_box() const {
        return box;
    }


    Hit_result intersection(Ray ray) override{        
        if (intersection_box(ray, box)) {
            Hit_result left_result;
            Hit_result right_result;
            if (left) left_result = left->intersection(ray);
            if (right) right_result = right->intersection(ray);
            
            if (left_result.distance < 0)
                return right_result;
            if (right_result.distance < 0)
                return left_result;
            if (left_result.distance <= right_result.distance)
                return left_result;
            else
                return right_result;
        }
        return Hit_result();
    }
};

Tree* build_tree(std::vector<Tree*> &leaves) {
    if (leaves.empty())
        return nullptr;
    
    if (leaves.size() == 1) {
        return leaves[0];
    }

    //Multiple spheres, subdivide
    Node* node = new Node();
    node->left = nullptr;
    node->right = nullptr;

    //Compute bounding box
    node->box = leaves[0]->get_bounding_box();
    for (unsigned int i = 1; i < leaves.size(); i++)
    {
        Box sphere_box = leaves[i]->get_bounding_box();
        node->box = create_box_from_boxes(node->box, sphere_box);
    }
    float dx = node->box.max.x - node->box.min.x;
    float dy = node->box.max.y - node->box.min.y;
    float dz = node->box.max.z - node->box.min.z;
    bool CUT_X = dx >= dy && dx >= dz;
    bool CUT_Y = !CUT_X && dy >= dz;
    bool CUT_Z = !CUT_X && !CUT_Y;
    if (CUT_X) std::sort(leaves.begin(), leaves.end(), Tree::compare_x_box);
    if (CUT_Y) std::sort(leaves.begin(), leaves.end(), Tree::compare_y_box);
    if (CUT_Z) std::sort(leaves.begin(), leaves.end(), Tree::compare_z_box);

    int middle = leaves.size() / 2;
    std::vector<Tree*> left;
    std::vector<Tree*> right;
    for(unsigned int i = 0; i < leaves.size(); i++)
    { 
        if (i < middle)
            left.push_back(leaves[i]);
        else
            right.push_back(leaves[i]);
    }

    if (!left.empty()) node->left = build_tree(left);
    if (!right.empty()) node->right = build_tree(right);
    return node;
}
