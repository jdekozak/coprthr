/* nbody_init.c */

#include <stdlib.h>

#include <stdcl.h>

#include <math.h>

#define frand() (rand()/(cl_float)RAND_MAX)

void nbody_init( int n, cl_float4* pos, cl_float4* vel )
{

   int i;

   srand(2112);

   for(i=0;i<n/2;++i) {
     cl_float u = 2*frand()-1;
     cl_float square = sqrt(1-u*u);
     cl_float theta = 2*M_PI*frand();
     if(i%2) {
       pos[i] = (cl_float4){square*cos(theta),square*sin(theta),u,frand()};
     } else {
       pos[i] = (cl_float4){0.8*square*cos(theta),0.8*square*sin(theta),0.8*u,-frand()};
     }
      vel[i] = (cl_float4){5.0f,5.0f,5.0f,0.0f};
   }

   for(;i<n;++i) {
     cl_float u = 2*frand()-1;
     cl_float square = sqrt(1-u*u);
     cl_float theta = 2*M_PI*frand();
     if(i%2) {
       pos[i] = (cl_float4){0.6*square*cos(theta),0.6*square*sin(theta),0.6*u,frand()};
     } else {
       pos[i] = (cl_float4){0.4*square*cos(theta),0.4*square*sin(theta),0.4*u,-frand()};
     }
      vel[i] = (cl_float4){5.0f,5.0f,5.0f,0.0f};
   }

}

