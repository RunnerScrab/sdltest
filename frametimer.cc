#include "frametimer.h"
#include <cstring>
#include <time.h>

FrameTimer::FrameTimer()
{
	ts1 = std::make_unique<struct timespec>();
	ts2 = std::make_unique<struct timespec>();
	memset(ts1.get(), 0, sizeof(struct timespec));
	memset(ts2.get(), 0, sizeof(struct timespec));
	clock_gettime(CLOCK_REALTIME, ts1.get());
}

void FrameTimer::MarkTime()
{
	/* Call this once every frame */
	clock_gettime(CLOCK_REALTIME, ts2.get());
	fps = 1.0E9/((static_cast<double>(ts2->tv_sec)*1.0E9 + ts2->tv_nsec) -
		(static_cast<double>(ts1->tv_sec)*1.0E9 + ts1->tv_nsec));
	ts1.swap(ts2);
}

double FrameTimer::GetFPS() const
{
	return fps;
}
