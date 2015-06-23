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

//Rendered texture
extern LTexture gTextTexture1;
extern LTexture gTextTexture2;
extern LTexture gTextTexture3;

//The window renderer
extern SDL_Renderer* gRenderer;

//Globally used font
TTF_Font *gFont;

//The window we'll be rendering to
SDL_Window* gWindow;

LTexture::LTexture() {
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture() {
	//Deallocate
	free();
}

bool LTexture::loadFromRenderedText(const std::string textureText, const SDL_Color textColor) {
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Blended(gFont, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
	}
	else
	{
		//Create texture from surface pixels
		mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
		if (mTexture == NULL)
		{
			printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface(textSurface);
	}

	//Return success
	return mTexture != NULL;
}

void LTexture::free() {
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(const int x, const int y, const SDL_Rect* clip, const float angle, const SDL_Point* center, const SDL_RendererFlip flip) {
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL) {
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth() {
	return mWidth;
}

int LTexture::getHeight() {
	return mHeight;
}

bool loadMedia() {
	//Open the font
	gFont = TTF_OpenFont("FreeSansBold.ttf", FONT_SIZE);
	if (gFont == NULL)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	//Render text
	if (!gTextTexture1.loadFromRenderedText("Boids: " + std::to_string(NUMBER_OF_BOIDS), textColor)) {
		printf("Failed to render text texture!\n");
		return false;
	}

	if (!gTextTexture2.loadFromRenderedText("Threads: " + std::to_string(numCPU), textColor)) {
		printf("Failed to render text texture!\n");
		return false;
	}

	return true;
}

bool initSDL() {

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Linear texture filtering not enabled.");

	//Get size
	SDL_DisplayMode currentMode;
	SDL_GetCurrentDisplayMode(0, &currentMode);
	width = currentMode.w;
	height = currentMode.h;
	fWidth = (float)width;
	fHeight = (float)height;

	//Create window
	gWindow = SDL_CreateWindow("Boids", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, sdlFlags);
	if (gWindow == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Initialize SDL_ttf
	if (TTF_Init() == -1) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	//SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_SetWindowIcon(gWindow, loadSurface("boid.bmp"));

	return true;
}

void saveScreenshotBMP(const std::string filepath) {
	SDL_Surface *sshot = SDL_CreateRGBSurface(0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(gRenderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	SDL_SaveBMP(sshot, filepath.c_str());
	SDL_FreeSurface(sshot);
}

void close()
{
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

SDL_Surface* loadSurface(const std::string path) {

	//Load image at specified path
	SDL_Surface* loadedSurface = SDL_LoadBMP(path.c_str());
	if (loadedSurface == NULL)
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s. SDL Error: %s\n", path.c_str(), SDL_GetError());

	return loadedSurface;
}