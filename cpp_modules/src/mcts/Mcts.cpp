//
// Created by marco on 07/11/2024.
//

#include "Mcts.h"

#include <algorithm>
#include <random>

#include "Sc2State.h"
using namespace Sc2::Mcts;

//Find the highest value of the nodes
double Mcts::maxNodeValue(const std::map<Action, std::shared_ptr<Node> > &nodes) {
	auto [action, node] = *std::max_element(nodes.begin(), nodes.end(),
	                                        [](const std::pair<Action, std::shared_ptr<Node> > &a,
	                                           const std::pair<Action, std::shared_ptr<Node> > &b) {
		                                        return a.second->value() > b.second->value();
	                                        });

	return node->value();
}

std::vector<std::shared_ptr<Node> > Mcts::getMaxNodes(std::map<Action, std::shared_ptr<Node> > &children,
                                                      double maxValue) {
	std::vector<std::shared_ptr<Node> > maxNodes = {};

	for (const auto &[action, child]: children) {
		if (child->value() == maxValue) {
			maxNodes.emplace_back(child);
		}

	}
	return maxNodes;
}

std::shared_ptr<Node> Mcts::RandomChoice(const std::vector<std::shared_ptr<Node> > &nodes) {
	std::uniform_int_distribution<std::mt19937::result_type> dist(0, nodes.size() - 1);

	return nodes[dist(_rng)];
}

std::shared_ptr<Node> Mcts::RandomChoice(const std::map<Action, std::shared_ptr<Node> > &nodes) {
	std::uniform_int_distribution<std::mt19937::result_type> dist(0, nodes.size() - 1);

	auto iter = nodes.begin();

	std::advance(iter, dist(_rng));

	return iter->second;
}


Mcts::NodeStatePair Mcts::selectNode() {
	auto state = State::DeepCopy(*_rootState);
	auto node = _rootNode;

	while (!node->children.empty()) {

		const double maxValue = maxNodeValue(node->children);

		std::vector<std::shared_ptr<Node> > maxNodes = getMaxNodes(node->children, maxValue);

		node = RandomChoice(maxNodes);
		state->performAction(node->getAction());

		if (node->N == 0) {
			return {node, state};
		}
	}

	node->expand(state);
	node = RandomChoice(node->children);
	state->performAction(node->getAction());

	return {node, state};
}

void Mcts::expand(const std::shared_ptr<Node> &node, const std::shared_ptr<State> &state) {
	const std::vector<Action> actions = state->getLegalActions();
	node->addChildren(actions);
}
