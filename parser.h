#pragma once 
#include "vec3.h"
#include "triangle.h"
#include "scene.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Triangle* read_file(string filename, int size){

  string buff;
  ifstream file (filename);

  if (file.is_open()){

    getline(file, buff); // OFF

    int nb_vertices, nb_triangles;
    char delim=' ';

    getline(file,buff, delim); 
    nb_vertices = stoi(buff);
    getline(file,buff, delim); 
    nb_triangles = stoi(buff);
    getline(file,buff, delim);  // nb_edges


    Vec3<float> vertices[nb_vertices];
    Triangle* triangles = (Triangle *) malloc (sizeof (Triangle) * nb_triangles);

    for(int i = 0; i < nb_vertices; i++){

        getline(file, buff, delim);
        float x = stof(buff) * (float) size;
        getline(file, buff, delim);
        float y = stof(buff) * (float) size;
        getline(file, buff, delim);
        float z = stof(buff) * (float) size;

        vertices[i] = {x, y, z};

    }

    for( int j = 0; j < nb_triangles; j++){

        Triangle triangle;

        getline(file, buff, delim); // 3

        getline(file, buff, delim);
        triangle.a = vertices[stoi(buff)];
        getline(file, buff, delim);
        triangle.b = vertices[stoi(buff)];
        getline(file, buff, delim);
        triangle.c = vertices[stoi(buff)];

        triangle.color = {1, 0, 0};

        triangles[j] = triangle;


     }
    file.close();

    return triangles;
  }

  return 0;
}