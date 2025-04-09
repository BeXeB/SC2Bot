//
// Created by marco on 07/11/2024.
//

#ifndef MCTS_H
#define MCTS_H
#include <random>
#include <utility>
#include <Sc2State.h>
#include <thread>
#include <mutex>
#include <sstream>

#include "Node.h"
#include "ValueHeuristicEnum.h"
#include "RolloutHeuristicEnum.h"


namespace Sc2::Mcts {
	class Node;

	class Mcts {
		std::mt19937 _rng;

		const double EXPLORATION = sqrt(2);
		int _rolloutEndTime = 100;
		ValueHeuristic _valueHeuristic = ValueHeuristic::UCT;
		RolloutHeuristic _rolloutHeuristic = RolloutHeuristic::Random;

		std::shared_ptr<Node> _rootNode;
		int _runTime = 0;
		int _nodeCount = 0;
		unsigned int _numberOfRollouts = 0;

		std::thread _searchThread;
		std::mutex _mctsMutex;
		std::atomic<bool> _running = false;
		std::atomic<bool> _mctsRequestsPending;

		std::vector<double> _actionWeights = std::vector<double>(10);
		std::discrete_distribution<int> _weightedDist = std::discrete_distribution<int>();

		std::shared_ptr<std::map<int, Action> > _enemyActions = std::make_shared<std::map<int, Action> >();
		std::shared_ptr<std::map<int, std::tuple<double, double> > > _combatBiases = std::make_shared<std::map<int,
			std::tuple<double, double> > >();

		// Upper confidence bound applied to trees
		[[nodiscard]] double uct(const std::shared_ptr<Node> &node) const;
		[[nodiscard]] static double ucb1Normal2(const std::shared_ptr<Node> &node);
		[[nodiscard]] static double ucb1Normal(const std::shared_ptr<Node> &node);
		[[nodiscard]] double epsilonGreedy(const std::shared_ptr<Node> &node);
		[[nodiscard]] double value(const std::shared_ptr<Node> &node);

		std::vector<std::shared_ptr<Node> > getMaxNodes(
			std::map<Action, std::shared_ptr<Node> > &children);
		void singleSearch();
		void threadedSearch();
		void threadedSearchRollout(int numberOfRollouts);

	public:
		[[nodiscard]] std::shared_ptr<Node> getRootNode() {
			_mctsMutex.lock();
			auto node = _rootNode;
			_mctsMutex.unlock();
			return node;
		}

		[[nodiscard]] std::shared_ptr<State> getRootState() {
			_mctsMutex.lock();
			auto state = _rootNode->getState();
			_mctsMutex.unlock();
			return state;
		}

		auto getBias() { return _combatBiases; }

		auto getEnemyActions() { return _enemyActions; }

		void setEndTime(const int time) {
			_mctsMutex.lock();
			_rolloutEndTime = time;
			_mctsMutex.unlock();
		}

		std::shared_ptr<Node> randomChoice(const std::map<Action, std::shared_ptr<Node> > &nodes);

		template<typename Container>
		auto randomChoice(const Container &container) -> decltype(*std::begin(container));

		std::shared_ptr<Node> selectNode();

		static void expand(const std::shared_ptr<Node> &node, const std::shared_ptr<State> &state);

		Action weightedChoice(const std::vector<Action> &actions);
		static double calculateTotalWinProbability(const std::vector<double> &winProbabilities, const std::vector<double> &continueProbabilities);
		double rollout(const std::shared_ptr<Node> &node);

		static void backPropagate(std::shared_ptr<Node> node, double outcome);

		void startSearchRolloutThread(int numberOfRollouts);
		void search(int timeLimit);
		void searchRollout(int rollouts);
		void stopSearchThread();
		void startSearchThread();

		void performAction(Action action);

		Action getBestAction();
		void updateRootState(const std::shared_ptr<State> &state);

		void updateRootState(const StateBuilderParams &params) {
			std::uniform_int_distribution<unsigned int> dist;
			const auto state = State::InternalStateBuilder(params,dist(_rng) , _enemyActions, _combatBiases);

			updateRootState(state);
		}

		[[nodiscard]] unsigned int getNumberOfRollouts() {
			_mctsRequestsPending = true;
			_mctsMutex.lock();
			const auto n = _numberOfRollouts;
			_mctsMutex.unlock();
			_mctsRequestsPending = false;
			return n;
		}

		void initializeActionsAndBiases(const int timeSteps) {
			// Over the span of 60 seconds we assume that the enemy:
			// Specifies how many enemy units will be built
			const double buildUnitAction = 7;
			// Specifies how many times the enemy will attack
			const double attackAction = 0.2;
			// Specifies how many times the enemy will do nothing
			const double noneAction = 60 - buildUnitAction - attackAction;

			const auto actionWeights = {noneAction, buildUnitAction, attackAction};
			std::discrete_distribution<int> dist(actionWeights.begin(), actionWeights.end());
			std::uniform_real_distribution<double> combatDist(0.0, 2.0);
			for (int i = 0; i < timeSteps; i++) {
				// 0: None, 1: Build unit, 2: Attack
				switch (dist(_rng)) {
					case 1:
						(*_enemyActions)[i] = Action::addEnemyUnit;
						break;
					case 2:
						(*_enemyActions)[i] = Action::attackPlayer;
						(*_combatBiases)[i] = std::tuple(combatDist(_rng), combatDist(_rng));
						break;
					default:
						break;
				}
			}
		}

		[[nodiscard]] std::string toString() const {
			std::string rolloutHeuristicStr;
			switch (_rolloutHeuristic) {
				case RolloutHeuristic::Random:
					rolloutHeuristicStr = "Random";
					break;
				case RolloutHeuristic::WeightedChoice:
					rolloutHeuristicStr = "WeightedChoice";
					break;
				default:
					rolloutHeuristicStr = "Unknown";
					break;
			}

			std::string valueHeuristicStr;
			switch (_valueHeuristic) {
				case ValueHeuristic::UCT:
					valueHeuristicStr = "UCT";
					break;
				default:
					valueHeuristicStr = "Unknown";
					break;
			}

			std::ostringstream str;
			str << "MCTS: { \n"
			<< "Exploration: " << EXPLORATION << "\n"
			<< "Rollout Depth: " << _rolloutEndTime << "\n"
			<< "Value Heuristic: " << valueHeuristicStr << "\n"
			<< "Rollout Heuristic: " << rolloutHeuristicStr << "\n"
			<< "} \n";
			return str.str();
		};

		explicit Mcts(const std::shared_ptr<State> &rootState, const unsigned int seed, const int rolloutEndTime,
		              const double exploration, const ValueHeuristic valueHeuristic,
		              const RolloutHeuristic rolloutHeuristic) : EXPLORATION(exploration),
		                                                         _rolloutEndTime(rolloutEndTime),
		                                                         _valueHeuristic(valueHeuristic),
		                                                         _rolloutHeuristic(rolloutHeuristic) {
			_rng = std::mt19937(seed);
			initializeActionsAndBiases(_rolloutEndTime);
			const auto deepCopy = State::DeepCopy(*rootState);
			deepCopy->setBiases(_combatBiases);
			deepCopy->setEnemyActions(_enemyActions);
			_rootNode = std::make_shared<Node>(Node(Action::none, nullptr, deepCopy));
		}

		explicit Mcts(const std::shared_ptr<State> &rootState) {
			const auto seed = std::random_device{}();
			_rng = std::mt19937(seed);
			initializeActionsAndBiases(_rolloutEndTime);
			const auto deepCopy = State::DeepCopy(*rootState);
			deepCopy->setBiases(_combatBiases);
			deepCopy->setEnemyActions(_enemyActions);
			_rootNode = std::make_shared<Node>(Node(Action::none, nullptr, deepCopy));
		}

		Mcts() {
			const auto seed = std::random_device{}();
			_rng = std::mt19937(seed);
			initializeActionsAndBiases(_rolloutEndTime);
			auto rootState = std::make_shared<State>(_rolloutEndTime, seed, _enemyActions, _combatBiases);
			_rootNode = std::make_shared<Node>(Action::none, nullptr, rootState);
		}
	};

	inline std::ostream &operator<<(std::ostream &os, const Mcts &mcts) {
		os << mcts.toString();
		return os;
	}
}

#endif //MCTS_H
