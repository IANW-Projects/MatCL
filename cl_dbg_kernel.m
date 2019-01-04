%CL_DBG_KERNEL Build and run OpenCL kernels with printf redirection
%
% 
%   [run_time] = cl_dbg_kernel(ocl_dev_id, 'kernel_url.cl', 'defines', 'kernel_function', ...
%                              global_range, local_range, in1, out1, [rw_flags]);
%
%   Inputs
%   -------
%   ocl_dev_id: ID of the OpenCL device to be used
%   kernel_url.cl: URL of the kernel file
%   defines: List of OpenCL compiler defines
%   kernel_function: Cell array of kernel functions to execute (can also be a single string for just one kernel)
%   global_range: 3D global OpenCL range (see NDRange). If this vector has six entires, the first three define the 3D work offset followed by the 3D work size.
%   local_range: Local OpenCL range (see NDRange)
%   in1, out1: List of variables to pass from/to kernel
%   rw_flags: read/write flag for the Kernel variables, this can either be scalar (all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read only / 2 - kernel write only
%
%   Outputs
%   -------
%   run_time:  Microseconds it took to execute the kernels (might be slower due to printf redirection)
