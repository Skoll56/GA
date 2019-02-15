#include <iostream>
#include <random>
#include <ctime>
#include <SDL\SDL.h>
#include <GLM\glm.hpp>
#include <thread>
#include <mutex>
#include <sstream>
#include "Genetic.h"
#include "Entity.h"
#include "MapLoader.h"
#include "Astar.h"

#define WINDOW_WIDTH 980
#define WINDOW_HEIGHT 800
#define RED 255, 0, 0
#define BLUE 0, 0, 255
#define GREEN 0, 255, 0
#define WHITE 255, 255, 255
#define BLACK 0, 0, 0
#define BOXSIZE 40


SDL_Window* initWindow();
SDL_Renderer* initRenderer(SDL_Window* _window);
void drawWalls(SDL_Renderer* _renderer, std::vector<Entity*> _wall);
void drawGrid(SDL_Renderer* _renderer, int _x, int _y);
void drawSquare(SDL_Renderer* _renderer, Entity _entity, bool _fill);


int main(int argc, char *argv[])
{
	SDL_Window *window = initWindow();
	SDL_Renderer* renderer = initRenderer(window);

	
	
	Map map = readFile("map.txt");
	
	
	Entity player(map.m_start);
	player.m_col = Colour(0, 255, 0, 255);
	bool genetic, star;
	genetic = false;
	star = false;
	
	

	SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);	

	///Draw Code
	//Re-set the image
	SDL_SetRenderDrawColor(renderer, WHITE, 255);
	SDL_RenderClear(renderer);

	//Draw the map
	drawGrid(renderer, map.m_gridX, map.m_gridY);
	drawWalls(renderer, map.m_walls);

	//Draw the player square	
	drawSquare(renderer, player, true); 

	// Draw end square	
	drawSquare(renderer, map.m_end, true); 

	SDL_RenderPresent(renderer);
	int ans;
	std::cout << "Genetic (0) or A-Star(1) ?" << std::endl;
	std::cin >> ans;
	if (ans)
	{
		star = true;
	}
	else
	{
		genetic = true;
	}

	Gene path; 
	 

	std::string route; 
	int moves;
	std::vector<int> direction;

	if (genetic)
	{
		path = Genetics::geneticPathfind(player, map);
		moves = path.m_cSomes.size();
		for (int i = 0; i < moves; i++)
		{
			direction.push_back(path.m_cSomes[i]);
		}
	}
	else
	{
		route = astar(map);
		moves = route.size();
		for (int i = 0; i < moves; i++)
		{
			char c = route[i];
			std::string str;
			str += c;
			std::stringstream s(str);
			int t;
			s >> t;

			switch (t) // Convert the A* direction into my directions
			{
			case 0: t = 2; break;
			case 1: t = 7; break;
			case 2: t = 4; break;
			case 3: t = 6; break;
			case 4: t = 1; break;
			case 5: t = 5; break;
			case 6: t = 3; break;
			case 7: t = 8; break;

			}

			direction.push_back(t);
		}
	}
	player.m_xPos = map.m_start.m_xPos;
	player.m_yPos = map.m_start.m_yPos;
	std::cout << "This path uses " << direction.size() << " moves" << std::endl;
	std::cout << "Route: ";
	for (int i = 0; i < direction.size(); i++)
	{
		 std::cout << direction[i];
	}
	std::cout << std::endl;

	for (int i = 0; i < moves; i++)
	{
		//Re-set the image
		SDL_SetRenderDrawColor(renderer, WHITE, 255);
		SDL_RenderClear(renderer);

		//Draw the map
		drawGrid(renderer, map.m_gridX, map.m_gridY);
		drawWalls(renderer, map.m_walls);
		
		// Draw end square
		drawSquare(renderer, map.m_end, true);

		//Draw the player square
		Genetics::movePlayer(player, direction[i], map);
		drawSquare(renderer, player, true);		
		
		SDL_RenderPresent(renderer);
		SDL_Delay(500);
		if (player.m_xPos == map.m_end.m_xPos && player.m_yPos == map.m_end.m_yPos)
		{
			break;
		}
	} 

	

	system("PAUSE");
	return 0;
}





// Tests the % chance for each number between 1-8 to appear, to ensure they are being uniformly generated.
void Genetics::randomTest(std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution, int _iterations) 
{
	int one, two, three, four, five, six, seven, eight;
	one = two = three = four = five = six = seven = eight = 0;

	for (int i = 0; i < _iterations; i++)
	{
		double r = _distribution(_generator);
		if (r < 0.125)
		{
			r = 1.0;
			one++;
		}
		else if (r < 0.25)
		{
			r = 2.0;
			two++;
		}
		else if (r < 0.375)
		{
			r = 3.0;
			three++;
		}
		else if (r < 0.5)
		{
			r = 4.0;
			four++;
		}
		else if (r < 0.625)
		{
			r = 5.0;
			five++;
		}
		else if (r < 0.75)
		{
			r = 6.0;
			six++;
		}
		else if (r < 0.875)
		{
			r = 7.0;
			seven++;
		}
		else if (r <= 1.0)
		{
			r = 8.0;
			eight++;
		}
	}

	int sum = one + two + three + four + five + six + seven + eight;
	std::cout << "One= " << (float)one / (float)sum << "%" << std::endl;
	std::cout << "Two= " << (float)two / (float)sum << "%" << std::endl;
	std::cout << "Three= " << (float)three / (float)sum << "%" << std::endl;
	std::cout << "Four= " << (float)four / (float)sum << "%" << std::endl;
	std::cout << "Five= " << (float)five / (float)sum << "%" << std::endl;
	std::cout << "Six= " << (float)six / (float)sum << "%" << std::endl;
	std::cout << "Seven= " << (float)seven / (float)sum << "%" << std::endl;
	std::cout << "Eight=  " << (float)eight / (float)sum << "%" << std::endl;

}

//Initialises the Window
SDL_Window* initWindow() 
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		throw std::exception();
	}

	SDL_Window *window = SDL_CreateWindow("Pathfinder AI",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT, 0);

	if (window == nullptr)
	{
		throw std::exception();
	}

	return window;
}

SDL_Renderer* initRenderer(SDL_Window* _window)
{
	SDL_Renderer* renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) { throw std::exception(); }
	return renderer;
}

void drawGrid(SDL_Renderer* _renderer, int _x, int _y)
{	
	Entity t;
	t.m_col = Colour(80, 80, 80, 255);
	
	for (int x = 0; x < _x; x++)
	{
		t.m_xPos = x;
		for (int y = 0; y < _y; y++)
		{
			t.m_yPos = y;
			drawSquare(_renderer, t, false);
		}
	}
}

void drawWalls(SDL_Renderer* _renderer, std::vector<Entity*> _wall)
{
	
	for (int i = 0; i < _wall.size(); i++)
	{
		drawSquare(_renderer, *_wall[i], true);
	}
}

void drawSquare(SDL_Renderer* _renderer, Entity _entity, bool _fill)
{
	SDL_Rect square;	
	square.x = _entity.m_xPos * BOXSIZE;
	square.y = _entity.m_yPos * BOXSIZE;
	square.w = BOXSIZE;
	square.h = BOXSIZE;
	SDL_SetRenderDrawColor(_renderer, _entity.m_col.r, _entity.m_col.g, _entity.m_col.b, _entity.m_col.a);
	
	if (_fill)
	{
		SDL_RenderFillRect(_renderer, &square);
	}
	else
	{
		SDL_RenderDrawRect(_renderer, &square);
	}
}