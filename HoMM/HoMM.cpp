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

struct Position
{
	// Start Position player
	int startPosX;
	int startPosY;

	// Current Cell
	int currentCellX;
	int currentCellY;

	// Current Cell
	int targetCellX;
	int targetCellY;

	// Finish Cell
	int finishCellX = 0;
	int finishCellY = 0;
};
struct Player
{
	float speed;

	Position* position;
	SDL_Texture* texture;

	void Init(SDL_Texture* newTexture, int speedPlayer, int startPositionPlayerX, int startPositionPlayerY);
};
typedef struct Player Player;

void Player::Init(SDL_Texture* newTexture, int speedPlayer, int startPositionPlayerX, int startPositionPlayerY)
{
	texture = newTexture;
	speed = speedPlayer;
	position->startPosX = startPositionPlayerX;
	position->startPosY = startPositionPlayerY;

	position->targetCellX = (position->startPosX * CELL_SIZE) + (CELL_SIZE / 2);
	position->targetCellY = (position->startPosY * CELL_SIZE) + (CELL_SIZE / 2);
	position->currentCellX = 0;
	position->currentCellY = 0;
}
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
void Grassfire(Board* board);
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);
void SetRect(SDL_Rect* rect, int x, int y, int w, int h);
void MoveToCell(Board board, Player* player);


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

	Player player;

	Position playerPosition;
	player.position = &playerPosition;

	player.Init(SetTexture(surface, renderer, "stickXd.png"), 200.f, 4, 3);

	Board board;
	board.obstacleTexture = SetTexture(surface, renderer, "obstacle.png");
	board.defaultTexture = SetTexture(surface, renderer, "default.png");


	int maxDestinyReached = 2;

	float x = player.position->targetCellX;
	float y = player.position->targetCellY;



	float deltaTime = 0.f;
	float lastTick = 0.f;

	// Bye-bye the surface
	SDL_FreeSurface(surface);


	SDL_Event sdl_event;
	SDL_Rect rect;

	CreateBoard(&board);

	bool startGrassfire = false;
	bool pathIsFinded = false;
	bool canMove = false;
	bool cellAreReached = true;
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

					int cellX, cellY = 0;
					player.position->finishCellX = mousePosX / tex_width;
					player.position->finishCellY = mousePosY / tex_height;

					if (board.cells[player.position->finishCellY][player.position->finishCellX] == 255)
					{
						canMove = false;
						break;
					}

					canMove = true;
					startGrassfire = true;
					CreateBoard(&board);
					board.cells[player.position->finishCellY][player.position->finishCellX] = 1;

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
			player.position->currentCellX = x / CELL_SIZE;
			player.position->currentCellY = y / CELL_SIZE;

			cellAreReached = true;
			if (fabs(x - player.position->targetCellX) >= maxDestinyReached) {
				if (x > player.position->targetCellX) {
					x -= player.speed * deltaTime;
				}
				else {
					x += player.speed * deltaTime;
				}

				cellAreReached = false;
			}
			if (fabs(y - player.position->targetCellY) >= maxDestinyReached) {
				if (y > player.position->targetCellY) {
					y -= player.speed * deltaTime;
				}
				else {
					y += player.speed * deltaTime;
				}
				cellAreReached = false;
			}

			if (cellAreReached && startGrassfire)
			{
				Grassfire(&board);

				startGrassfire = false;
			}

			if (cellAreReached && (player.position->finishCellX != player.position->currentCellX || player.position->finishCellY != player.position->currentCellY))
			{
				MoveToCell(board, &player);
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
void MoveToCell(Board board, Player* player)
{
	int minValue = 255;
	int side = 0;

	int currentCellX = player->position->currentCellX;
	int currentCellY = player->position->currentCellY;

	int newCellX = currentCellX;
	int newCellY = currentCellY;

	if (currentCellX > 0)
	{
		side = board.cells[currentCellY][(currentCellX - 1)];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = (currentCellX - 1);
			newCellY = currentCellY;
		}
	}
	if (currentCellX < CELLS_X - 1)
	{
		side = board.cells[currentCellY][(currentCellX + 1)];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = (currentCellX + 1);
			newCellY = currentCellY;
		}
	}
	if (currentCellY > 0)
	{
		side = board.cells[(currentCellY - 1)][currentCellX];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = currentCellX;
			newCellY = (currentCellY - 1);
		}
	}
	if (currentCellY < CELLS_Y - 1)
	{
		side = board.cells[(currentCellY + 1)][currentCellX];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = currentCellX;
			newCellY = (currentCellY + 1);
		}
	}

	player->position->targetCellX = (newCellX * CELL_SIZE) + (CELL_SIZE / 2);
	player->position->targetCellY = (newCellY * CELL_SIZE) + (CELL_SIZE / 2);

}
float FindCellX(Board board, Player* player)
{
	int minValue = 255;
	int side = 0;

	int currentCellX = player->position->currentCellX;
	int currentCellY = player->position->currentCellY;

	int newCellX = currentCellX;
	int newCellY = currentCellY;

	if (currentCellY > 0)
	{
		side = board.cells[(currentCellY - 1)][currentCellX];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = currentCellX;
			newCellY = (currentCellY - 1);
		}
	}
	if (currentCellY < CELLS_Y - 1)
	{
		side = board.cells[(currentCellY + 1)][currentCellX];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = currentCellX;
			newCellY = (currentCellY + 1);
		}
	}

	return (newCellX * CELL_SIZE) + (CELL_SIZE / 2);

}
float FindCellY(Board board, Player* player)
{
	int minValue = 255;
	int side = 0;

	int currentCellX = player->position->currentCellX;
	int currentCellY = player->position->currentCellY;

	int newCellX = currentCellX;
	int newCellY = currentCellY;

	if (currentCellX > 0)
	{
		side = board.cells[currentCellY][(currentCellX - 1)];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = (currentCellX - 1);
			newCellY = currentCellY;
		}
	}
	if (currentCellX < CELLS_X - 1)
	{
		side = board.cells[currentCellY][(currentCellX + 1)];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCellX = (currentCellX + 1);
			newCellY = currentCellY;
		}
	}

	return ((newCellY * CELL_SIZE) + (CELL_SIZE / 2));

}
void Grassfire(Board* board)
{

	bool S = true;
	while (S)
	{
		S = false;
		memcpy(board->cellsGrassfire, board->cells, sizeof(board->cells));
		for (int i = 0; i < CELLS_Y; i++)
		{
			for (int j = 0; j < CELLS_X; j++)
			{
				int A = board->cellsGrassfire[i][j];
				if (A != 255 && A != 0)
				{
					int B = A + 1;

					int x;
					if (j > 0)
					{
						x = board->cellsGrassfire[i][j - 1];
						if (x == 0)
						{
							board->cells[i][j - 1] = B;
							S = true;
						}
					}
					if (j < CELLS_X - 1)
					{
						x = board->cellsGrassfire[i][j + 1];
						if (x == 0)
						{
							board->cells[i][j + 1] = B;
							S = true;
						}
					}
					if (i > 0)
					{
						x = board->cellsGrassfire[i - 1][j];
						if (x == 0)
						{
							board->cells[i - 1][j] = B;
							S = true;
						}
					}
					if (i < CELLS_Y - 1)
					{
						x = board->cellsGrassfire[i + 1][j];
						if (x == 0)
						{
							board->cells[i + 1][j] = B;
							S = true;
						}
					}
				}
			}
		}
	}
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