#pragma once 
#include "vec3.h"
#include "triangle.h"
#include "scene.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;

std::vector<Triangle> read_file(string filename, float size, Vec3<float> pos) {
    std::vector<Triangle> triangles;

    string buff;
    ifstream file (filename);

    if (file.is_open()){

        getline(file, buff); // OFF

        int nb_vertices = 0;
        int nb_triangles = 0;
        char delim_space=' ';
        char delim_endline='\n';

        getline(file,buff, delim_space); 
        nb_vertices = stoi(buff);
        getline(file,buff, delim_space); 
        nb_triangles = stoi(buff);
        getline(file,buff, delim_endline);  // nb_edges


        Vec3<float> vertices[nb_vertices];

        for(int i = 0; i < nb_vertices; i++){
            getline(file, buff, delim_space);
            float x = stof(buff) * (float) size + pos.x;
            getline(file, buff, delim_space);
            float y = stof(buff) * (float) size + pos.y;
            getline(file, buff, delim_endline);
            float z = stof(buff) * (float) size + pos.z;

            vertices[i] = {x, y, z};  
            // DEBUG     
            // std::cout << vertices[i] << std::endl; 

        }

        for( int j = 0; j < nb_triangles; j++){
            Triangle triangle;

            getline(file, buff, delim_space); // 3

            getline(file, buff, delim_space);
            triangle.a = vertices[stoi(buff)];
            getline(file, buff, delim_space);
            triangle.b = vertices[stoi(buff)];
            getline(file, buff, delim_endline);
            triangle.c = vertices[stoi(buff)];

            triangle.color = {0, 1, 1};

            // triangles[j] = triangle;
            triangles.push_back(triangle);
        }
        file.close();

    }
    return triangles;
}