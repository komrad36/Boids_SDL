/*******************************************************************
*   params.h
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// This module contains user-configurable and other defines.

#ifndef PARAMS_H
#define PARAMS_H

#include <SDL.h>

//################ User-configurable Parameters ################

#define FULL_SCREEN

#define DYNAMIC_COLOR_MODE

#define USE_CRAZY_STUPID_FAST_SQRT_APPROX

// Integer defines
#define		NUMBER_OF_BOIDS							(4500)
#define		LINE_LENGTH								(7)
#define		FPS_UPDATE_MS							(100)
#define		FONT_SIZE								(14)
#define		TEXT_DISPLACEMENT						(3)
#define		TEXT_LINE_HEIGHT						(18)
#define		ALIGNMENT_STRENGTH_DIVISOR				(45)
//#define	OVERRIDE_CPU_COUNT_AUTODETECT			(1)

// Float defines
#define		V_LIM									(300.0f)
#define		TICK_DIVISOR							(180.0f)
#define		REPULSION_STRENGTH_FACTOR				(190.0f)
#define		CENTER_OF_MASS_STRENGTH_DIVISOR			(9.0f)
#define		WEAK_MOUSE_DOWN_STRENGTH_FACTOR			(150.0f)
#define		STRONG_DOWN_STRENGTH_FACTOR				(9000.0f)

// Color defines
#define		BLANKING_COLOR							0, 0, 0
#define		BOID_COLOR_IF_NOT_DYNAMIC_MODE			0, 0, 255
const SDL_Color textColor = { 255, 0, 0 };

//##############################################################




#ifdef FULL_SCREEN
const int sdlFlags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN;
#else
const int sdlFlags = SDL_WINDOW_SHOWN;
#endif

#ifdef USE_CRAZY_STUPID_FAST_SQRT_APPROX
#ifndef sqrt
#define sqrt crazyStupidFastSqrtApprox
#endif
#endif

#define MS_PER_SECOND	(1000)
#define START_YEAR		(1900)
#define START_MONTH		(1)

// Square coordinate system max value (for screen independence)
#define P_MAX			(10000)
#define HALF_fP_MAX		(5000.0f)
#define fP_MAX			(10000.0f)

// Max distance to be considered a neighbor
#define NEIGHBOR_DISTANCE (P_MAX / 12)
#define NEIGHBOR_DISTANCE_SQUARED (NEIGHBOR_DISTANCE * NEIGHBOR_DISTANCE)

#define	V_LIM_2 (V_LIM * V_LIM)

#define PREVENT_ZERO_RETURN (0.0001f)

#endif