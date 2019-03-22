%This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

close all
clear all
clc

%Change OpenCL library path according to your setup
mex -g COMPFLAGS='$COMPFLAGS -O2' '-L C:\Intel\OpenCL\sdk\lib\x64\' '-I C:\Intel\OpenCL\sdk\include\' -lOpenCL src\cl_get_devices.cpp src\ocl_dev_mgr.cpp
mex -g COMPFLAGS='$COMPFLAGS -O2' '-L C:\Intel\OpenCL\sdk\lib\x64\' '-I C:\Intel\OpenCL\sdk\include\' -lOpenCL src\cl_run_kernel.cpp src\ocl_dev_mgr.cpp
mex -g COMPFLAGS='$COMPFLAGS -O2' '-L C:\Intel\OpenCL\sdk\lib\x64\' '-I C:\Intel\OpenCL\sdk\include\' -lOpenCL src\cl_dbg_kernel.cpp src\ocl_dev_mgr.cpp

[dev_name,dev_type,max_mem,wg_size,lw_size,compute_units]=cl_get_devices;
