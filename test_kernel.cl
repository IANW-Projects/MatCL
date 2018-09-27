/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
    #error "IEEE-754 double precision not supported by OpenCL implementation."
#endif

kernel void test1(global double4 *d_1,global double4 *d_2)
{
  uint idx = get_global_id(0);

  //Simple test just add a constant DT to the value of d_1. The constant DT is a kernel define to increase performance
  d_1[idx].x=d_1[idx].x+DT;

};


kernel void test2(global double4 *d_1,global double4 *d_2)
{
  uint idx = get_global_id(0);
  printf("Test: %d \n",idx);
    d_1[idx].w=d_2[idx].w+DT;

};
