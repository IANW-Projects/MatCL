/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <math.h>
#include "mex.h"
#include "matrix.h"
#include <iostream>
#include <string>
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#define access    _access_s
#else
#include <unistd.h>
#endif




#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/cl2.hpp>
#include "ocl_dev_mgr.hpp"



#include "MatCL.hpp"


class mystream : public std::streambuf
{
protected:
	virtual std::streamsize xsputn(const char *s, std::streamsize n) { mexPrintf("%.*s", n, s); return n; }
	virtual int overflow(int c = EOF) { if (c != EOF) { mexPrintf("%.1s", &c); } return 1; }
};
class scoped_redirect_cout
{
public:
	scoped_redirect_cout() { old_buf = std::cout.rdbuf(); std::cout.rdbuf(&mout); }
	~scoped_redirect_cout() { std::cout.rdbuf(old_buf); }
private:
	mystream mout;
	std::streambuf *old_buf;
};
static scoped_redirect_cout mycout_redirect;




void mexFunction( int nlhs, mxArray *plhs[],   int nrhs, const mxArray*prhs[] )
{

	size_t buflen;
	char *buf;
	char *settings;
	char *kernel_name_c;
	bool blocking = CL_FALSE;
	uint64_t mem_needed = 0;

	std::vector<cl::Buffer> data_in;
	std::vector<uint64_t> data_size;
	std::vector<cl::Buffer*> dev_Buffers;

	uint32_t global_range_x = 1;
	uint32_t global_range_y = 1;
	uint32_t global_range_z = 1;
	uint32_t range_start_x = 0;
	uint32_t range_start_y = 0;
	uint32_t range_start_z = 0;
	cl::NDRange range_start = cl::NullRange;
	cl::NDRange global_range;
	cl::NDRange local_range;

	uint64_t startTransfer, transferTime;

	Timer timer; //used to track performance

	ocl_dev_mgr& dev_mgr = ocl_dev_mgr::getInstance();

	uint32_t device = (uint32_t)mxGetScalar(prhs[0]) - 1;

    if (nrhs>2) {

	if (device<dev_mgr.get_avail_dev_num() ) {


		bool old_instance = false;
		bool compile_only = false;

		//reuse context  - kernels are already compiled
		if ((dev_mgr.get_context_num() > 0) && (mxIsChar(prhs[2]) == 0)) {
			old_instance = true;
		//	mexPrintf("Old instance found, running kernels only...\n");
		}

		//only build kernels - does not execute anything
		if (nrhs == 3) {
			compile_only = true;

			if (mxIsCell(prhs[1]) == true) {
				mexPrintf("Building multiple kernel files...\n");
			}
			else {
				mexPrintf("Building single kernel file...\n");
			}
		}

		if (compile_only == true) {

			mexPrintf("Device:  %s\n", dev_mgr.get_avail_dev_info(device).name.c_str());

			dev_mgr.deinitalize();

			dev_mgr.init_device(device);

			buflen = mxGetN(prhs[nrhs - 1]) + 1; //get Kernel Settings
			settings = (char *)mxMalloc(buflen);
			mxGetString(prhs[nrhs - 1], settings, (mwSize)buflen);


		//get Kernel URL
			getKernel_info(plhs, nrhs, prhs, &dev_mgr);


			uint64_t kernels_found = 0;
			uint64_t comp_time;
			comp_time= timer.getTimeMicroseconds();

			kernels_found = dev_mgr.compile_kernel(0, "ocl_Kernel", settings);

			uint64_t  *comp_time_ptr;
			plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
			comp_time_ptr = (uint64_t *)mxGetData(plhs[0]);
			comp_time_ptr[0] = timer.getTimeMicroseconds()-comp_time;

			//  transferTime = (timer.getTimeMicroseconds() - startTransfer);
			// mexPrintf("Copy:  %d\n", transferTime);

			if (kernels_found > 0) {
				mxArray * tmp_str;
				mxArray *cell_array_ptr;
				cell_array_ptr = mxCreateCellMatrix((mwSize)kernels_found, 1);
				for (uint32_t i = 0; i < kernels_found; i++) {
					//	mexPrintf("test: %s\n ", dev_mgr.getKernelbyID(i)->getInfo<CL_KERNEL_FUNCTION_NAME>());
					std::string kernel_name(dev_mgr.getKernelbyID(0, "ocl_Kernel",i)->getInfo<CL_KERNEL_FUNCTION_NAME>());
					tmp_str = mxCreateString(kernel_name.c_str());
					mxSetCell(cell_array_ptr, i, mxDuplicateArray(tmp_str));
				}

				plhs[1] = cell_array_ptr;

			}

			//	mexLock(); //prevent matlab from unloading mex file to keep context alive

		}

	//this part only runs the kernel
	if ((compile_only == false) && (old_instance == true)) {

		uint32_t num_in = (uint32_t)nrhs-5;//Number of input buffers

		uint32_t var_offset = 4;
		uint64_t  copy_time;

		std::vector<std::string> kernel_list;

		mwSize cell_dims;
		mxArray *cellElement;
		if (mxIsCell(prhs[1]) == true) {


			cell_dims = mxGetNumberOfElements(prhs[1]);
			for (uint32_t icell = 0; icell < cell_dims; icell++) {
				cellElement = mxGetCell(prhs[1], icell);

				buflen = mxGetN(cellElement) + 1;
				//mexPrintf("Size:  %d\n", buflen);

				char *kernel_name_c;
				kernel_name_c = (char *)mxMalloc(buflen);
				mxGetString(cellElement, kernel_name_c, (mwSize)buflen);
				kernel_list.push_back(std::string(kernel_name_c));
			//	mexPrintf("Kernel-Name:  %s\n", kernel_name_c);
			}
		}
		else {
			buflen = mxGetN(prhs[1]) + 1; //get Kernel Name
			kernel_name_c = (char *)mxMalloc(buflen);
			mxGetString(prhs[1], kernel_name_c, (mwSize)buflen);
			kernel_list.push_back(std::string(kernel_name_c));
			//mexPrintf("Kernel-Name:  %s\n", kernel_name.c_str());

		}

		//NDRange settings
		//global range

		size_t mrows = mxGetM(prhs[2]);
		size_t ncols = mxGetN(prhs[2]);

		if ((mxIsDouble(prhs[2]) || (mxGetClassID(prhs[2]) == mxUINT32_CLASS)) && !mxIsComplex(prhs[2]) && (mrows * ncols == 3)) {
			if (mxIsDouble(prhs[2])) {
				double  *range_ptr;
			range_ptr = mxGetPr(prhs[2]);

			global_range_x = (uint32_t)round(range_ptr[0]);
			global_range_y = (uint32_t)round(range_ptr[1]);
			global_range_z = (uint32_t)round(range_ptr[2]);
			}
			else {
				uint32_t  *range_ptr;
				range_ptr = (uint32_t *)mxGetData(prhs[2]);

				global_range_x = (uint32_t)(range_ptr[0]);
				global_range_y = (uint32_t)(range_ptr[1]);
				global_range_z = (uint32_t)(range_ptr[2]);

			}

			global_range = cl::NDRange(global_range_x, global_range_y, global_range_z);

		}
		else {
			if ((mxIsDouble(prhs[2]) || (mxGetClassID(prhs[2]) == mxUINT32_CLASS)) && !mxIsComplex(prhs[2]) && (mrows * ncols == 6)) {
				if (mxIsDouble(prhs[2])) {
					double  *range_ptr;
					range_ptr = mxGetPr(prhs[2]);
				range_start_x = (uint32_t)round(range_ptr[0]);
				range_start_y = (uint32_t)round(range_ptr[1]);
				range_start_z = (uint32_t)round(range_ptr[2]);

				global_range_x = (uint32_t)round(range_ptr[3]);
				global_range_y = (uint32_t)round(range_ptr[4]);
				global_range_z = (uint32_t)round(range_ptr[5]);

				}
				else {
					uint32_t  *range_ptr;
					range_ptr = (uint32_t *)mxGetData(prhs[2]);

					range_start_x = (uint32_t)(range_ptr[0]);
					range_start_y = (uint32_t)(range_ptr[1]);
					range_start_z = (uint32_t)(range_ptr[2]);

					global_range_x = (uint32_t)(range_ptr[3]);
					global_range_y = (uint32_t)(range_ptr[4]);
					global_range_z = (uint32_t)(range_ptr[5]);
				}

				range_start = cl::NDRange(range_start_x, range_start_y, range_start_z);
				global_range = cl::NDRange(global_range_x, global_range_y, global_range_z);
			}
			else {
				mexErrMsgIdAndTxt("OpenCL:NDRange", "Invalid global range defined!");
			}
		}

		//local range

		mrows = mxGetM(prhs[3]);
		ncols = mxGetN(prhs[3]);

		if ((mxIsDouble(prhs[3]) || (mxGetClassID(prhs[3]) == mxUINT32_CLASS)) && !mxIsComplex(prhs[3]) && (mrows + ncols == 4)) {
			if (mxIsDouble(prhs[3])) {
				double  *range_ptr;
				range_ptr = mxGetPr(prhs[3]);
			global_range_x = (uint32_t)round(range_ptr[0]);
			global_range_y = (uint32_t)round(range_ptr[1]);
			global_range_z = (uint32_t)round(range_ptr[2]);

		}
		else {
			uint32_t  *range_ptr;
			range_ptr = (uint32_t *)mxGetData(prhs[3]);

			global_range_x = (uint32_t)(range_ptr[0]);
			global_range_y = (uint32_t)(range_ptr[1]);
			global_range_z = (uint32_t)(range_ptr[2]);
		}

			local_range = cl::NDRange(global_range_x, global_range_y, global_range_z);

		}
		else {
			if (mrows + ncols == 2) {
				local_range = cl::NullRange;
			}
			else {
				mexErrMsgIdAndTxt("OpenCL:NDRange", "Invalid local range defined!");
				return;
			}
		}

		runkernel(plhs, nrhs, prhs, kernel_list, num_in,var_offset, &dev_mgr, device, range_start,global_range, local_range, false,false,copy_time);

		uint64_t  *copy_time_ptr;
		plhs[1] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
		copy_time_ptr = (uint64_t *)mxGetData(plhs[1]);
		copy_time_ptr[0] = copy_time;



	}

	//this part compiles and runs kernel
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	if ((compile_only == false) && (old_instance == false)) {
		//mexPrintf("Compile and run...\n");

		dev_mgr.deinitalize();

		compilerun(plhs, nrhs, prhs, &dev_mgr, device, false,false);
	}
	}

	else {
		mexErrMsgIdAndTxt("MATLAB:cl_dev", "OpenCl Device not found!");
	}
    } else {
        mexErrMsgIdAndTxt("MATLAB:syntax", "Incorrect Syntax!");
    }



	return;

}
