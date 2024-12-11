//
// Created by marco on 07/11/2024.
//

#include "Mcts.h"

#include <chrono>
#include <random>
#include <ranges>
#include <thread>

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

Action Mcts::weightedChoice(const std::vector<Action> &actions) {
	std::vector<double> weights(actions.size());

	for (int i = 0; i < actions.size(); ++i) {
		switch (actions[i]) {
			case Action::none:
				throw std::runtime_error("Cannot choose none as an action.");
			case Action::buildWorker:
				weights[i] = 22.0;
				break;
			case Action::buildHouse:
				weights[i] = 1.0;
				break;
			case Action::buildBase:
				weights[i] = 1.0;
				break;
			case Action::buildVespeneCollector:
				weights[i] = 2.0;
				break;
		}
	}

	std::discrete_distribution<int> dist(weights.begin(), weights.end());
	const auto index = dist(_rng);
	return actions[index];
}

std::vector<std::shared_ptr<Node> > Mcts::getMaxNodes(std::map<Action, std::shared_ptr<Node> > &children) const {
	double maxValue = value(children.begin()->second);
	std::vector<std::shared_ptr<Node> > maxNodes = {};

	for (const auto &child: std::ranges::views::values(children)) {
		const auto childValue = value(child);
		if (childValue > maxValue) {
			maxNodes.clear();
			maxNodes.push_back(child);
			maxValue = childValue;
		} else if (childValue == maxValue) {
			maxNodes.push_back(child);
		}
	}
	return maxNodes;
}


std::shared_ptr<Node> Mcts::selectNode() {
	auto node = _rootNode;

	// Debug test for node
	if (node->children.empty()) {
		std::cout << "Expanding node because it has no children" << std::endl;
		node->expand();
		std::cout << "Note expanded. Number of children: " << node->children.size() << std::endl;
	}

	int iteration = 0;
	while (!node->children.empty()) {
		std::vector<std::shared_ptr<Node> > maxNodes = getMaxNodes(node->children);

		node = randomChoice(maxNodes);

		if (node->N == 0) {
			return node;
		}

		iteration++;
		if (iteration > 100) {
			std::cerr << "Too many iterations!" << std::endl;
			return node;
		}
	}

	node->expand();
	node = randomChoice(node->children);

	return node;
}


int Mcts::rollout(const std::shared_ptr<Node> &node) {
	const auto state = State::DeepCopy(*node->getState());
	for (int i = 0; i <= MAX_DEPTH; i++) {
		auto legalActions = state->getLegalActions();

		Action action;
		switch (_rolloutHeuristic) {
			case RolloutHeuristic::Random:
				action = randomChoice(legalActions);
				break;
			case RolloutHeuristic::WeightedChoice:
				action = weightedChoice(legalActions);
				break;
			default:
				throw std::runtime_error("Invalid rollout heuristic.");
		}

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
	const auto node = selectNode();
	const auto outcome = rollout(node);
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
double Mcts::uct(const std::shared_ptr<Node> &node) const {
	return node->Q / static_cast<float>(node->N) + EXPLORATION * sqrt(
		       log(static_cast<double>(node->getParent()->N) / static_cast<double>(node->N)));
}

double Mcts::value(const std::shared_ptr<Node> &node) const {
	if (node->N == 0) {
		if (EXPLORATION == 0) {
			return 0;
		}

		return INFINITY;
	}
	switch (_valueHeuristic) {
		case ValueHeuristic::UCT:
			return uct(node);
		case ValueHeuristic::UcbNormal:
			return ucb(node);
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
			_rootNode = _rootNode->children[action];
			_rootNode->setParent(nullptr);
			return;
		}
	}
}

Action Mcts::getBestAction() {
	const auto maxNodes = getMaxNodes(_rootNode->children);

	const auto bestNode = randomChoice(maxNodes);

	return bestNode->getAction();
}

void Mcts::updateRootState(const std::shared_ptr<State> &state) {
	_rootNode = std::make_shared<Node>(Node(Action::none, nullptr, State::DeepCopy(*state)));
}

double Mcts::ucb(const std::shared_ptr<Node> &node) const {
	std::cout << "Now evaluating UCB for node with " << node->children.size() << " children" << std::endl;

	if (node->children.empty()) {
		std::cout << "Node has no children" << std::endl;
		return -1;
	}

	// Debug for numarms
	if (node->children.size() <= 0) {
		throw std::runtime_error("Invalid number of arms");
	}
	std::cout << "Creating UcbNormal with " << node->children.size() << " arms" << std::endl;
	UCB1Normal2 ucbBandit(node->children.size());

	// Debug
	std::cout << "Checking children size: " << node->children.size() << std::endl;

	// Add rewards
	for (const auto& [action, child] : node->children) {
		std::cout << "Child: " << static_cast<int>(action) << ", N: " << child->N << ", Q: " << child->Q << std::endl;
		if (child->N > 0) {
			std::cout << "Adding reward for action " << action << std::endl;
			std::cout << "Reward is: " << child->Q / child->N << std::endl;

			// Ensure the arm index is valid (e.g., use the correct range for valid actions)
			int armIndex = static_cast<int>(action) - 1;  // Subtract 1 to match action's index in the map
			ucbBandit.addReward(armIndex, child->Q / child->N);
		} else {
			std::cout << "No reward for action " << action << std::endl;
		}
	}

	// Select the arm with the highest score
	int bestAction = ucbBandit.selectArm();
	std::cout << "Best action: " << bestAction << std::endl;

	// Return the best one
	return bestAction;
}


