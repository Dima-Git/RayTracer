#include <iostream>
#include <chrono>
#include "AppSystem.h"
#include "Vec3.h"
#include "Body.h"

std::chrono::high_resolution_clock::time_point start_time_point;

Uint32 framerate_event;
Uint32 calculated_event;
Uint32 thread_done_event;

SDL_atomic_t process_threads;

//TODO: generalize synchronization stuff
SDL_mutex * pixel_queue_mutex;
volatile int pixel_queue_current = 0;
int AppSystem::pixel_queue_next() {
	int pqc = -1;
	SDL_LockMutex(pixel_queue_mutex);
	if (pixel_queue_current < WINDOW_HEIGHT * WINDOW_WIDTH) {
		pqc = pixel_queue_current++;
	}
	SDL_UnlockMutex(pixel_queue_mutex);
	return pqc;
}

Uint32 framerate_timer_callback(Uint32 interval, void * param)
{
	SDL_Event e;
	SDL_zero(e);
	e.type = framerate_event;
	if (SDL_PushEvent(&e) < 0) {
		// Event queue is full, wait until we can push event
		SDL_Delay(interval);
	}
	return interval;
}

AppSystem::AppSystem(const Configurer & config)
	: render(config)
{
	this->WINDOW_TITLE = config.window_title;
	this->WINDOW_WIDTH = config.window_width;
	this->WINDOW_HEIGHT = config.window_height;
	this->FRAMERATE = config.framerate;
	this->THREADS_COUNT = config.threads_count;
}

AppSystem::InitPhase AppSystem::init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		std::cerr << "SDL_Init() error : " << SDL_GetError() << std::endl;
		return InitPhase::FAIL_INIT_SDL;
	}

	window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow error() : " << SDL_GetError() << std::endl;
		return InitPhase::FAIL_CREATE_WINDOW;
	}

	surface = SDL_GetWindowSurface(window);
	if (surface == nullptr) {
		std::cerr << "SDL_GetWindowSurface() error : " << SDL_GetError() << std::endl;
		return InitPhase::FAIL_GET_SURFACE;
	}
	SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0x80, 0x80, 0x80));

	framerate_event = SDL_RegisterEvents(3);
	if (framerate_event == (Uint32)-1) {
		std::cerr << "SDL_RegisterEvents() error : " << SDL_GetError() << std::endl;
		return InitPhase::FAIL_REGISTER_EVENTS;
	}
	calculated_event = framerate_event + 1;
	thread_done_event = calculated_event + 1;

	framerate_timer = SDL_AddTimer(1000 / FRAMERATE, framerate_timer_callback, nullptr);
	if (framerate_timer == 0) {
		std::cerr << "SDL_AddTimer() error : " << SDL_GetError() << std::endl;
		return InitPhase::FAIL_ADD_TIMER;
	}

	pixel_queue_mutex = SDL_CreateMutex();
	if (pixel_queue_mutex == nullptr) {
		std::cerr << "SDL_CreateMutex() error : " << SDL_GetError() << std::endl;
		return InitPhase::FAIL_CREATE_MUTEX;
	}

	start_time_point = std::chrono::high_resolution_clock::now();

	threads = new SDL_Thread *[THREADS_COUNT];
	SDL_AtomicSet(&process_threads, 1);
	for (int i = 0; i < THREADS_COUNT; ++i) {
		std::cout << "Creating thread " << i << "... " << std::flush;
		threads[i] = SDL_CreateThread(calculation_thread_function_wrapper, nullptr, this);
		if (threads[i] == nullptr) {
			std::cerr << "SDL_CreateThread() [" << i << "] error : " << SDL_GetError() << std::endl;
		}
		std::cout << "Done!" << std::endl;
	}

	return InitPhase::SUCCESS;
}

void AppSystem::cleanup(InitPhase init_result)
{
	switch (init_result) {
	default:
	case InitPhase::SUCCESS:
		delete[] threads;
		std::cout << "Destroying mutex..." << std::endl;
		SDL_DestroyMutex(pixel_queue_mutex);
	case InitPhase::FAIL_CREATE_MUTEX:
		std::cout << "Removing timer..." << std::endl;
		SDL_RemoveTimer(framerate_timer);
	case InitPhase::FAIL_ADD_TIMER:
	case InitPhase::FAIL_REGISTER_EVENTS:
	case InitPhase::FAIL_GET_SURFACE:
		std::cout << "Destroying window..." << std::endl;
		SDL_DestroyWindow(window);
	case InitPhase::FAIL_CREATE_WINDOW:
	case InitPhase::FAIL_INIT_SDL:
		std::cout << "SDL Quit..." << std::endl;
		SDL_Quit();
		std::cout << "Done!" << std::endl;
	}
}

int AppSystem::calculation_thread_function_wrapper(void * data)
{
	AppSystem * self = (AppSystem *)data;
	return self->calculation_thread_function(nullptr);
}

int AppSystem::calculation_thread_function(void * data)
{
	int px;
	while ((px = pixel_queue_next()) != -1 && SDL_AtomicGet(&process_threads)) {

		int x = px % WINDOW_WIDTH;
		int y = px / WINDOW_WIDTH;

		Vec3 col = render.pixel_color(x, y);
		
		Uint32 clr = SDL_MapRGB(surface->format, 
			static_cast<Uint8>(col.r * 0xFF), 
			static_cast<Uint8>(col.g * 0xFF), 
			static_cast<Uint8>(col.b * 0xFF));

		SDL_UserEvent ue = {};
		ue.type = calculated_event;
		ue.data1 = (void*)((long long)px);
		ue.data2 = (void*)((long long)clr);
		SDL_Event e = {};
		e.type = calculated_event;
		e.user = ue;
		
		while (SDL_AtomicGet(&process_threads) && SDL_PushEvent(&e) < 0) {
			SDL_Delay(10);
		};
	}

	SDL_UserEvent ue = {};
	ue.type = thread_done_event;
	SDL_Event e = {};
	e.user = ue;

	while (SDL_AtomicGet(&process_threads) && SDL_PushEvent(&e) < 0) {
		SDL_Delay(10);
	};

	return 0;
}

void AppSystem::loop()
{
	int threads_done = 0;
	bool processing = true;
	SDL_Event e;

	while (processing) {

		if (SDL_WaitEvent(&e) == 0) {
			std::cerr << "SDL_WaitEvent() error : " << SDL_GetError() << std::endl;
			break;
		}
		else if (e.type == SDL_QUIT) {
			// Quit program
			processing = false;
			SDL_AtomicSet(&process_threads, 0);
			for (int i = 0; i < THREADS_COUNT; ++i) {
				std::cout << "Waiting " << i << "... " << std::flush;
				int ecode = 0;
				//TODO: SDL_WaitThread() crashes sometimes, why?
				SDL_WaitThread(threads[i], &ecode);
				// //SDL_DetachThread(threads[i]);
				std::cout << "Done! Code : " << ecode << std::endl;
			}
		}
		else if (e.type == framerate_event) {
			// Refresh frame
			if (SDL_UpdateWindowSurface(window) != 0) {
				std::cerr << "SDL_UpdateWindowSurface() error : " << SDL_GetError() << std::endl;
			}
		}
		else if (e.type == calculated_event) {
			// Add calculated pixel to a framebuffer
			int i = static_cast<int>((long long)e.user.data1);
			Uint32 clr = static_cast<Uint32>((long long)e.user.data2);
			SDL_LockSurface(surface);
			((Uint32 *)surface->pixels)[i] = clr;
			SDL_UnlockSurface(surface);
		}
		else if (e.type == thread_done_event) {
			threads_done++;
			if (threads_done == THREADS_COUNT) {
				std::cout << "Threads are done!" << std::endl;
				long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time_point).count();
				std::cout << "Render time : " << ms << " ms." << std::endl;
			}
		}

	}
}