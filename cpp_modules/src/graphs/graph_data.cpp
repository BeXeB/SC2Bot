#include <iostream>
#include "Mcts.h"
#include <fstream>

using namespace Sc2::Mcts;


int main() {
    constexpr unsigned int seed = 1;
    constexpr int numberOfRollouts = 1'000'000;
    constexpr int endTime = 480;
    constexpr double exploration = 0.2;
    constexpr ValueHeuristic valueHeuristic = ValueHeuristic::UCT;
    constexpr RolloutHeuristic rolloutHeuristic = RolloutHeuristic::WeightedChoice;
    constexpr int endProbabilityFunction = 2;
    constexpr Sc2::ArmyValueFunction armyValueFunction = Sc2::ArmyValueFunction::MinPower;

    auto state = std::make_shared<Sc2::State>(endTime, endProbabilityFunction , armyValueFunction, seed);

    // state->performAction(Action::buildHouse);
    // state->performAction(Action::buildWorker);
    // state->performAction(Action::buildBarracks);
    // state->performAction(Action::buildVespeneCollector);

    const auto mcts = new Mcts(state, seed, endTime, exploration, valueHeuristic, rolloutHeuristic, endProbabilityFunction, armyValueFunction);

    std::vector<double> qValues(numberOfRollouts);

    for (auto i = 0; i < numberOfRollouts; i++) {
        mcts->singleSearch();
        const auto root = mcts->getRootNode();
        const auto avgValue = root->Q/ root->N;
        // std::cout << i << " | " << avgValue << std::endl;
        qValues[i] = avgValue;
    }

    std::string fileName = "graphdata_no_tanks_from_root.csv";

    std::ofstream file(fileName);

    // Check if the file is open
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
    }
    for (auto i = 0; i < numberOfRollouts; i++) {
        file <<  i << "," << qValues[i] << std::endl;

    }
    file.close();

    std::cout << "Hello, World!\n";
}
