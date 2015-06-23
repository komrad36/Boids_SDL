/*******************************************************************
*   mySDL.h
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// This module contains SDL interfacing for graphics.

#ifndef MYSDL_H
#define MYSDL_H

#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

#include "Boids.h"
#include "params.h"

class LTexture {
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Creates image from font string
	bool loadFromRenderedText(const std::string, const SDL_Color);

	//Deallocates texture
	void free();

	//Renders texture at given point
	void render(const int x, const int y, const SDL_Rect* clip = NULL, const float angle = 0.0, const SDL_Point* center = NULL, const SDL_RendererFlip flip = SDL_FLIP_NONE);

	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

// loads TTF
bool loadMedia();

//Starts up SDL and creates window
bool initSDL();

// save BMP screenshot to filepath
void saveScreenshotBMP(const std::string filepath);

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Surface* loadSurface(const std::string path);

#endif