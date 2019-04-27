/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream> 
#include "ocl_dev_mgr.hpp"


#if defined(_WIN32)
#include <io.h>
#define access _access_s
#else
#include <unistd.h>
#endif


// macros
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define ERROR_INFO "Error in line " STRINGIZE(__LINE__) " of " __FILE__ ":\n "


// file system functions

inline bool fileExists(std::string const& filename)
{
  return access(filename.c_str(), 0) == 0;
}



inline void compile(cl::Program& cl_prog, char const* options)
{
  std::string compile_options = std::string(" ") + std::string(options);

	try {
    cl_prog.build(compile_options.c_str());
	}
	catch (cl::BuildError error) {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << ERROR_INFO << "Build error:\n" << log << std::endl;
	}
	catch (cl::Error err) {
    std::cerr << ERROR_INFO << "Exception:" << err.what() << std::endl;
	}
}


inline std::string loadProgram(std::string const& input_filename)
{
  std::ifstream input(input_filename.c_str());
  if (!input.is_open()) {
    std::cerr << ERROR_INFO << "Cannot open file '" << input_filename << "'." << std::endl;
    exit(1);
  }

  return std::string(std::istreambuf_iterator<char>(input), (std::istreambuf_iterator<char>()));
}


ocl_dev_mgr::ocl_dev_mgr() {
    initialize();
}


cl::Kernel* ocl_dev_mgr::getKernelbyName(cl_uint context_idx, std::string const& prog_name, std::string const& kernel_name)
{
  auto it_p = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
  if (it_p == con_list.at(context_idx).prog_names.end()) {
    return nullptr;
  }

  uint32_t idx = distance(con_list.at(context_idx).prog_names.begin(), it_p);

  if (con_list.at(context_idx).kernels.at(idx).size() > 1) {
    for (cl_uint i = 0; i < con_list.at(context_idx).kernels.at(idx).size(); i++) {
      if (kernel_name == con_list.at(context_idx).kernel_names.at(idx).at(i)) {
        return &(con_list.at(context_idx).kernels.at(idx).at(i));
      }
    }
  }

  return &(con_list.at(context_idx).kernels.at(idx).at(0));
}

cl::Kernel* ocl_dev_mgr::getKernelbyID(cl_uint context_idx, std::string const& prog_name, cl_ulong kernel_id)
{
  auto it_p = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
  if (it_p == con_list.at(context_idx).prog_names.end()) {
    return nullptr;
  }

  uint32_t idx = distance(con_list.at(context_idx).prog_names.begin(), it_p);

  return &(con_list.at(context_idx).kernels.at(idx).at(kernel_id));
}


std::string ocl_dev_mgr::getDeviceType(cl_uint avail_device_idx)
{
  if (available_devices.at(avail_device_idx).type == CL_DEVICE_TYPE_CPU) {
    return(type_cpu_str);
  }
  else if (available_devices.at(avail_device_idx).type == CL_DEVICE_TYPE_GPU) {
    return(type_gpu_str);
  }
  else if (available_devices.at(avail_device_idx).type == CL_DEVICE_TYPE_ACCELERATOR) {
    return(type_acc_str);
  }
  else {
    return(type_other_str);
  }
}

std::string ocl_dev_mgr::getDevicePCIeID(cl_uint avail_device_idx)
{
#define CL_DEVICE_PCI_BUS_ID_NV 0x4008
#define CL_DEVICE_PCI_SLOT_ID_NV 0x4009
#define CL_DEVICE_TOPOLOGY_AMD  0x4037
typedef union
{
  struct { cl_uint type; cl_uint data[5]; } raw;
  struct { cl_uint type; cl_char unused[17]; cl_char bus; cl_char device; cl_char function; } pcie;
} cl_device_topology_amd;

  cl_device_topology_amd amd_topo;
	cl_int bus_id;
	cl_int slot_id;
	std::ostringstream tmp_stream;

  std::size_t found = 0;
  found = available_devices.at(avail_device_idx).vendor.find("NVIDIA");
  if (found != std::string::npos) {
    available_devices.at(avail_device_idx).device.getInfo(CL_DEVICE_PCI_BUS_ID_NV,&bus_id);
	  available_devices.at(avail_device_idx).device.getInfo(CL_DEVICE_PCI_SLOT_ID_NV, &slot_id);

	  cl_uint domain, bus, dev, func;
	  domain = bus_id >> 8;
	  bus = bus_id & 0xff;
	  tmp_stream << domain << ":" << bus << ":" << slot_id;
  }
  else
  {
    found = available_devices.at(avail_device_idx).vendor.find("Advanced Micro Devices");
    if (found != std::string::npos) {
      available_devices.at(avail_device_idx).device.getInfo(CL_DEVICE_TOPOLOGY_AMD, &amd_topo);
      tmp_stream << "0:" << (unsigned int)amd_topo.pcie.bus << ":" << (unsigned int)amd_topo.pcie.device; //Domain is not returned?
    }
  }

	
	return tmp_stream.str();
}

cl_int bus_id;
cl_int slot_id;

cl_ulong ocl_dev_mgr::getDeviceList(std::vector<cl::Device>& devices)
{
  // Get list of platforms 
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);

  // Enumerate devices
  for (cl::Platform const& platform : platforms)
  {
    std::vector<cl::Device> plat_devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &plat_devices);
    devices.insert(devices.end(), plat_devices.begin(), plat_devices.end());
  }

  return devices.size();
}


cl_ulong ocl_dev_mgr::init_device(cl_uint avail_device_idx)
{
  ocl_context tmp_context;

  tmp_context.devices.push_back(available_devices.at(avail_device_idx));

  std::vector<cl::Device> tmp_devices;
  tmp_devices.push_back(available_devices.at(avail_device_idx).device);

  cl::Context context(tmp_devices, NULL);
  tmp_context.context = context;

  tmp_context.queues.push_back(cl::CommandQueue(tmp_context.context, CL_QUEUE_PROFILING_ENABLE));
	//push second queue for async copy
  tmp_context.queues.push_back(cl::CommandQueue(tmp_context.context, CL_QUEUE_PROFILING_ENABLE));

  con_list.push_back(tmp_context);

  return con_list.size();
}

cl::CommandQueue& ocl_dev_mgr::get_queue(cl_uint context_idx, cl_uint queue_idx)
{
  return con_list.at(context_idx).queues.at(queue_idx);
}

cl::Context& ocl_dev_mgr::get_context(cl_uint context_idx)
{
  return con_list.at(context_idx).context;
}

cl_ulong ocl_dev_mgr::get_avail_dev_num()
{
  return num_available_devices;
}

cl_ulong ocl_dev_mgr::get_context_num()
{
  return con_list.size();
}


bool ocl_dev_mgr::add_program_url(cl_uint context_idx, std::string prog_name, std::string const& url)
{
  if (!fileExists(url)) {
    return false;
  }

  return add_program_str(context_idx, prog_name, loadProgram(url));
}

bool ocl_dev_mgr::add_program_str(cl_uint context_idx, std::string prog_name, std::string kernel)
{
  con_list.at(context_idx).programs.push_back(cl::Program(con_list.at(context_idx).context, kernel));
  con_list.at(context_idx).prog_names.push_back(prog_name);
  con_list.at(context_idx).kernels.resize(con_list.at(context_idx).kernels.size() + 1);
  con_list.at(context_idx).kernel_names.resize(con_list.at(context_idx).kernel_names.size() + 1);
  return true;
}


cl::Program& ocl_dev_mgr::get_program(cl_uint context_idx, std::string const& prog_name)
{
  auto it_p = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
  if (it_p != con_list.at(context_idx).prog_names.end()) {
    return con_list.at(context_idx).programs.at(distance(con_list.at(context_idx).prog_names.begin(), it_p));
  }
  else {
    std::cerr << ERROR_INFO << "Program '" << prog_name << "' not found." << std::endl;
    //TODO: Exception?
    return con_list.at(context_idx).programs.at(0);
  }
}


ocl_dev_mgr::ocl_device_info& ocl_dev_mgr::get_avail_dev_info(cl_uint avail_device_idx)
{
  return available_devices.at(avail_device_idx);
}


ocl_dev_mgr::ocl_device_info& ocl_dev_mgr::get_context_dev_info(cl_uint context_idx, cl_uint device_idx)
{
  return con_list.at(context_idx).devices.at(device_idx);
}


// return execution time in µs
cl_ulong ocl_dev_mgr::execute_kernel(cl::Kernel& kernel, cl::CommandQueue& queue,
  cl::NDRange global_range, cl::NDRange local_range,
  std::vector<cl::Buffer*>& dev_Buffers)
{
  cl::Event event;
  cl_ulong time_start, time_end;

  try {
    for (cl_uint i = 0; i < dev_Buffers.size(); i++) {
      kernel.setArg(i, *dev_Buffers[i]);
    }

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_range, local_range, NULL, &event);
    event.wait();
    event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);
    event.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &time_start);
  }
  catch (cl::BuildError error) {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << ERROR_INFO << "Build error:\n" << log << std::endl;
	}
  catch (cl::Error err) {
    std::cerr << ERROR_INFO << "Exception:" << err.what() << std::endl;
  }

  return (time_end - time_start) / 1000;
}


// return execution time in µs
cl_ulong ocl_dev_mgr::execute_kernelNA(cl::Kernel& kernel, cl::CommandQueue& queue,
cl::NDRange range_start, cl::NDRange global_range, cl::NDRange local_range)
{
  cl::Event event;
  cl_ulong time_start, time_end;

  try {
    queue.enqueueNDRangeKernel(kernel, range_start, global_range, local_range, NULL, &event);
    event.wait();
    event.getProfilingInfo(CL_PROFILING_COMMAND_END, &time_end);
    event.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &time_start);
	}
  catch (cl::BuildError error) {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << ERROR_INFO << "Build error:\n" << log << std::endl;
  }
  catch (cl::Error err) {
    std::cerr << ERROR_INFO << "Exception:" << err.what() << std::endl;
  }

  return (time_end - time_start) / 1000;
}

// don't return execution time in µs
void ocl_dev_mgr::execute_kernel_async(cl::Kernel& kernel, cl::CommandQueue& queue,
  cl::NDRange global_range, cl::NDRange local_range,
  std::vector<cl::Buffer*>& dev_Buffers)
{
  try {
    for (cl_uint i = 0; i < dev_Buffers.size(); i++) {
    kernel.setArg(i, *dev_Buffers[i]);
    }

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_range, local_range, NULL, NULL);
  }
  catch (cl::BuildError error) {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << ERROR_INFO << "Build error:\n" << log << std::endl;
  }
  catch (cl::Error err) {
    std::cerr << ERROR_INFO << "Exception:" << err.what() << std::endl;
  }
}


// Compile kernels and return the number of compiled kernels.
cl_ulong ocl_dev_mgr::compile_kernel(cl_uint context_idx, std::string const& prog_name, std::string const& options)
{
  std::string compile_options = std::string(" ") + options;

  auto it_p = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
  if (it_p == con_list.at(context_idx).prog_names.end()) {
    std::cerr << ERROR_INFO << "Program '" << prog_name << "' not found." << std::endl;
    //TODO: Exception?
    return 0;
  }

  int32_t idx = distance(con_list.at(context_idx).prog_names.begin(), it_p);

  try {
    con_list.at(context_idx).programs.at(idx).build(compile_options.c_str());
  }
  catch (cl::BuildError error) {
    std::string log = error.getBuildLog()[0].second;
    std::cerr << ERROR_INFO << "Build error:\n" << log << std::endl;
  }
  catch (cl::Error err) {
    std::cerr << ERROR_INFO << "Exception:" << err.what() << std::endl;
  }

  con_list.at(context_idx).programs.at(idx).createKernels(&(con_list.at(context_idx).kernels.at(idx)));

  con_list.at(context_idx).kernel_names.at(idx).clear(); //make sure to clear kernel_names list

  for (uint32_t i = 0; i < con_list.at(context_idx).kernels.at(idx).size(); i++) {
    con_list.at(context_idx).kernel_names.at(idx).push_back(con_list.at(context_idx).kernels.at(idx).at(i).getInfo<CL_KERNEL_FUNCTION_NAME>());
  }

  return con_list.at(context_idx).kernels.at(idx).size();
}


cl_ulong ocl_dev_mgr::get_kernel_names(cl_uint context_idx, std::string const& prog_name, std::vector<std::string>& found_kernels)
{
  auto it_p = find(con_list.at(context_idx).prog_names.begin(), con_list.at(context_idx).prog_names.end(), prog_name);
  if (it_p == con_list.at(context_idx).prog_names.end()) {
    std::cerr << ERROR_INFO << "Program '" << prog_name << "' not found." << std::endl;
		//TODO: Exception?
    return 0;
  }

  int32_t idx = distance(con_list.at(context_idx).prog_names.begin(), it_p);

  for (uint32_t kernel_id = 0; kernel_id < con_list.at(context_idx).kernel_names.at(idx).size(); kernel_id++) {
    found_kernels.push_back(con_list.at(context_idx).kernel_names.at(idx).at(kernel_id));
  }

  return con_list.at(context_idx).kernel_names.at(idx).size();
}


void ocl_dev_mgr::initialize()
{
  std::vector<cl::Device> tmp_devices;
  getDeviceList(tmp_devices);
  num_available_devices = tmp_devices.size();

  available_devices = std::vector<ocl_device_info>(num_available_devices);

for (size_t i = 0; i < tmp_devices.size(); i++) {

    available_devices.at(i).device = tmp_devices.at(i);
    std::vector<size_t> tmp_size;

    available_devices.at(i).device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &available_devices.at(i).max_mem);
    available_devices.at(i).device.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &available_devices.at(i).max_mem_alloc);
    available_devices.at(i).device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &available_devices.at(i).lw_dim);
    available_devices.at(i).device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &available_devices.at(i).wg_size);
    available_devices.at(i).device.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &tmp_size);
    available_devices.at(i).lw_size = tmp_size.at(0);
    available_devices.at(i).device.getInfo(CL_DEVICE_NAME, &available_devices.at(i).name);
    available_devices.at(i).device.getInfo(CL_DEVICE_VERSION, &available_devices.at(i).ocl_version);
    available_devices.at(i).device.getInfo(CL_DEVICE_TYPE, &available_devices.at(i).type);
    available_devices.at(i).device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &available_devices.at(i).compute_units);
    available_devices.at(i).device.getInfo(CL_DEVICE_PLATFORM, &available_devices.at(i).platform);
    available_devices.at(i).device.getInfo(CL_DEVICE_VENDOR, &available_devices.at(i).vendor);
    available_devices.at(i).platform.getInfo(CL_PLATFORM_NAME, &available_devices.at(i).platform_name);
	}
}


void ocl_dev_mgr::deinitalize()
{
	//Deinitialization should be performed automatically, but there seems to be segfaults
	//under certain conditions using Windows, hence the vetor is cleared manually
  con_list.clear();
}