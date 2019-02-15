#include <string>
class Map;

std::string astar(Map &_map);

std::string pathFind(const int & xStart, const int & yStart,
	const int & xFinish, const int & yFinish);