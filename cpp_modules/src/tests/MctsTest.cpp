//
// Created by marco on 07/11/2024.
//
#include <Mcts.h>
#include <ranges>

#include "doctest.h"
#include "Node.h"
#include "Sc2state.h"

using namespace Sc2::Mcts;

TEST_SUITE("Test MCTS") {
	TEST_CASE("Can create a Node") {
		const auto state = std::make_shared<Sc2::State>();
		auto node = std::make_shared<Node>(Action::none, nullptr, state);
		CHECK(node->N == 0);
		CHECK(node->children.size() == 0);

		SUBCASE("Can add children to a node") {
			const std::vector possibleActions = {Action::buildWorker, Action::buildBase};
			node->addChildren(possibleActions);
			CHECK(node->children.size() == possibleActions.size());
		}

		const std::vector possibleActions = {Action::buildWorker, Action::buildBase};
		node->addChildren(possibleActions);

		SUBCASE("The children of the nodes parent points to the correct object") {
			CHECK(node->children.size() > 0);
			for (auto const &child: std::ranges::views::values(node->children)) {
				CHECK((child->getParent()) == node);
			}
		}
	}


	TEST_CASE("Can create and run an MCTS") {
		SUBCASE("Can create and run an MCTS, by giving it a root state") {
			const auto state = std::make_shared<Sc2::State>();

			auto mcts = std::make_unique<Mcts>(state);

			mcts->searchRollout(1000);

			const auto bestMove = mcts->getBestAction();

			CHECK(bestMove != Action::none);
		}
		SUBCASE("Can create and run an MCTS with a default constructor") {
			auto mcts = std::make_unique<Mcts>();

			mcts->searchRollout(1000);

			const auto bestMove = mcts->getBestAction();

			CHECK(bestMove != Action::none);
		}
	}

	TEST_CASE("Can update the state correctly") {
		const auto state = std::make_shared<Sc2::State>();

		auto mcts = Mcts(state);

		SUBCASE("Can update the state in an MCTS using a new state") {
			const auto updatedState = std::make_shared<Sc2::State>();
			updatedState->performAction(Action::buildWorker);
			updatedState->performAction(Action::buildBase);
			updatedState->performAction(Action::buildWorker);
			updatedState->performAction(Action::buildVespeneCollector);
			updatedState->wait(updatedState->getBuildBaseCost().buildTime + 100);

			updatedState->performAction(Action::buildWorker);
			updatedState->performAction(Action::buildVespeneCollector);

			mcts.updateRootState(updatedState);

			auto rootState = mcts.getRootState();

			CHECK(rootState->getPopulation() == updatedState->getPopulation());
			CHECK(rootState->getPopulation() != state->getPopulation());

			CHECK(rootState->getBases().size() == updatedState->getBases().size());
			CHECK(rootState->getBases().size() != state->getBases().size());

			CHECK(rootState->getPopulation() == updatedState->getPopulation());
			CHECK(rootState->getPopulation() != state->getPopulation());

			CHECK(rootState->getVespeneCollectorsAmount() == updatedState->getVespeneCollectorsAmount());
			CHECK(rootState->getVespeneCollectorsAmount() != state->getVespeneCollectorsAmount());

			CHECK(rootState->getConstructions().size() == updatedState->getConstructions().size());
			CHECK(rootState->getConstructions().size() != state->getConstructions().size());
		}
		SUBCASE("Can update the state in an MCTS using values of a state") {
			const auto updatedState = std::make_shared<Sc2::State>();
			updatedState->performAction(Action::buildWorker);
			updatedState->performAction(Action::buildBase);
			updatedState->performAction(Action::buildWorker);
			updatedState->performAction(Action::buildVespeneCollector);
			updatedState->wait(updatedState->getBuildBaseCost().buildTime + 100);

			updatedState->performAction(Action::buildWorker);
			updatedState->performAction(Action::buildVespeneCollector);

			auto minerals = updatedState->getMinerals();
			auto vespene = updatedState->getVespene();
			auto workerPopulation = updatedState->getWorkerPopulation();
			auto marinePopulation = updatedState->getMarinePopulation();
			auto tankPopulation = updatedState->getTankPopulation();
			auto vikingPopulation = updatedState->getVikingPopulation();
			auto incomingWorkers = updatedState->getIncomingWorkers();
			auto incomingMarines = updatedState->getIncomingMarines();
			auto incomingTanks = updatedState->getIncomingTanks();
			auto incomingVikings = updatedState->getIncomingVikings();
			auto populationLimit = updatedState->getPopulationLimit();
			auto occupiedWorkerTimers = updatedState->getOccupiedWorkerTimers();
			auto bases = updatedState->getBases();
			auto barracksAmount = updatedState->getBarracksAmount();
			auto factoryAmount = updatedState->getFactoryAmount();
			// auto factoryTechLabAmount = updatedState->getFactoryTechLabAmount();
			auto starPortAmount = updatedState->getStarPortAmount();
			auto constructions = updatedState->getConstructions();
			auto enemyCombatUnits = updatedState->getEnemyCombatUnits();
			auto hasHouse = updatedState->getHasHouse();
			auto incomingHouse = updatedState->getIncomingHouse();
			auto incomingBarracks = updatedState->getIncomingBarracks();
			auto incomingFactory = updatedState->getIncomingFactory();
			// auto incomingFactoryTechLab = updatedState->getIncomingFactoryTechLab();
			auto incomingBases = updatedState->getIncomingBases();
			auto maxBases = updatedState->getMaxBases();

			mcts.updateRootState({
				.minerals = minerals,
				.vespene = vespene,
				.workerPopulation = workerPopulation,
				.marinePopulation = marinePopulation,
				.tankPopulation = tankPopulation,
				.vikingPopulation = vikingPopulation,
				.incomingWorkers = incomingWorkers,
				.incomingMarines = incomingMarines,
				.incomingTanks = incomingTanks,
				.incomingVikings = incomingVikings,
				.populationLimit = populationLimit,
				.bases = bases,
				.barracksAmount = barracksAmount,
				.factoryAmount = factoryAmount,
				// .factoryTechLabAmount = factoryTechLabAmount,
				.starPortAmount = starPortAmount,
				.constructions = constructions,
				.occupiedWorkerTimers = occupiedWorkerTimers,
				.currentTime = 0,
				.endTime = 1000,
				.enemyCombatUnits = enemyCombatUnits,
				.hasHouse = hasHouse,
				.incomingHouse = incomingHouse,
				.incomingBarracks = incomingBarracks,
				.incomingFactory = incomingFactory,
				// .incomingFactoryTechLab = incomingFactoryTechLab,
				.incomingBases = incomingBases,
				.maxBases = maxBases
			});

			auto rootState = mcts.getRootState();

			CHECK(rootState->getPopulation() == updatedState->getPopulation());
			CHECK(rootState->getPopulation() != state->getPopulation());

			CHECK(rootState->getBases().size() == updatedState->getBases().size());
			CHECK(rootState->getBases().size() != state->getBases().size());

			CHECK(rootState->getPopulation() == updatedState->getPopulation());
			CHECK(rootState->getPopulation() != state->getPopulation());

			CHECK(rootState->getVespeneCollectorsAmount() == updatedState->getVespeneCollectorsAmount());
			CHECK(rootState->getVespeneCollectorsAmount() != state->getVespeneCollectorsAmount());

			CHECK(rootState->getConstructions().size() == updatedState->getConstructions().size());
			CHECK(rootState->getConstructions().size() != state->getConstructions().size());
		}
	}

	TEST_CASE("State constructions can be generated with actions and times") {
		const auto state = std::make_shared<Sc2::State>();
		auto mcts = Mcts(state);

		state->performAction(Action::buildWorker);

		CHECK(mcts.getRootState()->getConstructions().size() != state->getConstructions().size());

		state->wait(1);

		auto minerals = state->getMinerals();
		auto vespene = state->getVespene();
		auto workerPopulation = state->getWorkerPopulation();
		auto marinePopulation = state->getMarinePopulation();
		auto tankPopulation = state->getTankPopulation();
		auto vikingPopulation = state->getVikingPopulation();
		auto incomingWorkers = state->getIncomingWorkers();
		auto incomingMarines = state->getIncomingMarines();
		auto incomingTanks = state->getIncomingTanks();
		auto incomingVikings = state->getIncomingVikings();
		auto populationLimit = state->getPopulationLimit();
		auto occupiedWorkerTimers = state->getOccupiedWorkerTimers();
		auto bases = state->getBases();
		auto barracksAmount = state->getBarracksAmount();
		auto factoryAmount = state->getFactoryAmount();
		// auto factoryTechLabAmount = state->getFactoryTechLabAmount();
		auto starPortAmount = state->getStarPortAmount();
		auto enemyCombatUnits = state->getEnemyCombatUnits();
		auto hasHouse = state->getHasHouse();
		auto incomingHouse = state->getIncomingHouse();
		auto incomingBarracks = state->getIncomingBarracks();
		auto incomingFactory = state->getIncomingFactory();
		// auto incomingFactoryTechLab = state->getIncomingFactoryTechLab();
		auto incomingBases = state->getIncomingBases();
		auto maxBases = state->getMaxBases();

		auto constructions = std::list<Sc2::Construction>();
		constructions.emplace_back(state->getBuildWorkerCost().buildTime - 1, Action::buildWorker);


		mcts.updateRootState({
				.minerals = minerals,
				.vespene = vespene,
				.workerPopulation = workerPopulation,
				.marinePopulation = marinePopulation,
				.tankPopulation = tankPopulation,
				.vikingPopulation = vikingPopulation,
				.incomingWorkers = incomingWorkers,
				.incomingMarines = incomingMarines,
				.incomingTanks = incomingTanks,
				.incomingVikings = incomingVikings,
				.populationLimit = populationLimit,
				.bases = bases,
				.barracksAmount = barracksAmount,
				.factoryAmount = factoryAmount,
				// .factoryTechLabAmount = factoryTechLabAmount,
				.starPortAmount = starPortAmount,
				.constructions = constructions,
				.occupiedWorkerTimers = occupiedWorkerTimers,
				.currentTime = 0,
				.endTime = 1000,
				.enemyCombatUnits = enemyCombatUnits,
				.hasHouse = hasHouse,
				.incomingHouse = incomingHouse,
				.incomingBarracks = incomingBarracks,
				.incomingFactory = incomingFactory,
				// .incomingFactoryTechLab = incomingFactoryTechLab,
				.incomingBases = incomingBases,
				.maxBases = maxBases
			});

		CHECK(mcts.getRootState()->getConstructions().size() == state->getConstructions().size());
		CHECK(mcts.getRootState()->getConstructions().size() == 1);
		CHECK(mcts.getRootState()->getConstructions().begin()->getTimeLeft() == state->getConstructions().begin()->
			getTimeLeft());
		CHECK(mcts.getRootState()->getConstructions().begin()->getTimeLeft() == mcts.getRootState()->getBuildWorkerCost(
		).buildTime - 1);

		SUBCASE("Constructions added with actions and times will eventually be built") {
			auto initialPopulation = mcts.getRootState()->getPopulation();
			mcts.getRootState()->wait(mcts.getRootState()->getBuildWorkerCost().buildTime);

			CHECK(mcts.getRootState()->getConstructions().size() == 0);
			CHECK(mcts.getRootState()->getPopulation() == initialPopulation + 1);
		}
	}


	TEST_CASE("Select Node will select a node that has not been fully explored") {
		const auto state = std::make_shared<Sc2::State>();

		auto mcts = std::make_unique<Mcts>(state);

		mcts->searchRollout(1000);

		auto node = mcts->selectNode();

		CHECK(node->N == 0);
		CHECK(node->children.size() == 0);
		CHECK(node->getDepth() > 1);
	}


	TEST_CASE("Expand will expand with all available actions in a state") {
		const auto rootState = std::make_shared<Sc2::State>();
		auto mcts = Mcts(rootState);

		SUBCASE(
			"Will expand to all actions, when there is available vespene geysers,"
			" and the population limit has not been reached") {
			mcts.searchRollout(1000);

			auto node = mcts.selectNode();

			auto state = node->getState();

			auto numberOfLegalActions = state->getLegalActions().size();

			node->expand();

			CHECK(node->children.size() == numberOfLegalActions);
		}

		SUBCASE("expand will not include build worker when the population limit is reached") {
			auto node = mcts.selectNode();
			auto state = node->getState();

			for (auto i = 0; i < state->getPopulationLimit(); i++) {
				state->buildWorker();
			}
			state->wait(state->getBuildWorkerCost().buildTime);

			node->expand();

			for (const auto action: node->children | std::views::keys) {
				CHECK(action != Action::buildWorker);
			}
			CHECK(node->children.size() == state->getLegalActions().size());
		}
		SUBCASE("expand will not include build vespene collector when there is no available geysers") {
			auto node = mcts.selectNode();
			auto state = node->getState();

			auto availableGeysers = state->getVespeneGeysersAmount() - state->getVespeneCollectorsAmount();

			for (auto i = 0; i < availableGeysers; i++) {
				state->buildVespeneCollector();
			}

			state->wait(state->getBuildVespeneCollectorCost().buildTime);

			availableGeysers = state->getVespeneGeysersAmount() - state->getVespeneCollectorsAmount();
			CHECK(availableGeysers == 0);

			node->expand();
			for (const auto action: node->children | std::views::keys) {
				CHECK(action != Action::buildVespeneCollector);
			}
			CHECK(node->children.size() == state->getLegalActions().size());
		}
	}


	// TEST_CASE("Rollout") {
	// 	const auto rootState = std::make_shared<Sc2::State>();
	// 	const auto mcts = new Mcts(rootState);
	// 	mcts->search(1000);
	// 	auto [node, state] = mcts->selectNode();
	//
	// 	auto stateValue = state->getValue();
	// 	auto rolledOutValue = mcts->rollout(state);
	//
	// 	// CHECK(rolledOutValue > stateValue);
	// }

	// TEST_CASE("Backpropagate") {
	//
	// }
}

