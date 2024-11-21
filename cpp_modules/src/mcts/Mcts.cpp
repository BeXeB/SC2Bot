//
// Created by marco on 07/11/2024.
//

#include "Mcts.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <ranges>

#include "Sc2State.h"
using namespace Sc2::Mcts;


std::shared_ptr<Node> Mcts::randomChoice(const std::map<Action, std::shared_ptr<Node> > &nodes) {
	if (nodes.empty()) {
		throw std::runtime_error("Cannot select a random element from an empty container.");
	}

	std::uniform_int_distribution<std::mt19937::result_type> dist(0, nodes.size() - 1);

	auto iter = nodes.begin();

	std::advance(iter, dist(_rng));

	return iter->second;
}

template<typename Container>
auto Mcts::randomChoice(const Container &container) -> decltype(*std::begin(container)) {
	if (container.empty()) {
		throw std::runtime_error("Cannot select a random element from an empty container.");
	}

	// Get a random index
	std::uniform_int_distribution<std::mt19937::result_type> dist(
		0, std::distance(container.begin(), container.end()) - 1);

	// Advance the iterator to the random index
	auto it = container.begin();
	std::advance(it, dist(_rng));
	return *it;
}

//Find the highest value of the nodes
double Mcts::getMaxNodeValue(const std::map<Action, std::shared_ptr<Node> > &nodes) {
	auto [action, node] = *std::ranges::max_element(nodes,
	                                                [](const std::pair<Action, std::shared_ptr<Node> > &a,
	                                                   const std::pair<Action, std::shared_ptr<Node> > &b) {
		                                                return a.second->value() > b.second->value();
	                                                });

	return node->value();
}

std::vector<std::shared_ptr<Node> > Mcts::getMaxNodes(std::map<Action, std::shared_ptr<Node> > &children,
                                                      const double maxValue) {
	std::vector<std::shared_ptr<Node> > maxNodes = {};

	for (const auto &child: std::ranges::views::values(children)) {
		if (child->value() == maxValue) {
			maxNodes.emplace_back(child);
		}
	}
	return maxNodes;
}


Mcts::NodeStatePair Mcts::selectNode() {
	auto state = State::DeepCopy(*_rootState);
	auto node = _rootNode;

	while (!node->children.empty()) {
		const double maxValue = getMaxNodeValue(node->children);

		std::vector<std::shared_ptr<Node> > maxNodes = getMaxNodes(node->children, maxValue);

		node = randomChoice(maxNodes);
		state->performAction(node->getAction());

		if (node->N == 0) {
			return {node, state};
		}
	}

	node->expand(state);
	node = randomChoice(node->children);
	state->performAction(node->getAction());

	return {node, state};
}

int Mcts::rollout(const std::shared_ptr<State> &state) {
	for (int i = 0; i <= MAX_DEPTH; i++) {
		auto legalActions = state->getLegalActions();

		const auto action = randomChoice(legalActions);

		state->performAction(action);
	}

	return state->getValue();
}

void Mcts::backPropagate(std::shared_ptr<Node> node, const int outcome) {
	while (node != nullptr) {
		node->N += 1;
		node->Q += outcome;
		node = node->getParent();
	}
}

void Mcts::search(const int timeLimit) {
	const auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(timeLimit);

	while (std::chrono::steady_clock::now() < endTime) {
		auto [node, state] = selectNode();
		const auto outcome = rollout(state);
		backPropagate(node, outcome);
	}
}

void Mcts::performAction(Action action) {
	// Check if the action matches any explored nodes
	for (const auto childAction: _rootNode->children | std::views::keys) {
		if (childAction == action) {
			_rootState->performAction(action);
			_rootNode = _rootNode->children[action];
			return;
		}
	}

	// If the action is not explored it will still be performed, but the root node will be reset.
	_rootState->performAction(action);
	_rootNode = std::make_shared<Node>(Action::none, nullptr);
}

Action Mcts::getBestAction() {
	const auto maxValue = getMaxNodeValue(_rootNode->children);
	const auto maxNodes = getMaxNodes(_rootNode->children, maxValue);

	const auto bestNode = randomChoice(maxNodes);

	return bestNode->getAction();
}
