%This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

close all
clear all
clc

% Apple does not ship cl2.hpp in general
mkdir('CL');
websave('CL/cl2.hpp', 'https://github.com/KhronosGroup/OpenCL-CLHPP/releases/download/v2.0.10/cl2.hpp');

%Change OpenCL library path according to your setup
mex -g COMPFLAGS='$COMPFLAGS -std=c++11 -O2 -framework OpenCL' -I./ LDFLAGS='$LDFLAGS -framework OpenCL' src/cl_get_devices.cpp src/ocl_dev_mgr.cpp
mex -g COMPFLAGS='$COMPFLAGS -std=c++11 -O2 -framework OpenCL' -I./ LDFLAGS='$LDFLAGS -framework OpenCL' src/cl_run_kernel.cpp src/ocl_dev_mgr.cpp
mex -g COMPFLAGS='$COMPFLAGS -std=c++11 -O2 -framework OpenCL' -I./ LDFLAGS='$LDFLAGS -framework OpenCL' src/cl_dbg_kernel.cpp src/ocl_dev_mgr.cpp

[dev_name,dev_type,max_mem,wg_size,lw_size]=cl_get_devices;
