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

struct Player
{
	SDL_Texture* texture;

	int startPosX;
	int startPosY;

	// Direction
	int destinyX;
	int destinyY;
	float speed;

	// Current Cell
	int currentCellX;
	int currentCellY;
};
typedef struct Player Player;
struct Board
{
	SDL_Texture* defaultTexture;
	SDL_Texture* obstacleTexture;

	unsigned char cells[CELLS_Y][CELLS_X];
	unsigned char cellsGrassfire[CELLS_Y][CELLS_X];
};
typedef struct Board Board;

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* fileName);
void CreateBoard(Board* board);
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);
void SetRect(SDL_Rect* rect, int x, int y, int w, int h);
void SetSidesValue(Board* board, int i, int j, int B, bool& S);
void SetNextCellTempDest(Board board, int& targetCellX, int& targetCellY, int& minValue, int currentCellX, int currentCellY);
void MoveToCell(Board board, Player* player, bool& pathIsFinded, int targetCellX, int targetCellY);


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

	SDL_Surface* surface = nullptr;

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = CELL_SIZE;
	int tex_height = CELL_SIZE;

	int targetCellX = 0;
	int targetCellY = 0;
	int finishCellX = 0;
	int finishCellY = 0;

	Player player;
	player.destinyX = tex_height;
	player.destinyY = tex_width;
	player.speed = 200.f;
	player.startPosX = 4;
	player.startPosY = 3;

	player.currentCellX = 0;
	player.currentCellY = 0;

	player.texture = SetTexture(surface, renderer, "stickXd.png");

	Board board;
	board.obstacleTexture = SetTexture(surface, renderer, "obstacle.png");
	board.defaultTexture = SetTexture(surface, renderer, "default.png");


	int maxDestinyReached = 2;

	float x = (player.startPosX * CELL_SIZE) + (CELL_SIZE / 2);
	float y = (player.startPosY * CELL_SIZE) + (CELL_SIZE / 2);

	// Bye-bye the surface
	SDL_FreeSurface(surface);


	SDL_Event sdl_event;
	SDL_Rect rect;

	float deltaTime = 0.f;
	float lastTick = 0.f;

	CreateBoard(&board);

	bool pathIsFinded = false;
	bool cellAreReached = true;
	bool canMove = false;
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
				case SDL_BUTTON_LEFT:
				{
					int mousePosX, mousePosY = 0;
					SDL_GetMouseState(&mousePosX, &mousePosY);

					int targetX = mousePosX / tex_width;
					int targetY = mousePosY / tex_height;

					finishCellX = targetX;
					finishCellY = targetY;

					pathIsFinded = true;

					CreateBoard(&board);

					if (board.cells[targetY][targetX] == 255)
					{
						canMove = false;
						break;
					}

					board.cells[targetY][targetX] = 1;

					canMove = true;
					bool S = true;
					while (S)
					{
						S = false;
						memcpy(board.cellsGrassfire, board.cells, sizeof(board.cells));
						for (int i = 0; i < CELLS_Y; i++)
						{
							for (int j = 0; j < CELLS_X; j++)
							{
								int A = board.cellsGrassfire[i][j];
								if (A != 255 && A != 0)
								{
									int B = A + 1;

									if (j > 0)
									{
										SetSidesValue(&board, i, j - 1, B, S);
									}
									if (j < CELLS_X - 1)
									{
										SetSidesValue(&board, i, j + 1, B, S);
									}
									if (i > 0)
									{
										SetSidesValue(&board, i - 1, j, B, S);
									}
									if (i < CELLS_Y - 1)
									{
										SetSidesValue(&board, i + 1, j, B, S);
									}
								}
							}
						}
					}
					player.currentCellX = x / tex_width;
					player.currentCellY = y / tex_height;

					MoveToCell(board, &player, pathIsFinded, targetCellX, targetCellY);
					break;
				}
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
				SetRect(&rect, j * tex_width, i * tex_height, tex_width - 2, tex_height - 2);
				DrawImage(renderer, board.cells[i][j] == 255 ? board.obstacleTexture : board.defaultTexture, rect);
			}
		}

		SetRect(&rect, (int)round(x - tex_width / 2), (int)round(y - tex_height / 2), (int)tex_width, (int)tex_height);
		DrawImage(renderer, player.texture, rect);

		SDL_RenderPresent(renderer);

		if (canMove)
		{
			player.currentCellX = x / tex_width;
			player.currentCellY = y / tex_height;

			cellAreReached = true;
			if (fabs(x - player.destinyX) >= maxDestinyReached) {
				if (x > player.destinyX) {
					x -= player.speed * deltaTime;
				}
				else {
					x += player.speed * deltaTime;
				}

				cellAreReached = false;
			}
			if (fabs(y - player.destinyY) >= maxDestinyReached) {
				if (y > player.destinyY) {
					y -= player.speed * deltaTime;
				}
				else {
					y += player.speed * deltaTime;
				}
				cellAreReached = false;
			}

			if (cellAreReached && pathIsFinded && (finishCellX != player.currentCellX || finishCellY != player.currentCellY))
			{
				MoveToCell(board, &player, pathIsFinded, targetCellX, targetCellY);
			}
		}
	}

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
SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* fileName)
{
	surface = IMG_Load(fileName);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", fileName, IMG_GetError());
		return NULL;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!newTexture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return NULL;
	}
	SDL_FreeSurface(surface);

	return newTexture;
}
void SetRect(SDL_Rect* rect, int x, int y, int w, int h)
{
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}

void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect)
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

	board->cells[0][7] = 255;
	board->cells[1][7] = 255;
	board->cells[1][8] = 255;

	board->cells[2][8] = 255;
	board->cells[2][9] = 255;
	board->cells[3][9] = 255;

	board->cells[3][9] = 255;
	board->cells[3][10] = 255;
	board->cells[4][10] = 255;

	board->cells[4][10] = 255;
	board->cells[4][11] = 255;
	board->cells[5][11] = 255;

	board->cells[5][11] = 255;
	board->cells[5][12] = 255;
	board->cells[6][12] = 255;

	board->cells[6][12] = 255;
	board->cells[6][13] = 255;
	board->cells[7][13] = 255;
	board->cells[7][14] = 255;


	board->cells[3][1] = 255;
	board->cells[4][1] = 255;
	board->cells[5][1] = 255;
	board->cells[6][1] = 255; 
	board->cells[7][1] = 255; 
	board->cells[7][2] = 255;
	board->cells[7][3] = 255;
	board->cells[7][4] = 255;
	board->cells[7][5] = 255;
	board->cells[7][6] = 255;
	board->cells[7][7] = 255;
	board->cells[7][8] = 255;
	board->cells[7][9] = 255;
	board->cells[7][10] = 255;
	board->cells[7][11] = 255;
	board->cells[7][12] = 255;
}
void SetSidesValue(Board* board, int i, int j, int B, bool& S)
{
	int x = board->cellsGrassfire[i][j];
	if (x == 0)
	{
		board->cells[i][j] = B;
		S = true;
	}
}
void SetNextCellTempDest(Board board, int& targetCellX, int& targetCellY, int& minValue, int y, int x)
{
	int side = board.cells[y][x];

	if (side < minValue && side != 0)
	{
		minValue = side;

		targetCellX = x;
		targetCellY = y;
	}
}
void MoveToCell(Board board, Player* player, bool& pathIsFinded, int targetCellX, int targetCellY)
{
	int minValue = 255;

	int currentCellX = player->currentCellX;
	int currentCellY = player->currentCellY;

	targetCellX = currentCellX;
	targetCellY = currentCellY;

	if (currentCellX > 0)
	{
		SetNextCellTempDest(board, targetCellX, targetCellY, minValue, currentCellY, (currentCellX - 1));
	}
	if (currentCellX < CELLS_X - 1)
	{
		SetNextCellTempDest(board, targetCellX, targetCellY, minValue, currentCellY, (currentCellX + 1));
	}
	if (currentCellY > 0)
	{
		SetNextCellTempDest(board, targetCellX, targetCellY, minValue, (currentCellY - 1), currentCellX);
	}
	if (currentCellY < CELLS_Y - 1)
	{
		SetNextCellTempDest(board, targetCellX, targetCellY, minValue, (currentCellY + 1), currentCellX);
	}

	if (minValue == 255)
	{
		pathIsFinded = false;
	}

	player->destinyX = (targetCellX * CELL_SIZE) + (CELL_SIZE / 2);
	player->destinyY = (targetCellY * CELL_SIZE) + (CELL_SIZE / 2);

}