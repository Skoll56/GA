#include "MapLoader.h"
#include "Entity.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

Map readFile(std::string _location)
{
	std::vector<char> mapChars;
	std::vector<std::string> value;
	std::ifstream file;
	std::vector<int> mapFile;
	file.open(_location);

	if (file.is_open())
	{
		while (!file.eof())
		{
			std::string t;
			std::getline(file, t, ' '); // Reads the line, saves it as a string. 
			value.push_back(t);
		}
	}
	else { throw std::exception(); }
	file.close();

	for (int i = 0; i < value.size(); i++)
	{
		int t = cutSpace(value[i]);
		mapFile.push_back(t);	//After this, we have a vector with all the map parts in int form
	}	
	
	Map map(mapFile[0], mapFile[1]);

	int location = 2; // Start at 2 in order to ignore the first two places

	for (int y = 0; y < mapFile[1]; y++) // This is the vertical
	{
		for (int x = 0; x < mapFile[0]; x++) // Horizontal
		{
			switch (mapFile[location])
			{
			case 1:
				map.m_walls.push_back(new Entity(x, y));
				break;
			case 2:
				map.m_start = Entity(x, y);
				break;
			case 3:
				map.m_end = Entity(x, y);
				map.m_end.m_col = Colour(0, 0, 255, 255);
				break;

			default: // Whitespace
				break;
			}
			location++;
		}
	}
	return map;
}

//Cuts out any whitespace and returns just the pure numbers
int cutSpace(std::string &_str)
{
	std::vector<char> num;
	std::string temp;

	for (int i = 0; i < _str.size(); i++)
	{
		switch (_str[i])
		{
		case '1': 
			num.push_back(_str[i]);
			break;
		case '2':
			num.push_back(_str[i]);
			break;
		case '3':
			num.push_back(_str[i]);
			break;
		case '4':
			num.push_back(_str[i]);
			break;
		case '5':
			num.push_back(_str[i]);
			break;
		case '6':
			num.push_back(_str[i]);
			break;
		case '7':
			num.push_back(_str[i]);
			break;
		case '8':
			num.push_back(_str[i]);
			break;
		case '9':
			num.push_back(_str[i]);
			break;
		case '0':
			num.push_back(_str[i]);
			break;

		default: // Not a valid character. Doesn't get recorded.
			break;
		}
	}
	temp = num[0]; // We now have a string holding the first digit (or the only digit)

	if (num.size() == 2) // If a two-digit number
	{
		temp += num[1];
	}
	std::stringstream s(temp);
	int val;
	s >> val;
	return val;
}
