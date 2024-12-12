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

int benchmarkOnActions(const int benchmarkIndex, const unsigned int seed, const int numberOfActions,
                       const int numberOfRollouts,
                       const int rolloutEndTime, const double exploration, const ValueHeuristic valueHeuristic,
                       const RolloutHeuristic rolloutHeuristic, const bool shouldPrintActions = false) {
	auto state = std::make_shared<Sc2::State>(rolloutEndTime);

	const auto mcts = new Mcts(state, seed, rolloutEndTime, exploration, valueHeuristic, rolloutHeuristic);

	std::cout << "MCTS Benchmark " << benchmarkIndex << ": {" << std::endl
			<< "\t" << "Seed: " << seed << std::endl
			<< "\t" << "Number of actions: " << numberOfActions << std::endl
			<< "\t" << "Number of rollouts: " << numberOfRollouts << std::endl
			<< "\t" << "Exploration: " << exploration << std::endl
			<< "\t" << "Rollout depth: " << rolloutEndTime << std::endl
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
		// state->resetCurrentTime();
	}
	const int stateValue = state->getValue();
	std::cout << "Benchmark " << benchmarkIndex << " State value: " << stateValue << std::endl << std::endl
			<< "Current time of state: " << state->getCurrentTime() << std::endl << std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;

	return stateValue;
}

int benchmarkOnTime(const int benchmarkIndex, const unsigned int seed, const int numberOfRollouts,
                    const int endTime, const double exploration, const ValueHeuristic valueHeuristic,
                    const RolloutHeuristic rolloutHeuristic, const bool shouldPrintActions = false) {
	auto state = std::make_shared<Sc2::State>(endTime);

	const auto mcts = new Mcts(state, seed, endTime, exploration, valueHeuristic, rolloutHeuristic);

	std::cout << "MCTS Benchmark " << benchmarkIndex << ": {" << std::endl
			<< "\t" << "Seed: " << seed << std::endl
			<< "\t" << "Time to perform actions: " << endTime << std::endl
			<< "\t" << "Number of rollouts: " << numberOfRollouts << std::endl
			<< "\t" << "Exploration: " << exploration << std::endl
			<< "\t" << "Rollout depth: " << endTime << std::endl
			<< "\t" << "Value heuristic: " << valueHeuristic << std::endl
			<< "\t" << "Rollout heuristic: " << rolloutHeuristic << std::endl
			<< "}" << std::endl;

	int i = 0;
	while (!state->endTimeReached()) {
		mcts->updateRootState(state);

		mcts->searchRollout(numberOfRollouts);
		const Action action = mcts->getBestAction();

		if (shouldPrintActions)
			std::cout << "Action: " << action << ", Index: " << i << std::endl
					<< "Current time of state: " << state->getCurrentTime() << std::endl;


		state->performAction(action);
		i++;
	}
	const int stateValue = state->getValue();
	std::cout << "Benchmark " << benchmarkIndex << " State value: " << stateValue << std::endl << std::endl
			<< "Current time of state: " << state->getCurrentTime() << std::endl << std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;

	return stateValue;
}

void dumbBenchmark() {
	const auto state = std::make_shared<Sc2::State>(100);

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
	int numberOfRollouts = 2000;
	int rolloutEndTime = 500;
	double exploration = sqrt(2);
	auto valueHeuristic = ValueHeuristic::UCT;
	auto rolloutHeuristic = RolloutHeuristic::Random;
	std::vector<int> results;

	int result;

	// result = benchmarkOnActions(1, seed, numberOfActions, numberOfRollouts, rolloutEndTime, exploration, valueHeuristic,
	//                             rolloutHeuristic);
	// results.push_back(result);
	//
	// rolloutHeuristic = RolloutHeuristic::WeightedChoice;
	//
	//
	// result = benchmarkOnActions(2, seed, numberOfActions, numberOfRollouts, rolloutEndTime, exploration, valueHeuristic,
	//                             rolloutHeuristic);
	//
	// results.push_back(result);
	//
	// rolloutHeuristic = RolloutHeuristic::Random;
	// result = benchmarkOnTime(3, seed, numberOfRollouts, rolloutEndTime, exploration, valueHeuristic, rolloutHeuristic);
	// results.push_back(result);
	rolloutHeuristic = RolloutHeuristic::WeightedChoice;
	result = benchmarkOnTime(4, seed, numberOfRollouts, rolloutEndTime, exploration, valueHeuristic, rolloutHeuristic);
	results.push_back(result);

	printResults(results);
}
