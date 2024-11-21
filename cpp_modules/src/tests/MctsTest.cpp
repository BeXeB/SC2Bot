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
		auto node = std::make_shared<Node>(Action::none, nullptr);
		CHECK(node->N == 0);
		CHECK(node->children.size() == 0);

		SUBCASE("Can add children to a node") {
			// auto childNode = Node(Action::buildBase, std::make_shared<Node>(node));
			const std::vector possibleActions = {Action::none, Action::buildWorker, Action::buildBase};
			node->addChildren(possibleActions);
			CHECK(node->children.size() == 3);
		}

		const std::vector possibleActions = {Action::none, Action::buildWorker, Action::buildBase};
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

		mcts->search(1);

		const auto bestMove = mcts->getBestAction();

		CHECK(bestMove != Action::none);
	}

	TEST_CASE("Select Node will select a node that has not been fully explored") {
		const auto state = std::make_shared<Sc2::State>();

		const auto mcts = new Mcts(state);

		mcts->search(1);

		auto [node, _] = mcts->selectNode();

		CHECK(node->N == 0);
		CHECK(node->children.size() == 0);
		CHECK(node->getDepth() > 1);
	}

	TEST_CASE("Expand will expand with all available actions in a state") {
		const auto state = std::make_shared<Sc2::State>();

		SUBCASE("Will expand to all actions, when there is enough resources") {
			const auto mcts = new Mcts(state);
			mcts->search(1);

			auto [node, _] = mcts->selectNode();

			state->wait(500);

			node->expand(state);

			CHECK(node->children.size() == 5);
		}
		SUBCASE("Nodes will only expand to what there is enough resources for") {
			auto node = std::make_shared<Node>(Action::none, nullptr);
			node->expand(state);

			CHECK(node->children.size() == 1);
		}
	}

	// TEST_CASE("Rollout") {
	// 	const auto rootState = std::make_shared<Sc2::State>();
	// 	const auto mcts = new Mcts(rootState);
	// 	mcts->search(1);
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

