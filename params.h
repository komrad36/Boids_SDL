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

// Recommended for performance and aesthetics
#define FULL_SCREEN

#define SCREEN_WRAP

#define DYNAMIC_COLOR_MODE

// Integer defines
#define		NUMBER_OF_BOIDS							(3500)
#define		LINE_LENGTH								(7)
#define		FPS_UPDATE_MS							(100)
#define		FONT_SIZE								(14)
#define		TEXT_DISPLACEMENT						(3)
#define		TEXT_LINE_HEIGHT						(18)
//#define	OVERRIDE_CPU_COUNT_AUTODETECT			(1)

// Float defines
#define		V_LIM									(220.0f)
#define		TICK_FACTOR								(0.006f)
#define		ALIGNMENT_STRENGTH_FACTOR				(0.25f)
#define		REPULSION_STRENGTH_FACTOR				(210.0f)
#define		EDGE_REPULSION_STRENGTH_FACTOR			(0.0008f)
#define		CENTER_OF_MASS_STRENGTH_FACTOR			(0.2f)
#define		WEAK_MOUSE_DOWN_STRENGTH_FACTOR			(150.0f)
#define		STRONG_DOWN_STRENGTH_FACTOR				(9000.0f)

// Color defines
#define		BLANKING_COLOR							0, 0, 0
#define		BOID_COLOR_IF_NOT_DYNAMIC_MODE			0, 0, 255
#define		TEXT_COLOR								{ 255, 0, 0 }

// Other defines
#define		FONT_NAME								"FreeSansBold.ttf"
#define		ICON_FILE								"boid.bmp"
#define		WINDOW_TITLE							"Boids"

//##############################################################

#ifdef FULL_SCREEN
#define SDL_FLAGS (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN)
#else
#define SDL_FLAGS (SDL_WINDOW_SHOWN)
#endif

#define MS_PER_SECOND	(1000)
#define START_YEAR		(1900)
#define START_MONTH		(1)

// Square coordinate system for boids (for screen independence)
#define P_MAX			(10000)
#define fHALF_P_MAX		(5000.0f)
#define fP_MAX			(10000.0f)

// Max distance to be considered a neighbor
#define NEIGHBOR_DISTANCE (P_MAX / 13)
#define NEIGHBOR_DISTANCE_SQUARED (NEIGHBOR_DISTANCE * NEIGHBOR_DISTANCE)

#define	V_LIM_2 (V_LIM * V_LIM)

#define PREVENT_ZERO_RETURN (0.0000001f)

#define fPI (3.1415926535897f)
#define THREE_OVER_PI (0.95492965855137f)

// for direction-of-travel to color conversion
#define NUM_HSV_SECTORS				(6)
#define MAX_RGB						(255)
#define fMAX_RGB					(255.0f)

#endif