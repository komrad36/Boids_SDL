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

unsigned int curTicks;

unsigned int fpsFrameCount = 0;
unsigned int fpsTimeCount = 0;

//Screen dimension constants
int width, height;
float fWidth, fHeight;

float time_since_last_frame;
int mousexpos, mouseypos;

int mouse_buttons_down = 0;
float repulsion_multiplier = 1.0;
bool repulsion_boost = false;

unsigned int numCPU;

#ifdef DYNAMIC_COLOR_MODE
RGB colors[NUMBER_OF_BOIDS];
#endif

int drawingPoints[4 * NUMBER_OF_BOIDS];

//Rendered texture
LTexture gTextTexture1;
LTexture gTextTexture2;
LTexture gTextTexture3;

//The window renderer
SDL_Renderer* gRenderer;

bool notPaused = true;

const std::string currentDateTime() {
	// get date and time to second accuracy
	time_t t = time(nullptr);
	tm *now = localtime(&t);

	// grab high accuracy current time for ms
	std::chrono::system_clock::time_point p = std::chrono::system_clock::now();

	// get fractional seconds
	size_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(p.time_since_epoch()).count() % MS_PER_SECOND;

	std::stringstream out;
	out << (now->tm_year + START_YEAR) << '-' << (now->tm_mon + START_MONTH) << '-' << now->tm_mday << ' ' << now->tm_hour << '.' << now->tm_min << '.' << now->tm_sec << '.' << ms;
	return out.str();
}

int main(int argc, char* argv[]) {

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// silence compiler
	argc; argv;

	if (!initSDL())
		return EXIT_FAILURE;

	initThreads();

	spawnBoids();

	if (!loadMedia())
		return EXIT_FAILURE;

	bool spaceUp = true, continueRunning = true;

	//Event handler
	SDL_Event e;

	unsigned int totalTime, frameRate, deltaTime;

	//While application is running
	while (continueRunning) {

		//Handle events on queue
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					continueRunning = false;
					break;
				case SDLK_SPACE:
					spaceUp = !spaceUp;
					break;
				case SDLK_LCTRL:
					repulsion_multiplier = -repulsion_multiplier;
					break;
				case SDLK_LSHIFT:
					repulsion_boost = !repulsion_boost;
					break;
				case SDLK_PRINTSCREEN:
					saveScreenshotBMP(currentDateTime() + ".bmp");
					break;
				case SDLK_c:
					notPaused = !notPaused;
					break;
				case SDLK_r:
					spawnBoids();
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				++mouse_buttons_down;
				break;
			case SDL_MOUSEBUTTONUP:
				--mouse_buttons_down;
			}
		}

		SDL_GetMouseState(&mousexpos, &mouseypos);
		totalTime = SDL_GetTicks();
		frameRate;
		deltaTime = (totalTime - fpsTimeCount);
		if (deltaTime > FPS_UPDATE_MS) {
			frameRate = MS_PER_SECOND * fpsFrameCount / deltaTime;
			fpsTimeCount = totalTime;
			fpsFrameCount = 0;
			gTextTexture3.loadFromRenderedText("FPS: " + std::to_string(frameRate), textColor);
		}
		time_since_last_frame = (float) (totalTime - curTicks);
		curTicks = totalTime;

		if (notPaused)
			process_rules();

		//Clear screen
		if (spaceUp) {
			SDL_SetRenderDrawColor(gRenderer, BLANKING_COLOR, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(gRenderer);
		}

		++fpsFrameCount;

		//Draw lines
#ifndef DYNAMIC_COLOR_MODE
		SDL_SetRenderDrawColor(gRenderer, BOID_COLOR_IF_NOT_DYNAMIC_MODE, SDL_ALPHA_OPAQUE);
#endif
		for (unsigned int i = 0; i < NUMBER_OF_BOIDS; ++i) {
#ifdef DYNAMIC_COLOR_MODE
			SDL_SetRenderDrawColor(gRenderer, colors[i].R, colors[i].G, colors[i].B, SDL_ALPHA_OPAQUE);
#endif
			SDL_RenderDrawLine(gRenderer, drawingPoints[4 * i], drawingPoints[4 * i + 1], drawingPoints[4 * i + 2], drawingPoints[4 * i + 3]);
		}

		// draw text
		gTextTexture1.render(TEXT_DISPLACEMENT, TEXT_DISPLACEMENT);
		gTextTexture2.render(TEXT_DISPLACEMENT, TEXT_LINE_HEIGHT + TEXT_DISPLACEMENT);
		gTextTexture3.render(TEXT_DISPLACEMENT, 2 * TEXT_LINE_HEIGHT + TEXT_DISPLACEMENT);

		// Flip buffer
		SDL_RenderPresent(gRenderer);
	}

	deInitBoids();

	close();

	return EXIT_SUCCESS;
}