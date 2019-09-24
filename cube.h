#include "vec3.h"

struct Cube{
    Vec3<float> P;
    float size;
};

Vec3<float> randomPoint(Cube cube){
    float randx =  (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*cube.size + cube.P.x;
    float randy =  (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*cube.size + cube.P.y;
    float randz =  (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*cube.size + cube.P.z;

    Vec3<float> vec = {randx,randy,randz};
    // DEBUG
    // printf("randx : %f     randy : %f     randz : %f\n", randx, randy, randz);
    return vec;
}