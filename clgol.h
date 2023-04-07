#ifndef CLGOL_H_
#define CLGOL_H_
#include <string>
#include <vector>
#include <CL/cl.h>

class CLManager
{
public:
	CLManager();
	~CLManager();
	int InitCL();
	int CreateKernel();
	int SetKernelArgs(unsigned int, unsigned int, unsigned int);
protected:

private:
	cl_command_queue m_queue = 0;
	cl_kernel m_kernel = 0;
	cl_context m_context = 0;

	cl_device_id m_deviceid = 0;
	std::vector<cl_platform_id> m_platforms;

	cl_mem m_input, m_output;
};

#endif /* CLGOL_H_ */
