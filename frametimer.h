#ifndef FRAMETIMER_H_
#define FRAMETIMER_H_
#include <time.h>

class FrameTimer
{
public:
	FrameTimer();
	void MarkTime();
	double GetFPS() const;
	double GetElapsedTime() const;
private:
	struct timespec ts1, ts2, first_ts;
	double fps;
};


#endif /* FRAMETIMER_H_ */
