/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#ifndef DEV_MGR_H
#define DEV_MGR_H

#include <vector>
#include <iostream>
#include <iterator>
#include <sstream>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120


//some headers seem to def strict ansi, which breaks the OpenCL typedefs
#if defined( __STRICT_ANSI__ )
#undef  __STRICT_ANSI__
#endif

//disable strange warnings for newer versions of GCC
#pragma GCC diagnostic ignored "-Wignored-attributes"



#include <CL/cl2.hpp>

#define DREAL cl_float
#define DREAL4 cl_float4
#define CL_REAL float


#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

class ocl_dev_mgr {
 public:
        ~ocl_dev_mgr() {};

        static ocl_dev_mgr& getInstance() {
        static ocl_dev_mgr instance;
        return instance;
    }

        struct ocl_device_info{
         cl::Device device;
         std::string name;
         cl::Platform platform;
         cl_device_type type;
		 std::string ocl_version;
         cl_ulong max_mem;
		 cl_ulong max_mem_alloc;
         size_t wg_size;
		 cl_uint lw_dim;
		 size_t lw_size;
		 cl_uint compute_units;
		 cl_uint copy_perf;
         cl_uint double_perf;
         cl_uint float_perf;

     };



     cl_ulong init_device(cl_uint avail_device_idx);
     cl::CommandQueue& get_queue(cl_uint context_idx, cl_uint queue_idx);
     cl::Context& get_context(cl_uint context_idx);
     cl::Program& get_program(cl_uint context_idx, std::string prog_name);
     cl_ulong get_avail_dev_num();
     cl_ulong get_context_num();
     ocl_device_info& get_avail_dev_info(cl_uint avail_device_idx);
     ocl_device_info& get_context_dev_info(cl_uint context_idx,cl_uint device_idx);
     cl_ulong compile_kernel(cl_uint context_idx, std::string prog_name,const char* options);
    cl_ulong get_kernel_names(cl_uint context_idx,std::string prog_name,std::vector<std::string>& found_kernels);
	// void compile_thread(cl::Program& cl_prog, char* options);
	 //cl_ulong finish_compile(cl::Program& cl_prog);
     cl_ulong execute_kernel(cl::Kernel& kernel, cl::CommandQueue& queue, cl::NDRange global_range, cl::NDRange local_range, std::vector<cl::Buffer*>& dev_Buffers);
	 cl_ulong execute_kernelNA(cl::Kernel& kernel, cl::CommandQueue& queue, cl::NDRange range_start, cl::NDRange global_range, cl::NDRange local_range);
	 cl_ulong execute_kernel_async(cl::Kernel& kernel, cl::CommandQueue& queue, cl::NDRange global_range, cl::NDRange local_range, std::vector<cl::Buffer*>& dev_Buffers);
     cl_int add_program_url(cl_uint context_idx,std::string prog_name,std::string url);
	 cl_int add_program_str(cl_uint context_idx, std::string prog_name, std::string kernel);
	 cl::Kernel * getKernelbyName(cl_uint context_idx, std::string prog_name,std::string kernel_name);
	 cl::Kernel * getKernelbyID(cl_uint context_idx, std::string prog_name,cl_ulong kernel_id);
     std::string getDeviceType(cl_uint avail_device_idx);
	 void deinitalize();
private:

   const std::string type_cpu_str = "CPU";
   const std::string type_gpu_str = "GPU";
   const std::string type_acc_str = "ACCELERATOR";
   const std::string type_other_str = "OTHER";

    struct ocl_context{
      cl::Context context;
      std::vector<cl::CommandQueue> queues;
      std::vector<cl::Program> programs;
	  std::vector<std::string> prog_names;
      std::vector<std::vector<cl::Kernel>> kernels;
	  std::vector<std::vector<std::string>> kernel_names;
      std::vector<ocl_device_info> devices;
    };

    void initialize();
     ocl_dev_mgr();
     cl_ulong getDeviceList(std::vector<cl::Device>& devices);

    ocl_device_info *available_devices;
    cl_ulong num_available_devices;
    std::vector<ocl_context> con_list;

//	std::vector<std::thread> compile_threads;
};

#endif // DEV_MGR_H
