//
// Created by marco on 25/11/2024.
//

#include <iostream>
#include <chrono>
#include <fstream>

#include "Mcts.h"
// #include "Sc2State.h"
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
	const int endProbabilityFunction = 0;
	const Sc2::ArmyValueFunction armyValueFunction = Sc2::ArmyValueFunction::None;
	bool shouldPrintActions = false;
};

struct BenchmarkResult {
	static constexpr int indexSize = 5;
	static constexpr int rolloutSize = 8;
	static constexpr int endTimeSize = 4;
	static constexpr int explorationSize = 7;
	static constexpr int valueHeuristicSize = 14;
	static constexpr int rolloutHeuristicSize = 10;
	static constexpr int timeElapsedSize = 7;
	static constexpr int numberOfActionsSize = 7;
	static constexpr int finalStateValueSize = 11;
	static constexpr int finalVespeneSize = 7;
	static constexpr int finalMineralSize = 7;
	static constexpr int valuePerActionSize = 9;
	static constexpr int valuePerSecondSize = 9;
	int benchmarkIndex = 0;
	int numberOfRollouts = 0;
	int endTime = 0;
	double exploration = 1;
	ValueHeuristic valueHeuristic = ValueHeuristic::UCT;
	RolloutHeuristic rolloutHeuristic = RolloutHeuristic::Random;
	double timeElapsed = 0;
	int numberOfActions = 0;
	float numberOfWorkers = 0;
	float numberOfBases = 0;
	float numberOfVespeneCollectors = 0;
	float numberOfMarines = 0;
	float numberOfVikings = 0;
	float numberOfTanks = 0;
	double finalStateValue = 0;
	int finalVespene = 0;
	int finalMinerals = 0;
	float valuePerAction = 0;
	float valuePerSecond = 0;

	[[nodiscard]] std::string toString() const {
		std::ostringstream stream;
		stream << std::to_string(benchmarkIndex) << ", "
		<< std::to_string(numberOfRollouts) << ","
		<< std::to_string(endTime) << ","
		<< std::to_string(exploration)<< ","
		<< valueHeuristicToString(valueHeuristic)<< ","
		<< rolloutHeuristicToString(rolloutHeuristic)<< ","
		<< std::to_string(timeElapsed)<< ","
		<< std::to_string(numberOfActions) << ","
		<< std::to_string(finalStateValue) << ","
		<< std::to_string(finalVespene) << ","
		<< std::to_string(finalMinerals) << ","
		<< std::to_string(valuePerAction) << ","
		<< std::to_string(valuePerSecond) << ","
		<< std::to_string(numberOfWorkers) << ","
		<< std::to_string(numberOfMarines) << ","
		<< std::to_string(numberOfTanks) << ","
		<< std::to_string(numberOfVikings) << ","
		<< std::to_string(numberOfBases) << ","
		<< std::to_string(numberOfVespeneCollectors);
		return stream.str();
	}

	[[nodiscard]] std::string toCsvLine() const {
		// std::string str = std::to_string(benchmarkIndex);
		// str += std::to_string(numberOfRollouts);
		// str += std::to_string(endTime);
		// str += std::to_string(exploration);
		// str += std::to_string(valueHeuristicSize);
		// str += std::to_string(rolloutHeuristicSize);
		// str += std::to_string(timeElapsed);
		// str += std::to_string(numberOfActions);
		// str += std::to_string(finalStateValue);
		// str += std::to_string(finalVespene);
		// str += std::to_string(finalMinerals);
		// str += std::to_string(valuePerAction);
		// str += std::to_string(valuePerSecond);
		// str += std::to_string(numberOfWorkers);
		// str += std::to_string(numberOfBases);
		// str += std::to_string(numberOfVespeneCollectors);

		return toString();

	}
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


BenchmarkResult benchmarkOnTime(const BenchmarkParams &params) {
	BenchmarkResult result = {
		.benchmarkIndex = params.benchmarkIndex,
		.numberOfRollouts = params.numberOfRollouts,
		.endTime = params.endTime,
		.exploration = params.exploration,
		.valueHeuristic = params.valueHeuristic,
		.rolloutHeuristic = params.rolloutHeuristic,
	};

	auto state = std::make_shared<Sc2::State>(params.endTime, params.endProbabilityFunction , params.armyValueFunction,params.seed);

	const auto mcts = new Mcts(state, params.seed, params.endTime, params.exploration, params.valueHeuristic,
	                           params.rolloutHeuristic, params.endProbabilityFunction, params.armyValueFunction);

	std::cout << "MCTS Benchmark " << params.benchmarkIndex << ": {" << std::endl
			<< "\t" << "Seed: " << params.seed << std::endl
			<< "\t" << "Time to perform actions: " << params.endTime << std::endl
			<< "\t" << "Number of rollouts: " << params.numberOfRollouts << std::endl
			<< "\t" << "Exploration: " << params.exploration << std::endl
			<< "\t" << "Rollout time: " << params.endTime << std::endl
			<< "\t" << "Value heuristic: " << params.valueHeuristic << std::endl
			<< "\t" << "Rollout heuristic: " << params.rolloutHeuristic << std::endl
			<< "\t" << "End probabability function: " << params.endProbabilityFunction << std::endl
			<< "\t" << "Army value function: " << params.armyValueFunction << std::endl
			<< "}" << std::endl;

	int actionsTaken = 0;
	while (!state->GameOver()) {
		mcts->updateRootState(state);

		mcts->searchRollout(params.numberOfRollouts);
		const Action action = mcts->getBestAction();

		if (params.shouldPrintActions)
			std::cout << "--- Index: " << actionsTaken << " ---" << std::endl <<
					"Action: " << action << std::endl
					<< "Current time of state: " << state->getCurrentTime() << std::endl
					<< "Minerals: " << state->getMinerals() << std::endl
					<< "Vespene: " << state->getVespene() << std::endl
					<< "Our Units (M/T/V): " << state->getMarinePopulation() << "/" << state->getTankPopulation() << "/"
					<< state->getVikingPopulation() << std::endl
					<< "Enemy marines: " << state->getEnemyCombatUnits() << std::endl
					<< "Enemy Ground (Pwr/Prod): " << state->getEnemy().groundPower << "/" << state->getEnemy().
					groundProduction << std::endl
					<< "Enemy Air (Pwr/Prod): " << state->getEnemy().airPower << "/" << state->getEnemy().
					airProduction << std::endl;

		if (action == Action::none) {
			state->wait();
			continue;
		}
		state->performAction(action);
		actionsTaken++;
	}

	const double stateValue = state->getValue();
	result.finalMinerals = state->getMinerals();
	result.finalVespene = state->getVespene();
	result.finalStateValue = stateValue;
	result.timeElapsed = state->getCurrentTime();
	result.numberOfActions = actionsTaken;
	result.valuePerAction = static_cast<float>(stateValue) / static_cast<float>(actionsTaken);
	result.valuePerSecond = static_cast<float>(stateValue) / static_cast<float>(state->getCurrentTime());
	result.numberOfWorkers = state->getWorkerPopulation();
	result.numberOfMarines = state->getMarinePopulation();
	result.numberOfTanks = state->getTankPopulation();
	result.numberOfVikings = state->getVikingPopulation();
	result.numberOfBases = static_cast<int>(state->getBases().size());
	result.numberOfVespeneCollectors = state->getVespeneCollectorsAmount();
	std::cout << std::endl << "Benchmark " << params.benchmarkIndex << " State value: " << stateValue <<
			std::endl <<
			"Marines: " << state->getMarinePopulation() << std::endl <<
			"Tanks: " << state->getTankPopulation() << std::endl <<
			"Vikings: " << state->getVikingPopulation() << std::endl <<
			"Enemy Ground Power: " << state->getEnemy().groundPower << std::endl <<
			"Enemy Air Power: " << state->getEnemy().airPower << std::endl
			<< std::endl
			<< "------------------------------------------------------------------" << std::endl << std::endl;

	delete mcts;
	return result;
}

void dumbBenchmark() {
	const auto state = std::make_shared<Sc2::State>(100, 0, Sc2::ArmyValueFunction::MinPower, 0);

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
	const auto state = std::make_shared<Sc2::State>(time, 0, Sc2::ArmyValueFunction::MinPower, 0);

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
	const auto state = std::make_shared<Sc2::State>(params.endTime, params.endProbabilityFunction, params.armyValueFunction, 0);
	auto mcts = Mcts(state, params.seed, params.endTime, params.exploration, params.valueHeuristic,
	                 params.rolloutHeuristic, params.endProbabilityFunction, params.armyValueFunction);
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

void printBenchmarks(const std::vector<BenchmarkResult> &results) {
	// std::cout << std::format(
	// 			"|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|",
	// 			"Index", BenchmarkResult::indexSize,
	// 			"Rollout", BenchmarkResult::numberOfActionsSize,
	// 			"end", BenchmarkResult::endTimeSize,
	// 			"explore", BenchmarkResult::explorationSize,
	// 			"value", BenchmarkResult::valueHeuristicSize,
	// 			"rollout", BenchmarkResult::rolloutHeuristicSize,
	// 			"time", BenchmarkResult::timeElapsedSize,
	// 			"actions", BenchmarkResult::numberOfActionsSize,
	// 			"final", BenchmarkResult::finalStateValueSize,
	// 			"final", BenchmarkResult::finalVespeneSize,
	// 			"final", BenchmarkResult::finalMineralSize,
	// 			"value per", BenchmarkResult::valuePerActionSize,
	// 			"value per", BenchmarkResult::valuePerSecondSize) << std::endl
	// 		<< std::format(
	// 			"|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|{:<{}}|",
	// 			"", BenchmarkResult::indexSize,
	// 			"", BenchmarkResult::numberOfActionsSize,
	// 			"time", BenchmarkResult::endTimeSize,
	// 			"", BenchmarkResult::explorationSize,
	// 			"heuristic", BenchmarkResult::valueHeuristicSize,
	// 			"heuristic", BenchmarkResult::rolloutHeuristicSize,
	// 			"elapsed", BenchmarkResult::timeElapsedSize,
	// 			"taken", BenchmarkResult::numberOfActionsSize,
	// 			"state value", BenchmarkResult::finalStateValueSize,
	// 			"vespene", BenchmarkResult::finalVespeneSize,
	// 			"mineral", BenchmarkResult::finalMineralSize,
	// 			"action", BenchmarkResult::valuePerActionSize,
	// 			"second", BenchmarkResult::valuePerSecondSize) << std::endl
	// 		<< "-----------------------------------------------------------------------------------------------------------------------"
	// 		<<
	// 		std::endl;

	for (auto result: results) {
		std::cout << result.toString() << std::endl;
	}

	std::cout << std::endl << std::endl
			<< "-------------------------------------------------------------------------------------"
			<< std::endl << std::endl;
}

void writeBenchmarksToFile(const std::vector<BenchmarkResult> &results) {
	std::string fileName = "benchmarks.csv";

	std::ofstream file(fileName);

	// Check if the file is open
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << fileName << std::endl;
	}

	file	<< "index,"
			<< "rollout,"
			<< "end time,"
			<< "explore,"
			<< "value heuristics,"
			<< "rollout heuristic,"
			<< "time elapsed,"
			<< "actions taken,"
			<< "final state value,"
			<< "final vespene,"
			<< "final mineral,"
			<<"value per action,"
			<<"value per second,"
			<< "workers,"
			<< "marines, "
			<< "tanks, "
			<< "vikings,"
			<< "bases,"
			<< "vespene collectors"
			<< std::endl;

	// file << std::format("{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",
	//                     "index",
	//                     "rollout",
	//                     "end time",
	//                     "explore",
	//                     "value heuristics",
	//                     "rollout heuristic",
	//                     "time elapsed",
	//                     "actions taken",
	//                     "final state value",
	//                     "final vespene",
	//                     "final mineral",
	//                     "value per action",
	//                     "value per second",
	//                     "workers",
	//                     "bases",
	//                     "vespene collectors");

	for (auto result: results) {
		file << result.toCsvLine() << std::endl;
	}

	file.close();
}

std::vector<BenchmarkResult> BenchmarkSuite(unsigned int seed, int numberOfRollouts, int endTime,Sc2::ArmyValueFunction armyValueFunction, int endProbabilityFunction,
                                            int benchmarkIndex = 0) {
	std::vector<BenchmarkResult> results;
	BenchmarkResult result = {};

	// ------ UCT ------
	//----- weighted ------
	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
		.shouldPrintActions = true,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = sqrt(2),
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = 0.4,
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = 0.8,
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = 2,
		.valueHeuristic = ValueHeuristic::UCT,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
	});
	results.push_back(result);

	// ------ Epsilon greedy --------
	//--------- Weighted choice ----------
	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = 0.2,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,

	});
	results.push_back(result);


	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = 0.5,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
	});
	results.push_back(result);

	benchmarkIndex++;
	result = benchmarkOnTime({
		.benchmarkIndex = benchmarkIndex,
		.seed = seed,
		.numberOfRollouts = numberOfRollouts,
		.endTime = endTime,
		.exploration = 0.8,
		.valueHeuristic = ValueHeuristic::EpsilonGreedy,
		.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
		.endProbabilityFunction = endProbabilityFunction,
		.armyValueFunction = armyValueFunction,
	});
	results.push_back(result);

	return results;
}

std::vector<BenchmarkResult> RunBenchmarks(const unsigned int seed, const int endTime) {
	std::vector<BenchmarkResult> allResults;
	int index = 0;
	std::vector<BenchmarkResult> results = {};

	results = BenchmarkSuite(seed, 5000, endTime, Sc2::ArmyValueFunction::AveragePower,0, index);
	allResults.insert(allResults.end(), results.begin(), results.end());

	index += 8;
	results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::MarinePower,0, index);
	allResults.insert(allResults.end(), results.begin(), results.end());

	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::MinPower,1, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::MinPower,2, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::AveragePower,0, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::AveragePower,1, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::AveragePower,2, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::ScaledPower,0, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::ScaledPower,1, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());
	//
	// index += 8;
	// results = BenchmarkSuite(seed, 10000, endTime, Sc2::ArmyValueFunction::ScaledPower,2, index);
	// allResults.insert(allResults.end(), results.begin(), results.end());

	return allResults;
}

std::vector<BenchmarkResult>
calculateAverageBenchmarks(const std::vector<std::vector<BenchmarkResult> > &benchmarkRuns) {
	const auto numberOfRuns = benchmarkRuns.size();
	const auto numberOfBenchmarks = benchmarkRuns[0].size();
	std::vector<BenchmarkResult> averageBenchmarkResult(numberOfBenchmarks);

	for (int benchIndex = 0; benchIndex < numberOfBenchmarks; benchIndex++) {
		for (int runIndex = 0; runIndex < numberOfRuns; runIndex++) {
			averageBenchmarkResult[benchIndex].timeElapsed += benchmarkRuns[runIndex][benchIndex].timeElapsed;
			averageBenchmarkResult[benchIndex].numberOfActions += benchmarkRuns[runIndex][benchIndex].numberOfActions;
			averageBenchmarkResult[benchIndex].finalStateValue += benchmarkRuns[runIndex][benchIndex].finalStateValue;
			averageBenchmarkResult[benchIndex].finalMinerals += benchmarkRuns[runIndex][benchIndex].finalMinerals;
			averageBenchmarkResult[benchIndex].finalVespene += benchmarkRuns[runIndex][benchIndex].finalVespene;
			averageBenchmarkResult[benchIndex].valuePerAction += benchmarkRuns[runIndex][benchIndex].valuePerAction;
			averageBenchmarkResult[benchIndex].valuePerSecond += benchmarkRuns[runIndex][benchIndex].valuePerSecond;
			averageBenchmarkResult[benchIndex].numberOfWorkers += benchmarkRuns[runIndex][benchIndex].numberOfWorkers;
			averageBenchmarkResult[benchIndex].numberOfMarines += benchmarkRuns[runIndex][benchIndex].numberOfMarines;
			averageBenchmarkResult[benchIndex].numberOfTanks += benchmarkRuns[runIndex][benchIndex].numberOfTanks;
			averageBenchmarkResult[benchIndex].numberOfVikings += benchmarkRuns[runIndex][benchIndex].numberOfVikings;
			averageBenchmarkResult[benchIndex].numberOfBases += benchmarkRuns[runIndex][benchIndex].numberOfBases;
			averageBenchmarkResult[benchIndex].numberOfVespeneCollectors += benchmarkRuns[runIndex][benchIndex].
					numberOfVespeneCollectors;

			averageBenchmarkResult[benchIndex].benchmarkIndex = benchmarkRuns[runIndex][benchIndex].benchmarkIndex;
			averageBenchmarkResult[benchIndex].numberOfRollouts = benchmarkRuns[runIndex][benchIndex].numberOfRollouts;
			averageBenchmarkResult[benchIndex].endTime = benchmarkRuns[runIndex][benchIndex].endTime;
			averageBenchmarkResult[benchIndex].exploration = benchmarkRuns[runIndex][benchIndex].exploration;
			averageBenchmarkResult[benchIndex].valueHeuristic = benchmarkRuns[runIndex][benchIndex].valueHeuristic;
			averageBenchmarkResult[benchIndex].rolloutHeuristic = benchmarkRuns[runIndex][benchIndex].rolloutHeuristic;
		}
	}

	for (auto benchIndex = 0; benchIndex < numberOfBenchmarks; benchIndex++) {
		averageBenchmarkResult[benchIndex].timeElapsed /= static_cast<double>(numberOfRuns);
		averageBenchmarkResult[benchIndex].numberOfActions = static_cast<int>(
			averageBenchmarkResult[benchIndex].numberOfActions / static_cast<double>(numberOfRuns));
		averageBenchmarkResult[benchIndex].numberOfWorkers /= static_cast<float>(numberOfRuns);
		averageBenchmarkResult[benchIndex].numberOfMarines /= static_cast<float>(numberOfRuns);
		averageBenchmarkResult[benchIndex].numberOfTanks /= static_cast<float>(numberOfRuns);
		averageBenchmarkResult[benchIndex].numberOfVikings /= static_cast<float>(numberOfRuns);
		averageBenchmarkResult[benchIndex].numberOfBases /= static_cast<float>(numberOfRuns);
		averageBenchmarkResult[benchIndex].numberOfVespeneCollectors /= static_cast<float>(numberOfRuns);
		averageBenchmarkResult[benchIndex].finalStateValue /= static_cast<double>(numberOfRuns);
		averageBenchmarkResult[benchIndex].finalMinerals = static_cast<int>(
			averageBenchmarkResult[benchIndex].finalMinerals / static_cast<double>(numberOfRuns));
		averageBenchmarkResult[benchIndex].finalVespene = static_cast<int>(
			averageBenchmarkResult[benchIndex].finalVespene / static_cast<double>(numberOfRuns));
		averageBenchmarkResult[benchIndex].valuePerAction = static_cast<float>(
			averageBenchmarkResult[benchIndex].valuePerAction / static_cast<double>(numberOfRuns));
		averageBenchmarkResult[benchIndex].valuePerSecond = static_cast<float>(
			averageBenchmarkResult[benchIndex].valuePerSecond / static_cast<double>(numberOfRuns));
	}

	return averageBenchmarkResult;
}

int main() {
	unsigned int seed = 3942438310;
	constexpr int numberOfRuns = 10;
	constexpr int runTime = 480;
	std::mt19937_64 rng(seed);
	std::uniform_int_distribution<unsigned int> dist;
	std::vector<std::vector<BenchmarkResult> > benchmarkRuns = {};

	for (auto i = 0; i < numberOfRuns; i++) {
		seed = dist(rng);
		auto benchmarkRun = RunBenchmarks(seed, runTime);
		benchmarkRuns.push_back(benchmarkRun);
	}


	const auto res = calculateAverageBenchmarks(benchmarkRuns);

	printBenchmarks(res);
	writeBenchmarksToFile(res);


	// unsigned int seed = 236865667;
	//
	// benchmarkOnTime({
	// 	.benchmarkIndex = 1,
	// 	.seed = seed,
	// 	.numberOfRollouts = 10000,
	// 	.endTime = 480,
	// 	.exploration = 0.4,
	// 	.valueHeuristic = ValueHeuristic::UCT,
	// 	.rolloutHeuristic = RolloutHeuristic::WeightedChoice,
	// 	.endProbabilityFunction = 1,
	// 	.armyValueFunction = Sc2::ArmyValueFunction::AveragePower,
	// 	.shouldPrintActions = true,
	//
	// });
}
