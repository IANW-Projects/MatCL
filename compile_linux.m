%This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

close all
clear all
clc

%Change OpenCL library path according to your setup
mex -g COMPFLAGS='$COMPFLAGS -std=c++11 -O2' '-LC /usr/lib/x86_64-linux-gnu' -lOpenCL src/cl_get_devices.cpp src/ocl_dev_mgr.cpp
mex -g COMPFLAGS='$COMPFLAGS -std=c++11 -O2' '-LC /usr/lib/x86_64-linux-gnu' -lOpenCL src/cl_run_kernel.cpp src/ocl_dev_mgr.cpp
mex -g COMPFLAGS='$COMPFLAGS -std=c++11 -O2' '-LC /usr/lib/x86_64-linux-gnu' -lOpenCL src/cl_dbg_kernel.cpp src/ocl_dev_mgr.cpp

[dev_name,dev_type,max_mem,wg_size,lw_size]=cl_get_devices;
