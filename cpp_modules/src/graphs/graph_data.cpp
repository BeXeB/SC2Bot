#include <array>
#include <iostream>
#include "Mcts.h"
#include <fstream>

using namespace Sc2::Mcts;


int main() {
	// int seeds[10] = {0, 3, 69, 420, 12435, 12335, 43645423, 573127825, 10257384, 12379123};
	int treePrintIterations[10] = {10, 1000, 5000, 10000, 25000, 50000, 100000, 250000, 500000, 1000000};
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

		auto mcts = new Mcts(state, seed, endTime, exploration, valueHeuristic, rolloutHeuristic,
		                           endProbabilityFunction, armyValueFunction);

		// std::vector<double> qValues(numberOfRollouts);
		// std::list<std::tuple<Action, int> > bestActions = {};
		//
		// std::string fileName = "Graph_" + std::to_string(seed) + ".gv";
		// std::ofstream file(fileName);
		//
		// //run the rollouts and make the tree graphs
		// //gather the root value for each iteration
		// auto treei = 0;
		// for (auto i = 0; i < numberOfRollouts; i++) {
		// 	mcts->singleSearch();
		// 	const auto root = mcts->getRootNode();
		// 	const auto avgValue = root->Q / root->N;
		// 	// std::cout << i << " | " << avgValue << std::endl;
		// 	const auto bestAction = mcts->getBestAction();
		// 	if (bestActions.empty() || std::get<Action>(bestActions.back()) != bestAction) {
		// 		bestActions.emplace_back(bestAction, i);
		// 	}
		// 	qValues[i] = avgValue;
		// 	if (treePrintIterations[treei] == i-1) {
		// 		file << mcts->getRootNode()->toDotString(3, 4, "G_"+std::to_string(treei));
		// 		treei++;
		// 	}
		// }
		//
		// file.close();
		//
		// //write the root values to a file
		// fileName = "graphdata_from_root_" + std::to_string(seed) + ".csv";
		// file = std::ofstream(fileName);
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
		// //write when the best action changed to a file
		// fileName = "graphdata_action_changed_" + std::to_string(seed) + ".csv";
		// file = std::ofstream(fileName);
		//
		// if (!file.is_open()) {
		// 	std::cerr << "Error opening file: " << fileName << std::endl;
		// }
		// for (auto ba: bestActions) {
		// 	file << std::get<int>(ba) << "," << actionToString(std::get<Action>(ba)) << std::endl;
		// }
		// file.close();

		// write the top 10 actions to a file
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

		delete mcts;

		std::vector<double> winProbabilities;
		std::vector<double> continueProbabilities;
		std::tuple<double, double, double> wlc;
		std::vector<Action> actions = {
			Action::buildWorker,
			Action::buildWorker,
			Action::buildHouse,
			Action::buildWorker,
			Action::buildWorker,
			Action::buildBarracks,
			Action::buildVespeneCollector,
			Action::buildWorker,
			Action::buildWorker,
			Action::buildWorker,
			Action::buildMarine,
			Action::buildWorker,
			Action::buildBase,
			Action::buildMarine,
			Action::buildWorker,
			Action::buildHouse,
			Action::buildWorker,
			Action::buildFactory,
			Action::buildMarine,
			Action::buildVespeneCollector,
			Action::buildWorker,
			Action::buildMarine,
			Action::buildWorker,
			Action::buildStarPort,
			Action::buildMarine,
			Action::buildTank,
			Action::buildWorker,
			Action::buildWorker,
			Action::buildWorker,
			Action::buildMarine,
			Action::buildViking,
			Action::buildTank,
		};

		for (int i = 0; i < actions.size(); ++i) {
			auto action = actions[i];
			state->performAction(action);
			wlc = state->getWinProbabilities();
			winProbabilities.emplace_back(std::get<0>(wlc));
			continueProbabilities.emplace_back(std::get<2>(wlc));
			if (i == actions.size() - 1) {
				auto lastWin = winProbabilities.back();
				winProbabilities.back() = lastWin * (1 / (lastWin + (1 - lastWin - continueProbabilities.back())));
				continueProbabilities.back() = 0;
			}
			std::cout << "Win: " << winProbabilities.back() << " Continue: " << continueProbabilities.back() << std::endl;
		}
		std::cout << "Total Win:" << Mcts::calculateTotalWinProbability(winProbabilities, continueProbabilities) << std::endl;

		// we might not need rollouts
		// we could use the figures in the report, be it the experiments or the explanation
		// make graphs where the rollout is shorter

		// mcts = new Mcts(state, seed, endTime, exploration, valueHeuristic, rolloutHeuristic,
		// 						   endProbabilityFunction, armyValueFunction);
		//
		// for (auto i = 0; i < numberOfRollouts; i++) {
		// 	mcts->singleSearch();
		// }

		// fileName = "graphdata_premade_value_" + std::to_string(seed) + ".txt";
		// file = std::ofstream(fileName);
		// auto root = mcts->getRootNode();
		// file << std::to_string(root->Q / root->N);
		//
		// file.close();

		// fileName = "Graph_premade_" + std::to_string(seed) + ".gv";
		// file = std::ofstream(fileName);
		//
		// file << mcts->getRootNode()->toDotString(3, 4);
		//
		// file.close();

		std::cout << "Hello, World!\n";
		delete mcts;
	}
}
