%CL_GET_DEVICES Enumerate OpenCL devices (returns a list whose i-th entry corresponds to the i-th OpenCL device)
%
%
%   [names, dev_class, max_mem, max_wg_size, max_local_work_size, compute_units] = cl_get_devices;
%
%
%   Outputs
%   -------
%
%   names: Names of all available devices
%   dev_class: The device class (CPU, GPU or Other for other or unknown Accelerators)
%   max_mem: The available device memory in bytes
%   max_wg_size: Max. size of OpenCL work group
%   max_local_work_size: Max. size of work items
%   compute_units: Number of compute units (e.g. CPU cores) of the device
%
