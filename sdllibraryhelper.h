#ifndef SDLLIBRARYHELPER_H_
#define SDLLIBRARYHELPER_H_
#include <SDL_version.h>

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

#endif /* SDLLIBRARYHELPER_H_ */
