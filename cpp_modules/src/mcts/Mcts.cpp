//
// Created by marco on 07/11/2024.
//

#include <chrono>
#include <complex>
#include <random>
#include <ranges>

#include "Mcts.h"
#include "Sc2State.h"

using namespace Sc2::Mcts;
using namespace std::chrono;


std::shared_ptr<Node> Mcts::randomChoice(const std::map<Action, std::shared_ptr<Node> > &nodes) {
	if (nodes.empty()) {
		throw std::runtime_error("Cannot select a random node from an empty container.");
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
	if (container.size() == 1) {
		return *std::begin(container);
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
	_actionWeights.resize(actions.size());
	if (actions.empty()) {
		throw std::runtime_error("Cannot select a random action from an empty container.");
	}
	if (actions.size() == 1) {
		return actions[0];
	}

	for (int i = 0; i < actions.size(); ++i) {
		switch (actions[i]) {
			case Action::none:
				throw std::runtime_error("Cannot choose none as an action.");
			case Action::buildWorker:
				_actionWeights[i] = 22.0;
				break;
			case Action::buildHouse:
				_actionWeights[i] = 1.0;
				break;
			case Action::buildBase:
				_actionWeights[i] = 1.0;
				break;
			case Action::buildVespeneCollector:
				_actionWeights[i] = 2.0;
				break;
			case Action::buildBarracks:
				_actionWeights[i] = 3.0;
				break;
			case Action::buildMarine:
				_actionWeights[i] = 15.0;
				break;
			case Action::buildFactory:
				_actionWeights[i] = 2.0;
				break;
			case Action::buildTank:
				_actionWeights[i] = 5.0;
				break;
			case Action::buildFactoryTechLab:
				_actionWeights[i] = 2.0;
				break;
			case Action::buildViking:
				_actionWeights[i] = 5.0;
				break;
			case Action::buildStarPort:
				_actionWeights[i] = 2.0;
				break;
			default:
				throw std::runtime_error("Cannot choose " + actionToString(actions[i]) + " as an action.");;
		}
	}

	std::discrete_distribution<int> dist(_actionWeights.begin(), _actionWeights.end());
	const auto index = dist(_rng);

	return actions[index];
}

std::vector<std::shared_ptr<Node> > Mcts::getMaxNodes(std::map<Action, std::shared_ptr<Node> > &children) {
	if (children.empty()) {
		return {};
	}
	double maxValue = -INFINITY;
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

	if (!node->gameOver()) {
		node->expand();
		if (!node->children.empty()) {
			node = randomChoice(node->children);
		}
	}

	return node;
}


double Mcts::rollout(const std::shared_ptr<Node> &node) {
	const auto state = State::DeepCopy(*node->getState(), true);
	while (!state->GameOver()) {
		auto legalActions = state->getLegalActions();

		if (legalActions[0] == Action::none) {
			state->wait();
			continue;
		}

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

void Mcts::backPropagate(std::shared_ptr<Node> node, const double outcome) {
	while (node != nullptr) {
		const auto oldMean = node->N == 0 ? 0 : node->Q / node->N;

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
	_numberOfRollouts++;
}

void Mcts::threadedSearch() {
	while (_running) {
		if (!_mctsRequestsPending) {
			_mctsMutex.lock();
			singleSearch();
			_mctsMutex.unlock();
		}
	}
}

void Mcts::threadedSearchRollout(const int numberOfRollouts) {
	while (_numberOfRollouts < numberOfRollouts) {
		if (!_mctsRequestsPending) {
			_mctsMutex.lock();
			singleSearch();
			_mctsMutex.unlock();
		}
	}
}


void Mcts::stopSearchThread() {
	_running = false;
	_searchThread.join();
}

void Mcts::startSearchRolloutThread(int numberOfRollouts) {
	_running = true;
	_searchThread = std::thread(&Mcts::threadedSearchRollout, this, numberOfRollouts);
}

void Mcts::startSearchThread() {
	_running = true;
	_searchThread = std::thread(&Mcts::threadedSearch, this);
}

void Mcts::search(const int timeLimit) {
	const auto endTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch())
	                     .count() + timeLimit;

	while (duration_cast<milliseconds>(system_clock::now().time_since_epoch())
	       .count() < endTime) {
		singleSearch();
	}
}

void Mcts::searchRollout(const int rollouts) {
	for (int i = 0; i < rollouts; i++) {
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

	return mean + variance * sqrt(2 * std::log(totalTrials));
}

double Mcts::ucb1Normal(const std::shared_ptr<Node> &node) {
	if (node->N < 2) {
		return INFINITY;
	}
	const auto totalTrials = node->getParent()->N;
	const auto trials = node->N;
	const auto variance = node->getSampleVariance();
	const auto mean = node->Q / trials;
	return mean + variance * std::sqrt((16 * std::log(totalTrials - 1)) / trials);
}

double Mcts::epsilonGreedy(const std::shared_ptr<Node> &node) {
	if (node->N < 1) {
		return INFINITY;
	}

	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	if (dist(_rng) > EXPLORATION) {
		//exploit
		return node->Q / node->N;
	} else {
		// explore
		return INFINITY;
	}
}

double Mcts::value(const std::shared_ptr<Node> &node) {
	if (node->N == 0) {
		if (EXPLORATION == 0) {
			return 0;
		}

		return INFINITY;
	}
	switch (_valueHeuristic) {
		case ValueHeuristic::UCT:
			return uct(node);
		case ValueHeuristic::UCB1Normal2:
			return ucb1Normal2(node);
		case ValueHeuristic::UCB1Normal:
			return ucb1Normal(node);
		case ValueHeuristic::EpsilonGreedy:
			return epsilonGreedy(node);
		default:
			return 0;
	}
}


void Mcts::performAction(const Action action) {
	_mctsRequestsPending = true;
	_mctsMutex.lock();

	// Check if the action matches any explored nodes
	for (const auto childAction: _rootNode->children | std::views::keys) {
		if (childAction == action) {
			_rootNode = _rootNode->children[action];
			_rootNode->setParent(nullptr);
			_mctsMutex.unlock();
			_mctsRequestsPending = false;
			return;
		}
	}

	auto actions = _rootNode->getState()->getLegalActions();
	if (std::ranges::find(actions, action) != actions.end()) {
		_rootNode->getState()->performAction(action);
		_mctsMutex.unlock();
		_mctsRequestsPending = false;
		return;
	}
	std::cout << "action not found: " << action << std::endl;
	_mctsMutex.unlock();
	_mctsRequestsPending = false;
}

Action Mcts::getBestAction() {
	_mctsRequestsPending = true;
	_mctsMutex.lock();

	if (_rootNode->children.empty()) {
		_mctsMutex.unlock();
		_mctsRequestsPending = false;
		return Action::none;
	}

	auto bestNode = _rootNode->children.begin()->second;
	double maxValue = bestNode->Q / bestNode->N;
	std::vector<std::shared_ptr<Node> > maxNodes = {};

	for (const auto &child: std::ranges::views::values(_rootNode->children)) {
		// only give an action if all children has been explored once
		if (child->N < 1) {
			maxNodes.clear();
			break;
		}

		const auto childValue = child->Q / child->N;
		if (childValue > maxValue) {
			maxNodes.clear();
			maxNodes.push_back(child);
			maxValue = childValue;
		} else if (childValue == maxValue) {
			maxNodes.push_back(child);
		}
	}

	_mctsMutex.unlock();
	_mctsRequestsPending = false;

	if (maxNodes.empty()) {
		return Action::none;
	}

	bestNode = randomChoice(maxNodes);
	return bestNode->getAction();
}

void Mcts::updateRootState(const std::shared_ptr<State> &state) {
	_mctsRequestsPending = true;
	_mctsMutex.lock();
	_rootNode = std::make_shared<Node>(Node(Action::none, nullptr, State::DeepCopy(*state)));
	_rootNode->getState()->setBiases(_combatBiases);
	_rootNode->getState()->setEnemyActions(_enemyActions);
	_numberOfRollouts = 0;
	_mctsMutex.unlock();
	_mctsRequestsPending = false;
}
