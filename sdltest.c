#include <SDL.h>
#include <SDL_version.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_error.h>



#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

class FrameTimer
{
public:
	FrameTimer()
	{
		memset(&ts1, 0, sizeof(struct timespec));
		memset(&ts2, 0, sizeof(struct timespec));
		clock_gettime(CLOCK_REALTIME, &ts1);
	}
	void MarkTime()
	{
		/* Call this once every frame */
		clock_gettime(CLOCK_REALTIME, &ts2);
		fps = 1.0E9/((static_cast<double>(ts2.tv_sec)*1.0E9 + ts2.tv_nsec) -
			(static_cast<double>(ts1.tv_sec)*1.0E9 + ts1.tv_nsec));
		ts1 = ts2;
	}

	double GetFPS() const
	{
		return fps;
	}
private:
	struct timespec ts1, ts2;
	double fps;
};

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

	m_pScreenSurface = SDL_CreateRGBSurface(0, m_screenwidth, m_screenheight, 32, 0, 0, 0, 0);
	m_pSurfaceTexture = SDL_CreateTextureFromSurface(m_pRenderer, m_pScreenSurface);

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
}

void SDLProgram::Run()
{
	m_bRunning = true;
	this->Init();

	SDL_Event ev;

	SDL_SetRenderTarget(m_pRenderer, m_pSurfaceTexture);

	unsigned int pixel_fmt = 0;
	int pixel_access = 0, pixel_w = 0, pixel_h = 0;
	SDL_QueryTexture(m_pSurfaceTexture, &pixel_fmt, &pixel_access, &pixel_w, &pixel_h);
	printf("Pixel format: %d Pixel Access: %d Width: %d Height %d\n",
		pixel_fmt, pixel_access, pixel_w, pixel_h);

	unsigned int pixelcount = (m_screenwidth*m_screenheight);
	std::vector<unsigned int> pixeldat(pixelcount);

	memset(&pixeldat[0], 0xFF, pixelcount);

	for(unsigned int idx = 0; idx < pixelcount; ++idx)
	{
		if(idx & 1)
			pixeldat[idx] = 0xFF000000;
	}

	FrameTimer stopwatch;

	printf("Entering loop\n");
	while(m_bRunning)
	{

		memset(&pixeldat[0], 0xFF, pixelcount);

		for(unsigned int idx = 0; idx < pixelcount; ++idx)
		{
			if(idx & 1)
				pixeldat[idx] = 0xFF000000;
		}

		SDL_UpdateTexture(m_pSurfaceTexture, 0, &pixeldat[0], 4);
		SDL_RenderCopy(m_pRenderer, m_pSurfaceTexture, 0, 0);
		SDL_RenderPresent(m_pRenderer);

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

class SDLLibraryHelper
{

public:
	SDLLibraryHelper();
	~SDLLibraryHelper();
	bool InitLibrary();

	const char* GetStaticVerStr()
	{
		return m_statver_str;
	}

	const char* GetDynamicVerStr()
	{
		return m_dynver_str;
	}

private:
	SDL_version m_statver, m_dynver;
	bool m_bLibraryStarted = 0;
	char m_statver_str[32] = {0};
	char m_dynver_str[32] = {0};
};

SDLLibraryHelper::SDLLibraryHelper()
{
	memset(&m_statver, 0, sizeof(SDL_version));
	memset(&m_dynver, 0, sizeof(SDL_version));
	SDL_VERSION(&m_statver);
	SDL_GetVersion(&m_dynver);
	snprintf(m_statver_str, 32, "%u.%u.%u",
		m_statver.major, m_statver.minor, m_statver.patch);
	snprintf(m_dynver_str, 32, "%u.%u.%u",
		m_dynver.major, m_dynver.minor, m_dynver.patch);
}

SDLLibraryHelper::~SDLLibraryHelper()
{
	SDL_Quit();
}

bool SDLLibraryHelper::InitLibrary()
{
	m_bLibraryStarted = (0 == SDL_Init(SDL_INIT_VIDEO));
	return m_bLibraryStarted;
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
	prog.CreateWindow(768, 768);
	prog.Run();

	SDL_Quit();
	return 0;
}
