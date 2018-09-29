/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <math.h>
#include "mex.h"
#include "matrix.h"


#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/cl2.hpp>
#include "ocl_dev_mgr.hpp"


void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray*prhs[] ) {

    uint32_t devices_availble=0;

    ocl_dev_mgr& dev_mgr = ocl_dev_mgr::getInstance();
   devices_availble=dev_mgr.get_avail_dev_num();

   //get OpenCl device names
       mxArray * tmp_str;
       mxArray *cell_array_ptr;
   mxArray *matrix_ptr;

   char name_string[100]="";


   cell_array_ptr = mxCreateCellMatrix((mwSize)devices_availble,1);
   for (uint32_t i=0;i<devices_availble;i++) {
	   snprintf(name_string, 100, "%s(%s)", dev_mgr.get_avail_dev_info(i).name.c_str(),dev_mgr.get_avail_dev_info(i).ocl_version.c_str());
    tmp_str = mxCreateString(name_string);
   mxSetCell(cell_array_ptr,i,mxDuplicateArray(tmp_str));
   }

    plhs[0] = cell_array_ptr;

//get OpenCl Device type

cell_array_ptr = mxCreateCellMatrix((mwSize)devices_availble,1);
   for (uint32_t i=0;i<devices_availble;i++) {
     tmp_str = mxCreateString(dev_mgr.getDeviceType(i).c_str());
   mxSetCell(cell_array_ptr,i,mxDuplicateArray(tmp_str));
   }

    plhs[1] = cell_array_ptr;


       //get OpenCl Device mem size
    uint64_t  *pointer;

    matrix_ptr= mxCreateNumericMatrix(devices_availble, 1, mxUINT64_CLASS, mxREAL);
    pointer =(uint64_t  *) mxGetData(matrix_ptr);

   for (uint32_t i=0;i<devices_availble;i++) {
   pointer[i]=dev_mgr.get_avail_dev_info(i).max_mem;
   }

   plhs[2] = matrix_ptr;


   //get OpenCl Device WorkGroup size

   matrix_ptr = mxCreateNumericMatrix(devices_availble, 1, mxUINT64_CLASS, mxREAL);
   pointer = (uint64_t  *)mxGetData(matrix_ptr);

   for (uint32_t i = 0; i<devices_availble; i++) {
	   pointer[i] = dev_mgr.get_avail_dev_info(i).wg_size;
   }

   plhs[3] = matrix_ptr;

   //get OpenCl Device LocalWork size

   matrix_ptr = mxCreateNumericMatrix(devices_availble, 1, mxUINT64_CLASS, mxREAL);
   pointer = (uint64_t  *)mxGetData(matrix_ptr);

   for (uint32_t i = 0; i<devices_availble; i++) {
	   pointer[i] = dev_mgr.get_avail_dev_info(i).lw_size;
   }

   plhs[4] = matrix_ptr;

   //get OpenCl Device Compute units

   matrix_ptr = mxCreateNumericMatrix(devices_availble, 1, mxUINT64_CLASS, mxREAL);
   pointer = (uint64_t  *)mxGetData(matrix_ptr);

   for (uint32_t i = 0; i<devices_availble; i++) {
	   pointer[i] = dev_mgr.get_avail_dev_info(i).compute_units;
   }

   plhs[5] = matrix_ptr;
    return;

}
