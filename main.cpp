#include <stdio.h>
#include <math.h>
#include "ray.h"
#include "sphere.h"
#include "light.h"
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

            // creating a light source
            Light light;
            light.P= {300,300,100};
            light.intensity =0.5;
            easyppm_set(&ppm, light.P.y, light.P.x, easyppm_rgb(255,0,0)); // show the light source with a red pixel

            float f = intersection(r,s);             

            if(f>0){ // case where the sphere is in the screen
                

                Vec3<float> X = {r.P.x+f*r.D.x,r.P.y+f*r.D.y,r.P.z+f*r.D.z}; // coordinates for the point on the surface
                Ray sphere_to_light; // ray from the surface to the light source
                sphere_to_light.P = X;
                sphere_to_light.D = {(light.P.x -X.x), (light.P.y -X.y),(light.P.z -X.z)};

                //easyppm_set(&ppm, j, i, easyppm_rgb(red,green,blue));

                int k;


                
                for (k=0; k<nb_spheres;k++){
                    Sphere s2 = spheres[k];

                    float f2 = intersection_max(sphere_to_light, s2);

                    Vec3<float> O = {sphere_to_light.P.x+f2*sphere_to_light.D.x,
                                     sphere_to_light.P.y+f2*sphere_to_light.D.y,
                                     sphere_to_light.P.z+f2*sphere_to_light.D.z}; // coordinates for the point on the obstacle
                    Vec3<float> d = {(O.x -X.x), (O.y -X.y),(O.z -X.z)};
                    

                    if (f2==0 || norm(d)<2 ){ //case where no object is obstructing the light

                        //easyppm_set(&ppm, j, i, easyppm_rgb(red*255,green*255,blue*255)); // plain sphere
                        //easyppm_set(&ppm, j, i, easyppm_rgb(red-f/2,green-f/2,blue-f/2)); // with 3d effect using depth between the sphere and the screen
                        float dist_to_light = norm(sphere_to_light.D);
                        

                        Ray center_to_surface;
                        center_to_surface.P = s.C;
                        center_to_surface.D = {X.x -s.C.x, X.y -s.C.y, X.z -s.C.z};

                        float n = norm(center_to_surface.D);
                        Vec3<float> normal = {center_to_surface.D.x/n, center_to_surface.D.y/n, center_to_surface.D.z/n};
                        float intensity = 150 * (1/((dist_to_light*dist_to_light))*(dot(normal,sphere_to_light.D)));

                        // DEBUG
                        // printf("Distance to light : %f        Intensity: %f \n", dist, intensity);

                        // with 3d effect using depth between the sphere and the light
                        easyppm_set(&ppm, j, i, easyppm_rgb(intensity*red*255,intensity*green*255,intensity*blue*255));                                               
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

    sscanf("imgs/out.ppm","%s", filename);
    printf("Writing %s ... ", filename);

    Sphere s1;
    s1.C = {150,150,300};
    s1.R = 100;
    spheres[0]=s1;

    Sphere s2;
    s2.C = {400,400,400};
    s2.R = 100;
    spheres[1]=s2;

    Sphere s3;
    s3.C = {50,600,300};
    s3.R = 200;
    spheres[2]=s3;

    Sphere s4;
    s4.C = {480,480,200};
    s4.R = 50;
    spheres[3]=s4;

    PPM ppm = easyppm_create(nb_col, nb_line, IMAGETYPE_PPM);
    draw_sphere_ppm(ppm,s4,1,0.3,0.3,spheres,4);
    draw_sphere_ppm(ppm,s3,0,1,1,spheres,4);
    draw_sphere_ppm(ppm,s2,1,1,0,spheres,4);
    draw_sphere_ppm(ppm,s1,0.2,1,0.2,spheres,4);

    easyppm_write(&ppm, filename);
    printf("OK\n");

    return 0;

}
