#include "devrand.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


DevRand::DevRand()
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
