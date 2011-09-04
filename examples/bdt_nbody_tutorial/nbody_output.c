/* nbody_output.c */

#include <stdio.h>
#include <stdcl.h>

//#define cl_float4_x(f) (((cl_float*)&(f))[0])

void nbody_output( int n, cl_float4* pos, cl_float4* vel, int step )
{

   int i;

   char filename[20];
   sprintf(filename,"sample_%.4d.csv",step);
   FILE* fd = fopen(filename,"w+");

   cl_float4 center = (cl_float4){0.0f,0.0f,0.0f,0.0f};

   fprintf(fd,"x,y,z,m\n");

   for(i=0;i<n;i++) {

      cl_float4_x(center) += cl_float4_x(pos[i]);
      cl_float4_y(center) += cl_float4_y(pos[i]);
      cl_float4_z(center) += cl_float4_z(pos[i]);
      cl_float4_w(center) += cl_float4_w(pos[i]);

      fprintf(fd,"%e,%e,%e,%d\n",
	      cl_float4_x(pos[i]),cl_float4_y(pos[i]),cl_float4_z(pos[i]),(cl_float4_w(pos[i])>0)?1:0 );

   }

   cl_float4_x(center) /= n;
   cl_float4_y(center) /= n;
   cl_float4_z(center) /= n;
   cl_float4_w(center) /= n;

   //   printf("center => {%e,%e,%e}\n",
   //      cl_float4_x(center),cl_float4_y(center),cl_float4_z(center) );

   fclose(fd);

}

