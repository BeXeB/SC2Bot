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
		auto node = std::make_shared<Node>(Action::none, nullptr, ValueHeuristic::UCT);
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

		const auto mcts = new Mcts(state);

		mcts->search(1000);

		const auto bestMove = mcts->getBestAction();

		CHECK(bestMove != Action::none);
		std::cout << bestMove << std::endl;
	}


	TEST_CASE("Select Node will select a node that has not been fully explored") {
		const auto state = std::make_shared<Sc2::State>();

		const auto mcts = new Mcts(state);

		mcts->search(1000);

		auto [node, _] = mcts->selectNode();

		CHECK(node->N == 0);
		CHECK(node->children.size() == 0);
		CHECK(node->getDepth() > 1);
	}


	TEST_CASE("Expand will expand with all available actions in a state") {
		const auto rootState = std::make_shared<Sc2::State>();
		const auto mcts = new Mcts(rootState);

		SUBCASE(
			"Will expand to all actions, when there is available vespene geysers,"
			" and the population limit has not been reached") {
			mcts->search(1000);

			auto [node, state] = mcts->selectNode();

			state->wait(500);

			node->expand(state);

			CHECK(node->children.size() == 4);
		}

		SUBCASE("expand will not include build worker when the population limit is reached") {
			auto [node, state] = mcts->selectNode();

			for (auto i = 0; i < state->getPopulationLimit(); i++) {
				state->buildWorker();
			}
			state->wait(state->getBuildWorkerCost().buildTime);

			node->expand(state);

			for (const auto action: node->children | std::views::keys) {
				CHECK(action != Action::buildWorker);
			}
			CHECK(node->children.size() == 3);
		}
		SUBCASE("expand will not include build vespene collector when there is no available geysers") {
			auto [node, state] = mcts->selectNode();
			auto availableGeysers = state->getVespeneGeysersAmount() - state->getVespeneCollectorsAmount();

			for (auto i = 0; i < availableGeysers; i++) {
				state->buildVespeneCollector();
			}

			state->wait(state->getBuildVespeneCollectorCost().buildTime);

			availableGeysers = state->getVespeneGeysersAmount() - state->getVespeneCollectorsAmount();
			CHECK(availableGeysers == 0);

			node->expand(state);
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

