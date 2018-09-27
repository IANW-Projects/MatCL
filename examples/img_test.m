% This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

clear all
clc
close all

%Select OpenCL device
device=1;

%Either load test image from .mat file or take an image using a camera(requires webcam support package)
%cam_img = snapshot(webcam);
load('imgData.mat');
cam_img = rgb2gray(cam_img); %transform to grayscale image
dims=size(cam_img); %get image size

%add artificial noise to the image
nI=imnoise(cam_img,'salt & pepper',0.06);

%Run native matlab medfilt function and track execution time
tic
K = medfilt2(nI);
cpu_time=toc;


%%
%Set OpenCL workgroup dimensions depending on the size of the image(take care of bounds)
global_range=[3 3 0 dims(2)-6 dims(1)-6 1];
local_range=[0];

%Convert data to uint8 and transform to 1x(dims(1)*dims(2)) vector
imgData=uint8(reshape(nI',[1,dims(1)*dims(2)]));

%Preallocate destination array
destI=uint8(zeros(1,(dims(2))*(dims(1))));

%Set OpenCL kernel defines
settings=sprintf(' -DWIDTH=%d ', dims(2));
%Precompile filter kernel
[comp_time,kernels]=cl_run_kernel(device,'filter.cl',settings);
%Execute OpenCL median filter kernel and track total execution time
tic
[run_time,copy_time]=cl_run_kernel(device,'filter',global_range,local_range,imgData,destI,0);
ocl_time=toc;
%%
%Output results to console
cl_times=sprintf('Buffer copy time: %.3f ms    Kernel runtime: %.3f ms',double(copy_time)/1000,double(run_time)/1000);

%convert image data back to matlab dims(1)xdims(2) style
newImg=reshape(destI,[dims(1),dims(2)])';

%Generate figure with results and runtimes
cpu_title=sprintf('CPU Runtime: %.3f ms',cpu_time*1000);
ocl_title=sprintf('OpenCL Runtime: %.3f ms',ocl_time*1000);

figure('units','normalized','outerposition',[0 0 1 1])
subplot(2,2,[1,2])
imshow(nI)
title('Original')

subplot(2,2,3)
imshow(K)
title(cpu_title)

subplot(2,2,4)
imshow(newImg)
title(ocl_title)
xlabel(cl_times)
