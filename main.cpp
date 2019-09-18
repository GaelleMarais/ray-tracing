#include <stdio.h>
#include <math.h>
#include "ray.h"
#include "sphere.h"
#include "vec3.h"
#include "easyppm.h"
#include "easyppm.c"


void draw_sphere_ppm(PPM ppm, Sphere s, float red, float green, float blue, Sphere spheres[], int nb_spheres){
    int i,j;
    for(i=0; i<ppm.height;i++){
        for(j=0; j<ppm.width;j++){

            Ray r;
            r.P = {(float)i,(float)j,0};
            r.D = {0,0,1};

            Vec3<float> light= {30,300,300};
            easyppm_set(&ppm, light.y, light.x, easyppm_rgb(255,0,0));
            float f = intersection(r,s);

            if(f>0){ // L'objet est dans l'ecran
                

                // Est ce que l'objet est eclairé ?
                Vec3<float> X = {r.P.x+f*r.D.x,r.P.y+f*r.D.y,r.P.z+f*r.D.z}; // Le point de la surface
                Ray sphere_to_light;
                sphere_to_light.P = X;
                sphere_to_light.D = {(light.x -X.x), (light.y -X.y),(light.z -X.z)};

                easyppm_set(&ppm, j, i, easyppm_rgb(red,green,blue));

                // int k;
                // for (k=0;k<nb_spheres;k++){ // Pour toutes les autres spheres
                //     Sphere sph= spheres[k]; // Une sphere qui pourrait obstruer la lumiere
                //     int colision = 0;
                //     if(!(s==sph)){

                //         Vec3<float> X = {r.P.x+f*r.D.x,r.P.y+f*r.D.y,r.P.z+f*r.D.z};
                //         Ray sphere_to_light;
                //         sphere_to_light.P = X;
                //         sphere_to_light.D = {std::sqrt(powf(light.x -X.x, 2)), std::sqrt(powf(light.y -X.y, 2)), std::sqrt(powf(light.z -X.z, 2))};

                //         if(!(s==sph)){
                //             float ff = intersection(sphere_to_light,sph);
                //             if (ff!=0){ // Pas d'obstacle
                //                 colision=1;
                //             }
                //         }
                //     }
                //     if (colision==0){
                //         easyppm_set(&ppm, j, i, easyppm_rgb(red-f/2,green-f/2,blue-f/2));
                //     }
                // }
            }
        }
    }
}


int main(int argc, char* argv[])
{
    char filename[250];
    Sphere spheres[2];
    int nb_col=600;
    int nb_line=600;

    sscanf("imgs/out.ppm","%s", filename);
    printf("Writing %s ... ", filename);

    Sphere s1;
    s1.C = {300,300,300};
    s1.R = 150;
    spheres[0]=s1;

    Sphere s2;
    s2.C = {200,350,200};
    s2.R = 100;
    spheres[1]=s2;

    PPM ppm = easyppm_create(nb_col, nb_line, IMAGETYPE_PPM);
    draw_sphere_ppm(ppm,s1,200,0,0,spheres,2);
    draw_sphere_ppm(ppm,s2,0,0,200,spheres,2);

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;

}
