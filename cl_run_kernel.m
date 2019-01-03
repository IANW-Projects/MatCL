%CL_RUN_KERNEL Build and run OpenCL kernels
%   ------------
%   Build Kernel
%   ------------
%
%   [comp_time, kernels] = cl_run_kernel(ocl_dev_id, 'kernel_url.cl', 'defines');
%
%   Inputs
%   -------
%   ocl_dev_id: ID of the OpenCL device to be used
%   kernel_url: URL of the kernel file
%   defines: List of OpenCL compiler defines
%
%   Outputs
%   -------
%   comp_time: Microseconds it took to compile the kernels
%   kernels: List with names of all available kernels
%
%
%   ----------
%   Run Kernel
%   ----------
%
%   [run_time, copy_time] = cl_run_kernel(ocl_dev_id, {'kernel_function1','kernel_function2'}, ...
%                                         global_range, local_range, in1, out1, [rw_flags]);
%
%   Inputs
%   -------
%   ocl_dev_id: ID of the OpenCL device to be used
%   kernel_function: Cell array of kernel functions to execute (can also be a single string for just one kernel)
%   global_range: 3D global OpenCL range (see NDRange). If this vector has six entires, the first three define the 3D work offset followed by the 3D work size.
%   local_range: 3D local OpenCL range (see NDRange)
%   in1, out1: List of variables to pass from/to kernel
%   rw_flags: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only
%
%   Outputs
%   -------
%   run_time:  Microseconds it took to execute the kernels
%   copy_time:  Microseconds it took to copy all buffers
%
%
%   ------------------
%   Build & Run Kernel
%   ------------------
%
%   [run_time] = cl_run_kernel(ocl_dev_id, 'kernel_url.cl', 'defines', 'kernel_function', ...
%                              global_range, local_range, in1, out1, [rw_flags]);
%
%   Inputs
%   -------
%   ocl_dev_id: ID of the OpenCL device to be used
%   kernel_url.cl: URL of the kernel file
%   defines: List of OpenCL compiler defines
%   kernel_function: Cell array of kernel functions to execute (can also be a signal string)
%   global_range: 3D global OpenCL range (see NDRange). If this vector has six entires, the first three define the 3D work offset followed by the 3D work size.
%   local_range: Local OpenCL range (see NDRange)
%   in1, out1: List of variables to pass from/to kernel
%   rw_flags: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only
%
%   Outputs
%   -------
%   run_time:  Microseconds it took to execute the kernels
