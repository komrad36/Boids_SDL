/*******************************************************************
*   mySDL.cpp
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// This module contains SDL interfacing for graphics.

#include "mySDL.h"

bool LTexture::loadFromRenderedText(TTF_Font* font, SDL_Renderer* renderer, const std::string& textureText, const SDL_Color text_color) {
	free();

	SDL_Surface* text_sfc = TTF_RenderText_Blended(font, textureText.c_str(), text_color);
	if (!text_sfc) {
		std::cerr << "ERROR: unable to render text surface! SDL_ttf Error: " << TTF_GetError() << ". Aborting." << std::endl;
		return false;
	}

	if (!(mTexture = SDL_CreateTextureFromSurface(renderer, text_sfc))) {
		std::cerr << "ERROR: unable to create texture from rendered text! SDL Error: " << SDL_GetError() << ". Aborting." << std::endl;
		return false;
	}

	width = text_sfc->w;
	height = text_sfc->h;

	SDL_FreeSurface(text_sfc);

	return true;
}

void LTexture::free() {
	if (mTexture) {
		SDL_DestroyTexture(mTexture);
		mTexture = nullptr;
		width = height = 0;
	}
}

void LTexture::render(SDL_Renderer* renderer, const int x, const int y, const SDL_Rect* const clip, const float angle, const SDL_Point* const center, const SDL_RendererFlip flip) {
	// set render space
	SDL_Rect renderQuad = { x, y, width, height };

	// set clip rendering dimensions
	if (clip) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	// render to screen
	SDL_RenderCopyEx(renderer, mTexture, clip, &renderQuad, angle, center, flip);
}

bool mySDL::loadFonts(const int num_CPU) {
	font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
	if (!font) {
		std::cerr << "ERROR: Failed to load font! SDL_ttf Error: " << TTF_GetError() << ". Aborting." << std::endl;
		return false;
	}

	if (!text_texture1.loadFromRenderedText(font, renderer, "Boids: " + std::to_string(NUMBER_OF_BOIDS), TEXT_COLOR)) {
		std::cerr << "ERROR: Failed to render text texture! Aborting." << std::endl;
		return false;
	}

	if (!text_texture2.loadFromRenderedText(font, renderer, "Threads: " + std::to_string(num_CPU), TEXT_COLOR)) {
		std::cerr << "Failed to render text texture! Aborting." << std::endl;
		return false;
	}

	return true;
}

bool mySDL::initSDL(float& fWidth, float& fHeight) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Failed to initialize SDL! SDL Error: " << SDL_GetError() << ". Aborting." << std::endl;
		return false;
	}

	// set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		std::cout << "WARN: Linear texture filtering not enabled." << std::endl;

	// get size
	SDL_DisplayMode currentMode;
	SDL_GetCurrentDisplayMode(0, &currentMode);
	width = currentMode.w;
	height = currentMode.h;
	fWidth = static_cast<float>(width);
	fHeight = static_cast<float>(height);

	// create window
	if (!(window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_FLAGS))) {
		std::cerr << "ERROR: Window could not be created! SDL Error: " << SDL_GetError() << ". Aborting." << std::endl;
		return false;
	}

	// create renderer
	if (!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))) {
		std::cerr << "ERROR: Renderer could not be created! SDL Error: " << SDL_GetError() << ". Aborting." << std::endl;
		return false;
	}

	// init font engine
	if (TTF_Init() == -1) {
		std::cerr << "ERROR: SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << ". Aborting." << std::endl;
		return false;
	}

	// (try to) set icon
	SDL_SetWindowIcon(window, loadSurface(ICON_FILE));

	return true;
}

void mySDL::saveScreenshotBMP(const std::string& file_path) {
	SDL_Surface* sshot = SDL_CreateRGBSurface(0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	SDL_SaveBMP(sshot, file_path.c_str());
	SDL_FreeSurface(sshot);
}

mySDL::~mySDL() {
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;

	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}

SDL_Surface* loadSurface(const std::string& path) {
	SDL_Surface* sfc = SDL_LoadBMP(path.c_str());
	if (!sfc) {
		std::cerr << "ERROR: unable to load image " << path << ". SDL Error: " << SDL_GetError() << '.' << std::endl;
	}

	return sfc;
}