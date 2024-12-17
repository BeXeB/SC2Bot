//
// Created by marco on 07/11/2024.
//

#ifndef MCTS_H
#define MCTS_H
#include <random>
#include <utility>
#include <Sc2State.h>

#include "Node.h"
#include "ValueHeuristicEnum.h"
#include "RolloutHeuristicEnum.h"
#include "Ucb1Normal2.h"


namespace Sc2::Mcts {
	class Node;

	class Mcts {
		typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<State> > NodeStatePair;
		std::mt19937 _rng;

		const double EXPLORATION = sqrt(2);
		const int ROLLOUT_DEPTH = 100;
		ValueHeuristic _valueHeuristic = ValueHeuristic::UCT;
		RolloutHeuristic _rolloutHeuristic = RolloutHeuristic::Random;

		std::shared_ptr<Node> _rootNode;
		int _runTime = 0;
		int _nodeCount = 0;
		int _numberOfRollouts = 0;

		const int MAX_DEPTH = 100;

		// UCB
		[[nodiscard]] static double ucb1Normal2(const std::shared_ptr<Node> &node);
		mutable UCB1Normal2 _ucbBandit;

		// Upper confidence bound applied to trees
		[[nodiscard]] double uct(const std::shared_ptr<Node> &node) const;
		[[nodiscard]] double value(const std::shared_ptr<Node> &node) const;
		[[nodiscard]] double getMaxNodeValue(const std::map<Action, std::shared_ptr<Node> > &nodes) const;

		std::vector<std::shared_ptr<Node> > getMaxNodes(
			std::map<Action, std::shared_ptr<Node> > &children) const;
		void singleSearch();

	public:
		[[nodiscard]] std::shared_ptr<Node> getRootNode() { return _rootNode; }
		[[nodiscard]] std::shared_ptr<State> getRootState() const { return _rootNode->getState(); }

		std::shared_ptr<Node> randomChoice(const std::map<Action, std::shared_ptr<Node> > &nodes);

		template<typename Container>
		auto randomChoice(const Container &container) -> decltype(*std::begin(container));

		std::shared_ptr<Node> selectNode();

		static void expand(const std::shared_ptr<Node> &node, const std::shared_ptr<State> &state);

		Action weightedChoice(const std::vector<Action> &actions);
		int rollout(const std::shared_ptr<Node> &node);

		static void backPropagate(std::shared_ptr<Node> node, int outcome);

		void search(int timeLimit);
		void searchRollout(int rollouts);

		void performAction(Action action);

		Action getBestAction();
		void updateRootState(const std::shared_ptr<State> &state);

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

			std::string str;
			str += "MCTS: { \n";
			str += std::format("Exploration: {}", EXPLORATION) + "\n";
			str += std::format("Rollout Depth: {}", ROLLOUT_DEPTH) + "\n";
			str += std::format("Value Heuristic: {} ", valueHeuristicStr) + "\n";
			str += std::format("Rollout Heuristic: {} ", rolloutHeuristicStr) + "\n";
			str += "} \n";
			return str;
		};

		explicit Mcts(const std::shared_ptr<State> &rootState, const unsigned int seed, const int rolloutDepth,
		              const double exploration, const ValueHeuristic valueHeuristic,
		              const RolloutHeuristic rolloutHeuristic) : EXPLORATION(exploration), ROLLOUT_DEPTH(rolloutDepth),
		                                                         _valueHeuristic(valueHeuristic),
		                                                         _rolloutHeuristic(rolloutHeuristic),
		                                                         _rootNode(std::make_shared<Node>(
			                                                         Node(Action::none, nullptr,
			                                                              State::DeepCopy(*rootState)))),
		                                                         _ucbBandit(0) {
			_rng = std::mt19937(seed);
		}

		Mcts(const std::shared_ptr<State> &rootState): _rootNode(
			                                               std::make_shared<Node>(Node(
				                                               Action::none, nullptr, State::DeepCopy(*rootState)))),
		                                               _ucbBandit(0) {
			_rng = std::mt19937(std::random_device{}());
		}
	};

	inline std::ostream &operator<<(std::ostream &os, const Mcts &mcts) {
		os << mcts.toString();
		return os;
	}
}

#endif //MCTS_H
