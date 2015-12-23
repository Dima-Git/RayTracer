#pragma once

#ifndef _APPSYSTEM_H_
#define _APPSYSTEM_H_

#include <SDL2/SDL.h>
#include "Configurer.h"
#include "Render.h"

class AppSystem {

public:

	enum class InitPhase {
		SUCCESS,
		FAIL_INIT_SDL,
		FAIL_CREATE_WINDOW,
		FAIL_GET_SURFACE,
		FAIL_REGISTER_EVENTS,
		FAIL_ADD_TIMER,
		FAIL_CREATE_MUTEX,
	};

	AppSystem(const Configurer & config);

	InitPhase init();
	void loop();
	void cleanup(InitPhase init_result);

private:

	Render render;

	int pixel_queue_next();
	static int calculation_thread_function_wrapper(void * data);
	int calculation_thread_function(void * data);

	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	int THREADS_COUNT;
	int FRAMERATE;
	std::string WINDOW_TITLE;

	SDL_Window * window;
	SDL_Surface * surface;
	SDL_TimerID framerate_timer;
	SDL_Thread ** threads;
};

#endif // _APPSYSTEM_H_