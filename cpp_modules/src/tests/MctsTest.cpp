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


	TEST_CASE("Can create run an MCTS") {
		const auto state = std::make_shared<Sc2::State>();

		auto mcts = Mcts(state);

		mcts.search(1000);

		const auto bestMove = mcts.getBestAction();

		CHECK(bestMove != Action::none);

		SUBCASE("Can update the state in an MCTS using a new state") {
			const auto updatedState = std::make_shared<Sc2::State>();
			updatedState->performAction(Action::buildWorker);
			updatedState->wait(updatedState->getBuildWorkerCost().buildTime);

			mcts.updateRootState(updatedState);

			auto rootState = mcts.getRootState();

			CHECK(rootState->getPopulation() == updatedState->getPopulation());
			CHECK(rootState->getPopulation() != state->getPopulation());
		}
		SUBCASE("Can update the state in an MCTS using values of a state") {
			const auto updatedState = std::make_shared<Sc2::State>();
			updatedState->performAction(Action::buildWorker);
			updatedState->wait(updatedState->getBuildWorkerCost().buildTime);

			auto minerals = updatedState->getMinerals();
			auto vespene = updatedState->getVespene();
			auto incomingPopulation = updatedState->getIncomingPopulation();
			auto population = updatedState->getPopulation();
			auto populationLimit = updatedState->getPopulationLimit();
			auto occupiedWorkerTimers = updatedState->getOccupiedWorkerTimers();
			auto bases = updatedState->getBases();
			auto constructions = updatedState->getConstructions();

			mcts.updateRootState(minerals, vespene, population, incomingPopulation, populationLimit, bases,
			                     constructions, occupiedWorkerTimers);

			auto rootState = mcts.getRootState();

			CHECK(rootState->getPopulation() == updatedState->getPopulation());
			CHECK(rootState->getPopulation() != state->getPopulation());
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
			auto incomingPopulation = updatedState->getIncomingPopulation();
			auto population = updatedState->getPopulation();
			auto populationLimit = updatedState->getPopulationLimit();
			auto occupiedWorkerTimers = updatedState->getOccupiedWorkerTimers();
			auto bases = updatedState->getBases();
			auto constructions = updatedState->getConstructions();

			mcts.updateRootState(minerals, vespene, population, incomingPopulation, populationLimit, bases,
			                     constructions, occupiedWorkerTimers);

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


	TEST_CASE("Select Node will select a node that has not been fully explored") {
		const auto state = std::make_shared<Sc2::State>();

		auto mcts = Mcts(state);

		mcts.search(1000);

		auto node = mcts.selectNode();

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
			mcts.search(1000);

			auto node = mcts.selectNode();

			auto state = node->getState();

			state->wait(500);

			node->expand();

			CHECK(node->children.size() == 4);
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
			CHECK(node->children.size() == 3);
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
			CHECK(node->children.size() == 3);
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

