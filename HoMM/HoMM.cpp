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
};

enum World
{
	_Character = 253,
	_Obstacle = 255,

};

struct Attributes
{
	float health;
	float attackPower;
};

struct Position
{
	Vec2f currentPosition;

	// Current Cell
	Vec2i currentCell;

	// Finish Cell
	Vec2i finishCell;
};

struct Image
{
	Vec2f position;
	Vec2i size;
	SDL_Texture* texture;

	void Init(Vec2f pos, Vec2i size, SDL_Texture* texture);
	void Render(SDL_Renderer* renderer, Vec2f pos);
};
void Image::Init(Vec2f pos, Vec2i s, SDL_Texture* tex)
{
	position = pos;
	size = s;
	texture = tex;
}
void Image::Render(SDL_Renderer* renderer, Vec2f pos)
{
	SDL_Rect rect;
	rect.x = pos.x * size.x;
	rect.y = pos.y * size.y;
	rect.w = size.x;
	rect.h = size.y;

	SDL_RenderCopyEx(renderer, // Already know what is that
		texture, // The image
		nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
		&rect, // The destination rectangle on the screen.
		0, // An angle in degrees for rotation
		nullptr, // The center of the rotation (when nullptr, the rect center is taken)
		SDL_FLIP_NONE); // We don't want to flip the image
}


struct Character
{
	float speed;
	bool canMoveToCell;
	bool myTurn;

	Attributes attributes;
	Position position;
	Image characterImage;

	Character* next;

	void Init(Vec2i size, SDL_Texture* texture, int speedPlayer, bool canMoveToCel_, Vec2i startPos, Attributes attributes);
	void Render(SDL_Renderer* renderer);
};
typedef struct Character Character;

void Character::Init(Vec2i size, SDL_Texture* texture, int speedPlayer, bool canMoveToCel_, Vec2i startPos, Attributes attributesCharacter)
{
	myTurn = true;

	speed = speedPlayer;
	attributes = attributesCharacter;
	canMoveToCell = canMoveToCel_;

	position.currentCell = startPos;
	position.currentPosition = { (ufloat)(startPos.x * CELL_SIZE) , (ufloat)(startPos.y * CELL_SIZE) };

	characterImage.Init( { position.currentPosition.x , position.currentPosition.y } , size, texture);
}
void Character::Render(SDL_Renderer* renderer)
{
	characterImage.Render(renderer, { position.currentPosition.x / CELL_SIZE , position.currentPosition.y / CELL_SIZE } );
}


struct Team
{

	int lengthTeam = 0;
	Character* firstCharacter = nullptr;

	void DeleteFirstCharacter();
	void AddCharacter(Vec2i size, SDL_Texture* texture, int speedPlayer, bool canMoveToCel_, Vec2i startPos, Attributes attributes);

	void DisplayCharacters(SDL_Renderer* renderer);
	void TakeDamage(float damage);
	float CheckHealth();
	
	Character* GetCharacterTurn();
	Character* GetRanomCharacter();

	void Clear();

	bool ExistCharacter(Vec2i position);
};

void Team::DeleteFirstCharacter()
{
	Character* next_character = firstCharacter->next;
	free(firstCharacter);
	firstCharacter = next_character; 
}
void Team::AddCharacter(Vec2i size, SDL_Texture* texture, int speedPlayer, bool canMoveToCel_, Vec2i startPos, Attributes attributes)
{
	Character* new_character = (Character*)malloc(sizeof(Character));
	new_character->next = nullptr;

	new_character->Init(size, texture, speedPlayer, canMoveToCel_, startPos, attributes);
	lengthTeam += 1;
	if (!firstCharacter)
	{
		firstCharacter = new_character;
		return;
	}

	Character* last_element = firstCharacter;
	while (last_element->next)
		last_element = last_element->next;

	last_element->next = new_character;

}
Character* Team::GetCharacterTurn()
{
	Character* currentCharacter = firstCharacter;
	while (currentCharacter)
	{
		if (currentCharacter->myTurn)
		{
			currentCharacter->myTurn = false;

			return currentCharacter;
		}

		currentCharacter = currentCharacter->next;
	}

	currentCharacter = firstCharacter;
	while (currentCharacter)
	{
		currentCharacter->myTurn = true;

		currentCharacter = currentCharacter->next;
	}

	return firstCharacter;
}
Character* Team::GetRanomCharacter()
{
	Vec2i ranomPos[CHARACTERS_COUNT];

	Character* currentCharacter = firstCharacter;
	if (!currentCharacter) return nullptr;

	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{

		ranomPos[i] = currentCharacter->position.currentCell;


		currentCharacter = currentCharacter->next;
		if (!currentCharacter)
			break;
	}
	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{
		printf("\n x: %i y: %i \n" ,ranomPos[i].x, ranomPos[i].y);
	}

	int indexPos = rand() % lengthTeam;

	printf("\n x: %i \n",indexPos);

	currentCharacter = firstCharacter;
	while (currentCharacter)
	{
		if (currentCharacter->position.currentCell == ranomPos[indexPos])
		{
			return currentCharacter;
		}

		currentCharacter = currentCharacter->next;
	}

	printf("\n---------------\n");


	return nullptr;
}
void Team::DisplayCharacters(SDL_Renderer* renderer)
{
	Character* currentCharacter = firstCharacter;
	while (currentCharacter)
	{
		currentCharacter->Render(renderer);

		currentCharacter = currentCharacter->next;
	}
}
void Team::TakeDamage(float damage)
{
	if (!firstCharacter) return;

	float restLife = firstCharacter->attributes.health - damage;

	firstCharacter->attributes.health = restLife;

	if (restLife < 0)
	{
		if (firstCharacter->next)
		{
			firstCharacter->next->attributes.health = restLife;
		}
	}
}
float Team::CheckHealth()
{
	return firstCharacter->attributes.health;
}
bool Team::ExistCharacter(Vec2i position)
{
	Character* currentCharacter = firstCharacter;
	while (currentCharacter)
	{
		if (currentCharacter->position.currentCell == position)
		{
			return true;
		}

		currentCharacter = currentCharacter->next;
	}

	return false;
}
void Team::Clear()
{
	while (!firstCharacter)
	{
		DeleteFirstCharacter();
	}
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

struct ListNode
{
	ListNode* nextNode;
	Vec2i position;
};

struct Queue
{
	void AddNode(Vec2i nodePosition);
	bool IsEmpty();
	void DeleteFirstNode();
	void Clear();

	ListNode* firstNode = nullptr;
};


void Queue::DeleteFirstNode()
{
	ListNode* next_node = firstNode->nextNode;
	free(firstNode);
	firstNode = next_node;

}
void Queue::Clear()
{
	while (firstNode)
	{
		DeleteFirstNode();
	}
}

void Queue::AddNode(Vec2i nodePosition)
{
	ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
	new_node->nextNode = nullptr;
	new_node->position = nodePosition;

	if (!firstNode)
	{
		firstNode = new_node;
		return;
	}

	ListNode* last_element = firstNode;
	while (last_element->nextNode)
		last_element = last_element->nextNode;

	last_element->nextNode = new_node;
}


bool Queue::IsEmpty()
{
	return !firstNode;
}

Queue Grassfire(Board* board, Character* character, bool isThereCharacter);
void MoveToCell(Board board, Character* character);

void SetRect(SDL_Rect* rect, int x, int y, int w, int h);
SDL_Texture* SetTexture(SDL_Renderer* renderer, const char* fileName);
bool InitSDL(SDL_Renderer** renderer, SDL_Window** window);

void DrawCharacters(Team characters, SDL_Renderer* renderer);
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);

void CreateBoard(Board* board, Team playerCharacters, Team enemyCharacters);
void GenerateObstacles(Board* board);
void GenerateRandomDestination(Character* character, Board* board, Vec2i& newPos);
bool AreObstacleExsist(Board* board, Vec2i pos, int i);
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

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = CELL_SIZE;
	int tex_height = CELL_SIZE;

	//Image characterImage;
	//characterImage.Init({ 0,0 }, { tex_width,tex_height }, SetTexture(surface, renderer, "stickXd.png"));
	//
	//Image enemyImage;
	//enemyImage.Init({ 0,0 }, { tex_width,tex_height }, SetTexture(surface, renderer, "stickEnemy.png"));

	Team team1;
	int StartPosX = 0;
	int StartPosY = 0;
	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{
		team1.AddCharacter({ tex_width,tex_height }, SetTexture(renderer, "stickXd.png"), 5.f, false, { StartPosX, StartPosY } ,{ 100.f , 85.f} );

		StartPosY++;
	}


	Team team2;
	StartPosX = CELLS_X - 1;
	StartPosY = 0;
	for (int i = 0; i < CHARACTERS_COUNT; i++)
	{
		team2.AddCharacter({ tex_width,tex_height }, SetTexture(renderer, "stickEnemy.png"), 5.f, false, { StartPosX, StartPosY }, { 100.f , 50.f });

		StartPosY++;
	}


	printf(" %i length team 1", team1.lengthTeam);
	printf(" %i length team 2", team2.lengthTeam);

	Board board;
	GenerateObstacles(&board);
	CreateBoard(&board, team1, team2);

	board.obstacleTexture = SetTexture(renderer, "obstacle.png");
	board.defaultTexture = SetTexture(renderer, "default.png");

	bool posReached = true;

	bool startCounting = false;

	bool playerTurn = true;
	bool finalNodeReached = false;

	int currentCharacterIdx = 0;
	Character* currentCharacter = team1.GetCharacterTurn();

	int maxDestinyReached = 1;

	float deltaTime = 0.f;
	float lastTick = 0.f;

	Queue path;
	Vec2i point = currentCharacter->position.currentCell;
	Vec2f direction = { point.x , point.y } ;

	SDL_Event sdl_event;
	SDL_Rect rect;

	bool startGrassfire = false;
	bool cellAreReached = true;
	bool done = false;
	bool gameOver = false;

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

						currentCharacter->position.finishCell = { mousePosX / tex_width , mousePosY / tex_height };

						currentCharacter->canMoveToCell = true;
						startGrassfire = true;	
					}
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
				SetRect(&rect, j * tex_width, i * tex_height, tex_width - 2, tex_height - 2);
				DrawImage(renderer, board.cellsWithoutCharacters[i][j] == _Obstacle ? board.obstacleTexture : board.defaultTexture, rect);
			}
		}

		team1.DisplayCharacters(renderer);
		team2.DisplayCharacters(renderer);

		//printf("\n ------- \n");

		//return 0;

		SDL_RenderPresent(renderer);

		if (!gameOver)
		{

			if (playerTurn && finalNodeReached)
			{
				printf(" \n Zmiana na 1 gracz do kurwy nędzy \n");


				currentCharacter = team1.GetCharacterTurn();

				if (!team1.firstCharacter)
				{
					// Team2 dead
					printf("\n Game Over Win Team 2 \n");
					gameOver = true;
					continue;
				}

				finalNodeReached = false;
			}
			else if (!playerTurn && finalNodeReached)
			{
				printf(" \n Zmiana na 2 gracz do kurwy nędzy \n");


				currentCharacter = team2.GetCharacterTurn();


				if (!team2.firstCharacter)
				{
					// Team2 dead
					printf("\n Game Over Win Team 1 \n");
					gameOver = true;
					continue;
				}



				finalNodeReached = false;
			}

			if (startGrassfire)
			{
				// follow to team 2
				if (!team1.ExistCharacter(currentCharacter->position.finishCell) && playerTurn)
				{
					CreateBoard(&board, team1, team2);
					path = Grassfire(&board, currentCharacter, team2.ExistCharacter(currentCharacter->position.finishCell));

					if (!path.firstNode)
					{
						currentCharacter->canMoveToCell = false;

						printf(" First node are null ");
						startGrassfire = false;
						continue;
					}

					printf("\n I move to cursor \n");

				}
				else if (!playerTurn)
				{
					CreateBoard(&board, team1, team2);

					Character* positionRandomCharacter = team1.GetRanomCharacter();
					if (!positionRandomCharacter)
					{
						gameOver = true;
						continue;
					}
					currentCharacter->position.finishCell = positionRandomCharacter->position.currentCell;

					path = Grassfire(&board, currentCharacter, team1.ExistCharacter(currentCharacter->position.finishCell));

					if (!path.firstNode)
					{
						currentCharacter->canMoveToCell = false;

						printf(" First node are null ");
						startGrassfire = true;
						continue;
					}

					currentCharacter->canMoveToCell = true;
				}
				else
				{
					currentCharacter->canMoveToCell = false;
				}

				startGrassfire = false;
			}

			


			if (currentCharacter->canMoveToCell)
			{
				currentCharacter->position.currentCell = { (int)round(currentCharacter->position.currentPosition.x / CELL_SIZE), (int)round(currentCharacter->position.currentPosition.y / CELL_SIZE) };
				if (!path.IsEmpty())
				{

					if (posReached)
					{
						point = path.firstNode->position;
						direction = { point.x - currentCharacter->position.currentPosition.x, point.y - currentCharacter->position.currentPosition.y };
						posReached = false;
					}

					currentCharacter->position.currentPosition.x += direction.x * currentCharacter->speed * deltaTime;
					currentCharacter->position.currentPosition.y += direction.y * currentCharacter->speed * deltaTime;

					if (fabs(currentCharacter->position.currentPosition.x - point.x) < maxDestinyReached && fabs(currentCharacter->position.currentPosition.y - point.y) < maxDestinyReached)
					{
						posReached = true;
						path.DeleteFirstNode();
					}


					finalNodeReached = false;
				}
				else
				{
					finalNodeReached = true;
					currentCharacter->canMoveToCell = false;

					if (team1.ExistCharacter(currentCharacter->position.finishCell) && !playerTurn)
					{
						team1.TakeDamage(currentCharacter->attributes.attackPower);

						printf(" \n team 1 health: %f \n", team1.CheckHealth());
						if (team1.CheckHealth() <= 0)
						{
							team1.DeleteFirstCharacter();
							team1.lengthTeam -= 1;
						}
					}
					else if (team2.ExistCharacter(currentCharacter->position.finishCell) && playerTurn)
					{
						team2.TakeDamage(currentCharacter->attributes.attackPower);
						printf(" \n team 2 health: %f \n", team2.CheckHealth());
						if (team2.CheckHealth() <= 0)
						{
							team2.DeleteFirstCharacter();
							team2.lengthTeam -= 1;
						}
					}


					playerTurn = !playerTurn;

					if (!playerTurn)
					{
						printf("\n start Grassfire \n");
						startGrassfire = true;
					}

					printf("\n path is empty\n");
				}
			}
		}

	}

	team1.Clear();
	team2.Clear();

	path.Clear();

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
		while (AreObstacleExsist(board, { x ,y } , i))
		{
			x = rand() % totalX + min.x;
			y = rand() % totalY + min.y;
		}


		board->obstacles[i] = { x , y };
		
	}

}
bool AreObstacleExsist(Board* board, Vec2i pos, int i)
{
	for (int j = 0; j < i; ++j)
	{
		if (pos.x == board->obstacles[j].x && pos.y == board->obstacles[j].y)
		{
			return true;
		}
	}
	return false;
}
void CreateBoard(Board* board, Team playerCharacters, Team enemyCharacters)
{
	memset(board->cells, 0, sizeof(board->cells));

	// Set the obstacle data
	for (int i = 0; i < OBSTACLES_COUNT; ++i)
	{
		board->cells[board->obstacles[i].y][board->obstacles[i].x] = _Obstacle;
	}

	memcpy(board->cellsWithoutCharacters, board->cells, sizeof(board->cells));


	Character* currentCharacter = playerCharacters.firstCharacter;

	while (currentCharacter)
	{
		board->cells[currentCharacter->position.currentCell.y][currentCharacter->position.currentCell.x] = _Character;

		currentCharacter = currentCharacter->next;

	}

	currentCharacter = enemyCharacters.firstCharacter;
	while (currentCharacter)
	{
		board->cells[currentCharacter->position.currentCell.y][currentCharacter->position.currentCell.x] = _Character;

		currentCharacter = currentCharacter->next;
	}
}
Queue Grassfire(Board* board, Character* character,bool isThereCharacter)
{
	Queue newPath;

	if (board->cells[character->position.finishCell.y][character->position.finishCell.x] == _Obstacle)
	{
		character->canMoveToCell = false;
		return newPath;
	}

	character->canMoveToCell = true;
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
				if (A != 0 && A != _Obstacle && A != _Character)
				{
					for (int y = i - 1; y <= i + 1; y++)
						for (int x = j - 1; x <= j + 1; x++)
							if (x == j|| y == i)
							{
								int B = A + 1;
								if (x >= 0 && x <= CELLS_X - 1 && y >= 0 && y <= CELLS_Y - 1)
								{
									int side = board->cellsGrassfire[y][x];
									if (side == 0)
									{
										board->cells[y][x] = B;
										S = true;
									}
								}
							}
				}
			}
		}
	}

	// Recontruct Path
	Vec2i move = character->position.currentCell;
	Vec2i newPoint = character->position.currentCell;

	//printf("\n current pos cell x: %i y: %i \n", character->position.currentCell.x, character->position.currentCell.y);
	//printf("\n current pos x: %i y: %i \n", (int)character->position.currentPosition.x / CELL_SIZE, (int)character->position.currentPosition.x / CELL_SIZE);

	while ( !(move == character->position.finishCell) )
	{
		int minValue = _Obstacle;
		for (int y = move.y - 1; y <= move.y + 1; y++)
		{
			for (int x = move.x - 1; x <= move.x + 1; x++)
			{   
				if (x == move.x || y == move.y)
				{
					if (x >= 0 && x <= CELLS_X - 1 && y >= 0 && y <= CELLS_Y - 1)
					{
						int side = board->cells[y][x];

						if (side < minValue && side != 0)
						{
							minValue = side;

							newPoint = { x, y };

						}


						/*printf("\n x: %i y: %i \n", x, y);*/
					}

				}
			}
		}


		if (minValue == _Obstacle || minValue == _Character)
		{
			return newPath;
		}

		move = newPoint;

		if ((move == character->position.finishCell) && isThereCharacter)
		{
			printf("\n newPoint: x: %i y: %i minValue: %i \n", newPoint.x, newPoint.y, minValue);

			if (!newPath.firstNode)
			{
				newPath.AddNode({ (character->position.currentCell.x * CELL_SIZE), (character->position.currentCell.y * CELL_SIZE) });
			}

			return newPath;
		}

		newPath.AddNode({ (move.x * CELL_SIZE), (move.y * CELL_SIZE) });

		/*printf("\n newPoint: x: %i y: %i minValue: %i \n", newPoint.x, newPoint.y,minValue);*/
	}

	return newPath;
}
SDL_Texture* SetTexture(SDL_Renderer* renderer, const char* fileName)
{
	SDL_Surface* surface = IMG_Load(fileName);
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
void DrawCharacters(Team characters, SDL_Renderer* renderer)
{

	Character* currentCharacter = characters.firstCharacter;
	while (!currentCharacter)
	{
		SDL_Rect rect;
		rect.x = (int)round(currentCharacter->characterImage.position.x - currentCharacter->characterImage.size.x / 2);
		rect.y = (int)round(currentCharacter->characterImage.position.y - currentCharacter->characterImage.size.y / 2);
		rect.w = currentCharacter->characterImage.size.x;
		rect.h = currentCharacter->characterImage.size.y;

		DrawImage(renderer, currentCharacter->characterImage.texture, rect);

		currentCharacter = currentCharacter->next;
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