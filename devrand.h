#ifndef DEVRAND_H_
#define DEVRAND_H_

class DevRand
{
public:
	DevRand();
	void FillRandBytes(unsigned char* out, int size);
	~DevRand();
private:
	int m_fd;
};


#endif /* DEVRAND_H_ */
