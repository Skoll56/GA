#include <vector>
#include <random>


class Gene
{
public:
	Gene(std::vector<int> _cSomes, int _id)
	{
		m_cSomes = _cSomes;
		m_id = _id;
	}
	Gene() {};

	int m_id;
	float m_fitness;
	int m_leftOver;
	bool m_hitGoal;
	std::vector<int> m_cSomes;
};

class Entity;
class Map;

namespace Genetics
{
	Gene geneticPathfind(Entity  _player, Map &_map);
	void crossOver(std::vector<Gene> &_parents, std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution);
	void movePlayer(Entity &_player, int _direction, Map &_map);
	void movePlayer(Entity &_player, Gene &_path, Map &_map, std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution);
	float handleGene(Entity &_player, std::vector<Gene> &_genePool, Map &_map, std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution, int _tNo, bool &_success);
	void randomTest(std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution, int _iterations);
	int genCsome(std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution);
	std::vector<int> genGene(std::default_random_engine &_generator, std::uniform_real_distribution<float> &_distribution, int _gSize);
}
