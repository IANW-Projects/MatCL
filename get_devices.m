%This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license.

clear all
close all
clc

%%
%To get a list of available OpenCl devices use 'cl_get_devices'. This
%functions returns the names of all availbale devices, the device class
%(CPU, GPU or Other for other or unknown Accelerators) and the availble
%device meory in bytes. To choose a device use the index of the
%corresponding entry in the names array.
[dev_name,dev_type,max_mem,wg_size,lw_size,compute_units]=cl_get_devices;
