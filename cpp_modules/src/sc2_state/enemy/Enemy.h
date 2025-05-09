//
// Created by marco on 01/05/2025.
//

#ifndef ENEMY_H
#define ENEMY_H
// #include <ActionEnum.h>
#include <random>
#include <stdexcept>
#include <string>
#include <map>
#include <optional>
#include <utility>
#include "UnitTypes.h"
#include "ProductionBuildings.h"
namespace Sc2 {
	enum class EnemyRace {
		Zerg,
		Protoss,
		Terran,
	};

	enum class EnemyAction {
		none,
		addEnemyUnit,
		addEnemyGroundPower,
		addEnemyAirPower,
		addEnemyGroundProduction,
		addEnemyAirProduction,
		addEnemyProduction,
		attackPlayer
	};
	struct Enemy {
		EnemyRace race = EnemyRace::Terran;
		std::map<EnemyUnitType, int> units = {};

		std::unordered_map<ProductionBuildingType, ProductionBuilding> productionBuildings = {};
		int groundPower = 0;
		double groundProduction = 1;
		int airPower = 0;
		double airProduction = 0;
		int enemyCombatUnits = 0;

		EnemyAction generateEnemyAction();
		EnemyAction takeAction(int currentTime, std::optional<EnemyAction> action = std::nullopt);

		void initializeUnits() {
			// Iterate through each value in the enum
			for (int i = static_cast<int>(EnemyUnitType::COLOSSUS); i != static_cast<int>(EnemyUnitType::Last); i++ ) {
				units[static_cast<EnemyUnitType>(i)] = 0;
			}
		}

		Enemy(const EnemyRace race,
			  std::map<EnemyUnitType, int> units,
			  std::unordered_map<ProductionBuildingType, ProductionBuilding> productionBuildings,
			  const unsigned int seed)
			: race(race), units(std::move(units)), productionBuildings(std::move(productionBuildings)) {
			_rng = std::mt19937(seed);
		}


		Enemy(const EnemyRace race,
		      const std::map<std::string, int> &units,
		      const std::map<ProductionBuildingType, int> &productionBuildings)
			: race(race), units(convertToEnum(units)), productionBuildings(convertToProductionBuildings(productionBuildings)) {
			_rng = std::mt19937(std::random_device{}());
		}


		Enemy(const EnemyRace race, const unsigned int seed):race(race) {
			initializeUnits();
			switch (race) {
				case EnemyRace::Terran:
					productionBuildings[ProductionBuildingType::Barracks] = BarracksProductionBuilding(0);
				productionBuildings[ProductionBuildingType::Starport] = StarportProductionBuilding(0);
				productionBuildings[ProductionBuildingType::Factory] = FactoryProductionBuilding(0);
				break;
				case EnemyRace::Zerg:
					productionBuildings[ProductionBuildingType::Hatchery] = HatcheryProductionBuilding(1);
				productionBuildings[ProductionBuildingType::Spire] = SpireProductionBuilding(0);
				productionBuildings[ProductionBuildingType::BanelingNest] = BanelingNestProductionBuilding(0);
				productionBuildings[ProductionBuildingType::HydraliskDen] = HydraliskProductionBuilding(0);
				productionBuildings[ProductionBuildingType::InfestationPit] = InfestationPitProductionBuilding(0);
				productionBuildings[ProductionBuildingType::LurkerDen] = LurkerDenProductionBuilding(0);
				productionBuildings[ProductionBuildingType::RoachWarren] = RoachWarrenProductionBuilding(0);
				productionBuildings[ProductionBuildingType::SpawningPool] = SpawningPoolProductionBuilding(0);
				productionBuildings[ProductionBuildingType::UltraliskCavern] = UltraliskCavernProductionBuilding(0);
				break;
				case EnemyRace::Protoss:
					productionBuildings[ProductionBuildingType::Gateway] = GatewayProductionBuilding(0);
				productionBuildings[ProductionBuildingType::Stargate] = StargateProductionBuilding(0);
				productionBuildings[ProductionBuildingType::RoboticsFacility] = RoboticsFacilityProductionBuilding(0);
				break;
			}
			_rng = std::mt19937(seed);
		}

		Enemy(const int groundPower, const int groundProduction, const int airPower, const int airProduction)
			: groundPower(groundPower), groundProduction(groundProduction), airPower(airPower), airProduction(airProduction) {
			_rng = std::mt19937(std::random_device{}());
		}

		Enemy(const Enemy& enemy) {
			_rng = enemy._rng;
			race = enemy.race;
			units = enemy.units;
			enemyCombatUnits = enemy.enemyCombatUnits;
			productionBuildings = enemy.productionBuildings;
			groundPower = enemy.groundPower;
			groundProduction = enemy.groundProduction;
			airPower = enemy.airPower;
			airProduction = enemy.airProduction;
		}

		Enemy() {
			race = EnemyRace::Terran;
			initializeUnits();
			productionBuildings[ProductionBuildingType::Barracks] = BarracksProductionBuilding(0);
			productionBuildings[ProductionBuildingType::Starport] = StarportProductionBuilding(0);
			productionBuildings[ProductionBuildingType::Factory] = FactoryProductionBuilding(0);
		}
	private:
		std::mt19937 _rng;

		void addEnemyUnit() { enemyCombatUnits += 1; }
		void addEnemyGroundPower() { groundPower += std::floor(groundProduction); }
		void addEnemyAirPower() { airPower += std::floor(airProduction); }
		void addEnemyGroundProduction() { groundProduction += 0.1; }
		void addEnemyAirProduction() { airProduction += 0.1; }
		void addProductionBuilding(int currentTime);
		void addUnits();

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
	};
}
#endif //ENEMY_H
