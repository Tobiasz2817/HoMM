#include <stdio.h>
#include <string.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#define CELLS_X 15
#define CELLS_Y 11

#define CELL_SIZE 100

int WINDOW_WIDTH = 15 * CELL_SIZE;
int WINDOW_HEIGHT = 11 * CELL_SIZE;

struct Board
{
	SDL_Texture* defaultTexture;
	SDL_Texture* obstacleTexture;

	unsigned char cells[CELLS_Y][CELLS_X];
};
typedef struct Board Board;

void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);
void SetRect(SDL_Rect* rect, int x, int y, int w, int h);
void CreateBoard(Board* board);

int main()
{
	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return -1;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return -1;
	}
	// Creating the window 1920x1080 (could be any other size)
	SDL_Window* window = SDL_CreateWindow("FirstSDL",
		0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN);

	if (!window)
		return -1;

	// Creating a renderer which will draw things on the screen
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		return -1;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(renderer, 128, 69, 69, 255);


	// Here the surface is the information about the image. It contains the color data, width, height and other info.
	SDL_Surface* surface = IMG_Load("stickXd.png");
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", "stickXd.png", IMG_GetError());
		return -1;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* playerTexture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!playerTexture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return -1;
	}
	// Bye-bye the surface
	SDL_FreeSurface(surface);
	// Here the surface is the information about the image. It contains the color data, width, height and other info.
	surface = IMG_Load("default.png");
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", "default.png", IMG_GetError());
		return -1;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* defaultTexture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!defaultTexture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return -1;
	}

	// Bye-bye the surface
	SDL_FreeSurface(surface);
	// Here the surface is the information about the image. It contains the color data, width, height and other info.
	surface = IMG_Load("obstacle.png");
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", "default.png", IMG_GetError());
		return -1;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* obstacleTexture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!obstacleTexture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return -1;
	}

	Board board;
	board.defaultTexture = defaultTexture;
	board.obstacleTexture = obstacleTexture;

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = CELL_SIZE;
	int tex_height = CELL_SIZE;

	int destinyX = tex_height;
	int destinyY = tex_width;

	int cellDestinyX = 0;
	int cellDestinyY = 0;

	int maxDestinyReached = 2;

	float x = (float)WINDOW_WIDTH / 2.f;
	float y = (float)WINDOW_WIDTH / 2.f;
	float speed = 200.f;

	// Bye-bye the surface
	SDL_FreeSurface(surface);


	SDL_Event sdl_event;
	SDL_Rect rect;

	float deltaTime = 0.f;
	float lastTick = 0.f;

	CreateBoard(&board);

	bool done = false;
	// The main loop
	// Every iteration is a frame
	while (!done)
	{

		float currentTick = (float)SDL_GetTicks() / 1000.f;
		deltaTime = currentTick - lastTick;
		lastTick = currentTick;

		// Polling the messages from the OS.\
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdl_event))
		{
			if (sdl_event.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdl_event.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdl_event.key.keysym.sym) // Which key?
				{
					case SDLK_ESCAPE: // Posting a quit message to the OS queue so it gets processed on the next step and closes the game
						SDL_Event event;
						event.type = SDL_QUIT;
						event.quit.type = SDL_QUIT;
						event.quit.timestamp = SDL_GetTicks();
						SDL_PushEvent(&event);
						break;
					default:
						break;
				}
			}
			else if (sdl_event.type == SDL_MOUSEBUTTONDOWN) // A key was pressed
			{
				switch (sdl_event.button.button) // Which key?
				{
					case SDL_BUTTON_LEFT: // Posting a quit message to the OS queue so it gets processed on the next step and closes the game
						SDL_GetMouseState(&destinyX, &destinyY);
						cellDestinyX = destinyX / CELL_SIZE;
						cellDestinyY = destinyY / CELL_SIZE;

						/*printf("x: %i ", cellDestinyX);
						printf("y: %i ", cellDestinyY);
						printf("x: %i ", destinyX);
						printf("y: %i ", destinyY);*/

						destinyX = ((destinyX / CELL_SIZE) * CELL_SIZE) + CELL_SIZE/2;
						destinyY = ((destinyY / CELL_SIZE) * CELL_SIZE) + CELL_SIZE/2;

							break;
					default:
						break;
				}
			}
		}

		// Print on screen
		SDL_RenderClear(renderer);

		for (int i = 0; i < CELLS_Y; ++i)
		{
			for (int j = 0; j < CELLS_X; ++j)
			{
				SetRect(&rect, j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE - 2, CELL_SIZE - 2);
				DrawImage(renderer, board.cells[i][j] == 255 ? board.obstacleTexture : board.defaultTexture ,rect);
			}
		}

		SetRect(&rect,(int)round(x - tex_width / 2), (int)round(y - tex_height / 2), (int)tex_width, (int)tex_height);
		DrawImage(renderer,playerTexture,rect);

		SDL_RenderPresent(renderer);



		if (fabs(x - destinyX) >= maxDestinyReached) {
			if (x > destinyX) {
				x -= speed * deltaTime;
			}
			else {
				x += speed * deltaTime;
			}
		}
		if (fabs(y - destinyY) >= maxDestinyReached) {
			if (y > destinyY) {
				y -= speed * deltaTime;
			}
			else {
				y += speed * deltaTime;
			}
		}
	}

	// If we reached here then the main loop stoped
	// That means the game wants to quit

	// Shutting down the renderer
	SDL_DestroyRenderer(renderer);

	// Shutting down the window
	SDL_DestroyWindow(window);

	// Quitting the Image SDL library
	IMG_Quit();
	// Quitting the main SDL library
	SDL_Quit();

	// Done.
	return 0;
}
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture,SDL_Rect rect)
{
	SDL_RenderCopyEx(renderer, // Already know what is that
		texture, // The image
		0, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
		&rect, // The destination rectangle on the screen.
		0, // An angle in degrees for rotation
		0, // The center of the rotation (when nullptr, the rect center is taken)
		SDL_FLIP_NONE); // We don't want to flip the image
}
void CreateBoard(Board* board)
{
	memset(board->cells, 0, sizeof(board->cells));

	board->cells[2][5] = 255;
	board->cells[6][9] = 255;
}
void SetRect(SDL_Rect* rect,int x,int y, int w,int h)
{
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}
