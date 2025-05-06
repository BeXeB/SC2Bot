//
// Created by marco on 01/05/2025.
//

#ifndef ENEMY_H
#define ENEMY_H
#include <random>
#include <stdexcept>
#include <string>
#include <map>
#include <optional>
#include <utility>
#include "UnitTypes.h"
#include "ProductionBuildings.h"

enum class EnemyRace {
	Zerg,
	Protoss,
	Terran,
};
struct Enemy {
	EnemyRace race = EnemyRace::Terran;
	std::map<EnemyUnitType, int> units = {
		{EnemyUnitType::COLOSSUS,0},
		{EnemyUnitType::MOTHERSHIP,0},
		{EnemyUnitType::SIEGETANK,0},
		{EnemyUnitType::VIKINGFIGHTER,0},
		{EnemyUnitType::SCV,0},
		{EnemyUnitType::MARINE,0},
		{EnemyUnitType::REAPER,0},
		{EnemyUnitType::GHOST,0},
		{EnemyUnitType::MARAUDER,0},
		{EnemyUnitType::THOR,0},
		{EnemyUnitType::HELLION,0},
		{EnemyUnitType::MEDIVAC,0},
		{EnemyUnitType::BANSHEE,0},
		{EnemyUnitType::RAVEN,0},
		{EnemyUnitType::BATTLECRUISER,0},
		{EnemyUnitType::ZEALOT,0},
		{EnemyUnitType::STALKER,0},
		{EnemyUnitType::HIGHTEMPLAR,0},
		{EnemyUnitType::DARKTEMPLAR,0},
		{EnemyUnitType::SENTRY,0},
		{EnemyUnitType::PHOENIX,0},
		{EnemyUnitType::CARRIER,0},
		{EnemyUnitType::VOIDRAY,0},
		{EnemyUnitType::WARPPRISM,0},
		{EnemyUnitType::OBSERVER,0},
		{EnemyUnitType::IMMORTAL,0},
		{EnemyUnitType::PROBE,0},
		{EnemyUnitType::ARCHON,0},
		{EnemyUnitType::ADEPT,0},
		{EnemyUnitType::HELLIONTANK,0},
		{EnemyUnitType::ORACLE,0},
		{EnemyUnitType::TEMPEST,0},
		{EnemyUnitType::WIDOWMINE,0},
		{EnemyUnitType::LIBERATOR,0},
		{EnemyUnitType::CYCLONE,0},
		{EnemyUnitType::DISRUPTOR,0}
	};
	std::unordered_map<ProductionBuildingType, ProductionBuilding> productionBuildings = {};
	int groundPower = 0;
	float groundProduction = 0;
	int airPower = 0;
	float airProduction = 0;
	int enemyCombatUnit = 0;

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
	void addEnemyUnit() { enemyCombatUnit += 1; }
	void addEnemyGroundPower() { groundPower += std::floor(groundProduction); }
	void addEnemyAirPower() { airPower += std::floor(airProduction); }
	void addEnemyGroundProduction() { groundProduction += 0.1; }
	void addEnemyAirProduction() { airProduction += 0.1; }


	Action generateEnemyAction() {
		// Over the span of 60 seconds we assume that the enemy:
		// Specifies how many enemy units will be built
		constexpr double buildUnitAction = 8;
		// Specifies how much ground power the enemy gets per production
		constexpr double groundPowerIncrease = 5;
		// Specifies how much air power the enemy gets per production
		constexpr double airPowerIncrease = 5;
		// Specifies how much ground production the enemy builds
		constexpr double groundProductionIncrease = 3;
		// Specifies how much air production the enemy builds
		constexpr double airProductionIncrease = 3;
		// Specifies how many times the enemy will attack
		constexpr double attackAction = 0.3;
		// Specifies how many times the enemy will do nothing
		constexpr double noneAction = 60 - buildUnitAction - attackAction - groundPowerIncrease - airPowerIncrease -
									  groundProductionIncrease - airProductionIncrease;

		const auto actionWeights = {
			noneAction, buildUnitAction, attackAction, groundPowerIncrease, airPowerIncrease,
			groundProductionIncrease, airProductionIncrease
		};
		std::discrete_distribution<int> dist(actionWeights.begin(), actionWeights.end());
		// 0: None, 1: Build unit, 2: Attack, 3: GroundPowerIncrease, 4: AirPowerIncrease, 5: Ground Production, 6: Air Production
		switch (dist(_rng)) {
			case 1:
				return Action::addEnemyUnit;
			case 2:
				return Action::attackPlayer;
			case 3:
				return Action::addEnemyGroundPower;
			case 4:
				return Action::addEnemyAirPower;
			case 5:
				return Action::addEnemyGroundProduction;
			case 6:
				return Action::addEnemyAirProduction;
			default:
				return Action::none;
		}
	}
	Action takeAction(const int currentTime, std::optional<Action> action = std::nullopt) {
		if (!action) {
			action = generateEnemyAction();
		}
		switch (action.value()) {
			case Action::addEnemyUnit:
				if (currentTime < 90) {
					break;
				}
			break;
			case Action::attackPlayer:
			break;
			case Action::addEnemyGroundPower:
				if (currentTime < 90) {
					break;
				}
			addEnemyGroundPower();
			break;
			case Action::addEnemyAirPower:
				if (currentTime < 120) {
					break;
				}
			addEnemyAirPower();
			break;
			case Action::addEnemyGroundProduction:
				if (currentTime < 90) {
					break;
				}
			addEnemyGroundProduction();
			break;
			case Action::addEnemyAirProduction:
				if (currentTime < 120) {
					break;
				}
			addEnemyAirProduction();
			break;
			// case Action::addEnemyUnit:
				// addUnits();
			// case Action::addEnemyProduction:
				// addProductionBuilding();
			case Action::none:
				break;
			default:
				throw std::invalid_argument("invalid action");
		}
		return action.value();
	}





	Enemy(const EnemyRace race, std::map<EnemyUnitType, int> units, std::unordered_map<ProductionBuildingType,ProductionBuilding> productionBuildings, const unsigned int seed)
		: race(race), units(std::move(units)), productionBuildings(std::move(productionBuildings)) {
		_rng = std::mt19937(seed);
	}

    Enemy(const EnemyRace race, const unsigned int seed):race(race) {
      switch (race) {
      	case EnemyRace::Terran:
        	productionBuildings[ProductionBuildingType::Barracks] = BarracksProductionBuilding(0);
      		productionBuildings[ProductionBuildingType::Starport] = StarportProductionBuilding(0);
			productionBuildings[ProductionBuildingType::Factory] = FactoryProductionBuilding(0);
            break;
        case EnemyRace::Zerg:
          	break;
        case EnemyRace::Protoss:
          	break;
      }
		_rng = std::mt19937(seed);
    }

	Enemy(int groundPower, int groundProduction, int airPower, int airProduction, unsigned int seed)
		: groundPower(groundPower), groundProduction(groundProduction), airPower(airPower), airProduction(airProduction) {
		_rng = std::mt19937(seed);
	}

	Enemy(Enemy& enemy) {
		_rng = enemy._rng;
		race = enemy.race;
		units = enemy.units;
		enemyCombatUnit = enemy.enemyCombatUnit;
		productionBuildings = enemy.productionBuildings;
		groundPower = enemy.groundPower;
		groundProduction = enemy.groundProduction;
		airPower = enemy.airPower;
		airProduction = enemy.airProduction;
	}

	Enemy() = default;
	private:
	std::mt19937 _rng;

	void addProductionBuilding(const int currentTime) {
		std::vector<ProductionBuildingType> availableBuildings = {};
		for (const auto &[type, building]: productionBuildings) {
			if (building.timeRequirement > currentTime)
				continue;

			auto requiredBuilding = building.buildingRequirement;
			if (productionBuildings[requiredBuilding].amount < 1 ) {
				continue;
			}

			availableBuildings.emplace_back(type);
		}
		const auto building = randomChoice(availableBuildings);

		productionBuildings[building].amount += 0.1;
	}

	void addUnits(){
		std::vector<EnemyUnitType> availableUnits = {};
		std::unordered_map<EnemyUnitType, ProductionBuildingType> unitBuildings = {};

		for (auto & [buildingType,building]: productionBuildings) {
			if (building.amount >= 1) {
				auto productionList = building.getProductionList();
				for (auto unit: productionList) {
					availableUnits.emplace_back(unit);
					unitBuildings[unit] = buildingType;
				}
			}
		}
		const auto unit = randomChoice(availableUnits);
		const auto building = unitBuildings[unit];
		const auto buildingAmount = productionBuildings[building].amount;

		units[unit] += std::floor(buildingAmount);
	}

};
#endif //ENEMY_H
