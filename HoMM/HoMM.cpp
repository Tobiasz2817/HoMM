#include <stdio.h>
#include <string.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <stdlib.h>
#include <time.h>

#define OBSTACLES_COUNT 10
#define CHARACTERS_COUNT 8

#define CELLS_X 15
#define CELLS_Y 11

#define RESOLUTION_X 1920
#define RESOLUTION_Y 1080

#define CELL_SIZE 50


int WINDOW_WIDTH = 15 * CELL_SIZE;
int WINDOW_HEIGHT = 11 * CELL_SIZE;

typedef int uint;
struct Vec2i
{
	uint x;
	uint y;

	bool operator==(Vec2i right)
	{
		return x == right.x && y == right.y;
	}
};

typedef float ufloat;
struct Vec2f
{
	ufloat x;
	ufloat y;

	bool operator==(Vec2f right)
	{
		return x == right.x && y == right.y;
	}
};
struct Position
{
	// Start Position player
	Vec2i startPos;

	// Current Cell
	Vec2i currentCell;

	// Target Cell
	Vec2i targetCell;

	// Finish Cell
	Vec2i finishCell;
};

struct Image
{
	Vec2f position;
	Vec2i size;
	SDL_Texture* texture;

	void Init(Vec2f pos, Vec2i size, SDL_Texture* texture);
};
void Image::Init(Vec2f pos, Vec2i s, SDL_Texture* tex)
{
	position = pos;
	size = s;
	texture = tex;
}

struct Character
{
	float speed;
	bool canMoveToCell;
	bool isMoving;

	Position position;
	Image characterImage;

	void Init(Image image, int speedPlayer, bool canMoveToCel_, bool isMoving_, Vec2i startPos);
};
typedef struct Character Character;

void Character::Init(Image image, int speedPlayer, bool canMoveToCel_, bool isMoving_, Vec2i startPos)
{
	characterImage = image;
	speed = speedPlayer;

	canMoveToCell = canMoveToCel_;
	isMoving = isMoving_;

	position.startPos = startPos;

	position.targetCell = { (position.startPos.x * CELL_SIZE) + (CELL_SIZE / 2) , (position.startPos.y * CELL_SIZE) + (CELL_SIZE / 2) };

	characterImage.position = { (float)position.targetCell.x, (float)position.targetCell.y };

	position.currentCell = startPos;
}
struct Board
{
	SDL_Texture* defaultTexture;
	SDL_Texture* obstacleTexture;

	unsigned char cells[CELLS_Y][CELLS_X];
	unsigned char cellsGrassfire[CELLS_Y][CELLS_X];
	unsigned char cellsWithoutCharacters[CELLS_Y][CELLS_X];
	Vec2i obstacles[OBSTACLES_COUNT];
};
typedef struct Board Board;

void Grassfire(Board* board, Character* character);
void DrawCharacters(Character* characters, SDL_Renderer* renderer);
void MoveToCell(Board board, Character* character);

void SetRect(SDL_Rect* rect, int x, int y, int w, int h);
SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* fileName);
bool InitSDL(SDL_Renderer** renderer, SDL_Window** window);

void DrawCharacters(Character* characters, SDL_Renderer* renderer);
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);

void CreateBoard(Board* board, Character* playerCharacters, Character* enemyCharacters);
void GenerateObstacles(Board* board);
bool AreObstacleExsist(Board* board, int x, int y, int i);
void GenerateRandomDestination(Character* character, Board* board);

int main()
{
	srand(time(NULL));

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	if (!InitSDL(&renderer, &window))
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return 0;
	}
	SDL_Surface* surface = nullptr;

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = CELL_SIZE;
	int tex_height = CELL_SIZE;

	float secounds = 1;


	Image characterImage;
	characterImage.Init({ 0,0 }, { tex_width,tex_height }, SetTexture(surface, renderer, "stickXd.png"));

	int StartPosX = 0;
	int StartPosY = 0;
	Character playerCharacters[CHARACTERS_COUNT];
	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{
		playerCharacters[i].Init(characterImage, 200.f, false, false, { StartPosX, StartPosY });

		StartPosY++;
	}

	Image enemyImage;
	enemyImage.Init({ 0,0 }, { tex_width,tex_height }, SetTexture(surface, renderer, "stickEnemy.png"));

	StartPosX = CELLS_X - 1;
	StartPosY = 0;
	Character enemyCharacters[CHARACTERS_COUNT];
	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{
		enemyCharacters[i].Init(enemyImage, 200.f, false, false, { StartPosX, StartPosY });

		StartPosY++;
	}

	Board board;
	GenerateObstacles(&board);
	CreateBoard(&board, playerCharacters, enemyCharacters);

	board.obstacleTexture = SetTexture(surface, renderer, "obstacle.png");
	board.defaultTexture = SetTexture(surface, renderer, "default.png");


	bool startCounting = false;

	bool playerTurn = true;
	int currentCharacterIdx = 0;
	Character* currentCharacter = &playerCharacters[0];

	int maxDestinyReached = 2;

	float deltaTime = 0.f;
	float lastTick = 0.f;

	// Bye-bye the surface
	SDL_FreeSurface(surface);


	SDL_Event sdl_event;
	SDL_Rect rect;

	bool startGrassfire = false;
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

					if (currentCharacter->isMoving)
						break;

					currentCharacter->position.finishCell = { mousePosX / tex_width , mousePosY / tex_height };

					currentCharacter->canMoveToCell = true;
					startGrassfire = true;

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
				DrawImage(renderer, board.cellsWithoutCharacters[i][j] == 255 ? board.obstacleTexture : board.defaultTexture, rect);
			}
		}

		DrawCharacters(playerCharacters, renderer);
		DrawCharacters(enemyCharacters, renderer);

		SDL_RenderPresent(renderer);

		if (currentCharacter->canMoveToCell)
		{
			currentCharacter->position.currentCell = { ((uint)(currentCharacter->characterImage.position.x / CELL_SIZE)), ((uint)(currentCharacter->characterImage.position.y / CELL_SIZE)) };

			cellAreReached = true;
			if (fabs(currentCharacter->characterImage.position.x - currentCharacter->position.targetCell.x) >= maxDestinyReached) {
				if (currentCharacter->characterImage.position.x > currentCharacter->position.targetCell.x) {
					currentCharacter->characterImage.position.x -= currentCharacter->speed * deltaTime;
				}
				else {
					currentCharacter->characterImage.position.x += currentCharacter->speed * deltaTime;
				}

				cellAreReached = false;
			}
			if (fabs(currentCharacter->characterImage.position.y - currentCharacter->position.targetCell.y) >= maxDestinyReached) {
				if (currentCharacter->characterImage.position.y > currentCharacter->position.targetCell.y) {
					currentCharacter->characterImage.position.y -= currentCharacter->speed * deltaTime;
				}
				else {
					currentCharacter->characterImage.position.y += currentCharacter->speed * deltaTime;
				}
				cellAreReached = false;
			}

			if (cellAreReached && startGrassfire && playerTurn)
			{
				CreateBoard(&board, playerCharacters, enemyCharacters);
				Grassfire(&board, currentCharacter);

				startGrassfire = false;
			}


			if (cellAreReached && !(currentCharacter->position.currentCell == currentCharacter->position.finishCell))
			{
				MoveToCell(board, currentCharacter);
			}
			else if (cellAreReached && currentCharacter->isMoving && currentCharacter->position.currentCell == currentCharacter->position.finishCell)
			{
				currentCharacter->isMoving = false;
				currentCharacter->position.currentCell = currentCharacter->position.finishCell;
				if (playerTurn)
				{
					playerTurn = false;
					currentCharacter = &enemyCharacters[currentCharacterIdx];
					CreateBoard(&board, playerCharacters, enemyCharacters);
					GenerateRandomDestination(currentCharacter, &board);
				}
				else
				{
					playerTurn = true;
					currentCharacterIdx++;
					if (currentCharacterIdx == CHARACTERS_COUNT)
					{
						currentCharacterIdx = 0;
					}
					currentCharacter = &playerCharacters[currentCharacterIdx];
				}
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

	Vec2i currentCell = character->position.currentCell;

	Vec2i newCurrentCell = currentCell;

	if (currentCell.x > 0)
	{
		side = board.cells[currentCell.y][(currentCell.x - 1)];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCurrentCell.x = (currentCell.x - 1);
			newCurrentCell.y = currentCell.y;
		}
	}
	if (currentCell.x < CELLS_X - 1)
	{
		side = board.cells[currentCell.y][(currentCell.x + 1)];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCurrentCell.x = (currentCell.x + 1);
			newCurrentCell.y = currentCell.y;
		}
	}
	if (currentCell.y > 0)
	{
		side = board.cells[(currentCell.y - 1)][currentCell.x];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCurrentCell.x = currentCell.x;
			newCurrentCell.y = (currentCell.y - 1);
		}
	}
	if (currentCell.y < CELLS_Y - 1)
	{
		side = board.cells[(currentCell.y + 1)][currentCell.x];

		if (side < minValue && side != 0)
		{
			minValue = side;

			newCurrentCell.x = currentCell.x;
			newCurrentCell.y = (currentCell.y + 1);
		}
	}


	character->position.targetCell = { (newCurrentCell.x * CELL_SIZE) + (CELL_SIZE / 2), (newCurrentCell.y * CELL_SIZE) + (CELL_SIZE / 2) };

}
void GenerateObstacles(Board* board)
{
	Vec2i max;
	max.x = 10;
	max.y = 7;
	Vec2i min;
	min.x = 4;
	min.y = 3;

	const int totalX = max.x - min.x + 1;
	const int totalY = max.y - min.y + 1;

	for (int i = 0; i < OBSTACLES_COUNT; ++i)
	{
		int x = rand() % totalX + min.x;
		int y = rand() % totalY + min.y;
		while (AreObstacleExsist(board, x, y, i))
		{
			x = rand() % totalX + min.x;
			y = rand() % totalY + min.y;
		}

		board->obstacles[i] = { x , y };
	}
}

bool AreObstacleExsist(Board* board, int x, int y, int i)
{
	for (int j = 0; j < i; ++j)
	{
		if (x == board->obstacles[j].x && y == board->obstacles[j].y)
		{
			return true;
		}
	}
	return false;
}

void GenerateRandomDestination(Character* character, Board* board)
{
	int x = rand() % CELLS_X;
	int y = rand() % CELLS_Y;
	while (board->cells[y][x] == 255)
	{
		x = rand() % CELLS_X;
		y = rand() % CELLS_Y;
	}

	character->position.finishCell = { x , y };

	Grassfire(board, character);
}
void CreateBoard(Board* board, Character* playerCharacters, Character* enemyCharacters)
{
	memset(board->cells, 0, sizeof(board->cells));

	// Set the obstacle data
	for (int i = 0; i < OBSTACLES_COUNT; ++i)
	{
		board->cells[board->obstacles[i].y][board->obstacles[i].x] = 255;
	}

	memcpy(board->cellsWithoutCharacters, board->cells, sizeof(board->cells));

	// Characters as obstacles
	for (int i = 0; i < CHARACTERS_COUNT; ++i)
	{
		board->cells[playerCharacters[i].position.currentCell.y][playerCharacters[i].position.currentCell.x] = 255;
	}
	for (int i = 0; i < CHARACTERS_COUNT; ++i)
	{
		board->cells[enemyCharacters[i].position.currentCell.y][enemyCharacters[i].position.currentCell.x] = 255;
	}
}
void Grassfire(Board* board, Character* character)
{

	if (board->cells[character->position.finishCell.y][character->position.finishCell.x] == 255)
	{
		character->canMoveToCell = false;
		return;
	}

	character->canMoveToCell = true;
	character->isMoving = true;
	board->cells[character->position.finishCell.y][character->position.finishCell.x] = 1;
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
				if (A != 0 && A != 255)
				{
					int B = A + 1;
					if (i > 0)
					{
						int x = board->cellsGrassfire[i - 1][j];
						if (x == 0)
						{
							board->cells[i - 1][j] = B;
							S = true;
						}
					}
					if (j < CELLS_X - 1)
					{
						int x = board->cellsGrassfire[i][j + 1];
						if (x == 0)
						{
							board->cells[i][j + 1] = B;
							S = true;
						}
					}
					if (i < CELLS_Y - 1)
					{
						int x = board->cellsGrassfire[i + 1][j];
						if (x == 0)
						{
							board->cells[i + 1][j] = B;
							S = true;
						}
					}
					if (j > 0)
					{
						int x = board->cellsGrassfire[i][j - 1];
						if (x == 0)
						{
							board->cells[i][j - 1] = B;
							S = true;
						}
					}
				}
			}
		}
	}

	MoveToCell(*board, character);
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
void DrawCharacters(Character* characters, SDL_Renderer* renderer)
{
	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{
		SDL_Rect rect;
		rect.x = (int)round(characters[i].characterImage.position.x - characters[i].characterImage.size.x / 2);
		rect.y = (int)round(characters[i].characterImage.position.y - characters[i].characterImage.size.y / 2);
		rect.w = characters[i].characterImage.size.x;
		rect.h = characters[i].characterImage.size.y;

		DrawImage(renderer, characters[i].characterImage.texture, rect);
	}
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
		RESOLUTION_X / 5, RESOLUTION_Y / 5,
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