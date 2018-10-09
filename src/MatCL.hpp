/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifndef MATCL_H
#define MATCL_H

#include <math.h>
#include "mex.h"
#include "matrix.h"
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <process.h>
#endif


#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/cl2.hpp>
#include "ocl_dev_mgr.hpp"


#include "utils.hpp"
inline bool FileExists(const std::string &Filename)
{
	return access(Filename.c_str(), 0) == 0;
}

inline std::string loadProgram(std::string input)
{
	std::ifstream stream(input.c_str());
	if (!stream.is_open()) {
		std::cout << "Cannot open file: " << input << std::endl;
		exit(1);
	}

	return std::string(
		std::istreambuf_iterator<char>(stream),
		(std::istreambuf_iterator<char>()));
}

inline void remove_empty_lines(std::istream& in, std::ostream& out)
{
	std::string line;

	while (std::getline(in, line)) {
		bool is_empty = true;
		if (!line.empty()) {
			for (uint32_t i = 0; i < line.length(); i++)
			{
				if ((line.at(i) != 32) && (line.at(i) != '\n')) {
					is_empty = false;
				}
			}
		}
		if (is_empty == false) {
			out << line << '\n';
		}
	}
}


int32_t getKernel_info(mxArray *plhs[], int nrhs, const mxArray*prhs[], ocl_dev_mgr *dev_mgr) {

	size_t buflen;

	//get Kernel URL
	std::string kernel_data;

	mwSize cell_dims;
	mxArray *cellElement;
	if (mxIsCell(prhs[1]) == true) {


		cell_dims = mxGetNumberOfElements(prhs[1]);
		for (uint32_t icell = 0; icell < cell_dims; icell++) {
			cellElement = mxGetCell(prhs[1], icell);

			buflen = mxGetN(cellElement) + 1;
			//mexPrintf("Size:  %d\n", buflen);

			char *kernel_url_c;
			kernel_url_c = (char *)mxMalloc(buflen);
			mxGetString(cellElement, kernel_url_c, (mwSize)buflen);
			std::string kernel_url(kernel_url_c);
			//mexPrintf("Kernel-URL:  %s\n", kernel_url_c);
			if (FileExists(kernel_url) == true) {
				kernel_data.append(loadProgram(kernel_url));
				kernel_data.append("\n");
			}
			else {
				mexErrMsgIdAndTxt("MATLAB:cl_program", "OpenCl Kernel file not found!");
				return -1;
			}

		}
		dev_mgr->add_program_str(0, "ocl_Kernel", kernel_data);
	}
	else {
		char *kernel_url_c;
		buflen = mxGetN(prhs[1]) + 1;
		kernel_url_c = (char *)mxMalloc(buflen);
		mxGetString(prhs[1], kernel_url_c, (mwSize)buflen);
		std::string kernel_url(kernel_url_c);
		//mexPrintf("Kernel-URL:  %s\n", kernel_url_c);
		if (dev_mgr->add_program_url(0, "ocl_Kernel", kernel_url) < 0) {  //Add kernel source
			mexErrMsgIdAndTxt("MATLAB:cl_program", "OpenCl Kernel file not found!");
			return -1;
		}

	}


	return 0;

}

int32_t runkernel(mxArray *plhs[], int nrhs, const mxArray*prhs[], std::vector<std::string> &kernel_list, uint32_t num_in, uint32_t mvar_offset, ocl_dev_mgr *dev_mgr, uint32_t device,  cl::NDRange range_start, cl::NDRange global_range, cl::NDRange local_range, bool debug_mode, bool log_file, uint64_t &copy_time)
{
	size_t buflen;
	char *buf;
	char *settings;
	char *kernel_name_c;
	bool blocking = CL_FALSE;
	uint64_t mem_needed = 0;

	std::vector<cl::Buffer> data_in;
	std::vector<uint64_t> data_size;

	uint64_t startTransfer, transferTime;
	Timer timer; //used to track performance

				 //used for kernel printf
#if defined(_WIN32)
	COORD buffer_size;
	SMALL_RECT rect;
#endif
#if !defined(_WIN32)
	char buffer[4096];
	auto fp = fmemopen(buffer, 4096, "w");
	auto old = stdout;

#endif


	uint32_t var_offset = mvar_offset;

	//this part compiles and runs kernel
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	//mexPrintf("Compile and run...\n");

	if ((debug_mode == true) || ((log_file == true))) {
#if defined(_WIN32)

		AllocConsole();

#define con_rows 150
#define con_cols 120


		//get info un biggest possible console buffer
		buffer_size = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));
		if (buffer_size.X > con_cols) {
			buffer_size.X = con_cols;
		}
		if (buffer_size.Y > con_rows) {
			buffer_size.Y = con_rows;
		}
		rect = { 0, 0,  buffer_size.X - 1,buffer_size.Y - 1 };
		//std::cout << buffer_size.X << "%" << buffer_size.Y << std::endl;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), buffer_size);
		SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

#endif
#if !defined(_WIN32)

		memset(buffer, 0, 4096);
		if (!fp) { printf("Error allocating buffer!"); return -1; }


		stdout = fp;

#endif
	}


	bool all_rw = mxIsScalar(prhs[var_offset + num_in]);
	double  *rw_flags_ptr;

	if (all_rw == true) {
		//no read/write flags specified - treat all as rw buffer
		rw_flags_ptr = new double[num_in];
		std::fill(rw_flags_ptr, rw_flags_ptr + num_in, 0);
	}
	else {
		rw_flags_ptr = mxGetPr(prhs[var_offset + num_in]);
	}

	uint64_t push_time, pull_time;
	push_time = timer.getTimeMicroseconds();

	//create input OCL buffer
	for (uint32_t i = 0; i < num_in; i++) {

		//mxGetM="datatype" 1=cl_float,2=cl_float2,4=cl_float4;
		//mxGetN=num_elements

		uint64_t buf_size = mxGetN(prhs[var_offset])*mxGetM(prhs[var_offset]);

		switch (mxGetClassID(prhs[var_offset])) {
		case mxSINGLE_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_float)); break;
		case mxDOUBLE_CLASS: buf_size = buf_size * uint64_t(sizeof(cl_double)); break;
		case mxINT8_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_char)); break;
		case mxUINT8_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_uchar)); break;
		case mxINT16_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_short)); break;
		case mxUINT16_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_ushort)); break;
		case mxINT32_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_int)); break;
		case mxUINT32_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_uint)); break;
		case mxINT64_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_long)); break;
		case mxUINT64_CLASS: buf_size = uint64_t(buf_size * sizeof(cl_ulong)); break;
		}
		//mexPrintf("I Buffer Size:  %ld,%d,%d\n", buf_size, mxGetN(prhs[var_offset]), mxGetM(prhs[var_offset]));
		//	std::cout << buf_size <<"/"<< dev_mgr.get_avail_dev_info(device).max_mem_alloc << std::endl;
		data_size.push_back(buf_size);
		mem_needed = mem_needed + buf_size;
		//	 mexPrintf("I Buffer Size:  %d\n", buf_size);
		//mexPrintf("Var Size:  %d\n", sizeof(cl_float));
		//	 mexPrintf("I Datatype: %s\n", mxGetClassName(prhs[var_offset]));

		if (dev_mgr->get_avail_dev_info(device).max_mem_alloc < buf_size) {
			mexWarnMsgIdAndTxt("OpenCL:Dev_Mem", "Buffer size bigger than CL_DEVICE_MAX_MEM_ALLOC_SIZE!");
		}
		if ((mxIsScalar(prhs[var_offset]) == true) && ((uint32_t)round(rw_flags_ptr[i]) == 1)) {
			//mexPrintf( "Scalar Var: %d\n",i);

			for (uint32_t kernel_idx = 0; kernel_idx < kernel_list.size(); kernel_idx++) {
				switch (mxGetClassID(prhs[var_offset])) {
				case mxSINGLE_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_float*)mxGetData(prhs[var_offset])); break;
				case mxDOUBLE_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_double*)mxGetData(prhs[var_offset])); break;
				case mxINT8_CLASS:   dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_char*)mxGetData(prhs[var_offset])); break;
				case mxUINT8_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_uchar*)mxGetData(prhs[var_offset])); break;
				case mxINT16_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_short*)mxGetData(prhs[var_offset])); break;
				case mxUINT16_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_ushort*)mxGetData(prhs[var_offset])); break;
				case mxINT32_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_int*)mxGetData(prhs[var_offset])); break;
				case mxUINT32_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_uint*)mxGetData(prhs[var_offset])); break;
				case mxINT64_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_long*)mxGetData(prhs[var_offset])); break;
				case mxUINT64_CLASS: dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, *(cl_ulong*)mxGetData(prhs[var_offset])); break;
				}
			}

			//	mexPrintf("Scalar Var2: %f\n", *(cl_double*)mxGetData(prhs[var_offset]));

		}
		else {
			//mexPrintf("Vec Var: %d\n", i);
			try {
				switch ((uint32_t)round(rw_flags_ptr[i])) {
				case 0:	data_in.push_back(cl::Buffer(dev_mgr->get_context(0), CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, data_size.at(i))); dev_mgr->get_queue(0, 0).enqueueWriteBuffer(data_in.at(data_in.size() - 1), blocking, 0, data_size.at(i), mxGetData(prhs[var_offset]));  break;
				case 1:	data_in.push_back(cl::Buffer(dev_mgr->get_context(0), CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, data_size.at(i))); dev_mgr->get_queue(0, 0).enqueueWriteBuffer(data_in.at(data_in.size() - 1), blocking, 0, data_size.at(i), mxGetData(prhs[var_offset]));  break;
				case 2:	data_in.push_back(cl::Buffer(dev_mgr->get_context(0), CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, data_size.at(i))); break;
				}
				//dev_Buffers.push_back(&(data_in.at(i)));

				for (uint32_t kernel_idx = 0; kernel_idx < kernel_list.size(); kernel_idx++) {
					//mexPrintf("Vec Var: %d for Kernel: %d with Name: %s\n", i,kernel_idx, kernel_list.at(kernel_idx));
					dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))->setArg(i, data_in.at(data_in.size() - 1));
				}

			}
			catch (cl::Error err) {
				mexErrMsgIdAndTxt("OpenCL:exception", err.what());
			}

		}
		var_offset++;
	}


	if (dev_mgr->get_avail_dev_info(device).max_mem < mem_needed*1.2) {
		mexWarnMsgIdAndTxt("OpenCL:Dev_Mem", "Device may be out of memory!");
	}


	//mexPrintf("kernel: %s\n ", dev_mgr.getKernelbyID(0, "ocl_Kernel", 0)->getInfo<CL_KERNEL_FUNCTION_NAME>());
	//mexPrintf("jernel: %s\n ", dev_mgr.getKernelbyID(0, "ocl_Kernel", 1)->getInfo<CL_KERNEL_FUNCTION_NAME>());

	uint64_t  *exec_time_ptr;
	plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
	exec_time_ptr = (uint64_t *)mxGetData(plhs[0]);

	dev_mgr->get_queue(0, 0).finish();//Buffer Copy is asynchornous
	push_time = timer.getTimeMicroseconds() - push_time;

	// transferTime = (timer.getTimeMicroseconds() - startTransfer);
	// mexPrintf("Copy:  %d\n", transferTime);
	exec_time_ptr[0] = 0;
	for (uint32_t kernel_idx = 0; kernel_idx < kernel_list.size(); kernel_idx++){
		exec_time_ptr[0] = exec_time_ptr[0] + dev_mgr->execute_kernelNA(*(dev_mgr->getKernelbyName(0, "ocl_Kernel", kernel_list.at(kernel_idx))), dev_mgr->get_queue(0, 0), range_start,global_range, local_range);
	}

	var_offset = mvar_offset;

	pull_time = timer.getTimeMicroseconds();

	uint32_t buffer_counter = 0;

	for (uint32_t i = 0; i < num_in; i++) {

		if ((mxIsScalar(prhs[var_offset]) == true) && ((uint32_t)round(rw_flags_ptr[i]) == 1)) {
			//mexPrintf( "Scalar Var2: %d\n",i);
		//  do something?
		}
		else {

			//  mexPrintf("O Buffer Size:  %d\n", data_size.at(i));
			//     mexPrintf("O Datatype: %s\n", mxGetClassName(prhs[var_offset]));
			try {
				switch ((uint32_t)round(rw_flags_ptr[i])) {

				case 0:  dev_mgr->get_queue(0, 0).enqueueReadBuffer(data_in.at(buffer_counter), blocking, 0, data_size.at(buffer_counter), mxGetData(prhs[var_offset])); break;
				case 1: break;
				case 2: dev_mgr->get_queue(0, 0).enqueueReadBuffer(data_in.at(buffer_counter), blocking, 0, data_size.at(buffer_counter), mxGetData(prhs[var_offset])); break;
				}

				buffer_counter++;

			}
			catch (cl::Error err) {
				mexErrMsgIdAndTxt("OpenCL:exception", err.what());
			}
		}
		var_offset++;
	}

	if ((debug_mode == true)||((log_file == true))) {
#if defined(_WIN32)

		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);

		COORD newpos = { 0,0 };
		CHAR_INFO chiBuffer[con_rows*con_cols];
		buffer_size = { csbiInfo.srWindow.Right,csbiInfo.srWindow.Bottom };

		std::stringstream console_output;
		std::stringstream final_output;

		//memset(chiBuffer,0,sizeof(CHAR_INFO));

		rect = { 0, 0,  csbiInfo.srWindow.Right, csbiInfo.srWindow.Bottom };
		ReadConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), chiBuffer, buffer_size, newpos, &rect);
		FreeConsole();
		//	std::cout << csbiInfo.dwMaximumWindowSize.X <<"%"<< csbiInfo.dwMaximumWindowSize.Y << std::endl;
		//	std::cout << csbiInfo.srWindow.Right << "%" << csbiInfo.srWindow.Bottom << std::endl;
		for (int32_t i = 0; i < rect.Bottom*rect.Right; i++) {
			if ((chiBuffer[i].Char.AsciiChar > 1) && (chiBuffer[i].Char.AsciiChar < 255))

				if (i % (rect.Right + 1) == 0) {
					console_output << std::endl << (char)chiBuffer[i].Char.AsciiChar;
				}
				else {
					console_output << (char)chiBuffer[i].Char.AsciiChar;
				}
		}
		remove_empty_lines(console_output, final_output);

		mxArray * tmp_str;

		tmp_str = mxCreateString(final_output.str().c_str());
		plhs[1] = tmp_str;

		if (debug_mode == true) {
			std::cout << final_output.str() << std::endl;
		}

		if (log_file == true) {
			FILE *fp;
			char log_timestamp[100];
			char log_filename[200];

			time_t now = time(0);
			strftime(log_timestamp, 100, "%m-%d_%H-%M-%S", localtime(&now));

				snprintf(log_filename, 199, "log_%s_%s.txt", log_timestamp, kernel_list.at(0).c_str());

		//	mexPrintf("%s...\n", log_filename);

			fp = fopen(log_filename, "w");
			if (!fp) {
				mexErrMsgIdAndTxt("MATLAB:FILE", "Can't create Log file!");
			}
			else {
				fputs(final_output.str().c_str(), fp);
				fclose(fp);
			}
		}



#endif
#if !defined(_WIN32)
		std::fclose(fp);
		stdout = old; //reset stdout
		mxArray * tmp_str;

		tmp_str = mxCreateString(buffer);
		plhs[1] = tmp_str;

		if (debug_mode == true) {
			std::cout << buffer << std::endl;
		}
		if (log_file == true) {
			FILE *fp;
			char log_timestamp[100];
			char log_filename[200];

			time_t now = time(0);
			strftime(log_timestamp, 100, "%m-%d_%H-%M-%S", localtime(&now));

			snprintf(log_filename, 199, "log_%s_%s.txt", log_timestamp, kernel_list.at(0).c_str());
			//	mexPrintf("%s...\n", log_filename);

			fp = fopen(log_filename, "w");
			if (!fp) {
				mexErrMsgIdAndTxt("MATLAB:FILE", "Can't create Log file!");
			}
			else {
				fputs(buffer, fp);
				fclose(fp);
			}
		}

#endif
	}


	dev_mgr->get_queue(0, 0).finish();

	pull_time = timer.getTimeMicroseconds() - pull_time;
	copy_time= push_time+pull_time;


	return 0;


}


int32_t compilerun(mxArray *plhs[], int nrhs, const mxArray*prhs[], ocl_dev_mgr *dev_mgr, uint32_t device, bool debug_mode,bool log_file)
{


	bool blocking = CL_FALSE;
	uint64_t mem_needed = 0;
	std::vector<std::string> kernel_list;

	std::vector<cl::Buffer> data_in;
	std::vector<uint64_t> data_size;

	uint32_t global_range_x = 1;
	uint32_t global_range_y = 1;
	uint32_t global_range_z = 1;
	uint32_t range_start_x = 0;
	uint32_t range_start_y = 0;
	uint32_t range_start_z = 0;
	cl::NDRange range_start= cl::NullRange;
	cl::NDRange global_range;
	cl::NDRange local_range;

	uint64_t  copy_time;

	Timer timer; //used to track performance

	//this part compiles and runs kernel
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	//mexPrintf("Compile and run...\n");

	dev_mgr->init_device(device);


	size_t buflen;
	char *buf;
	char *settings;

	buflen = mxGetN(prhs[2]) + 1; //get Kernel Settings
	settings = (char *)mxMalloc(buflen);
	mxGetString(prhs[2], settings, (mwSize)buflen);
	//mexPrintf("Kernel-Settings:  %s\n", settings);

	getKernel_info(plhs, nrhs, prhs, dev_mgr);

	mwSize cell_dims;
	mxArray *cellElement;
	if (mxIsCell(prhs[3]) == true) {


		cell_dims = mxGetNumberOfElements(prhs[3]);
		for (uint32_t icell = 0; icell < cell_dims; icell++) {
			cellElement = mxGetCell(prhs[3], icell);

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
		char *kernel_name_c;
		buflen = mxGetN(prhs[3]) + 1; //get Kernel Name
		kernel_name_c = (char *)mxMalloc(buflen);
		mxGetString(prhs[3], kernel_name_c, (mwSize)buflen);
		kernel_list.push_back(std::string(kernel_name_c));
		//mexPrintf("Kernel-Name:  %s\n", kernel_name.c_str());

	}

	uint64_t kernels_found = 0;

	kernels_found = dev_mgr->compile_kernel(0, "ocl_Kernel", settings);
	if (kernels_found == 0) {
		mexErrMsgIdAndTxt("OpenCL:Kernel", "No valid kernels found");
		return -1;
	}


	//NDRange settings
	//global range

	size_t mrows = mxGetM(prhs[4]);
	size_t ncols = mxGetN(prhs[4]);

	if ((mxIsDouble(prhs[4]) || (mxGetClassID(prhs[4])== mxUINT32_CLASS)) && !mxIsComplex(prhs[4]) && (mrows * ncols == 3)) {
		if (mxIsDouble(prhs[4])) {

			double  *range_ptr;
			range_ptr = mxGetPr(prhs[4]);
				global_range_x = (uint32_t)round(range_ptr[0]);
				global_range_y = (uint32_t)round(range_ptr[1]);
				global_range_z = (uint32_t)round(range_ptr[2]);
		}
		else {
			uint32_t  *range_ptr;
			range_ptr = (uint32_t *)mxGetData(prhs[4]);

			global_range_x = (uint32_t)(range_ptr[0]);
			global_range_y = (uint32_t)(range_ptr[1]);
			global_range_z = (uint32_t)(range_ptr[2]);

		}
		global_range = cl::NDRange(global_range_x, global_range_y, global_range_z);

	}
	else {
		if ((mxIsDouble(prhs[4]) || (mxGetClassID(prhs[4]) == mxUINT32_CLASS)) && !mxIsComplex(prhs[4]) && (mrows * ncols == 6)) {
			if (mxIsDouble(prhs[4])) {
			double  *range_ptr;
			range_ptr = mxGetPr(prhs[4]);

			range_start_x = (uint32_t)round(range_ptr[0]);
			range_start_y = (uint32_t)round(range_ptr[1]);
			range_start_z = (uint32_t)round(range_ptr[2]);

			global_range_x = (uint32_t)round(range_ptr[3]);
			global_range_y = (uint32_t)round(range_ptr[4]);
			global_range_z = (uint32_t)round(range_ptr[5]);

			}
			else {
				uint32_t  *range_ptr;
				range_ptr = (uint32_t *)mxGetData(prhs[4]);

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
			return -1;
		}
	}

	//local range

	mrows = mxGetM(prhs[5]);
	ncols = mxGetN(prhs[5]);

	if ((mxIsDouble(prhs[5]) || (mxGetClassID(prhs[5]) == mxUINT32_CLASS)) && (mrows + ncols == 4)) {
		if (mxIsDouble(prhs[5])) {
			double  *range_ptr;
			range_ptr = mxGetPr(prhs[5]);
			global_range_x = (uint32_t)round(range_ptr[0]);
			global_range_y = (uint32_t)round(range_ptr[1]);
			global_range_z = (uint32_t)round(range_ptr[2]);
		}
		else {
			uint32_t  *range_ptr;
			range_ptr = (uint32_t *)mxGetData(prhs[5]);

			global_range_x = (uint32_t)(range_ptr[0]);
			global_range_y = (uint32_t)(range_ptr[1]);
			global_range_z = (uint32_t)(range_ptr[2]);
		}
		local_range = cl::NDRange(global_range_x, global_range_y, global_range_z);
	//	printf("Local work Size: %d/%d/%d\n", global_range_x, global_range_y, global_range_z);

	}
	else {
		if (mrows + ncols == 2) {
			local_range = cl::NullRange;
		}
		else {
			mexErrMsgIdAndTxt("OpenCL:NDRange", "Invalid local range defined!");
			return -1;
		}

	}



	uint32_t num_in = (uint32_t)nrhs - 7;//Number of input buffers

	uint32_t var_offset = 6;



	runkernel(plhs, nrhs, prhs, kernel_list, num_in,var_offset, dev_mgr, device, range_start, global_range, local_range, debug_mode,log_file,copy_time);

	if (debug_mode == false) {
		uint64_t  *copy_time_ptr;
		plhs[1] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
		copy_time_ptr = (uint64_t *)mxGetData(plhs[1]);
		copy_time_ptr[0] = copy_time;
	}

	return 0;
}

#endif // MATCL_H
