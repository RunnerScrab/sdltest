#include "devrand.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

DevRand::DevRand() : RandProvider()
{
	m_fd = open("/dev/random", O_RDONLY);
}

DevRand::~DevRand()
{
	close(m_fd);
}

void DevRand::FillRandBytes(unsigned char* out, int size)
{
	/* size argument is in bytes */
		int bytes_read = 0;
		int total_read = 0;
		do
		{
			bytes_read = read(m_fd, &out[total_read], size);
			if(bytes_read <= 0)
				return;
			total_read += bytes_read;
		}
		while(total_read < size && bytes_read > 0 && bytes_read == size);

}

SeededRand::SeededRand(unsigned int seed) : m_seed(seed)
{
	srand(seed);
}

void SeededRand::FillRandBytes(unsigned char* out, int size)
{
	for(int idx = 0; idx < size; ++idx)
	{
		out[idx] = rand() & 255;
	}
}
