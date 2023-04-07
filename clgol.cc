#include "clgol.h"
#include <cstdio>
#include <cstdlib>

int LoadFileList(std::vector<std::string>& filenamelist, char*** outputbuffers, size_t** outputsizes)
{
  /* For use with clCreateProgramWithSource() */

	*outputbuffers = (char**) malloc(sizeof(char*) * filenamelist.size());
	*outputsizes = (size_t*) malloc(sizeof(size_t) * filenamelist.size());

	for(int idx = 0, z = filenamelist.size(); idx < z; ++idx)
  {
	  FILE* fp = fopen(filenamelist[idx].c_str(), "rb");
	  if(!fp)
	  {
		  return -1;
	  }

	  fseek(fp, 0, SEEK_END);
	  size_t len = ftell(fp);
	  fseek(fp, 0, SEEK_SET);
	  (*outputbuffers)[idx] = (char*) malloc(sizeof(char) * len + 1);
	  fread((*outputbuffers)[idx], sizeof(char), len, fp);
	  (*outputsizes)[idx] = len;
	  fclose(fp);
  }
	return 0;
}

CLManager::~CLManager()
{
	clReleaseDevice(m_deviceid);
	clReleaseCommandQueue(m_queue);
	clReleaseContext(m_context);
	if(m_kernel)
		clReleaseKernel(m_kernel);
}

int CLManager::SetKernelArgs(unsigned int pixelcount, unsigned int width, unsigned int height)
{
	cl_int retval = 0;
	m_input = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(unsigned int) * pixelcount, 0, 0);
	m_output = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(unsigned int) * pixelcount, 0, 0);

	retval = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &m_input);
	retval |= clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &m_output);
	retval |= clSetKernelArg(m_kernel, 2, sizeof(unsigned int), &height);
	retval |= clSetKernelArg(m_kernel, 3, sizeof(unsigned int), &width);

	return retval == CL_SUCCESS ? 0 : -1;
}

int CLManager::CreateKernel()
{
	cl_program prog = 0;
	cl_int retval = 0;

	std::vector<std::string> filenames = {"gol.cl"};

	char** buffers = 0;
	size_t* sizes = 0;
	LoadFileList(filenames, &buffers, &sizes);

	prog = clCreateProgramWithSource(m_context, filenames.size(), const_cast<const char**>(buffers), sizes, &retval);
	if(CL_SUCCESS != retval)
	{
		goto clean_and_return;
	}

	retval = clBuildProgram(prog, 1, &m_deviceid, "", 0, 0);
	if(CL_SUCCESS != retval)
	{
		size_t logsize = 0;
		clGetProgramBuildInfo(prog, m_deviceid, CL_PROGRAM_BUILD_LOG, 0, 0, &logsize);
		std::vector<char> log;
		log.resize(logsize + 1, 0);
		clGetProgramBuildInfo(prog, m_deviceid, CL_PROGRAM_BUILD_LOG, log.size(), &log[0], 0);
		printf("%s\n", &log[0]);
		goto clean_and_return;
	}

	m_kernel = clCreateKernel(prog, "life", &retval);

clean_and_return:
	clReleaseProgram(prog);
	for(unsigned int idx = 0; idx < filenames.size(); ++idx)
	{
		free(buffers[idx]);
	}
	free(buffers);
	free(sizes);
	return retval == CL_SUCCESS ? 0 : -1;
}

int CLManager::InitCL()
{
	cl_uint numplatforms = 0;
	cl_int retval = clGetPlatformIDs(0, 0, &numplatforms);
	if(CL_SUCCESS != retval)
	{
		return -1;
	}
	m_platforms.resize(numplatforms);

	retval = clGetPlatformIDs(numplatforms, &m_platforms[0], 0);
	if(CL_SUCCESS != retval)
	{
		return -1;
	}

	printf("Found %d CL platforms.\n", numplatforms);

	retval = clGetDeviceIDs(m_platforms[0], CL_DEVICE_TYPE_GPU, 1, &m_deviceid, 0);
	if(CL_SUCCESS != retval)
	{
		return -1;
	}

	m_context = clCreateContext(0, 1, &m_deviceid, 0, 0, &retval);

	if(CL_SUCCESS != retval)
	{
		return -1;
	}


	m_queue = clCreateCommandQueueWithProperties(m_context, m_deviceid, 0 , &retval);
	if(CL_SUCCESS != retval)
	{
		return -1;
	}


	return 0;
}

CLManager::CLManager()
{
	if(!InitCL())
		printf("CL Initialization success.\n");
	if(!CreateKernel())
	   printf("Kernel compilation succeeded.\n");
}
