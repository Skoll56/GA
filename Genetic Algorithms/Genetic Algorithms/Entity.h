#include <vector>

struct Colour
{
	Colour() {};
	Colour(int _r, int _g, int _b, int _a)
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	int r;
	int g;
	int b;
	int a;
};

struct Entity
{
	Entity() {};
	Entity(int _xPos, int _yPos)
	{
		m_xPos = _xPos;
		m_yPos = _yPos;
		m_col = Colour(150, 150, 150, 255);
	}
	int m_xPos, m_yPos;
	Colour m_col;
};

struct Map
{
	Map(int _gridX, int _gridY)
	{
		m_gridX = _gridX;
		m_gridY = _gridY;
	}
	std::vector<Entity*> m_walls;
	Entity m_end;
	int m_gridX, m_gridY;
	Entity m_start;
};
