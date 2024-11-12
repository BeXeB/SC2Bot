//
// Created by marco on 07/11/2024.
//
#include <Mcts.h>

#include "doctest.h"
#include "Node.h"
#include "Sc2state.h"

using namespace Sc2::Mcts;

TEST_SUITE("Test MCTS") {
	TEST_CASE("Can create a Node") {
		auto node = std::make_shared<Node>(Action::wait, nullptr);
		CHECK(node->N == 0);
		CHECK(node->children.size() == 0);

		// auto childNode = Node(Action::buildBase, std::make_shared<Node>(node));
		const std::vector possibleActions = {Action::wait, Action::buildWorker, Action::buildBase};

		node->addChildren(possibleActions);

		CHECK(node->children.size() == 3);

		for (auto const &[action, child]: node->children) {
			CHECK((child->getParent()) == node);
		}
	}

	TEST_CASE("Can create an MCTS") {
		const auto state = std::make_shared<Sc2::State>();

		auto mcts = new Mcts(state);

		auto node1 = mcts->selectNode();

		std::cout << *node1.first << std::endl;
		auto node2 = mcts->selectNode();

		std::cout << *node2.first << std::endl;
		std::cout << *mcts->getRootNode() << std::endl;


	}
}

