#include <SDL.h>
#include <SDL_version.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_error.h>

#include <vector>
#include <atomic>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>
#include <sys/sysinfo.h>
#include <deque>

#include "devrand.h"
#include "sdllibraryhelper.h"
#include "frametimer.h"

#include "lifescalar.h"

#include "threadpool.h"

class SDLProgram
{
public:
	SDLProgram(unsigned int width, unsigned int height, unsigned int maxthreads, bool bVsync = true);
    ~SDLProgram();
    bool CreateWindow();
    void Run();
    void RunST();
    void RunMT();
protected:
    void Init();
    void Teardown();
private:
    void InitializeBoard(unsigned int* pixeldat, unsigned int seed, unsigned int pixelcount);
    void HandleInput(SDL_Event* pev);
    void UpdateScreen(void* pixeldat, unsigned int pitch);
    void UpdateDebugDisplay();

    SDL_Window* m_pWindow = 0;
    volatile bool m_bRunning;

    SDL_Renderer* m_pRenderer = 0;
    SDL_Surface* m_pScreenSurface = 0;
    SDL_Texture* m_pSurfaceTexture = 0;

    unsigned int m_screenwidth = 0, m_screenheight = 0;
    unsigned int m_pixelcount = 0;
    bool m_bVsync;

    unsigned int m_maxthreads;



    ThreadPool* m_pthreadpool = 0;

    pthread_mutex_t m_outputbuffer_mutex;

    FrameTimer m_stopwatch;
    char m_window_caption[256] = {0};
};

bool SDLProgram::CreateWindow()
{
    m_pWindow = SDL_CreateWindow("SDLTest", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 m_screenwidth, m_screenheight, SDL_WINDOW_SHOWN);
    return 0 == m_pWindow;
}

void SDLProgram::Init()
{
    int renderdriver_count = SDL_GetNumRenderDrivers();
    printf("There are %d render drivers.\n", renderdriver_count);

    int idx = 0;
    for(; idx < renderdriver_count; ++idx)
    {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(idx, &info);

        printf("%d) %s\n", idx + 1, info.name);
    }

    SDL_ClearError();

    int render_flags = SDL_RENDERER_ACCELERATED;

    if(m_bVsync)
    {
        render_flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, render_flags);
    int vsync_succ = SDL_RenderSetVSync(m_pRenderer, m_bVsync);

    if(m_pRenderer)
    {
        printf("Renderer created %s vsync.\n", (!vsync_succ && m_bVsync) ? "with" : "without");
    }
    else
    {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_ClearError();
    }

    m_pScreenSurface = SDL_CreateRGBSurfaceWithFormat(0, m_screenwidth, m_screenheight, 32, SDL_PIXELFORMAT_ARGB8888);
    m_pSurfaceTexture = SDL_CreateTextureFromSurface(m_pRenderer, m_pScreenSurface);

    SDL_SetRenderTarget(m_pRenderer, m_pSurfaceTexture);

    unsigned int pixel_fmt = 0;
    int pixel_access = 0, pixel_w = 0, pixel_h = 0;
    SDL_QueryTexture(m_pSurfaceTexture, &pixel_fmt, &pixel_access, &pixel_w, &pixel_h);
    printf("Pixel format: %d Pixel Access: %d Width: %d Height %d\n",
           pixel_fmt, pixel_access, pixel_w, pixel_h);


    pthread_mutex_init(&m_outputbuffer_mutex, 0);

    m_bRunning = true;
    m_stopwatch.Init();
}

void SDLProgram::Teardown()
{
    printf("Teardown\n");

    if(m_pSurfaceTexture)
    {
        SDL_DestroyTexture(m_pSurfaceTexture);
        m_pSurfaceTexture = 0;
    }

    if(m_pScreenSurface)
    {
        SDL_FreeSurface(m_pScreenSurface);
        m_pScreenSurface = 0;
    }

    if(m_pRenderer)
    {
        SDL_DestroyRenderer(m_pRenderer);
        m_pRenderer = 0;
    }

    pthread_mutex_destroy(&m_outputbuffer_mutex);
}


class CGOLTask : public Task
{
public:
    CGOLTask(unsigned int* input, unsigned int* output, int startindex, int size, int height, int width) :
        m_pInput(input), m_pOutput(output),
        m_index(startindex), m_size(size),
        m_height(height), m_width(width)

    {
        printf("Segment starting at %d with size %d\n", startindex, size);
        m_segmentresult.resize(width * height);
    }
    virtual void operator()() override
    {
        for(int idx = m_index; idx < (m_size + m_index - 1); ++idx)
        {
            life((int*) m_pInput, (int*) &m_segmentresult[0], idx, m_height, m_width);
            //life((int*) m_pInput, (int*) &m_pOutput[0], idx, m_height, m_width);
        }
    }
    virtual bool ShouldDelete() {
        return false;
    }
    unsigned int* GetSegmentOutput()
    {
        return &m_segmentresult[0];
    }
    void SetInputPointer(unsigned int* input)
    {
        m_pInput = input;
    }
private:
    unsigned int* m_pInput, *m_pOutput;
    int m_index, m_size, m_height, m_width;
    std::vector<unsigned int> m_segmentresult;
};

void SDLProgram::InitializeBoard(unsigned int* pixeldat, unsigned int seed, unsigned int pixelcount)
{
    RandProvider* rng = new SeededRand(seed);
    rng->FillRandBytes(reinterpret_cast<unsigned char*>(pixeldat), sizeof(unsigned int) * pixelcount);

    /* Make black and white */
    for(unsigned int idx = 0; idx < pixelcount; ++idx)
    {
        pixeldat[idx] = (pixeldat[idx] > 0x7FFFFFFF) ?  0xFFFFFFFF : 0xFF000000;
    }
    delete rng;
}

void SDLProgram::Run()
{
	if(m_maxthreads > 1)
	{
		RunMT();
	}
	else
	{
		RunST();
	}
}

void SDLProgram::RunMT()
{
    m_pthreadpool = new ThreadPool(m_maxthreads);

    unsigned int frame_counter = 0;

    this->Init();

    SDL_Event ev = {0};

    std::vector<unsigned int> pixeldat(m_pixelcount);
    std::vector<unsigned int> pixeldat_backing(m_pixelcount);

    /* Create initial texture state */

    InitializeBoard(&pixeldat[0], 0xFEEDFACE, m_pixelcount);

    unsigned int segsize = m_pixelcount / m_maxthreads;

    printf("Pixel count: %d Threads: %d Segment size: %d\n", m_pixelcount, m_maxthreads, segsize);
    /* Initialize Thread Tasks */
    std::vector<CGOLTask> cgoltasks;

    cgoltasks.reserve(m_maxthreads);

    for(int idx = 0; idx < m_maxthreads; ++idx)
    {
        //FIXME : work distribution in case pixelcount is not divisible by # of threads !!!
        cgoltasks.emplace_back(CGOLTask(&pixeldat[0], &pixeldat_backing[0],
                                        idx * segsize,
                                        segsize,
                                        m_screenheight, m_screenwidth));
    }

    while(m_bRunning)
    {

        /* Begin Draw Code */

            m_pthreadpool->ResetResultCount();
            for(int idx = 0; idx < m_maxthreads; ++idx)
            {
                m_pthreadpool->AddTask(&cgoltasks[idx]);
            }

            m_pthreadpool->WaitForAllCurrentTasks(m_maxthreads);


            for(int idx = 0; idx < m_maxthreads; ++idx)
            {
                unsigned int* src = cgoltasks[idx].GetSegmentOutput();
                memcpy(&pixeldat_backing[idx * segsize], src + (idx * segsize), segsize * sizeof(unsigned int));

            }

        pixeldat.swap(pixeldat_backing);

        /* End Draw Code */

        /* Update window surface with updated texture */
	UpdateScreen(reinterpret_cast<void*>(&pixeldat[0]), 4*m_screenwidth);

        /* Poll for and handle GUI events */
	HandleInput(&ev);
        /* Compute FPS for this frame */
        m_stopwatch.MarkTime();

        if(!(frame_counter & 31))
        {
	    UpdateDebugDisplay();
            frame_counter = 0;
        }
        ++frame_counter;
    }

    m_pthreadpool->StopThreadPool();
    delete m_pthreadpool;

    this->Teardown();
}

void SDLProgram::RunST()
{
    unsigned int frame_counter = 0;

    this->Init();

    SDL_Event ev = {0};

    std::vector<unsigned int> pixeldat(m_pixelcount);
    std::vector<unsigned int> pixeldat_backing(m_pixelcount);

    /* Create initial texture state */

    InitializeBoard(&pixeldat[0], 0xFEEDFACE, m_pixelcount);

    while(m_bRunning)
    {
        /* Begin Draw Code */

       for(unsigned int idx = 0; idx < m_pixelcount; ++idx)
       {
            life((int*) &pixeldat[0], (int*) &pixeldat_backing[0], idx, m_screenheight, m_screenwidth);
       }

        pixeldat.swap(pixeldat_backing);

        /* End Draw Code */

        /* Update window surface with updated texture */
	UpdateScreen(reinterpret_cast<void*>(&pixeldat[0]), 4*m_screenwidth);

        /* Poll for and handle GUI events */
	HandleInput(&ev);

        /* Compute FPS for this frame */
        m_stopwatch.MarkTime();

        if(!(frame_counter & 31))
        {
		UpdateDebugDisplay();
            frame_counter = 0;
        }
        ++frame_counter;
    }

    m_pthreadpool->StopThreadPool();
    delete m_pthreadpool;

    this->Teardown();
}

void SDLProgram::UpdateScreen(void* pixeldat, unsigned int pitch)
{
        SDL_UpdateTexture(m_pSurfaceTexture, 0, pixeldat, pitch);
        SDL_RenderCopy(m_pRenderer, m_pSurfaceTexture, 0, 0);
        SDL_RenderPresent(m_pRenderer);
}

void SDLProgram::HandleInput(SDL_Event* pev)
{
        while(SDL_PollEvent(pev))
        {
            switch(pev->type)
            {
            case SDL_QUIT:
                m_bRunning = false;
                break;
            default:
                break;
            }
        }
}

void SDLProgram::UpdateDebugDisplay()
{
            snprintf(m_window_caption, 128, "Board: %d x %d Threads: %d CGOL AFPS: %f Elapsed: %fs Frames: %u",
		    m_screenwidth, m_screenheight, m_maxthreads,
                     m_stopwatch.GetAverageFPS(), m_stopwatch.GetElapsedTime(), m_stopwatch.GetElapsedFrames());
            SDL_SetWindowTitle(m_pWindow, m_window_caption);
}

SDLProgram::SDLProgram(unsigned int width, unsigned int height, unsigned int maxthreads, bool bVsync) :
m_screenwidth(width), m_screenheight(height), m_bVsync(bVsync), m_maxthreads(maxthreads), m_pthreadpool(0)
{
    m_pixelcount = width * height;
}

SDLProgram::~SDLProgram()
{
    if(m_pWindow)
    {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = 0;
    }
}




int main(void)
{
    SDLLibraryHelper sdl;
    bool result = sdl.InitLibrary();

    printf("Statically linked SDL version is %s.\n", sdl.GetStaticVerStr());
    printf("Dynamically linked SDL version is %s.\n", sdl.GetStaticVerStr());

    printf("SDL Initialization %s.\n",
           result ? "success" : "failure");

    //SDLProgram prog(1920, 1080, ThreadPool::GetMaxThreads(), false);
    SDLProgram prog(1920, 1080, 1, false);
    prog.CreateWindow();
    prog.Run();

    SDL_Quit();
    return 0;
}
