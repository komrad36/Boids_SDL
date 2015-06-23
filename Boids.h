/*******************************************************************
*   Boids.cpp
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// This module contains the code for the Boid math and rendering.

#ifndef BOIDS_H
#define BOIDS_H

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

float fastdiff(const float, const float);
float diff(const float, const float);

float crazyStupidFastSqrtApprox(const float);

void initThreads();

void spawnBoids();

void deInitBoids();

void process_rules();

struct RGB {
	uint8_t R, G, B;
};

RGB angleToRGB(float angle);

extern int width, height;
extern float fWidth, fHeight;

void launchThread(unsigned int, unsigned int);

extern float time_since_last_frame;
extern int mousexpos, mouseypos;

extern int mouse_buttons_down;
extern float repulsion_multiplier;
extern bool repulsion_boost;

extern unsigned int numCPU;

#ifdef DYNAMIC_COLOR_MODE
extern RGB colors[NUMBER_OF_BOIDS];
#endif

extern int drawingPoints[4 * NUMBER_OF_BOIDS];

#endif