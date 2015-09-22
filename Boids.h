/*******************************************************************
*   Boids.h
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// Boids simulates bird ("boid"), fish, crowd, etc. flocking behavior,
// resulting in emergent properties. The main focus of the project is
// showcasing high performance computing, so I refer you to the version
// titled Boids_CUDA_GL, which, as the name implies, uses CUDA for
// computation and then leverages CUDA-OpenGL interoperability to
// write output directly to an OpenGL texture for rendering, with
// no memory transfers or CPU involvement whatsoever. This causes
// epic performance.
//
// This is the multithreaded CPU version. As this is a computationally
// intensive problem with naive neighbor search, this will be slow
// at more than a few thousand birds.
//
// The simulation can be sped up using a k-d tree or neighborhood
// matrix, solutions I implement in the CUDA version. Again, this
// CPU version is a concept demo for machines without nVIDIA graphics.
//
// This simulation
// is primarily tuned for aesthetics, not physical accuracy, although
// careful selection of parameters can produce very flock-like emergent
// behavior. The color of each boid is determined by its direction of travel.
// Screen-wrapping and fullscreen are available as options in params.h as
// well as a variety of simulation parameters.
//
// Requires SDL and SDL_ttf.
//
// Commands:
//	Space			-	toggle screen blanking
//	P				-	pause
//	R				-	randomize boid positions
//	PRINT_SCREEN	-	save screenshot to <current time>.bmp so you don't have to quit or ALT-TAB to do so
//	LCTRL			-	switch between mouse attraction and repulsion
//	LSHIFT			-	toggle STRONG attraction/repulsion
//	ESC				-	quit
//	Hold mouse btn	-	enable attraction/repulsion to mouse

#ifndef BOIDS_H
#define BOIDS_H

// in debug mode, set up infrastructure
// for memory leak checking
#ifdef _DEBUG

#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif // DBG_NEW

#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _CRTDBG_MAP_ALLOC

#endif // _DEBUG

#include <algorithm>
#include <mutex>
#include <random>
#include <thread>

#include "params.h"

#ifdef DYNAMIC_COLOR_MODE
struct RGB {
	uint8_t R, G, B;

	RGB() {}
	RGB(const uint8_t R, const uint8_t G, const uint8_t B) : R(R), G(G), B(B) {}
};
#endif

struct Boid {
	float x, y, vx, vy;

#ifdef DYNAMIC_COLOR_MODE
	RGB color;
#endif

	int draw_x1, draw_y1, draw_x2, draw_y2;
};

class Physics {
public:
	int last_total_time;

	float time_since_last_frame;
	int mouse_x, mouse_y;

	int mouse_buttons_down;
	float repulsion_multiplier;
	bool repulsion_boost;

	int num_CPU;

	bool not_paused = true;

	float fWidth, fHeight;

	// current boid to process, for threading
	int cur_idx = 0;

	// for blocking cur_idx
	std::mutex mtx;

	Boid in_arr[NUMBER_OF_BOIDS];
	Boid out_arr[NUMBER_OF_BOIDS];

	Boid* in;
	Boid* out;

	std::thread* threads;

private:


public:
	// Singleton idiom - only one
	// instance of this class is permitted
	// since we need static methods as callbacks
	// but still want class functionality
	static Physics& getInstance() {
		static Physics instance;
		return instance;
	};

	~Physics() { delete[] threads; }

	void initThreads();

	void spawnBoids();

	void processRules();

private:
	Physics() : mouse_buttons_down(0), repulsion_boost(false), repulsion_multiplier(1.0f), threads(nullptr), in(in_arr), out(out_arr), mouse_x(0.0f), mouse_y(0.0f) {}

	// NO copy construction or copy assignment. This is a singleton.
	Physics(const Physics&) = delete;
	Physics& operator=(const Physics&) = delete;

	void launchThread(int start_idx, int end_idx);

};

#ifdef SCREEN_WRAP
// return shortest distance between coordinates c1 to c2,
// screenwrapping if necessary, *independent* of direction
// for speed (for uses that only require magnitude)
float fastdiff(const float c1, const float c2);

// shortcut to add sign info to fastdiff to produce equivalent
// results as diff
float fastdiffToDiff(const float fast_diff, const float c1, const float c2);

// return shortest distance between coordinates c1 to c2,
// screenwrapping if necessary and preserving direction
float diff(const float c1, const float c2);

#endif


#ifdef DYNAMIC_COLOR_MODE
// simplified HSV to RGB with S and V both 100%
RGB angleToRGB(const float angle);
#endif

#endif