%This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

clear all
close all
clc

%%
% Use cl_run_kernel to compile and launch kernels. It is possible to compile
% and run kernels in a two-stage process to increase performance or just use
% a single step approach, that does everything in one go.

global_range=[10,1,1];  %Set global OpenCl Range. Default indexing is 3D. To use a 1D index set y and z to 1
local_range=[0];        %Let OpenCL decide local range, otherwise specify range explicitly (like global range)

% Create input data for the kernel
for i=1:20
   in1(1,i)=double(1);
   in1(2,i)=double(1);
   in1(3,i)=double(1);
   in1(4,i)=double(1);

   in2(1,i)=double(2);
   in2(2,i)=double(2);
   in2(3,i)=double(2);
   in2(4,i)=double(2);
end

% This example shows how to only compile the kernel but not run it. The
% arguments are as follows:
% - OpenCl Device ID - see cl_get_devices
% - Kernel file URL
% - Kernel defines, can be used to efficently define constant values or set
% other compiler arguments
%
%This functions returns the compile time (in us) and an array with the names of the compiled kernel
%functions
%The OpenCL optimization flags -cl-mad-enable -cl-no-signed-zeros
%-cl-finite-math-only were tested on diffrent devices and sould not cause
%unexpected behaviour
[comp_time,kernels]=cl_run_kernel(1,'test_kernel.cl','-DDT=1.0 -cl-mad-enable -cl-no-signed-zeros -cl-finite-math-only');


% This example shows how to run a precompiled kernel. The
% arguments are as follows:
% - OpenCl Device ID - see cl_get_devices
% - Name of the function to run or cell array of kernel names to queue
% multiple kernels
% - Global OpenCL Range used to launch the kernel (see OpenCL NDRange)
% - Local OpenCL Range used to launch the kernel (see OpenCL NDRange). This
% value can be set to 0 to let OpenCL decide the best values
% - List of varaibles to be used by the kernel - they will be passed in the
% same order to the kernel itself. In case these variables get changed by
% the kernel, the value of the input variable will change automatically
% - read/write flag for the Kernel variables, this can either be scalar(all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read
% only / 2 - kernel write only.
%
%This function returns the runtime of the actual kernel and teh buffer copy time in us
[run_time,copy_time]=cl_run_kernel(1,'test1',global_range,local_range,in1,in2,0);

% This example shows how to compile and execute a kernel in a single pass.
%  The arguments are as follows:
% - OpenCl Device ID - see cl_get_devices
% - Kernel file URL
% - Kernel defines, can be used to efficently define constant values or set
% other compiler arguments
% - Name of the function to run or cell array of kernel names to queue
% multiple kernels
% - Global OpenCL Range used to launch the kernel (see OpenCL NDRange)
% - Local OpenCL Range used to launch the kernel (see OpenCL NDRange). This
% value can be set to 0 to let OpenCL decide the best values
% - List of varaibles to be used by the kernel - they will be passed in the
% same order to the kernel itself. In case these variables get changed by
% the kernel, the value of the input variable will change automatically
% - read/write flag for the Kernel variables, this can either be scalar(all variables are read&write) or a vector with an entry for each variable: 0 - read&write / 1 - kernel read
% only / 2 - kernel write only.
%
%This function returns the runtime of the actual kernel in ms
[run_time]=cl_run_kernel(1,'test_kernel.cl','-DDT=5.0 -cl-mad-enable -cl-no-signed-zeros -cl-finite-math-only','test2',global_range,local_range,in1,in2,[0 1]);


 %Same as above. but this functions pipes kernel printf to Matlab
[run_time]=cl_dbg_kernel(1,'test_kernel.cl','-DDT=5.0 -cl-mad-enable -cl-no-signed-zeros -cl-finite-math-only','test2',global_range,local_range,in1,in2,[0 1]);
