/*******************************************************************
*   main.cpp
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// This program simulates simple flocking behavior, resulting
// in emergent properties. Uses pure CPU. The color of each bird (boid) is
// determined by its direction of travel. Screen-wrapped.
// Work in progress! Plan to add a better GUI / more intuitive
// keys.

#include <chrono>
#include <SDL.h>
#include <sstream>

#include "Boids.h"
#include "mySDL.h"
#include "params.h"

const std::string currentDateTime() {
	// get current date and time to second precision
	time_t t = time(nullptr);
	tm *now = localtime(&t);

	// grab high precision current time for ms
	std::chrono::system_clock::time_point p = std::chrono::system_clock::now();

	// get fractional seconds
	int ms = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch()).count() % MS_PER_SECOND;

	std::stringstream out;
	out << now->tm_year + START_YEAR << '-' << now->tm_mon + START_MONTH << '-' << now->tm_mday << ' ' << now->tm_hour << '.' << now->tm_min << '.' << now->tm_sec << '.' << ms;
	return out.str();
}

int main(int argc, char* argv[]) {
	// flush denormals to zero on Intel
	// to prevent unexpected performance drops in FPU
	// comment out if causing compatibility issues
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

	// enable memory leak checking in debug mode
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Singleton idiom: use static instances created on
	// demand. Only one of each can ever be created and
	// they cannot be moved or copied. This way we get
	// the benefits of a class as opposed to just a struct
	// (methods, constructors, destructors, in this case,
	// no inheritance required) without worry of users
	// spawning multiple instances.
	mySDL& sdl = mySDL::getInstance();
	Physics& physics = Physics::getInstance();

	// have sdl inform the physics engine of the framebuffer dimensions
	if (!sdl.initSDL(physics.fWidth, physics.fHeight)) return EXIT_FAILURE;

	physics.initThreads();

	physics.spawnBoids();

	// inform the font engine of the CPU (==thread) count, which is
	// printed to screen
	if (!sdl.loadFonts(physics.num_CPU)) return EXIT_FAILURE;

	bool do_blank = true;
	bool continue_running = true;

	// event handler
	SDL_Event e;

	int total_time, framerate, delta_t;
	int fps_time = 0;
	int fps_frames = 0;

	while (continue_running) {
		// handle events on queue
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					continue_running = false;
					break;
				case SDLK_SPACE:
					do_blank = !do_blank;
					break;
				case SDLK_LCTRL:
					physics.repulsion_multiplier = -physics.repulsion_multiplier;
					break;
				case SDLK_LSHIFT:
					physics.repulsion_boost = !physics.repulsion_boost;
					break;
				case SDLK_PRINTSCREEN:
					sdl.saveScreenshotBMP(currentDateTime() + ".bmp");
					break;
				case SDLK_p:
					physics.not_paused = !physics.not_paused;
					break;
				case SDLK_r:
					physics.spawnBoids();
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				++physics.mouse_buttons_down;
				break;
			case SDL_MOUSEBUTTONUP:
				--physics.mouse_buttons_down;
			}
		}

		SDL_GetMouseState(&physics.mouse_x, &physics.mouse_y);

		// compute FPS since last measured
		total_time = SDL_GetTicks();
		delta_t = total_time - fps_time;
		if (delta_t > FPS_UPDATE_MS) {
			framerate = MS_PER_SECOND * fps_frames / delta_t;
			fps_time = total_time;
			fps_frames = 0;
			sdl.text_texture3.loadFromRenderedText(sdl.font, sdl.renderer, "FPS: " + std::to_string(framerate), TEXT_COLOR);
		}
		physics.time_since_last_frame = static_cast<float>(total_time - physics.last_total_time);
		physics.last_total_time = total_time;

		if (physics.not_paused) physics.processRules();

		// clear screen
		if (do_blank) {
			SDL_SetRenderDrawColor(sdl.renderer, BLANKING_COLOR, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(sdl.renderer);
		}

		++fps_frames;

		// draw lines
#ifndef DYNAMIC_COLOR_MODE
		SDL_SetRenderDrawColor(sdl.renderer, BOID_COLOR_IF_NOT_DYNAMIC_MODE, SDL_ALPHA_OPAQUE);
#endif

		for (int i = 0; i < NUMBER_OF_BOIDS; ++i) {
#ifdef DYNAMIC_COLOR_MODE
			SDL_SetRenderDrawColor(sdl.renderer, physics.out[i].color.R, physics.out[i].color.G, physics.out[i].color.B, SDL_ALPHA_OPAQUE);
#endif
			SDL_RenderDrawLine(sdl.renderer, physics.out[i].draw_x1, physics.out[i].draw_y1, physics.out[i].draw_x2, physics.out[i].draw_y2);
		}

		// draw text
		sdl.text_texture1.render(sdl.renderer, TEXT_DISPLACEMENT, TEXT_DISPLACEMENT);
		sdl.text_texture2.render(sdl.renderer, TEXT_DISPLACEMENT, TEXT_LINE_HEIGHT + TEXT_DISPLACEMENT);
		sdl.text_texture3.render(sdl.renderer, TEXT_DISPLACEMENT, 2 * TEXT_LINE_HEIGHT + TEXT_DISPLACEMENT);

		// flip buffer
		SDL_RenderPresent(sdl.renderer);

		// ping-pong buffers
		Boid* temp = physics.in;
		physics.in = physics.out;
		physics.out = temp;

	} // main loop

	return EXIT_SUCCESS;
}