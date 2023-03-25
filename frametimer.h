#ifndef FRAMETIMER_H_
#define FRAMETIMER_H_
#include <time.h>

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
        ts1 = ts2;
        ++m_elapsedframes; // At 140 FPS, this will roll over after around 8521 hours.
    }

    double GetAverageFPS() const
    {
        return static_cast<double>(GetElapsedFrames()) / GetElapsedTime();
    }

private:
    struct timespec ts1, ts2, first_ts;
    double fps;
    double m_first_time;
    unsigned int m_elapsedframes;
};


#endif /* FRAMETIMER_H_ */
