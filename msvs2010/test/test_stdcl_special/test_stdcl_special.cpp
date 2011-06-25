
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
//#include <sys/mman.h>

#include "stdcl.h"

#define SIZE 128
#define BLOCKSIZE 4

#define __mapfile(file,filesz,pfile) do { \
int fd = open(file,O_RDONLY); \
struct stat fst; fstat(fd,&fst); \
if (fst.st_size == 0 || !S_ISREG(fst.st_mode)) { \
fprintf(stderr,"error: open failed on ''\n",file); \
pfile=0; \
} else { \
filesz = fst.st_size; \
pfile = mmap(0,filesz,PROT_READ,MAP_PRIVATE,fd,0); \
} \
close(fd); \
} while(0);

//#define ____exit(err,line) \
//do { fprintf("error code %d\n",err); __exit(line); } while(0)

#define __exit(line) \
do { printf("error code at line %d\n",line); exit(line); } while(0)

int main( int argc, char** argv)
{
int i,j;
cl_uint err; 

CONTEXT* cp = 0;
unsigned int devnum = 0;
size_t size = SIZE;
size_t blocksize = BLOCKSIZE;

i=1;
char* arg;
while(i<argc) {
arg=argv[i++];
if (!strncmp(arg,"--size",6)) size = atoi(argv[i++]);
else if (!strncmp(arg,"--blocksize",11)) blocksize = atoi(argv[i++]);
else if (!strncmp(arg,"--dev",6)) { cp = stddev; devnum = atoi(argv[i++]); }
else if (!strncmp(arg,"--cpu",6)) { cp = stdcpu; devnum = atoi(argv[i++]); }
else if (!strncmp(arg,"--gpu",6)) { cp = stdgpu; devnum = atoi(argv[i++]); }
else {
fprintf(stderr,"unrecognized option ''\n",arg);
__exit(-1);
}
}

if (!cp) __exit(__LINE__);
if (devnum >= clgetndev(cp)) __exit(__LINE__);
int* aa0 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa0) __exit(__LINE__);
int* bb0 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb0) __exit(__LINE__);
int* aa1 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!aa1) __exit(__LINE__);
int* bb1 = (int*)clmalloc(cp,size*sizeof(int),0);
if (!bb1) __exit(__LINE__);
for(i=0;i<size;i++) { 
aa0[i] = i+13*0; bb0[i] = 0; 
aa1[i] = i+13*1; bb1[i] = 0; 
}
void* clh = clopen(cp,"test_special.cl",CLLD_NOW);
cl_kernel krn;
clndrange_t ndr = clndrange_init1d(0,size,blocksize);
cl_event ev[4];
int sum,sum_correct;
krn = clsym(cp,clh,"test_arg_1_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 1 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_1_2_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,bb0);
clarg_set_global(cp,krn,2,bb1);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb1,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<1;++i)
for(j=0;j<2;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
for(i=0;i<size;i++) sum += bb1[i];
printf("(%d) sum 1 2 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
krn = clsym(cp,clh,"test_arg_2_1_kern",CLLD_NOW);
if (!krn) __exit(__LINE__);
if (!clmsync(cp,0,aa0,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,aa1,CL_MEM_DEVICE|CL_EVENT_NOWAIT))
__exit(__LINE__);
clarg_set_global(cp,krn,0,aa0);
clarg_set_global(cp,krn,1,aa1);
clarg_set_global(cp,krn,2,bb0);
if (!clfork(cp,0,krn,&ndr,CL_EVENT_NOWAIT))
__exit(__LINE__);
if (!clmsync(cp,0,bb0,CL_MEM_HOST|CL_EVENT_NOWAIT))
__exit(__LINE__);
clwait(cp,0,CL_KERNEL_EVENT|CL_MEM_EVENT);
sum_correct = 0;
for(i=0;i<2;++i)
for(j=0;j<1;++j)
sum_correct += (j+1)*( size*(size-1)/2 + 14*i*size );
sum = 0;
for(i=0;i<size;i++) sum += bb0[i];
printf("(%d) sum 2 1 %d\n",sum_correct,sum);
if (sum != sum_correct) __exit(__LINE__);
printf("done!\n");
return(0);
}
