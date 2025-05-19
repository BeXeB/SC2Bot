#include <array>
#include <iostream>
#include "Mcts.h"
#include <fstream>

using namespace Sc2::Mcts;

// we could use the figures in the report, be it the experiments or the explanation
// make graphs where the rollout is shorter
// graphs with different wlc calcs

int main() {
    constexpr int seeds[5] = {0, 3, 69, 420, 12435};
    constexpr int treePrintIterations[10] = {100, 1000, 5000, 10000, 25000, 50000, 100000, 250000, 500000, 1000000};
    // int seeds[1] = {0};
    constexpr int endFunctions[3] = {0, 1, 2};
    constexpr int endtimes[2] = {480, 300};
    constexpr double explorationValues[3] = {0.2, 0.8, 1.414};
    constexpr int numberOfRollouts = 100'000;
    constexpr ValueHeuristic valueHeuristic = ValueHeuristic::UCT;
    constexpr RolloutHeuristic rolloutHeuristic = RolloutHeuristic::WeightedChoice;
    constexpr Sc2::ArmyValueFunction value_functions[4] = {
        Sc2::ArmyValueFunction::AveragePower, Sc2::ArmyValueFunction::MinPower, Sc2::ArmyValueFunction::ScaledPower,
        Sc2::ArmyValueFunction::Test
    };


    for (int seed: seeds) {
        for (int end_function: endFunctions) {
            for (int endtime: endtimes) {
                for (double exploration_value: explorationValues) {
                    for (auto value_function: value_functions) {
                        std::string data_name =
                                std::to_string(seed) + "_" + std::to_string(end_function) + "_" +
                                std::to_string(endtime) + "_" + std::to_string(exploration_value) + "_" +
                                Sc2::armyValueFunctionToString(value_function);

                        auto state = std::make_shared<Sc2::State>(endtime, end_function, value_function,
                                                                  seed);

                        auto mcts = new Mcts(state, seed, endtime, exploration_value, valueHeuristic, rolloutHeuristic,
                                             end_function, value_function);

                        std::vector<double> qValues(numberOfRollouts);
                        std::list<std::tuple<Action, int> > bestActions = {};

                        std::string fileName = "Data\\Graph_" + data_name + ".gv";
                        std::ofstream file(fileName);

                        //run the rollouts and make the tree graphs
                        //gather the root value for each iteration
                        auto treei = 0;
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
                            if (treePrintIterations[treei] == i + 1) {
                                file << mcts->getRootNode()->toDotString(3, 4, "G_" + std::to_string(treei));
                                treei++;
                            }
                        }

                        file.close();

                        //write the root values to a file
                        fileName = "Data\\graphdata_from_root_" + data_name + ".csv";
                        file = std::ofstream(fileName);

                        // Check if the file is open
                        if (!file.is_open()) {
                            std::cerr << "Error opening file: " << fileName << std::endl;
                        }
                        for (auto i = 0; i < numberOfRollouts; i++) {
                            file << i << "," << qValues[i] << std::endl;
                        }
                        file.close();

                        //write when the best action changed to a file
                        fileName = "Data\\graphdata_action_changed_" + data_name + ".csv";
                        file = std::ofstream(fileName);

                        if (!file.is_open()) {
                            std::cerr << "Error opening file: " << fileName << std::endl;
                        }
                        for (auto ba: bestActions) {
                            file << std::get<int>(ba) << "," << actionToString(std::get<Action>(ba)) << std::endl;
                        }
                        file.close();

                        std::vector<double> winProbabilities;
                        std::vector<double> continueProbabilities;
                        std::tuple<double, double, double> wlc;
                        std::vector actions = {
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

                        for (auto action: actions) {
                            state->performAction(action);
                            wlc = state->getWinProbabilities();
                            winProbabilities.emplace_back(std::get<0>(wlc));
                            continueProbabilities.emplace_back(std::get<2>(wlc));
                        }

                        delete mcts;
                        mcts = new Mcts(state, seed, endtime, exploration_value, valueHeuristic, rolloutHeuristic,
                                        end_function, value_function);

                        for (auto i = 0; i < numberOfRollouts / 10; i++) {
                            mcts->singleSearch();
                        }

                        fileName = "Data\\graphdata_premade_value_" + data_name + ".txt";
                        file = std::ofstream(fileName);
                        auto root = mcts->getRootNode();
                        auto outcome = root->Q / root->N;
                        auto winProb = 0.0;
                        auto continueProb = 0.0;
                        for (int i = winProbabilities.size() - 1; i >= 0; --i) {
                            winProb = winProbabilities[i];
                            continueProb = continueProbabilities[i];
                            outcome = winProb * 1 + continueProb * outcome;
                        }
                        file << std::to_string(outcome);

                        file.close();

                        delete mcts;
                        std::cout << "Hello, World!\n";
                    }
                }
            }
        }
    }
}
