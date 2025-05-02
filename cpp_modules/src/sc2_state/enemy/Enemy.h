//
// Created by marco on 01/05/2025.
//

#ifndef ENEMY_H
#define ENEMY_H
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

enum class EnemyRace {
	Zerg,
	Protoss,
	Terran,
};
class Enemy {
	EnemyRace race = EnemyRace::Terran;
	std::unordered_map<std::string, int> units = {};
	std::unordered_map<std::string, int> productionBuildings = {};
	int groundPower = 0;
	int groundProduction = 0;
	int airPower = 0;
	int airProduction = 0;
	template<typename Container>

	auto randomChoice(const Container &container) -> decltype(*std::begin(container)) {
		if (container.empty()) {
			throw std::runtime_error("Cannot select a random element from an empty container.");
		}
		if (container.size() == 1) {
			return *std::begin(container);
		}

		// Get a random index
		std::uniform_int_distribution<std::mt19937::result_type> dist(
			0, std::distance(container.begin(), container.end()) - 1);

		// Advance the iterator to the random index
		auto it = container.begin();
		std::advance(it, dist(_rng));
		return *it;
	}

	public:
	addProduction();
	addUnits();
	Enemy(const EnemyRace race, std::unordered_map<std::string, int> units, std::unordered_map<std::string, int> productionBuildings)
		: race(race), units(std::move(units)), productionBuildings(std::move(productionBuildings)) {  }
	Enemy(int groundPower, int groundProduction, int airPower, int airProduction)
		: groundPower(groundPower), groundProduction(groundProduction), airPower(airPower), airProduction(airProduction) {}
};
#endif //ENEMY_H
