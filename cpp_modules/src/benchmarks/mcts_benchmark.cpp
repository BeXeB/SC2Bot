//
// Created by marco on 25/11/2024.
//

#include <iostream>
#include <chrono>

#include "Mcts.h"
#include "Sc2State.h"
using namespace Sc2::Mcts;
using namespace std::chrono;

void printRootNode(const std::shared_ptr<Node> &rootNode) {
	std::cout << "-------ROOT-------\n"
			<< *rootNode << std::endl;

	//print children
	for (const auto &[action, node]: rootNode->children) {
		std::cout << "---CHILD---\n"
				<< *node << std::endl;
	}
}

int benchmark(const int benchmarkIndex, const unsigned int seed, const int numberOfActions, const int numberOfRollouts,
              const int rolloutDepth, const double exploration, const ValueHeuristic valueHeuristic,
              const RolloutHeuristic rolloutHeuristic, const bool shouldPrintActions = false) {
	const auto state = std::make_shared<Sc2::State>();

	const auto mcts = new Mcts(state, seed, rolloutDepth, exploration, valueHeuristic, rolloutHeuristic);

	std::cout << "MCTS Benchmark " << benchmarkIndex << ": {" << std::endl
			<< "\t" << "Seed: " << seed << std::endl
			<< "\t" << "Number of actions: " << numberOfActions << std::endl
			<< "\t" << "Number of rollouts: " << numberOfRollouts << std::endl
			<< "\t" << "Exploration: " << exploration << std::endl
			<< "\t" << "Rollout depth: " << rolloutDepth << std::endl
			<< "\t" << "Value heuristic: " << valueHeuristic << std::endl
			<< "\t" << "Rollout heuristic: " << rolloutHeuristic << std::endl
			<< "}" << std::endl;

	for (auto i = 0; i < numberOfActions; i++) {
		mcts->updateRootState(state);

		mcts->searchRollout(numberOfRollouts);
		const Action action = mcts->getBestAction();

		if (shouldPrintActions)
			std::cout << "Action: " << action << ", Index: " << i << std::endl;

		state->performAction(action);
	}
	const int stateValue = state->getValue();
	std::cout << "Benchmark " << benchmarkIndex << " State value: " << stateValue << std::endl << std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;

	return stateValue;
}

void dumbBenchmark() {
	const auto state = std::make_shared<Sc2::State>();

	for (auto i = 0; i < 20; i++) {
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildHouse);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildBase);

		state->performAction(Action::buildWorker);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildVespeneCollector);
		state->performAction(Action::buildBase);
	}

	std::cout << "State 2 value: " << state->getValue() << std::endl;
}

void printResults(const std::vector<int> &results) {
	std::cout << "Results: " << std::endl;

	for (int i = 0; i < results.size(); i++) {
		std::cout << "Benchmark " << i + 1 << " | " << results[i] << std::endl;
	}
}

int main() {
	constexpr unsigned int seed = 3942438306;
	// const unsigned int seed = std::random_device()();
	int numberOfActions = 50;
	int numberOfRollouts = 100;//2000;
	int rolloutDepth = 100;
	double exploration = sqrt(2);
	auto valueHeuristic = ValueHeuristic::UCT;
	auto rolloutHeuristic = RolloutHeuristic::Random;
	std::vector<int> results;

	int result;

	// Benchmark 1: UCT with Random as the rollout heuristic
	/*result = benchmark(1, seed, numberOfActions, numberOfRollouts, rolloutDepth, exploration, valueHeuristic,
	                   rolloutHeuristic);
	results.push_back(result);

	// Benchmark 2: UCT with weighted choice as the rollout heuristic
	rolloutHeuristic = RolloutHeuristic::WeightedChoice;

	result = benchmark(2, seed, numberOfActions, numberOfRollouts, rolloutDepth, exploration, valueHeuristic,
	                   rolloutHeuristic, true);
	results.push_back(result);*/

	// Benchmark 3: UCB1-Normal2 with random as the rollout heuristic
	valueHeuristic = ValueHeuristic::UcbNormal;
	rolloutHeuristic = RolloutHeuristic::Random;
	result = benchmark(3, seed, numberOfActions, numberOfRollouts, rolloutDepth, exploration, valueHeuristic,
						rolloutHeuristic);
	results.push_back(result);

	// Print all the results of our benchmarks
	printResults(results);
}
