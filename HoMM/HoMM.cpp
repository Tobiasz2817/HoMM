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

struct Vec2i
{
	unsigned int x;
	unsigned int y;
};
struct Position
{
	// Start Position
	Vec2i startPos;

	// Current Cell
	Vec2i currentCell;

	// Target Cell
	Vec2i targetCell;

	// Finish Cell
	Vec2i finishCell;
};
struct Character
{
	float speed;

	Position position;
	SDL_Texture* texture;

	void Init(SDL_Texture* newTexture, int speedCharacter, unsigned int startPositioncharacterX, unsigned int startPositioncharacterY);
};

void Character::Init(SDL_Texture* newTexture, int speedCharacter, unsigned startPositioncharacterX, unsigned int startPositioncharacterY)
{
	texture = newTexture;
	speed = speedCharacter;

	position.startPos = { startPositioncharacterX,startPositioncharacterY };
	position.targetCell = { (position.startPos.x * CELL_SIZE) + (CELL_SIZE / 2), (position.startPos.y * CELL_SIZE) + (CELL_SIZE / 2) };
	position.currentCell = { 0,0 };
}
struct Board
{
	SDL_Texture* defaultTexture;
	SDL_Texture* obstacleTexture;

	unsigned char cells[CELLS_Y][CELLS_X];
	unsigned char cellsGrassfire[CELLS_Y][CELLS_X];
};
struct ListNode
{
	ListNode* elemnet;
};
struct Queue
{
	ListNode* firstNode;
	void Init();
	void AddNode();
};
void Queue::Init()
{
	firstNode = nullptr;
}
void Queue::AddNode()
{
	ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
	new_node->elemnet = nullptr;

	if (!firstNode)
	{
		firstNode = nullptr;
		return;
	}

	ListNode* last_element = firstNode;
	while (last_element->elemnet)
		last_element = last_element->elemnet;

	last_element->elemnet = new_node;
}



SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* fileName);
void CreateBoard(Board* board);
void Grassfire(Board* board);
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);
void SetRect(SDL_Rect* rect, int x, int y, int w, int h);
void MoveToCell(Board board, Character* character);
bool InitSDL(SDL_Renderer** renderer, SDL_Window** window);


int main()
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	if (!InitSDL(&renderer,&window))
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return 0;
	}

	SDL_Surface* surface = nullptr;

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = CELL_SIZE;
	int tex_height = CELL_SIZE;

	Character character;

	character.Init(SetTexture(surface, renderer, "stickXd.png"), 200.f, 0, 1);

	Board board;
	board.obstacleTexture = SetTexture(surface, renderer, "obstacle.png");
	board.defaultTexture = SetTexture(surface, renderer, "default.png");


	int maxDestinyReached = 2;

	float x = character.position.targetCell.x;
	float y = character.position.targetCell.y;



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

					character.position.finishCell = { (unsigned)(mousePosX / tex_width), (unsigned)(mousePosY / tex_width) };

					if (board.cells[character.position.finishCell.y][character.position.finishCell.x] == 255)
					{
						canMove = false;
						break;
					}

					canMove = true;
					startGrassfire = true;
					CreateBoard(&board);
					board.cells[character.position.finishCell.y][character.position.finishCell.x] = 1;

					break;
				}
				default:
					break;
				}
			}
		}

		// Print on screen

		SDL_RenderClear(renderer);

		for (int i = 0; i < CELLS_Y; i++)
		{
			for (int j = 0; j < CELLS_X; j++)
			{
				SetRect(&rect, j * tex_width, i * tex_height, tex_width - 2, tex_height - 2);
				DrawImage(renderer, board.cells[i][j] == 255 ? board.obstacleTexture : board.defaultTexture, rect);
			}
		}

		SetRect(&rect, (int)round(x - tex_width / 2), (int)round(y - tex_height / 2), (int)tex_width, (int)tex_height);
		DrawImage(renderer, character.texture, rect);

		SDL_RenderPresent(renderer);


		if (canMove)
		{
			character.position.currentCell = { (unsigned)(x / CELL_SIZE), (unsigned)(y / CELL_SIZE) };

			cellAreReached = true;
			if (fabs(x - character.position.targetCell.x) >= maxDestinyReached) {
				if (x > character.position.targetCell.x) {
					x -= character.speed * deltaTime;
				}
				else {
					x += character.speed * deltaTime;
				}

				cellAreReached = false;
			}
			if (fabs(y - character.position.targetCell.y) >= maxDestinyReached) {
				if (y > character.position.targetCell.y) {
					y -= character.speed * deltaTime;
				}
				else {
					y += character.speed * deltaTime;
				}
				cellAreReached = false;
			}

			if (cellAreReached && startGrassfire)
			{
				Grassfire(&board);

				startGrassfire = false;
			}

			if (cellAreReached && (character.position.finishCell.x != character.position.currentCell.x || character.position.finishCell.y != character.position.currentCell.y))
			{
				MoveToCell(board, &character);
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
void MoveToCell(Board board, Character* character)
{
	int minValue = 255;
	int side = 0;

	int currentCellX = character->position.currentCell.x;
	int currentCellY = character->position.currentCell.y;

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

	character->position.targetCell = { (unsigned)(newCellX * CELL_SIZE) + (CELL_SIZE / 2), (unsigned)(newCellY * CELL_SIZE) + (CELL_SIZE / 2) };
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


	board->cells[1][7] = 255;
	board->cells[2][7] = 255;
	board->cells[3][7] = 255;
	board->cells[4][7] = 255;
	board->cells[5][7] = 255;
	board->cells[6][7] = 255;
	board->cells[7][7] = 255;
	board->cells[8][7] = 255;
	board->cells[9][7] = 255;
}

bool InitSDL(SDL_Renderer** renderer, SDL_Window** window)
{
	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return false;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return false;
	}
	// Creating the window 1920x1080 (could be any other size)
	*window = SDL_CreateWindow("FirstSDL",
		0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN);

	if (!*window)
		return false;

	// Creating a renderer which will draw things on the screen
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	if (!*renderer)
		return false;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(*renderer, 128, 69, 69, 255);

	return true;
}