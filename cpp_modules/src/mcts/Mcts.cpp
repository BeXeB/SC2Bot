//
// Created by marco on 07/11/2024.
//

#include "Mcts.h"

#include <chrono>
#include <complex>
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

	while (!node->children.empty()) {
		std::vector<std::shared_ptr<Node> > maxNodes = getMaxNodes(node->children);

		node = randomChoice(maxNodes);

		if (node->N == 0) {
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
		const auto oldMean = node->Q / node->N;

		node->N += 1;
		node->Q += outcome;

		const auto newMean = node->Q / node->N;
		const auto delta = outcome - oldMean;
		// M2 is updated using Welfords online algorithm
		node->M2 += delta * (outcome - newMean);

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

// Upper confidence bound normalized
double Mcts::ucb1Normal2(const std::shared_ptr<Node> &node) {

	// If the node has not been explored at least twice we will divide by 0 when getting the variance
	if (node->N < 2) {
		return INFINITY;
	}

	const double totalTrials = node->getParent()->N;
	const double trials = node->N;
	const auto mean = node->Q / trials;
	const auto variance = node->getSampleVariance();

	// const double value = mean + std::sqrt(2 * std::log(totalTrials) / trials) * std::sqrt(node->variance);
	const double value = mean + variance + sqrt(2 * std::log(totalTrials));
	return value;
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
			return ucb1Normal2(node);
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



