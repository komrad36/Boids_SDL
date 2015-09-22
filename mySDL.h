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

#include <iostream>
#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

#include "Boids.h"
#include "params.h"

class LTexture {
public:
	LTexture() : mTexture(nullptr) {}

	~LTexture() { free(); }

	// creates texture from text string
	bool LTexture::loadFromRenderedText(TTF_Font* font, SDL_Renderer* renderer, const std::string& textureText, const SDL_Color text_color);

	// deallocates hardware texture
	void free();

	// renders texture at (x, y)
	void LTexture::render(SDL_Renderer* renderer, const int x, const int y, const SDL_Rect* const clip = nullptr, const float angle = 0.0f, const SDL_Point* const center = nullptr, const SDL_RendererFlip flip = SDL_FLIP_NONE);

	int width, height;

private:
	// actual hardware texture
	SDL_Texture* mTexture;

};

class mySDL {
public:
	LTexture text_texture1;
	LTexture text_texture2;
	LTexture text_texture3;

	SDL_Renderer* renderer;

	TTF_Font* font;

private:

	SDL_Window* window;

	int width, height;

public:
	// Singleton idiom - only one
	// instance of this class is permitted
	// since we need static methods as callbacks
	// but still want class functionality
	static mySDL& getInstance() {
		static mySDL instance;
		return instance;
	};

	~mySDL();

	bool loadFonts(const int num_CPU);

	// start up SDL and creates window
	bool initSDL(float& fWidth, float& fHeight);

	// save screenshot as BMP to file_path
	// with single press of PRT_SC so
	// user doesn't have to ALT-TAB out or close program
	// to save each shot
	void saveScreenshotBMP(const std::string& file_path);

private:
	mySDL() : renderer(nullptr), window(nullptr), font(nullptr) {}

	// NO copy construction or copy assignment. This is a singleton.
	mySDL(const mySDL&) = delete;
	mySDL& operator=(const mySDL&) = delete;
};

// load individual image as texture
SDL_Surface* loadSurface(const std::string& path);

#endif