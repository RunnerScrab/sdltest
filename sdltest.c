#include <SDL.h>
#include <SDL_version.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_error.h>

#include <vector>

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
	SDLProgram(bool bVsync = true);
	~SDLProgram();
	bool CreateWindow(unsigned int width, unsigned int height);
	void Run();
protected:
	void Init();
	void Teardown();
private:
	SDL_Window* m_pWindow = 0;
	volatile bool m_bRunning;

	SDL_Renderer* m_pRenderer = 0;
	SDL_Surface* m_pScreenSurface = 0;
	SDL_Texture* m_pSurfaceTexture = 0;

	unsigned int m_screenwidth = 0, m_screenheight = 0;

	bool m_bVsync;

	ThreadPool m_threadpool;
};

bool SDLProgram::CreateWindow(unsigned int width, unsigned int height)
{
	m_screenwidth = width;
	m_screenheight = height;

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

	m_threadpool.StopThreadPool();
}


class CGOLTask : public Task
{
public:
CGOLTask(unsigned int* input, unsigned int* output, int startindex, int size, int height, int width) :
	m_pInput(input), m_pOutput(output),
		m_index(startindex), m_size(size),
		m_height(height), m_width(width)

	{

	}
	virtual void operator()() override
	{
		life((int*) m_pInput, (int*) m_pOutput, m_index, m_height, m_width);
	}
private:
	unsigned int* m_pInput, *m_pOutput;
	int m_index, m_size, m_height, m_width;
};

void SDLProgram::Run()
{
	m_bRunning = true;
	this->Init();

	SDL_Event ev = {0};
	FrameTimer stopwatch;


	unsigned int pixel_fmt = 0;
	int pixel_access = 0, pixel_w = 0, pixel_h = 0;
	SDL_QueryTexture(m_pSurfaceTexture, &pixel_fmt, &pixel_access, &pixel_w, &pixel_h);
	printf("Pixel format: %d Pixel Access: %d Width: %d Height %d\n",
		pixel_fmt, pixel_access, pixel_w, pixel_h);


	unsigned int pixelcount = (m_screenwidth*m_screenheight);
	std::vector<unsigned int> pixeldat(pixelcount, 0);
	std::vector<unsigned int> pixeldat_backing(pixelcount, 0);

	/* Create initial texture state */

	RandProvider* rng = new SeededRand(0xFEEDFACE);
	rng->FillRandBytes(reinterpret_cast<unsigned char*>(&pixeldat[0]), sizeof(unsigned int) * pixelcount);
	for(unsigned int idx = 0; idx < pixelcount; ++idx)
	{
		pixeldat[idx] = (pixeldat[idx] > 0x7FFFFFFF) ?  0xFFFFFFFF : 0xFF000000;
	}
	delete rng;

	printf("Entering loop\n");

	char window_caption[128] = {0};
	unsigned int frame_counter = 0;



	while(m_bRunning)
	{

		/* Begin Draw Code */
		for(unsigned int idx = 0; idx < pixelcount; ++idx)
		{
			life((int*) &pixeldat[0], (int*) &pixeldat_backing[0], idx, m_screenheight, m_screenwidth);
		}

		pixeldat.swap(pixeldat_backing);

		/* End Draw Code */

		/* Update window surface with updated texture */
		SDL_UpdateTexture(m_pSurfaceTexture, 0, &pixeldat[0], 4*m_screenwidth);
		SDL_RenderCopy(m_pRenderer, m_pSurfaceTexture, 0, 0);
		SDL_RenderPresent(m_pRenderer);

		/* Poll for and handle GUI events */
		while(SDL_PollEvent(&ev))
		{
			switch(ev.type)
			{
			case SDL_QUIT:
				m_bRunning = false;
				break;
			case SDL_KEYUP:
				printf("FPS: %f\n", stopwatch.GetFPS());
				break;
			default:
				break;
			}
		}

		/* Compute FPS for this frame */
		stopwatch.MarkTime();

		if(!(frame_counter & 31))
		{
			snprintf(window_caption, 128, "CGOL FPS: %f Elapsed: %f",
				stopwatch.GetFPS(), stopwatch.GetElapsedTime());
			SDL_SetWindowTitle(m_pWindow, window_caption);
			frame_counter = 0;
		}
		++frame_counter;
	}

	this->Teardown();
}


SDLProgram::SDLProgram(bool bVsync) : m_bVsync(bVsync)
{

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

	SDLProgram prog(true);
	prog.CreateWindow(1024, 1024);
	prog.Run();

	SDL_Quit();
	return 0;
}
