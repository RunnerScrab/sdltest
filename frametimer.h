#ifndef FRAMETIMER_H_
#define FRAMETIMER_H_
#include <time.h>

class FrameTimer
{
public:
	FrameTimer();
	void Init();
	void MarkTime();
	double GetFPS() const;
	double GetElapsedTime() const;
	unsigned int GetElapsedFrames() const;
	double GetAverageFPS() const;
private:
	struct timespec ts1, ts2, first_ts;
	double fps;
	unsigned int m_elapsedframes;
};


#endif /* FRAMETIMER_H_ */
