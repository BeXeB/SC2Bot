//
// Created by marco on 25/11/2024.
//

#include <iostream>
#include <chrono>

#include "Mcts.h"
#include "Sc2State.h"
using namespace Sc2::Mcts;
using namespace std::chrono;


struct BenchmarkParams {
	const int benchmarkIndex = 0;
	const unsigned int seed = 0;
	const int numberOfRollouts = 0;
	const int endTime = 0;
	const double exploration = 1;
	const ValueHeuristic valueHeuristic = ValueHeuristic::UCT;
	const RolloutHeuristic rolloutHeuristic = RolloutHeuristic::Random;
	bool shouldPrintActions = false;
};

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
	}
	const int stateValue = state->getValue();
	std::cout << "Benchmark " << benchmarkIndex << " State value: " << stateValue << std::endl << std::endl
			<< "Current time of state: " << state->getCurrentTime() << std::endl << std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;

	return stateValue;
}

float benchmarkOnTime(const BenchmarkParams &params) {
	auto state = std::make_shared<Sc2::State>(params.endTime);

	const auto mcts = new Mcts(state, params.seed, params.endTime, params.exploration, params.valueHeuristic,
	                           params.rolloutHeuristic);

	std::cout << "MCTS Benchmark " << params.benchmarkIndex << ": {" << std::endl
			<< "\t" << "Seed: " << params.seed << std::endl
			<< "\t" << "Time to perform actions: " << params.endTime << std::endl
			<< "\t" << "Number of rollouts: " << params.numberOfRollouts << std::endl
			<< "\t" << "Exploration: " << params.exploration << std::endl
			<< "\t" << "Rollout time: " << params.endTime << std::endl
			<< "\t" << "Value heuristic: " << params.valueHeuristic << std::endl
			<< "\t" << "Rollout heuristic: " << params.rolloutHeuristic << std::endl
			<< "}" << std::endl;

	int actionsTaken = 0;
	while (!state->endTimeReached()) {
		mcts->updateRootState(state);

		mcts->searchRollout(params.numberOfRollouts);
		const Action action = mcts->getBestAction();

		if (params.shouldPrintActions)
			std::cout << "--- Index: " << actionsTaken << " ---" << std::endl <<
					"Action: " << action << std::endl
					<< "Current time of state: " << state->getCurrentTime() << std::endl;


		state->performAction(action);
		actionsTaken++;
	}
	const int stateValue = state->getValue();
	std::cout << "Benchmark " << params.benchmarkIndex << " State value: " << stateValue << std::endl
			<< "number of actions taken: " << actionsTaken << std::endl
			<< "Current time of state: " << state->getCurrentTime() << std::endl
			<< "Final vespene: " << state->getVespene() << std::endl
			<< "Final minerals: " << state->getMinerals() << std::endl
			<< "Value per action: " << static_cast<float>(stateValue) / static_cast<float>(actionsTaken) << std::endl
			<< "value per second: " << static_cast<float>(stateValue) / static_cast<float>(state->getCurrentTime()) <<
			std::endl << std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;

	return static_cast<float>(stateValue);
}

void dumbBenchmark() {
	const auto state = std::make_shared<Sc2::State>(100);

	int actionsTaken = 0;
	for (auto i = 0; i < 8; i++) {
		actionsTaken += 10;
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

	std::cout << "dumb benchmark time taken: " << state->getCurrentTime() << std::endl;
	std::cout << "dumb benchmark actions taken: " << actionsTaken << std::endl;
	std::cout << "dumb benchmark value: " << state->getValue() << std::endl;
}

void heuristicBenchmark(int time, const unsigned int seed) {
	const std::vector<Action> actions{
		Action::buildWorker, Action::buildHouse, Action::buildBase, Action::buildVespeneCollector
	};
	std::vector<float> actionWeights{22.0, 1.0, 1.0, 2.0};

	std::discrete_distribution<int> dist(actionWeights.begin(), actionWeights.end());
	std::mt19937 rng(seed);


	int actionsTaken = 0;
	const auto state = std::make_shared<Sc2::State>(time);

	while (!state->endTimeReached()) {
		actionsTaken++;
		const auto index = dist(rng);
		const auto action = actions[index];

		state->performAction(action);
	}

	std::cout << "heuristic benchmark time taken: " << state->getCurrentTime() << std::endl;
	std::cout << "heuristic benchmark actions taken: " << actionsTaken << std::endl;
	std::cout << "heuristic benchmark value: " << state->getValue() << std::endl;
}

void printResults(const std::vector<float> &results) {
	std::cout << "Results: " << std::endl;

	for (int i = 0; i < results.size(); i++) {
		std::cout << "Benchmark " << i + 1 << " | " << results[i] << std::endl;
	}
}

float threadedMcts(const BenchmarkParams &params) {
	const auto state = std::make_shared<Sc2::State>(params.endTime);
	auto mcts = Mcts(state, params.seed, params.endTime, params.exploration, params.valueHeuristic,
	                 params.rolloutHeuristic);
	mcts.startSearchThread();
	int actionsTaken = 0;
	while (!state->endTimeReached()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto action = mcts.getBestAction();
		state->performAction(action);
		if (params.shouldPrintActions)
			std::cout << "Action: " << action << ", Index: " << actionsTaken << std::endl
					<< "Number of rollouts: " << mcts.getNumberOfRollouts() << std::endl;

		mcts.updateRootState(state);
		actionsTaken++;
	}
	mcts.stopSearchThread();
	const auto stateValue = state->getValue();
	std::cout << "number of actions: taken: " << actionsTaken << std::endl;
	std::cout << "threaded Mcts " << params.benchmarkIndex << " State value: " << stateValue << std::endl << std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;
	return static_cast<float>(stateValue);
}

int main() {
	constexpr unsigned int seed = 3942438306;

	std::vector<float> results;

	float result = 0;
	int benchmarkIndex = 0;

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 500,
		.endTime = 300,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::Random,

	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 1000,
		.endTime = 300,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::Random,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::Random,
		.shouldPrintActions = false
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 500,
		.endTime = 300,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 1000,
		.endTime = 300,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);
	//
	//
	// // benchmarkIndex++;
	// // result = threadedMcts({
	// // 	.benchmarkIndex = benchmarkIndex,
	// // 	.seed = seed,
	// // 	.numberOfRollouts = 0,
	// // 	.endTime = 300,
	// // 	.exploration = sqrt(2),
	// // 	.valueHeuristic = ValueHeuristic::UCT,
	// // 	.rolloutHeuristic = RolloutHeuristic::Random,
	// // 	.shouldPrintActions = false
	// // });
	// // results.push_back(result);
	//
	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 5000,
		.endTime = 300,
		.valueHeuristic = ValueHeuristic::UCB1Normal2,
		.rolloutHeuristic = RolloutHeuristic::Random,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 5000,
		.endTime = 300,
		.valueHeuristic = ValueHeuristic::UCB1Normal2,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 5000,
		.endTime = 300,
		.valueHeuristic = ValueHeuristic::UCB1Normal,
		.rolloutHeuristic = RolloutHeuristic::Random,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 5000,
		.endTime = 300,
		.valueHeuristic = ValueHeuristic::UCB1Normal,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.2,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::Random,
		.shouldPrintActions = true
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.2,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.4,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::Random,
		.shouldPrintActions = false
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.4,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.6,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::Random,
		.shouldPrintActions = false
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.6,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.8,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::Random,
		.shouldPrintActions = false
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.8,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.9,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::Random,
		.shouldPrintActions = false
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = 2000,
		.endTime = 300,
		.exploration = 0.9,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	});
	results.push_back(result);
	// Print all the results of our benchmarks
	printResults(results);
}
