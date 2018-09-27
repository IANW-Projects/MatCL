/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include "ocl_dev_mgr.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>

#ifdef _WIN32
#include <io.h>
#define access    _access_s
#else
#include <unistd.h>
#endif



inline void compile(cl::Program& cl_prog,const char* options) {
	std::stringstream default_options;
	default_options.setf(std::ios::fixed);
	default_options << " " << options;

	try {
		cl_prog.build(default_options.str().c_str());

	}
	catch (cl::BuildError error)
	{
		std::string log = error.getBuildLog()[0].second;
		std::cerr << std::endl << "Build error:" << std::endl << log << std::endl;
	}
	catch (cl::Error err)
	{
		std::cout << "Exception:" << std::endl
			<< "ERROR: "
			<< err.what()
			<< std::endl;
	}

}

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

ocl_dev_mgr::ocl_dev_mgr() {
	initialize();
}


cl::Kernel * ocl_dev_mgr::getKernelbyName(cl_uint context_idx, std::string prog_name,std::string kernel_name) {

   std::vector <std::string>::iterator it_p = con_list.at(context_idx).prog_names.begin();
	it_p = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
	//if (i != con_list.at(context_idx).prog_names.end())
	//{

    uint32_t idx=distance(con_list.at(context_idx).prog_names.begin(), it_p);
//	std::cout << idx << std::endl;

	if (con_list.at(context_idx).kernels.at(idx).size() > 1) {

		for (cl_uint i = 0; i < con_list.at(context_idx).kernels.at(idx).size(); i++) {
			//std::cout<< kernel_name <<":" << con_list.at(context_idx).kernel_names.at(idx).at(i) << std::endl;
			if (kernel_name.compare(con_list.at(context_idx).kernel_names.at(idx).at(i)) == 0) {
			//	std::cout << "1" << idx << std::endl;
			return &(con_list.at(context_idx).kernels.at(idx).at(i));
			}

		}
	}
		return &(con_list.at(context_idx).kernels.at(idx).at(0));
    //}
}

cl::Kernel * ocl_dev_mgr::getKernelbyID(cl_uint context_idx, std::string prog_name,cl_ulong kernel_id) {


    std::vector <std::string>::iterator i = con_list.at(context_idx).prog_names.begin();
	i = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
	if (i != con_list.at(context_idx).prog_names.end())
	{

    uint32_t idx=distance(con_list.at(context_idx).prog_names.begin(), i);

	return &(con_list.at(context_idx).kernels.at(idx).at(kernel_id));
    }
}

std::string ocl_dev_mgr::getDeviceType(cl_uint avail_device_idx) {
     if (available_devices[avail_device_idx].type==CL_DEVICE_TYPE_CPU) {
                 return(type_cpu_str);
           } else  if (available_devices[avail_device_idx].type==CL_DEVICE_TYPE_GPU) {
              return(type_gpu_str);
           }else  if (available_devices[avail_device_idx].type==CL_DEVICE_TYPE_ACCELERATOR) {
               return(type_acc_str);
           }else {
              return(type_other_str);
           }
}

cl_ulong ocl_dev_mgr::getDeviceList(std::vector<cl::Device>& devices) {
  // Get list of platforms
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);

  // Enumerate devices
  for (unsigned int i = 0; i < platforms.size(); i++)
  {
    std::vector<cl::Device> plat_devices;
    platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
    devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
  }

  return devices.size();
}

cl_ulong ocl_dev_mgr::init_device(cl_uint avail_device_idx){

   ocl_context tmp_context;

   tmp_context.devices.push_back(available_devices[avail_device_idx]);

   std::vector<cl::Device> tmp_devices;
    tmp_devices.push_back(available_devices[avail_device_idx].device);

   cl::Context context(tmp_devices, NULL);
    tmp_context.context=context;

    tmp_context.queues.push_back(cl::CommandQueue (tmp_context.context,CL_QUEUE_PROFILING_ENABLE));
    //push second queue for async copy
    tmp_context.queues.push_back(cl::CommandQueue (tmp_context.context,CL_QUEUE_PROFILING_ENABLE));

    con_list.push_back(tmp_context);

     return con_list.size();

  /*


    for (unsigned int i=0; i<device_idx.size();i++) {
        tmp_devices.push_back(devices[device_idx.at(i)].device);
    }
    */
}

cl::CommandQueue& ocl_dev_mgr::get_queue(cl_uint context_idx, cl_uint queue_idx){

    return con_list.at(context_idx).queues.at(queue_idx);
}

cl::Context& ocl_dev_mgr::get_context(cl_uint context_idx){

    return con_list.at(context_idx).context;
}

cl_ulong ocl_dev_mgr::get_avail_dev_num(){

    return num_available_devices;
}

cl_ulong ocl_dev_mgr::get_context_num(){

    return con_list.size();
}


cl_int ocl_dev_mgr::add_program_url(cl_uint context_idx, std::string prog_name,std::string url){

    if(FileExists(url)==true) {
    con_list.at(context_idx).programs.push_back(cl::Program (con_list.at(context_idx).context, loadProgram(url)));
	con_list.at(context_idx).prog_names.push_back(prog_name);
    con_list.at(context_idx).kernels.resize(con_list.at(context_idx).kernels.size()+1);
	con_list.at(context_idx).kernel_names.resize(con_list.at(context_idx).kernel_names.size() + 1);
    return 1;
    } else {
     return -1;
    }
}

cl_int ocl_dev_mgr::add_program_str(cl_uint context_idx, std::string prog_name, std::string kernel) {

		con_list.at(context_idx).programs.push_back(cl::Program(con_list.at(context_idx).context, kernel));
		con_list.at(context_idx).prog_names.push_back(prog_name);
		con_list.at(context_idx).kernels.resize(con_list.at(context_idx).kernels.size() + 1);
		con_list.at(context_idx).kernel_names.resize(con_list.at(context_idx).kernel_names.size() + 1);
		return 1;

}

cl::Program& ocl_dev_mgr::get_program(cl_uint context_idx, std::string prog_name){

	std::vector <std::string>::iterator i = con_list.at(context_idx).prog_names.begin();
	i = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
	if (i != con_list.at(context_idx).prog_names.end())
	{
		return con_list.at(context_idx).programs.at(distance(con_list.at(context_idx).prog_names.begin(), i));
	}
	else {
		return con_list.at(context_idx).programs.at(0);
	}
}

ocl_dev_mgr::ocl_device_info& ocl_dev_mgr::get_avail_dev_info(cl_uint avail_device_idx){
    return available_devices[avail_device_idx];
}

ocl_dev_mgr::ocl_device_info& ocl_dev_mgr::get_context_dev_info(cl_uint context_idx,cl_uint device_idx){
    return con_list.at(context_idx).devices.at(device_idx);
}



//return execution time in µs
cl_ulong ocl_dev_mgr::execute_kernel(cl::Kernel& kernel,cl::CommandQueue& queue,cl::NDRange global_range, cl::NDRange local_range, std::vector<cl::Buffer*>& dev_Buffers){
cl::Event event;
cl_ulong time_start, time_end;
    try{
    for (cl_uint i=0;i<dev_Buffers.size();i++) {
        kernel.setArg(i,*dev_Buffers[i]);
    }

    queue.enqueueNDRangeKernel(kernel,cl::NullRange,global_range,local_range,NULL,&event);
   // queue.finish();
    event.wait();
	event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);
    event.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &time_start);
    }
    catch (cl::BuildError error){
        std::string log = error.getBuildLog()[0].second;
        std::cerr << std::endl << "Build error:" << std::endl << log << std::endl;
    }
    catch (cl::Error err) {
    std::cout << "Exception:" << std::endl<< "ERROR: "<< err.what()<< std::endl;
    }
  return (time_end - time_start)/1000;
}


//return execution time in µs
cl_ulong ocl_dev_mgr::execute_kernelNA(cl::Kernel& kernel, cl::CommandQueue& queue, cl::NDRange range_start, cl::NDRange global_range, cl::NDRange local_range) {
	cl::Event event;
	cl_ulong time_start, time_end;
	try {

		queue.enqueueNDRangeKernel(kernel, range_start, global_range, local_range, NULL, &event);
		// queue.finish();
		event.wait();
		event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);
		event.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &time_start);
	}
	catch (cl::BuildError error) {
		std::string log = error.getBuildLog()[0].second;
		std::cerr << std::endl << "Build error:" << std::endl << log << std::endl;
	}
	catch (cl::Error err) {
		std::cout << "Exception:" << std::endl << "ERROR: " << err.what() << std::endl;
	}
	return (time_end - time_start) / 1000;
}

//don't return execution time in µs
cl_ulong ocl_dev_mgr::execute_kernel_async(cl::Kernel& kernel,cl::CommandQueue& queue,cl::NDRange global_range, cl::NDRange local_range, std::vector<cl::Buffer*>& dev_Buffers){
cl::Event event;
cl_ulong time_start, time_end;
    try{
    for (cl_uint i=0;i<dev_Buffers.size();i++) {
        kernel.setArg(i,*dev_Buffers[i]);
    }

    queue.enqueueNDRangeKernel(kernel,cl::NullRange,global_range,local_range,NULL,&event);
   // queue.finish();

    }
    catch (cl::BuildError error){
        std::string log = error.getBuildLog()[0].second;
        std::cerr << std::endl << "Build error:" << std::endl << log << std::endl;
    }
    catch (cl::Error err) {
    std::cout << "Exception:" << std::endl<< "ERROR: "<< err.what()<< std::endl;
    }
  return 0;
}


cl_ulong ocl_dev_mgr::compile_kernel(cl_uint context_idx, std::string prog_name,const char* options) {
        std::stringstream default_options;
    default_options.setf(std::ios::fixed);
    default_options << " " << options;

    std::vector <std::string>::iterator i = con_list.at(context_idx).prog_names.begin();
	i = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);

	if (i != con_list.at(context_idx).prog_names.end())
	{

    int32_t idx=distance(con_list.at(context_idx).prog_names.begin(), i);


try{

    con_list.at(context_idx).programs.at(idx).build(default_options.str().c_str());

}
     catch (cl::BuildError error)
  {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << std::endl << "Build error:" << std::endl << log << std::endl;
  }
  catch (cl::Error err)
  {
    std::cout << "Exception:" << std::endl
              << "ERROR: "
              << err.what()
              << std::endl;
  }

  con_list.at(context_idx).programs.at(idx).createKernels(&(con_list.at(context_idx).kernels.at(idx)));

  con_list.at(context_idx).kernel_names.at(idx).clear(); //make sure to clean kernel_names list

  for (uint32_t i = 0; i < con_list.at(context_idx).kernels.at(idx).size(); i++) {
	  con_list.at(context_idx).kernel_names.at(idx).push_back(con_list.at(context_idx).kernels.at(idx).at(i).getInfo<CL_KERNEL_FUNCTION_NAME>());
  }


  return con_list.at(context_idx).kernels.at(idx).size();
    }
}

cl_ulong ocl_dev_mgr::get_kernel_names(cl_uint context_idx,std::string prog_name,std::vector<std::string>& found_kernels) {
 std::vector <std::string>::iterator i = con_list.at(context_idx).prog_names.begin();
	i = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);

	if (i != con_list.at(context_idx).prog_names.end())
	{

    int32_t idx=distance(con_list.at(context_idx).prog_names.begin(), i);
for (uint32_t kernel_id=0; kernel_id<con_list.at(context_idx).kernel_names.at(idx).size();kernel_id++){
    found_kernels.push_back(con_list.at(context_idx).kernel_names.at(idx).at(kernel_id));
}
return con_list.at(context_idx).kernel_names.at(idx).size();
    }
    return 0;
}

/*
void ocl_dev_mgr::compile_thread(cl::Program& cl_prog, char* options) {

	compile_threads.push_back(std::thread(compile, std::ref(get_program(0, "ocl_Kernel")), options));

}

cl_ulong ocl_dev_mgr::finish_compile(cl::Program& cl_prog ) {

	compile_threads.at(0).join();
	cl_prog.createKernels(&kernels);
	compile_threads.clear();
	return kernels.size();
}
*/

void ocl_dev_mgr::initialize() {

    std::vector<cl::Device> tmp_devices;

    getDeviceList(tmp_devices);
    num_available_devices=tmp_devices.size();

    available_devices=new ocl_device_info[num_available_devices];


    for (unsigned int i=0; i<tmp_devices.size();i++) {

      available_devices[i].device= tmp_devices[i];

      available_devices[i].device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &available_devices[i].max_mem);
	  available_devices[i].device.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &available_devices[i].max_mem_alloc);
	  available_devices[i].device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &available_devices[i].lw_dim);
	  std::vector<size_t> tmp_size;
	  available_devices[i].device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &available_devices[i].wg_size);
	 available_devices[i].device.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &tmp_size);
	 available_devices[i].lw_size = tmp_size.at(0);
      available_devices[i].device.getInfo(CL_DEVICE_NAME, &available_devices[i].name);
	  available_devices[i].device.getInfo(CL_DEVICE_VERSION, &available_devices[i].ocl_version);
      available_devices[i].device.getInfo(CL_DEVICE_TYPE, &available_devices[i].type);
	  available_devices[i].device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &available_devices[i].compute_units);
    }

}

void ocl_dev_mgr::deinitalize() {
	//compile_threads.clear();
	con_list.clear();
}
