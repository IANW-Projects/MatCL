% This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

close all
clear all
clc


%%
%This scripts shows how to use cl_run_kernel to run a kernel. This example
%kernel implements a matrix multiplication and compares the runtime with
%the internal Matlab Implementation


for i=1:10000
A(i,1)=3;
A(i,2)=2;
A(i,3)=1;
end;
%%%%%%%%%%%
A=double(A);
B=double(A');

tmp=size(A);
num_rows=tmp(1);

tmp=size(B);
num_cols=tmp(2);
num_i=tmp(1);

C=double(zeros(num_rows,num_cols));
Range=[num_rows,num_cols,1];
settings = sprintf('-DNR=%d -DNC=%d -DNI=%d -DREAL=double',num_rows,num_cols,num_i);

tic;
mC=A*B;
toc

clearvars mC



%%
%run_kernel directly
tic;
[run_time]=cl_run_kernel(1,'mul_kernel.cl',settings,'MM',Range,0,A,B,C,[1 1 2]);
toc;
time_str=sprintf('OpenCL Kernel time is %f seconds.',double(run_time)/1000/1000);
 disp(time_str)

 C=double(zeros(num_rows,num_cols));


%%
%Build and run kernel seperately


%compile kernel
 [comp_time,kernels]=cl_run_kernel(1,'mul_kernel.cl',settings);
 time_str=sprintf('OpenCL Kernel compile time was %f seconds.',double(comp_time)/1000/1000);
disp(time_str)

disp("Run Kernel only:")
tic;
%run_kernel
[run_time,copy_time]=cl_run_kernel(1,'MM',Range,0,A,B,C,[1 1 2]);
toc;

time_str=sprintf('OpenCL Buffer copy time is: %f seconds.\n OpenCL Kernel runtime is %f seconds.',double(copy_time)/1000/1000,double(run_time)/1000/1000);
disp(time_str)
