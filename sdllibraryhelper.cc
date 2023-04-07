#include "sdllibraryhelper.h"
#include <SDL.h>
#include <SDL_version.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_error.h>

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
