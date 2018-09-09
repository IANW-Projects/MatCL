# MatCL

MatCL is an OpenCL interface for MathWorks Matlab. This MEX-based toolbox aims at providing a simple and easy to use solution to transfer memory and launch OpenCL kernels from Matlab using a single command.
In comparison to other Matlab OpenCL solutions, MatCL is not just an OpenCL API wrapper but encapsulates the low-level host API calls necessary to initialize devices, create OpenCL buffers from Matlab workspace variables and build and launch kernels.
MatCL is primarily intended to help in the development and testing of OpenCL kernels by allowing to transparently pass data from and to Matlab. 
Because MatCL handles the entire low-level process, this toolbox makes it possible to execute kernels without in depth knowledge of the host implementation necessary to support the execution of OpenCL kernels.
MatCL is also optimized to allow efficient execution of OpenCL kernels within Matlab to accelerate computationally intensive tasks without having to rely on Nvidia CUDA. In addition to single command kernel execution, MatCL also allows for an independent two-step kernel compilation and launch workflow to save the kernel compile time and allow efficient repetitive kernel execution. 

Tested using Nvidia (Tesla, GTX), AMD (Ryzen, Radeon R9, FirePro) and Intel (Xeon, Core, HD Graphics) devices with Matlab R2016b and up.

## Usage

- Enumerate OpenCL Devices:
  `[names,dev_class,max_mem,max_wg_size,max_local_work_size,compute_units]=cl_get_devices;`
  - `names`: Names of all available devices
  - `dev_class`: The device class (CPU, GPU or Other for other or unknown Accelerators)
  - `max_mem`: The available device memory in bytes
  - `max_wg_size`: Max. size of OpenCL work group
  - `max_local_work_size`: Max. size of work items
  - `compute_units`: Number of compute units (i.e. CPU cores) of the device

- Build Kernel:
  `[comp_time,kernels]=cl_run_kernel(ocl_dev_id,'kernel_url.cl','defines');`
  - `ocl_dev_id`: ID of the OpenCL device to be used
  - `kernel_url.cl`: URL of the kernel file
  - `defines`: List of OpenCL compiler defines
  - `kernels`: List with names of all available kernels

- Run Kernel:
  `[run_time,copy_time]=cl_run_kernel(ocl_dev_id,',{'kernel_function1','kernel_function2'},global_range,local_range,in1,out1,[rw_flags]);`
  - `ocl_dev_id`: ID of the OpenCL device to be used
  - `kernel_function`: Cell array of kernel functions to execute (can also be a signal string)
  - `global_range`: 3D global OpenCL range (see NDRange). If this vector has six entires, the first three define the 3D work offset followed by the 3D work size.
  - `local_range`: 3D local OpenCL range (see NDRange)
  - `in1, out1`: List of variables to pass from/to kernel
  - `rw_flags`: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only

- Build & Run Kernel:
  `[run_time]=cl_run_kernel(ocl_dev_id,'kernel_url.cl ','defines ','kernel_function',global_range,local_range,in1,out1,[rw_flags]);`
  - `ocl_dev_id`: ID of the OpenCL device to be used
  - `kernel_url.cl`: URL of the kernel file
  - `defines`: List of OpenCL compiler defines
  - `kernel_function`: Cell array of kernel functions to execute (can also be a signal string)
  - `global_range`: 3D global OpenCL range (see NDRange). If this vector has six entires, the first three define the 3D work offset followed by the 3D work size.
  - `local_range`: Local OpenCL range (see NDRange)
  - `in1, out1`: List of variables to pass from/to kernel
  - `rw_flags`: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only

- Build & Run Kernel (with Kernel printf redirection):
  `[run_time]=cl_dbg_kernel(ocl_dev_id,' kernel_url.cl ','defines ','kernel_function',global_range,local_range,in1,out1,[rw_flags]);`
  - `ocl_dev_id`: ID of the OpenCL device to be used
  - `kernel_url.cl`: URL of the kernel file
  - `defines`: List of OpenCL compiler defines
  - `kernel_function`: Cell array of kernel functions to execute (can also be a signal string)
  - `global_range`: 3D global OpenCL range (see NDRange). If this vector has six entires, the first three define the 3D work offset followed by the 3D work size.
  - `local_range`: Local OpenCL range (see NDRange)
  - `in1, out1`: List of variables to pass from/to kernel
  - `rw_flags`: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only


## Setup

Just use `git clone git@github.com:philipheinisch/MatCL.git` and run compile_lx.m or compile_win.m to compile MatCL. Depending on the OpenCL libraries used, the library path may have to be changed.
Than add the folder `MatCL` to the search path of Matlab.

There may be problems with old C libraries of Matlab under Linux, resulting in errors such as 
`Invalid MEX-file '/..../cl_get_devices.mex64'`, followed by many missing symbols. If you use
some Debian based system, install the package `matlab-support` via `sudo apt-get install matlab-support`
and choose the option to rename the GGC libraries of Matlab during setup.


## Reference

MatCL can be referenced using the DOI https://doi.org/10.1145/3204919.3204927

## License

This project is licensed under the terms of the Creative Commons CC BY-NC-ND 3.0 license.
