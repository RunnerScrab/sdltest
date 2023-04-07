#ifndef FRAMETIMER_H_
#define FRAMETIMER_H_
#include <vector>

#include <time.h>

struct FPSSample
{
    float t, fps;
	unsigned int framecount;
	FPSSample(float time, float fps, unsigned int framecount)
    {
        this->t = time;
        this->fps = fps;
	this->framecount = framecount;
    }
};

class FrameTimer
{
public:
    FrameTimer();
    void Init();

    double GetFPS() const
    {
        return fps;
    }

    double GetElapsedTime() const
    {
        return ((static_cast<double>(ts2.tv_sec)*1.0E9 + ts2.tv_nsec)
                - m_first_time)/1.0E9;
    }

    unsigned int GetElapsedFrames() const
    {
        return m_elapsedframes;
    }

    void MarkTime()
    {
        /* Call this once every frame */
        clock_gettime(CLOCK_REALTIME, &ts2);
        fps = 1.0E9/(static_cast<double>(ts2.tv_sec - ts1.tv_sec)*1.0E9 + ts2.tv_nsec - ts1.tv_nsec);
        ts1 = ts2;
        ++m_elapsedframes; // At 140 FPS, this will roll over after around 8521 hours.
    }

    double GetAverageFPS() const
    {
        return static_cast<double>(GetElapsedFrames()) / GetElapsedTime();
    }

    void RecordSample()
    {
	    m_samples.emplace_back(FPSSample(GetElapsedTime(), fps, m_elapsedframes));
    }

    std::vector<struct FPSSample>& GetSamples()
    {
        return m_samples;
    }
private:
    struct timespec ts1, ts2, first_ts;
    double fps;
    double m_first_time;
    unsigned int m_elapsedframes;

    std::vector<struct FPSSample> m_samples;
};


#endif /* FRAMETIMER_H_ */
