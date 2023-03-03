#include "frametimer.h"
#include <cstring>
#include <time.h>

FrameTimer::FrameTimer()
{
	memset(&ts1, 0, sizeof(struct timespec));
	memset(&ts2, 0, sizeof(struct timespec));
	memset(&first_ts, 0, sizeof(struct timespec));
	clock_gettime(CLOCK_REALTIME, &ts1);
	first_ts = ts1;
}

void FrameTimer::MarkTime()
{
	/* Call this once every frame */
	clock_gettime(CLOCK_REALTIME, &ts2);
	fps = 1.0E9/((static_cast<double>(ts2.tv_sec)*1.0E9 + ts2.tv_nsec) -
		(static_cast<double>(ts1.tv_sec)*1.0E9 + ts1.tv_nsec));
	ts1 = ts2;
}

double FrameTimer::GetElapsedTime() const
{
	return ((static_cast<double>(ts2.tv_sec)*1.0E9 + ts2.tv_nsec) -
		(static_cast<double>(first_ts.tv_sec)*1.0E9 + first_ts.tv_nsec))/1.0E9;
}

double FrameTimer::GetFPS() const
{
	return fps;
}
