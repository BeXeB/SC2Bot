//
// Created by marco on 07/11/2024.
//

#include "Mcts.h"

#include <chrono>
#include <random>
#include <ranges>

#include "Sc2State.h"
using namespace Sc2::Mcts;
using namespace std::chrono;


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
double Mcts::getMaxNodeValue(const std::map<Action, std::shared_ptr<Node> > &nodes) const {
	const auto firstNode = nodes.begin()->second;
	auto bestNodeValue = value(*firstNode);

	for (const auto &node: nodes | std::views::values) {
		const auto nodeValue = value(*node);
		if (nodeValue > bestNodeValue) {
			bestNodeValue = nodeValue;
		}
	}

	return bestNodeValue;
}

std::vector<std::shared_ptr<Node> > Mcts::getMaxNodes(std::map<Action, std::shared_ptr<Node> > &children,
                                                      const double maxValue) const {
	std::vector<std::shared_ptr<Node> > maxNodes = {};

	for (const auto &child: std::ranges::views::values(children)) {
		if (value(*child) == maxValue) {
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

void Mcts::singleSearch() {
	auto [node, state] = selectNode();
	const auto outcome = rollout(state);
	backPropagate(node, outcome);
}

void Mcts::search(const int timeLimit) {
	const auto endTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch())
	                     .count() + timeLimit;

	while (duration_cast<milliseconds>(system_clock::now().time_since_epoch())
	       .count() < endTime) {
		singleSearch();
	}
}

// Upper confidence bound applied to trees
// Q/N + C * (sqrt(log(parent.N/N)
double Mcts::uct(Node node) const {
	return node.Q / static_cast<float>(node.N) + EXPLORATION * sqrt(
		       log(static_cast<double>(node.getParent()->N) / static_cast<double>(node.N)));
}

double Mcts::value(const Node &node) const {
	if (node.N == 0) {
		if (EXPLORATION == 0) {
			return 0;
		}

		return INFINITY;
	}
	switch (_valueHeuristic) {
		case ValueHeuristic::UCT:
			return uct(node);
		default:
			return 0;
	}
}

void Mcts::searchRollout(const int rollouts) {
	for (int i = 0; i < rollouts; i++) {
		singleSearch();
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
}

Action Mcts::getBestAction() {
	const auto maxValue = getMaxNodeValue(_rootNode->children);
	const auto maxNodes = getMaxNodes(_rootNode->children, maxValue);

	const auto bestNode = randomChoice(maxNodes);

	return bestNode->getAction();
}
