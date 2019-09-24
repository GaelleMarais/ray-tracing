#pragma omp parallel for
#include <stdio.h>
#include <math.h>
#include "ray.h"
#include "sphere.h"
#include "cube.h"
#include "vec3.h"
#include "easyppm.h"
#include "easyppm.c"

int INTENS = 300;

void draw_sphere_ppm(PPM ppm, Sphere s, Sphere spheres[], int nb_spheres){
    int i,j;
    
    for(i=0; i<ppm.height;i++){
        for(j=0; j<ppm.width;j++){

            Ray r;
            r.P = {(float)i,(float)j,0};
            r.D = {0,0,1};

            // creating a cube light source
            Cube light;
            light.P = {500,200,100};
            light.size = 100;

            // show the cube light source with a red pixel
            easyppm_set(&ppm, light.P.y, light.P.x, easyppm_rgb(255,0,0)); 
            easyppm_set(&ppm, light.P.y+light.size, light.P.x, easyppm_rgb(255,0,0));
            easyppm_set(&ppm, light.P.y+light.size, light.P.x+light.size, easyppm_rgb(255,0,0));
            easyppm_set(&ppm, light.P.y, light.P.x+light.size, easyppm_rgb(255,0,0)); 

            float f = intersection(r,s);             

            if(f>0){ // case where the sphere is in the screen
                
                int l;
                for(int l=0;l<50;l++){
                    Vec3<float> lightPoint= randomPoint(light);
                    easyppm_set(&ppm, lightPoint.y, lightPoint.x, easyppm_rgb(255,255,255)); 

                    Vec3<float> X = {r.P.x+f*r.D.x,r.P.y+f*r.D.y,r.P.z+f*r.D.z}; // coordinates for the point on the surface
                    Ray sphere_to_light; // ray from the surface to the light source
                    sphere_to_light.P = X;
                    sphere_to_light.D = {(lightPoint.x -X.x), (lightPoint.y -X.y),(lightPoint.z -X.z)};

                    //easyppm_set(&ppm, j, i, easyppm_rgb(red,green,blue));

                    int k;
                    Sphere obstacle;
                    int obs = 0;
                    
                    for (k=0; k<nb_spheres;k++){
                        Sphere s2 = spheres[k];


                        float f2 = intersection(sphere_to_light, s2);

                        Vec3<float> O = {sphere_to_light.P.x+f2*sphere_to_light.D.x,
                                        sphere_to_light.P.y+f2*sphere_to_light.D.y,
                                        sphere_to_light.P.z+f2*sphere_to_light.D.z}; // coordinates for the point on the obstacle
                        Vec3<float> d = {(O.x -X.x), (O.y -X.y),(O.z -X.z)};
                        

                         if (f2==0){ //case where no object is obstructing the light

                            float dist_to_light = norm(sphere_to_light.D);  

                            Ray center_to_surface;
                            center_to_surface.P = s.C;
                            center_to_surface.D = {X.x -s.C.x, X.y -s.C.y, X.z -s.C.z};

                            float n = norm(center_to_surface.D);
                            Vec3<float> normal = {center_to_surface.D.x/n, center_to_surface.D.y/n, center_to_surface.D.z/n};
                            float intensity = INTENS * (1/((dist_to_light*dist_to_light))*(abs(dot(normal,sphere_to_light.D))));

                            // DEBUG
                            // printf("Distance to light : %f        Intensity: %f \n", dist, intensity);

                            // with 3d effect using depth between the sphere and the light
                            easyppm_set(&ppm, j, i, easyppm_rgb(intensity*s.color.x*255,intensity*s.color.y*255,intensity*s.color.z*255));  

                        }else{
                            // DEBUG
                            // easyppm_set(&ppm, j, i, easyppm_rgb(50,50,50));
                        }                                                                                                                           
                    } 
                    if(obs==0){
                        // float dist_to_light = norm(sphere_to_light.D);                            

                        // Ray center_to_surface;
                        // center_to_surface.P = s.C;
                        // center_to_surface.D = {X.x -s.C.x, X.y -s.C.y, X.z -s.C.z};

                        // float n = norm(center_to_surface.D);
                        // Vec3<float> normal = {center_to_surface.D.x/n, center_to_surface.D.y/n, center_to_surface.D.z/n};
                        // float intensity = INTENS * (1/((dist_to_light*dist_to_light))*(abs(dot(normal,sphere_to_light.D))));

                        // // DEBUG
                        // // printf("Distance to light : %f        Intensity: %f \n", dist, intensity);

                        // // with 3d effect using depth between the sphere and the light
                        // easyppm_set(&ppm, j, i, easyppm_rgb(intensity*s.color.x*255,intensity*s.color.y*255,intensity*s.color.z*255));            
                    }                   
                }
            }                    
        }
    }
}


int main(int argc, char* argv[])
{
    char filename[250];
    Sphere spheres[10];
    int nb_col=600;
    int nb_line=600;

    sscanf("imgs/light.ppm","%s", filename);
    printf("Writing %s ... ", filename);

    // VERT
    Sphere s1;
    s1.C = {250,150,600};
    s1.R = 80;
    s1.color={0,1,1};
    spheres[0]=s1;

    // JAUNE
    Sphere s2;
    s2.C = {300,400,600};
    s2.R = 70;
    s2.color={1,1,0};
    spheres[1]=s2;

    // ROUGE
    Sphere s3;
    s3.C = {50,300,900};
    s3.R = 300;
    s3.color={1,0,0};
    spheres[2]=s3;


    int nb_sphere=3;

    PPM ppm = easyppm_create(nb_col, nb_line, IMAGETYPE_PPM);

    draw_sphere_ppm(ppm,s3,spheres,nb_sphere);
    draw_sphere_ppm(ppm,s2,spheres,nb_sphere);
    draw_sphere_ppm(ppm,s1,spheres,nb_sphere);

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;

}
