#include <array>
#include <iostream>
#include "Mcts.h"
#include <fstream>

using namespace Sc2::Mcts;


int main() {
	// int seeds[10] = {0, 3, 69, 420, 12435, 12335, 43645423, 573127825, 10257384, 12379123};
	// constexpr unsigned int seed = 3;
	int seeds[1] = {0};
	constexpr int numberOfRollouts = 100'000;
	// int numberOfRollouts = 10;
	constexpr int endTime = 480;
	constexpr double exploration = 0.2;
	constexpr ValueHeuristic valueHeuristic = ValueHeuristic::UCT;
	constexpr RolloutHeuristic rolloutHeuristic = RolloutHeuristic::WeightedChoice;
	constexpr int endProbabilityFunction = 2;
	constexpr Sc2::ArmyValueFunction armyValueFunction = Sc2::ArmyValueFunction::MinPower;

	// state->performAction(Action::buildHouse);
	// state->performAction(Action::buildWorker);
	// state->performAction(Action::buildBarracks);
	// state->performAction(Action::buildVespeneCollector);


	for (int seed: seeds) {
		auto state = std::make_shared<Sc2::State>(endTime, endProbabilityFunction, armyValueFunction, seed);

		const auto mcts = new Mcts(state, seed, endTime, exploration, valueHeuristic, rolloutHeuristic,
		                           endProbabilityFunction, armyValueFunction);

		std::vector<double> qValues(numberOfRollouts);
		std::list<std::tuple<Action, int> > bestActions = {};

		for (auto i = 0; i < numberOfRollouts; i++) {
			mcts->singleSearch();
			const auto root = mcts->getRootNode();
			const auto avgValue = root->Q / root->N;
			// std::cout << i << " | " << avgValue << std::endl;
			const auto bestAction = mcts->getBestAction();
			if (bestActions.empty() || std::get<Action>(bestActions.back()) != bestAction) {
				bestActions.emplace_back(bestAction, i);
			}
			qValues[i] = avgValue;
		}
		//
		// std::string fileName = "graphdata_from_root_" + std::to_string(seed) + ".csv";
		//
		// std::ofstream file(fileName);
		//
		// // Check if the file is open
		// if (!file.is_open()) {
		// 	std::cerr << "Error opening file: " << fileName << std::endl;
		// }
		// for (auto i = 0; i < numberOfRollouts; i++) {
		// 	file << i << "," << qValues[i] << std::endl;
		//
		// }
		// file.close();
		//
		// fileName = "graphdata_action_changed" + std::to_string(seed) + ".csv";
		//
		// file = std::ofstream(fileName);
		// if (!file.is_open()) {
		// 	std::cerr << "Error opening file: " << fileName << std::endl;
		// }
		// for (auto ba: bestActions) {
		// 	file << std::get<int>(ba) << "," << actionToString(std::get<Action>(ba)) << std::endl;
		// }
		// file.close();
		//
		// fileName = "best_first_10_actions" + std::to_string(seed) + ".csv";
		// file = std::ofstream(fileName);
		// if (!file.is_open()) {
		// 	std::cerr << "Error opening file: " << fileName << std::endl;
		// }
		// for (int i = 0; i < 10; ++i) {
		// 	auto ba = mcts->getBestAction();
		// 	auto currentRootState = mcts->getRootState();
		// 	auto time = currentRootState->getCurrentTime();
		// 	auto minerals = currentRootState->getMinerals();
		// 	auto gas = currentRootState->getVespene();
		// 	auto mineralsPerSecond = currentRootState->mineralGainedPerTimestep();
		// 	auto gasPerSecond = currentRootState->vespeneGainedPerTimestep();
		// 	auto stateValue = currentRootState->getValue();
		// 	auto stateResourceValue = currentRootState->calculateResourceGatheredValue();
		// 	auto stateFloatingValue = currentRootState->calculateFloatingResourcesValue();
		// 	auto stateArmyValue = currentRootState->getValueMinArmyPower();
		// 	mcts->performAction(ba);
		// 	file << actionToString(ba) << " at:" << time << " floatingMinerals:" << minerals << " floatingGas:" << gas
		// 			<<
		// 			" mineralsPerSec:" << mineralsPerSecond << " gasPerSec:" << gasPerSecond << " value:" << stateValue
		// 			<<
		// 			" floatingValue:" << stateFloatingValue << " resourceValue:" << stateResourceValue << " armyValue:"
		// 			<<
		// 			stateArmyValue << std::endl;
		// }
		// file.close();

		std::string filename = "Graph.gv";
		std::ofstream out(filename);

		out << mcts->getRootNode()->toDotString(3);

		out.close();

		std::cout << "Hello, World!\n";
		delete mcts;
	}
}
