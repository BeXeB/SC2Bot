//
// Created by marco on 04/11/2024.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "Sc2State.h"
#include "Enemy.h"

#include <iostream>


TEST_SUITE("Test the Sc2State") {
	TEST_CASE("Check that the correct resources are gathered when waiting") {
		const auto state = std::make_shared<Sc2::State>();
		state->wait(100);

		auto currentVespene = state->getVespene();
		auto currentMinerals = state->getMinerals();

		auto waitTime = 50;
		auto mineralWorkers = state->getMineralWorkers();
		auto vespeneWorkers = state->getVespeneWorkers();

		state->wait(waitTime);
		CHECK(state->getMinerals() == currentMinerals + (mineralWorkers * MINERAL_PER_WORKER * waitTime));
		CHECK(state->getVespene() == currentVespene + (vespeneWorkers * VESPENE_PER_WORKER * waitTime));
	}

	TEST_CASE("Check that the correct resources are taken when an action is performed") {
		const auto state = std::make_shared<Sc2::State>();
		state->wait(150);

		auto currentMinerals = state->getMinerals();
		auto currentVespene = state->getVespene();
		SUBCASE(
			"Check that build house takes resources as cost, "
			"and occupies a worker") {
			state->buildHouse();

			currentMinerals = currentMinerals - state->getBuildHouseCost().minerals;
			CHECK(state->getMinerals() == currentMinerals);

			currentVespene = currentVespene - state->getBuildHouseCost().vespene;
			CHECK(state->getVespene() == currentVespene);
		}
		state->wait(50);
		SUBCASE(
			"Check that build base takes resources as cost, "
			"and occupies a worker") {
			currentMinerals = state->getMinerals();
			currentVespene = state->getVespene();

			state->buildBase();
			currentMinerals = currentMinerals - state->getBuildBaseCost().minerals;
			CHECK(state->getMinerals() == currentMinerals);

			currentVespene = currentVespene - state->getBuildBaseCost().vespene;
			CHECK(state->getVespene() == currentVespene);
		}
		state->wait(50);
		SUBCASE(
			"Check that build vespene collector takes resources as cost, "
			"and occupies a worker") {
			currentMinerals = state->getMinerals();
			currentVespene = state->getVespene();

			state->buildVespeneCollector();
			currentMinerals = currentMinerals - state->getBuildVespeneCollectorCost().minerals;
			CHECK(state->getMinerals() == currentMinerals);

			currentVespene = currentVespene - state->getBuildVespeneCollectorCost().vespene;
			CHECK(state->getVespene() == currentVespene);
		}
		state->wait(50);
		SUBCASE("Check build worker takes resources") {
			currentMinerals = state->getMinerals();
			currentVespene = state->getVespene();

			state->buildWorker();

			currentMinerals = currentMinerals - state->getBuildWorkerCost().minerals;
			CHECK(state->getMinerals() == currentMinerals);

			currentVespene = currentVespene - state->getBuildWorkerCost().vespene;
			CHECK(state->getVespene() == currentVespene);
		}
	}

	TEST_CASE("Legal constructions will eventually be performed even if they cannot initially be afforded") {
		const auto state = std::make_shared<Sc2::State>();
		state->setMinerals(0);

		SUBCASE("will eventually build worker even if it cannot initially be afforded") {
			auto initialPopulation = state->getPopulation();
			CHECK(state->getMinerals() < state->getBuildWorkerCost().minerals);

			state->buildWorker();
			CHECK(state->getPopulation() + state->getIncomingPopulation() > initialPopulation);
		}

		SUBCASE("will eventually build vespene collector even if it cannot initially be afforded") {
			auto initialConstructionSize = state->getConstructions().size();
			CHECK(state->getMinerals() < state->getBuildVespeneCollectorCost().minerals);

			state->buildVespeneCollector();

			CHECK(initialConstructionSize < state->getConstructions().size());
		}

		SUBCASE("will eventually build house even if it cannot initially be afforded") {
			auto initialConstructionSize = state->getConstructions().size();
			CHECK(state->getMinerals() < state->getBuildHouseCost().minerals);

			state->buildHouse();

			CHECK(initialConstructionSize < state->getConstructions().size());
		}

		SUBCASE("will eventually build base even if it cannot initially be afforded") {
			auto initialConstructionSize = state->getConstructions().size();
			CHECK(state->getMinerals() < state->getBuildBaseCost().minerals);

			state->buildBase();

			CHECK(initialConstructionSize < state->getConstructions().size());
		}
	}

	TEST_CASE("Test that anything in the build queue will eventually be built") {
		const auto state = std::make_shared<Sc2::State>();
		state->wait(200);

		SUBCASE("Check that workers can gather vespene after a delay when vespene collectors are built") {
			CHECK(state->getVespeneWorkers() == 0);
			state->buildVespeneCollector();

			for (int i = 0; i < state->getBuildVespeneCollectorCost().buildTime; i++) {
				CHECK(state->getVespeneWorkers() == 0);
				state->wait();
			}

			CHECK(state->getVespeneWorkers() != 0);
		}

		state->buildVespeneCollector();
		state->wait(50);
		SUBCASE("Check that population increases after a delay when workers are built") {
			auto workers = state->getPopulation();

			CHECK(state->getMinerals() >= state->getBuildWorkerCost().minerals);
			CHECK(state->getVespene() >= state->getBuildWorkerCost().vespene);
			state->buildWorker();

			// check that the worker is not added to the population before the build time is over
			for (int i = 0; i < state->getBuildWorkerCost().buildTime; i++) {
				CHECK(state->getPopulation() == workers);
				state->wait();
			}

			CHECK(state->getPopulation() == workers + 1);
		}
		SUBCASE("Check that the population cap increase after a delay when houses are built") {
			auto limit = state->getPopulationLimit();
			state->buildHouse();

			// check that the population limit is not increased before the build time is over
			for (int i = 0; i < state->getBuildHouseCost().buildTime; i++) {
				CHECK(state->getPopulationLimit() == limit);
				state->wait();
			}

			CHECK(state->getPopulationLimit() == limit + 8);
		}
		SUBCASE("Check that the population cap increase after a delay when bases are built") {
			auto limit = state->getPopulationLimit();
			state->buildBase();

			// check that the population limit is not increased before the build time is over
			for (int i = 0; i < state->getBuildBaseCost().buildTime; i++) {
				CHECK(state->getPopulationLimit() == limit);
				state->wait();
			}

			CHECK(state->getPopulationLimit() == limit + 15);
		}
	}

	TEST_CASE("Test that you can not build more workers than the population cap") {
		const auto state = std::make_shared<Sc2::State>();
		state->wait(100);
		state->buildVespeneCollector();
		state->wait(400);
		const auto workersToBuild = state->getPopulationLimit() + 100;

		for (auto i = 0; i < workersToBuild; i++) {
			CHECK(state->getMinerals() >= state->getBuildWorkerCost().minerals);
			CHECK(state->getVespene() >= state->getBuildWorkerCost().vespene);
			state->buildWorker();
		}

		state->wait(100);

		CHECK(state->getPopulation() == state->getPopulationLimit());
	}

	TEST_CASE("Test deep copy of states") {
		const auto state1 = std::make_shared<Sc2::State>();

		state1->wait(50);
		state1->buildVespeneCollector();
		state1->wait(50);
		state1->buildHouse();
		state1->buildWorker();

		const auto state2 = Sc2::State::DeepCopy(*state1);

		CHECK(state1->getPopulationLimit() == state2->getPopulationLimit());
		CHECK(state1->getPopulation() == state2->getPopulation());
		CHECK(state1->getMinerals() == state2->getMinerals());
		CHECK(state1->getVespene() == state2->getVespene());
		CHECK(state1->getIncomingPopulation() == state2->getIncomingPopulation());
		CHECK(state1->getBases().size() == state2->getBases().size());
		CHECK(state1->getVespeneWorkers() == state2->getVespeneWorkers());
	}

	TEST_CASE("Test that copies of a state does not share any part of the state") {
		const auto state1 = std::make_shared<Sc2::State>();

		state1->wait(500);
		state1->buildVespeneCollector();
		state1->wait(100);
		state1->buildHouse();
		state1->buildWorker();
		state1->buildBase();
		state1->buildVespeneCollector();
		state1->buildWorker();

		const auto state2 = Sc2::State::DeepCopy(*state1);
		state1->id = 0;
		state2->id = 1;
		state1->wait(500);
		SUBCASE("Check that state 1 is different from state2") {
			CHECK(state1->getPopulationLimit() != state2->getPopulationLimit());
			CHECK(state1->getOccupiedPopulation() != state2->getOccupiedPopulation());
			CHECK(state1->getPopulation() != state2->getPopulation());
			CHECK(state1->getMinerals() != state2->getMinerals());
			CHECK(state1->getVespene() != state2->getVespene());
			CHECK(state1->getIncomingPopulation() != state2->getIncomingPopulation());
			CHECK(state1->getBases().size() != state2->getBases().size());
			CHECK(state1->getVespeneWorkers() != state2->getVespeneWorkers());
		}
		state2->wait(500);
		SUBCASE("Check that if the same actions are taken they will become identical") {
			CHECK(state1->getPopulationLimit() == state2->getPopulationLimit());
			CHECK(state1->getOccupiedPopulation() == state2->getOccupiedPopulation());
			CHECK(state1->getPopulation() == state2->getPopulation());
			CHECK(state1->getMinerals() == state2->getMinerals());
			CHECK(state1->getVespene() == state2->getVespene());
			CHECK(state1->getIncomingPopulation() == state2->getIncomingPopulation());
			CHECK(state1->getBases().size() == state2->getBases().size());
			CHECK(state1->getVespeneWorkers() == state2->getVespeneWorkers());
		}
	}

	TEST_CASE("The state can build barracks and marines") {
		const auto state = std::make_shared<Sc2::State>();

		auto initialPopulation = state->getPopulation();
		state->buildHouse();
		state->wait(100);

		SUBCASE("Cannot build marine without a barracks") {
			state->wait(500);
			state->buildMarine();
			state->wait(500);
			CHECK(state->getPopulation() == initialPopulation);
		}

		SUBCASE("Can build a Barracks and a marine") {
			state->buildBarracks();
			state->wait(100);
			state->buildMarine();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation + 1);
		}

		SUBCASE("Can build a Barracks and a marine") {
			state->buildBarracks();
			state->wait(100);
			state->buildMarine();
			state->wait(500);
			CHECK(state->getPopulation() == initialPopulation + 1);
		}
		SUBCASE("building a marine while the barracks are occupied will advance time until the barracks is available") {
			state->buildBarracks();
			state->wait(100);
			state->buildMarine();
			state->buildMarine();
			CHECK(state->getPopulation() == initialPopulation + 1);
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation + 2);
		}
	}

	TEST_CASE("Test state can build Factory, FactoryTechLabs and Tanks") {
		const auto state = std::make_shared<Sc2::State>();

		auto initialPopulation = state->getPopulation();
		state->buildHouse();
		state->wait(100);
		state->buildVespeneCollector();
		state->wait(100);

		SUBCASE("Cannot build Factory without Barracks") {
			state->buildFactory();
			state->wait(100);
			CHECK(state->getFactoryAmount() == 0);
		}

		state->buildBarracks();
		state->wait(100);

		SUBCASE("Cannot build Tank without Factory") {
			state->buildTank();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation);
		}

		SUBCASE("Can build Factory") {
			state->buildFactory();
			state->wait(100);
			CHECK(state->getFactoryAmount() == 1);
		}

		state->buildFactory();
		state->wait(100);

		SUBCASE("Can build Tank") {
			state->buildTank();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation + TANK_SUPPLY);
		}

		while (state->getPopulationLimit() - state->getPopulation() > TANK_SUPPLY) {
			state->buildWorker();
			state->wait(100);
		}

		initialPopulation = state->getPopulation();

		SUBCASE("Can build Tank when there is just enough supply") {
			state->buildTank();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation + TANK_SUPPLY);
		}

		state->buildWorker();
		state->wait(100);
		initialPopulation = state->getPopulation();

		SUBCASE("Cannot build Tank if there is not enough supply") {
			state->buildTank();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation);
		}
	}

	TEST_CASE("Test state can build StarPort and Viking") {
		const auto state = std::make_shared<Sc2::State>();

		auto initialPopulation = state->getPopulation();
		state->buildHouse();
		state->wait(100);
		state->buildVespeneCollector();
		state->wait(100);
		state->buildBarracks();
		state->wait(100);

		SUBCASE("Cannot build StarPort without Factory") {
			state->buildStarPort();
			state->wait(100);
			CHECK(state->getStarPortAmount() == 0);
		}

		SUBCASE("Cannot build Viking without StarPort") {
			state->buildViking();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation);
		}

		state->buildFactory();
		state->wait(100);

		SUBCASE("Can build StarPort") {
			state->buildStarPort();
			state->wait(100);
			CHECK(state->getStarPortAmount() == 1);
		}

		state->buildStarPort();
		state->wait(100);

		SUBCASE("Can build Viking") {
			state->buildViking();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation + VIKING_SUPPLY);
		}

		while (state->getPopulationLimit() - state->getPopulation() > VIKING_SUPPLY) {
			state->buildWorker();
			state->wait(100);
		}

		initialPopulation = state->getPopulation();

		SUBCASE("Can build Viking if there is just enough supply") {
			state->buildViking();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation + VIKING_SUPPLY);
		}

		state->buildWorker();
		state->wait(100);
		initialPopulation = state->getPopulation();

		SUBCASE("Cannot build Viking if there is not enough supply") {
			state->buildViking();
			state->wait(100);
			CHECK(state->getPopulation() == initialPopulation);
		}
	}

	TEST_CASE("Test that enemy units are correctly added") {
		const auto state = std::make_shared<Sc2::State>();

		CHECK(state->getEnemyCombatUnits() == 0);
		state->addEnemyUnit();
		state->addEnemyUnit();
		state->addEnemyUnit();
		CHECK(state->getEnemyCombatUnits() == 3);
		Sc2::Enemy e = Sc2::Enemy(Sc2::EnemyRace::Terran, 0);
		auto a = e.generateEnemyAction();
		auto b = e;
		e.generateEnemyAction();
	}

	TEST_CASE("Test Enemy") {
		auto enemy = Sc2::Enemy(Sc2::EnemyRace::Terran, 1);

		SUBCASE("Enemies can add production buildings") {
			auto initialProduction = enemy.productionBuildings;
			enemy.takeAction(500, Sc2::EnemyAction::addEnemyProduction);

			bool productionIncreased = false;
			for (auto [type,building]: initialProduction) {
				productionIncreased = enemy.productionBuildings[type].amount > building.amount;
				if (productionIncreased) {
					break;
				}
			}
			CHECK(productionIncreased);
		}
		SUBCASE("Enemies can add units") {
			enemy.productionBuildings[Sc2::ProductionBuildingType::Barracks].amount = 1;
			auto initialUnits = enemy.units;
			auto initialEnemyCombatUnits = enemy.enemyCombatUnits;
			enemy.takeAction(500, Sc2::EnemyAction::addEnemyUnit);

			bool unitsIncreased = false;
			for (auto [unit, amount]: enemy.units) {
				unitsIncreased = initialUnits[unit] < amount;
				if (unitsIncreased) {
					break;
				}
			}
			CHECK(unitsIncreased);
			CHECK(enemy.enemyCombatUnits > initialEnemyCombatUnits);
		}
		SUBCASE("Enemies can add ground and air production") {
			auto initialAirProduction = enemy.airProduction;
			auto initialGroundProduction = enemy.groundProduction;
			enemy.takeAction(500, Sc2::EnemyAction::addEnemyAirProduction);
			enemy.takeAction(500, Sc2::EnemyAction::addEnemyGroundProduction);

			CHECK(enemy.groundProduction > initialGroundProduction);
			CHECK(enemy.airProduction > initialAirProduction);
		}
		SUBCASE("Enemies can add ground and air power") {
			auto initialGroundPower = enemy.groundPower;
			auto initialAirPower = enemy.airPower;
			enemy.airProduction = 1;
			enemy.groundProduction = 1;
			enemy.takeAction(500, Sc2::EnemyAction::addEnemyAirPower);
			enemy.takeAction(500, Sc2::EnemyAction::addEnemyGroundPower);

			CHECK(enemy.groundPower > initialGroundPower);
			CHECK(enemy.airPower > initialAirPower);
		}
		SUBCASE("Copied enemies will take the same actions") {
			enemy.takeAction(500);
			enemy.takeAction(500);

			auto enemy2 = enemy;
			
			for (int i = 0; i < 50; i++) {
				auto action = enemy.takeAction(500);
				auto action2 = enemy2.takeAction(500);
				CHECK(action == action2);
			}
		}
	}

	/*
	TEST_CASE("Enemy units can properly attack player") {
		const auto state = std::make_shared<Sc2::State>();

		auto initialBases = state->getBases().size();
		auto initialPopulationLimit = state->getPopulationLimit();
		auto initialWorkers = state->getWorkerPopulation();

		SUBCASE("If the player has no marines they will lose the fight and lose a base") {
			state->addEnemyUnit();
			state->addEnemyUnit();
			state->addEnemyUnit();

			state->attackPlayer();
			CHECK(initialBases -1 == state->getBases().size());
			CHECK(initialPopulationLimit - 15 == state->getPopulationLimit());
			CHECK(initialWorkers - 3 == state->getWorkerPopulation());
		}

		SUBCASE("If the enemy has no troops they will lose the fight and the player will not lose a base") {
			state->attackPlayer();
			CHECK(initialBases == state->getBases().size());
		}

		SUBCASE("When the enemy attacks the player, they both lose units") {
			state->buildBarracks();
			state->wait(500);
			for (int i = 0; i < 50; i++) {
				state->buildMarine();
				state->addEnemyUnit();
			}
			state->wait(500);

			auto marines = state->getMarinePopulation();
			auto enemies = state->getEnemyCombatUnits();

			state->attackPlayer();

			CHECK(marines > state->getMarinePopulation());
			CHECK(enemies > state->getEnemyCombatUnits());
		}

		SUBCASE("If the player has no base, they will only lose workers") {
			state->addEnemyUnit();
			state->addEnemyUnit();
			state->addEnemyUnit();

			CHECK(state->getBases().size() == 1);
			CHECK(initialWorkers == state->getWorkerPopulation());

			state->attackPlayer();

			CHECK(state->getBases().empty());
			CHECK(initialWorkers - 3 == state->getWorkerPopulation());

			state->attackPlayer();

			CHECK(state->getBases().empty());
			CHECK(initialWorkers - 6 != state->getWorkerPopulation());
			CHECK(0 == state->getWorkerPopulation());
		}
	}
	*/
}

