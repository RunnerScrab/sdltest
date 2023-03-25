#include "frametimer.h"
#include <cstring>
#include <time.h>

FrameTimer::FrameTimer()
{
	memset(&ts1, 0, sizeof(struct timespec));
	memset(&ts2, 0, sizeof(struct timespec));
	memset(&first_ts, 0, sizeof(struct timespec));
}

void FrameTimer::Init()
{
	m_elapsedframes = 0;
	clock_gettime(CLOCK_REALTIME, &ts1);
	first_ts = ts1;
	m_first_time = static_cast<double>(first_ts.tv_sec)*1.0E9 +
		first_ts.tv_nsec;
}
