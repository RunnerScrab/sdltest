#ifndef DEVRAND_H_
#define DEVRAND_H_

class RandProvider
{
public:
	RandProvider(){}
	virtual void FillRandBytes(unsigned char* out, int size) = 0;
	virtual ~RandProvider(){}
};

class DevRand : public RandProvider
{
public:
	DevRand();
	virtual void FillRandBytes(unsigned char* out, int size) override;
	~DevRand();
private:
	int m_fd;
};

class SeededRand : public RandProvider
{
public:
	SeededRand(unsigned int seed);
	virtual void FillRandBytes(unsigned char* out, int size) override;
	~SeededRand(){};
private:
	unsigned int m_seed;
};

#endif /* DEVRAND_H_ */
