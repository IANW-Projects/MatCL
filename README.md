# MatCL
MatCL is an OpenCL interface for MathWorks Matlab. This MEX-based toolbox aims at providing a simple and easy to use solution to transfer memory and launch OpenCL kernels from Matlab using a single command.
In comparison to other Matlab OpenCL solutions, MatCL is not just an OpenCL API wrapper but encapsulates the low-level host API calls necessary to initialize devices, create OpenCL buffers from Matlab workspace variables and build and launch kernels.
MatCL is primarily intended to help in the development and testing of OpenCL kernels by allowing to transparently pass data from and to Matlab. 
Because MatCL handles the entire low-level process, this toolbox makes it possible to execute kernels without in depth knowledge of the host implementation necessary to support the execution of OpenCL kernels.
MatCL is also optimized to allow efficient execution of OpenCL kernels within Matlab to accelerate computationally intensive tasks without having to rely on Nvidia CUDA. In addition to single command kernel execution, MatCL also allows for an independent two-step kernel compilation and launch workflow to save the kernel compile time and allow efficient repetitive kernel execution. 
<br>
Tested using Nvidia (Tesla, GTX), AMD (Ryzen, Radeon R9, FirePro) and Intel (Xeon, Core, HD Graphics) devices with Matlab R2016b and up.<br>
<br>
Usage:
<br>
Enumerate OpenCL Devices:<br/>
[names,dev_class,max_mem,max_wg_size,max_local_work_size]=cl_get_devices;<br/>
names: Names of all available devices<br/>
dev_class: The device class (CPU, GPU or Other for other or unknown Accelerators)<br/>
max_mem: The available device memory in bytes<br/>
max_wg_size: Max. size of OpenCL work group<br/>
max_local_work_size: Max. size of work items<br/>

Build Kernel:<br/>
[comp_time,kernels]=cl_run_kernel(ocl_dev_id,'kernel_url.cl','defines');<br/>
ocl_dev_id: ID of the OpenCL device to be used<br/>
kernel_url.cl: URL of the kernel file<br/>
defines: List of OpenCL compiler defines<br/>
kernels: List with names of all available kernels<br/>

Run Kernel:<br/>
[run_time,copy_time]=cl_run_kernel(ocl_dev_id,','kernel_function',global_range,local_range,in1,out1,[rw_flags]);<br/>
ocl_dev_id: ID of the OpenCL device to be used<br/>
kernel_function: Name of the kernel function to execute<br/>
global_range: Global OpenCL range (see NDRange)<br/>
local_range: Local OpenCL range (see NDRange)<br/>
in1, out1: List of variables to pass from/to kernel<br/>
rw_flags: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only<br/>

Build & Run Kernel:<br/>
[run_time]=cl_run_kernel(ocl_dev_id,' kernel_url.cl ','defines ','kernel_function',global_range,local_range,in1,out1,[rw_flags]);<br/>
ocl_dev_id: ID of the OpenCL device to be used<br/>
kernel_url.cl: URL of the kernel file<br/>
defines: List of OpenCL compiler defines<br/>
kernel_function: Name of the kernel function to execute<br/>
global_range: Global OpenCL range (see NDRange)<br/>
local_range: Local OpenCL range (see NDRange)<br/>
in1, out1: List of variables to pass from/to kernel<br/>
rw_flags: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only<br/>

Build & Run Kernel (with Kernel printf redirection):<br/>
[run_time]=cl_dbg_kernel(ocl_dev_id,' kernel_url.cl ','defines ','kernel_function',global_range,local_range,in1,out1,[rw_flags]);<br/>
ocl_dev_id: ID of the OpenCL device to be used<br/>
kernel_url.cl: URL of the kernel file<br/>
defines: List of OpenCL compiler defines<br/>
kernel_function: Name of the kernel function to execute<br/>
global_range: Global OpenCL range (see NDRange)<br/>
local_range: Local OpenCL range (see NDRange)<br/>
in1, out1: List of variables to pass from/to kernel<br/>
rw_flags: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only<br/>

