/* This project is licensed under the terms of the Creative Commons CC BY-NC-ND 4.0 license. */

#include <math.h>
#include "mex.h"
#include "matrix.h"
#include <iostream>
#include <string>
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <process.h>
#endif


#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120

#include <CL/cl2.hpp>
#include "ocl_dev_mgr.hpp"


#include "MatCL.hpp"


class mystream : public std::streambuf
{
protected:
	virtual std::streamsize xsputn(const char *s, std::streamsize n) { mexPrintf("%.*s", n, s); return n; }
	virtual int overflow(int c = EOF) { if (c != EOF) { mexPrintf("%.1s", &c); } return 1; }
};
class scoped_redirect_cout
{
public:
	scoped_redirect_cout() { old_buf = std::cout.rdbuf(); std::cout.rdbuf(&mout); }
	~scoped_redirect_cout() { std::cout.rdbuf(old_buf); }
private:
	mystream mout;
	std::streambuf *old_buf;
};
static scoped_redirect_cout mycout_redirect;



void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray*prhs[])

{

	ocl_dev_mgr& dev_mgr = ocl_dev_mgr::getInstance();

	uint32_t device = (uint32_t)mxGetScalar(prhs[0]) - 1;

	if (nrhs>2) {

		if (device<dev_mgr.get_avail_dev_num()) {
			dev_mgr.deinitalize();
			compilerun(plhs, nrhs, prhs, &dev_mgr, device, true,false);

		}

		else {
			mexErrMsgIdAndTxt("MATLAB:cl_dev", "OpenCl Device not found!");
		}
	}
	else {
		mexErrMsgIdAndTxt("MATLAB:syntax", "Incorrect Syntax!");
	}



	return;

}
