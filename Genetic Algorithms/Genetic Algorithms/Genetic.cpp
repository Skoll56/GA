#include "Genetic.h"
#include "Entity.h"
#include <ctime>
#include <SDL\SDL.h>
#include <GLM\glm.hpp>
#include <iostream>
#include <omp.h>
#define NUM_THREADS 4

//Genetic Algorithm Code
Gene Genetics::geneticPathfind(Entity  _player, Map &_map)
{
	omp_set_num_threads(NUM_THREADS);
	std::default_random_engine generator(std::time(0));   // This code has been sourced from http://www.cplusplus.com/reference/random/uniform_real_distribution/
	std::uniform_real_distribution<float> distribution(0.0, 1.0); // ^	
	int cSomeSize = 2 * ((_map.m_gridX + _map.m_gridY) / 3); // Make an initial cSome length based on the map dimensions. It must always be even.
	int popSize = NUM_THREADS * 2; // This MUST always be a multiple of NUM_THREADS else MP will struggle
	int generation = 0;
	int shortest = 1000;
	int lastRefine = 9999999999;
	int lastShortest = 1000;

	std::vector<Gene> genePool;
	Gene winGene;
	Gene path;
	float fitnessTotal = 0.0f; // The sum of all the gene's fitnessess
	

	for (int i = 0; i < popSize; i++)
	{
		Gene t(genGene(generator, distribution, cSomeSize), i);
		genePool.push_back(t);
	}
	std::vector<Gene> parents;
	parents.resize(2);
	std::vector<Gene> children;
	children.resize(genePool.size());

	double time = SDL_GetTicks();
	bool success = false;
	bool refine = false;
	bool bestRoute = false;
	int popUpgrade = 0;
	int growRate = 2;
	int longPath;

	while (!success || refine)
	{
		if (refine)
		{	

			// If we haven't refined the shortest route in a lot of generations relative to the cSome size
			if (generation >= lastRefine + (shortest * 10000)) 
			{
				bestRoute = true;
			}

			//If we're refining, call the THANOS function to eradicate half of all life in order to stop stalemating. It's fair, and random.
			else if (generation % (cSomeSize * 500) == 0)
				{
				std::cout << "*** SNAP ***" << std::endl;
					for (int i = 0; i < popSize; i += 2)
					{
						Gene t(genGene(generator, distribution, cSomeSize), i);
						genePool[i] = t;
					}
				}
		}

		//success = false;
		generation++;
		if (generation % 2000 == 0)
		{
			float dTime = SDL_GetTicks() - time;
			time = SDL_GetTicks();
			std::cout << "Generation: " << generation << std::endl;
			std::cout << "Time taken: " << dTime / 1000.0f << std::endl;
			std::cout << "Current Csome size= " << cSomeSize << std::endl;
			std::cout << "Current Gene size= " << genePool.size() << std::endl;
			
		}

		if (generation % 150000 == 0 && !refine) // If after 150,000 generations we still don't have the answer, it's time to purge the population and start afresh
		{
			popUpgrade++;
			if (popUpgrade == 10) // Every 10 lots of 150,000 generations
			{
				popUpgrade = 0;
				popSize += NUM_THREADS;
				for (int i = 0; i < NUM_THREADS; i++)
				{
					Gene t;
					genePool.push_back(t);
				}
				children.resize(genePool.size());
			}
			cSomeSize += growRate;
			growRate += growRate;
			for (int i = 0; i < popSize; i++)
			{
				Gene t(genGene(generator, distribution, cSomeSize), i);
				genePool[i] = t;
			}
			std::cout << "PURGE PROTOCOLS ENGAGED" << std::endl;			
		}

		fitnessTotal = 0.0f; //Reset the fitness total
		float f1, f2, f3, f4;
		
	
		#pragma omp parallel // This splits the genes between 4 threads for the processor to handle at the same time
		{
			#pragma omp sections
			{
				#pragma omp section
				{f1 = handleGene(_player, genePool, _map, generator, distribution, 1, success); }
				

				#pragma omp section
				{f2 = handleGene(_player, genePool, _map, generator, distribution, 2, success); }
				

				#pragma omp section
				{f3 = handleGene(_player, genePool, _map, generator, distribution, 3, success); }
				

				#pragma omp section
				{f4 = handleGene(_player, genePool, _map, generator, distribution, 4, success); }

				
			}
		}
		fitnessTotal = f1 + f2 + f3 + f4;

		if (success) // If we hit the goal this frame, or we're in the refining process
		{
			for (int i = 0; i < genePool.size(); i++)
			{
				if (genePool[i].m_hitGoal)
				{
					if (!refine)
					{
						shortest = genePool[i].m_cSomes.size() - genePool[i].m_leftOver; // Determine the shortest gene we currently have
					}
					winGene = genePool[i];					
					break;
				}
			}
		}

		for (int i = 0; i < genePool.size(); i++) // Check the whole population yet again
		{
			for (int j = i + 1; j < genePool.size(); j++) //Organising code sourced from https://www.techstudy.org/CplusplusLanguage/Write-C-plus-plus-program-to-sort-an-array-in-ascending-order
			{
				if (genePool[j].m_fitness < genePool[i].m_fitness) //Organises the array into order of fitness
				{
					Gene temp = genePool[i];
					genePool[i] = genePool[j];
					genePool[j] = temp;
				}
			}

		}
		
		for (int i = 0; i < genePool.size(); i++) // Check the whole population again
		{
			if (refine)
			{
				if (genePool[i].m_cSomes.size() - genePool[i].m_leftOver < shortest) // If there is a new shortest gene
				{
					shortest = genePool[i].m_cSomes.size() - genePool[i].m_leftOver; // Replace 'shortest' with the new shortest value
				}
			}

			genePool[i].m_id = i + 1;
			genePool[i].m_fitness /= fitnessTotal; // Its fitness becomes a number between 0-1 (To calculate its odds of selection)			
			if (i != 0) // Then add the fitness to the previous gene's fitness. This puts the parts of the 'roulette wheel' on top of eachother.
			{
				genePool[i].m_fitness += genePool[i - 1].m_fitness;
			}
		}

		
		if (shortest != lastShortest) // If the shortest path has changed
		{
			// This is to ensure the winning gene on the first run also gets trimmed
			
			for (int i = shortest; i < cSomeSize; i++)
			{
				winGene.m_cSomes.pop_back(); // Chop off un-necessary chromosomes
			}
			

			for (int j = 0; j < genePool.size(); j++)
			{
				for (int i = shortest; i < cSomeSize; i++)
				{
					genePool[j].m_cSomes.pop_back(); // Chop off un-necessary chromosomes
				}
			}
			cSomeSize = shortest;
			lastRefine = generation;
			lastShortest = shortest;
		}
	
		if (bestRoute) // If we are now looking for the best route because we've finished refining
		{
			return winGene;
		}

		for (int g = 0; g < genePool.size(); g += 2) // Do until all the children have been made
		{
			float r = distribution(generator); // Spin the wheel

											   //Pick two parents and save them into the parents vector
			for (int i = 0; i < genePool.size(); i++)  // Check which gene is selected as parent 1
			{
				if (r <= genePool[i].m_fitness) //When the first parent is picked
				{
					parents[0] = genePool[i]; // Save the parent

					bool selfMate = true; // A variable that causes the loop to repeat until it doesn't pick itself as its mate
					while (selfMate)
					{
						float r2 = distribution(generator);
						for (int j = 0; j < genePool.size(); j++) // Find the second parent
						{
							if (r2 <= genePool[j].m_fitness)
							{
								if (j == i) { break; } // You chose yourself. Foolish Narcissus.
								else
								{
									selfMate = false;
									parents[1] = genePool[j];  // Save the second chosen parent
									break;
								}
							}
						}
					}
					crossOver(parents, generator, distribution); // Breed the parents
					children[g] = parents[0]; // Save the result as a child
					children[g + 1] = parents[1]; // ^
					break; // Make more children, or end the loop if that's all
				}
			}
		}
		genePool = children; // Replace the population with the children


		if (success && !refine)
		{
			longPath = winGene.m_cSomes.size();
			char option;
			std::cout << "I have found a path using " << shortest << "moves. Type 'Y' to refine, or 'N' to end" << std::endl;
			std::cin >> option;
			if (option == 'Y' || option == 'y')
			{
				refine = true; // Start the refining process
				lastRefine = generation;
			}
		}
	}
	return winGene;
}

void Genetics::movePlayer(Entity& _player, int _direction, Map &_map)
{
	int nextX = _player.m_xPos;
	int	nextY = _player.m_yPos;


	switch (_direction)
	{
	case 1: //Left
		nextX--;
		break;
	case 2: // Right
		nextX++;
		break;
	case 3: // Up
		nextY--;
		break;
	case 4: // Down
		nextY++;
		break;
	case 5:
		nextY--;
		nextX--;
		break;
	case 6:
		nextY++;
		nextX--;
		break;

	case 7:
		nextY++;
		nextX++;
		break;

	case 8:
		nextY--;
		nextX++;
		break;
	}

	_player.m_xPos = nextX;
	_player.m_yPos = nextY;
}



void Genetics::movePlayer(Entity& _player, Gene &_path, Map &_map, std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution)
{
	int nextX;
	int	nextY, lastX, lastY;
	bool cornered = false;
	lastX = _player.m_xPos;
	lastY = _player.m_yPos;

	for (int i = 0; i < _path.m_cSomes.size(); i++)
	{
		
		bool collideWall;
		int moves = 0;
		
		do // This loop keeps repeating until the next move doesn't hit a wall
		{
			collideWall = false;
			nextX = _player.m_xPos;
			nextY = _player.m_yPos;
			switch (_path.m_cSomes[i])
			{
			case 1: //Left
				nextX--;
				break;
			case 2: // Right
				nextX++;
				break;
			case 3: // Up
				nextY--;
				break;
			case 4: // Down
				nextY++;
				break;
			case 5:
				nextY--;
				nextX--;
				break;
			case 6:
				nextY++;
				nextX--;
				break;

			case 7:
				nextY++;
				nextX++;
				break;

			case 8:
				nextY--;
				nextX++;
				break;
			}

			if (nextX >= _map.m_gridX || nextX < 0 || nextY >= _map.m_gridY || nextY < 0) // Don't go off the grid
			{
				collideWall = true;
			}

			// Don't go back to the last square
			else if (nextX == lastX && nextY == lastY)
			{
				collideWall = true;
			}

			// If we don't go off-grid, check if we go into a wall
			else 
			{
				for (int w = 0; w < _map.m_walls.size(); w++) // Check against the walls. Don't move if we collide.
				{
					if (nextX == _map.m_walls[w]->m_xPos && nextY == _map.m_walls[w]->m_yPos)
					{
						collideWall = true;
						break;
					}
				}
			}

			if (collideWall) // If we hit a wall, 
			{
				moves++;
				_path.m_cSomes[i] = Genetics::genCsome(_generator, _distribution); // Replace the chromosome with another random one
			}
			if (moves == 30) // We should've found a valid move by now. If not, we're cornered
			{
				cornered = true;
				break;
			}
			
		} while (collideWall); // Repeat until the next move is a valid one

		_path.m_leftOver--; // Tally down how many moves are left in the chromosome
		if (cornered)
		{
			_path.m_leftOver = 0;
			break;
		}
		lastX = _player.m_xPos;
		lastY = _player.m_yPos;
		_player.m_xPos = nextX;
		_player.m_yPos = nextY;
		if (_player.m_xPos == _map.m_end.m_xPos && _player.m_yPos == _map.m_end.m_yPos) // If we hit the end
		{
			break;
		}
		
	}
}

float Genetics::handleGene(Entity &_player, std::vector<Gene> &_genePool, Map &_map, std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution, int _tNo, bool &_success)
{
	float fitnessSum = 0.0f;
	for (int i = (_genePool.size() / NUM_THREADS) * (_tNo - 1); i < (_genePool.size() / NUM_THREADS) * _tNo; i++) // Check each gene 
	{
		_genePool[i].m_leftOver = _genePool[i].m_cSomes.size(); // Re-set the leftover value.
																// This value counts down after each chromosome to see how many we have left over if one finds the answer. 
																//This will be used to 'chop off' excess cSomes in order to find the more efficient method

		_genePool[i].m_hitGoal = false;
		Entity nextMove = _player; // Make a copy of the player, moving the actual player would cause complications

		Genetics::movePlayer(nextMove, _genePool[i], _map, _generator, _distribution); // Move according to the whole gene
		if (nextMove.m_xPos == _map.m_end.m_xPos && nextMove.m_yPos == _map.m_end.m_yPos)
		{
			_genePool[i].m_hitGoal = true;
			_success = true;
		}
		int cSqrd = pow(_map.m_end.m_xPos - nextMove.m_xPos, 2) + pow(_map.m_end.m_yPos - nextMove.m_yPos, 2); // Pythagoras to find the distance
		float distance = sqrt(cSqrd);
		_genePool[i].m_fitness = 1.0f / (distance + 1.0f);	
		_genePool[i].m_fitness += (_genePool[i].m_leftOver / 5.0f);
		

		fitnessSum += _genePool[i].m_fitness;
	}
	return fitnessSum;
}



//Generates a gene with a size equal to gSize
std::vector<int> Genetics::genGene(std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution, int _gSize)
{
	std::vector<int> x;

	for (int i = 0; i < _gSize; i++)
	{
		x.push_back(genCsome(_generator, _distribution));
	}
	return x;
}


//Generates a single chromosome (A number between 1-8)
int Genetics::genCsome(std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution)
{

	double r = _distribution(_generator);


	int n;
	if (r < 0.125)
	{
		n = 1;
	}
	else if (r < 0.25)
	{
		n = 2;
	}
	else if (r < 0.375)
	{
		n = 3;
	}
	else if (r < 0.5)
	{
		n = 4;
	}
	else if (r < 0.625)
	{
		n = 5;
	}
	else if (r < 0.75)
	{
		n = 6;
	}
	else if (r < 0.875)
	{
		n = 7;
	}
	else if (r <= 1.0)
	{
		n = 8;
	}
	return n;
}

void Genetics::crossOver(std::vector<Gene> &_parents, std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution)
{
	float crossRate = 0.7f;
	float mutateRate = 0.02f;
	int temp;
	float rand = _distribution(_generator);
	if (rand <= crossRate) // Check for crossover
	{
		for (int i = 0; i < _parents[0].m_cSomes.size(); i += 2)
		{			
			temp = _parents[0].m_cSomes[i];
			_parents[0].m_cSomes[i] = _parents[1].m_cSomes[i];
			_parents[1].m_cSomes[i] = temp;
		}
	}

	for (int i = 0; i < 2; i++) // Do for both parents
	{
		for (int j = 0; j < _parents[i].m_cSomes.size(); j++)  // For each chromosome 
		{
			float rand2 = _distribution(_generator);
			if (rand2 <= mutateRate) // Check for mutation
			{
				_parents[i].m_cSomes[j] = Genetics::genCsome(_generator, _distribution);
			}
		}
	}
}

