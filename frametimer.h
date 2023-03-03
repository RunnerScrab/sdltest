#ifndef FRAMETIMER_H_
#define FRAMETIMER_H_
#include <memory>

class FrameTimer
{
public:
	FrameTimer();
	void MarkTime();
	double GetFPS() const;
private:
	std::unique_ptr<struct timespec> ts1, ts2;
	double fps;
};


#endif /* FRAMETIMER_H_ */
