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


namespace Sc2::Mcts {
	class Node;

	class Mcts {
		typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<State> > NodeStatePair;
		std::mt19937 _rng;

		const double EXPLORATION = sqrt(2);
		const int ROLLOUT_DEPTH = 100;
		ValueHeuristic _valueHeuristic = ValueHeuristic::UCT;

		std::shared_ptr<State> _rootState;
		std::shared_ptr<Node> _rootNode;
		int _runTime = 0;
		int _nodeCount = 0;
		int _numberOfRollouts = 0;

		const int MAX_DEPTH = 100;

		// Upper confidence bound applied to trees
		[[nodiscard]] double uct(Node node) const;
		[[nodiscard]] double getMaxNodeValue(const std::map<Action, std::shared_ptr<Node> > &nodes) const;

		std::vector<std::shared_ptr<Node> > getMaxNodes(
			std::map<Action, std::shared_ptr<Node> > &children, double maxValue) const;
		void singleSearch();

	public:
		std::shared_ptr<Node> getRootNode() { return _rootNode; }
		std::shared_ptr<State> getRootState() { return _rootState; }

		std::shared_ptr<Node> randomChoice(const std::map<Action, std::shared_ptr<Node> > &nodes);

		template<typename Container>
		auto randomChoice(const Container &container) -> decltype(*std::begin(container));

		NodeStatePair selectNode();

		static void expand(const std::shared_ptr<Node> &node, const std::shared_ptr<State> &state);

		int rollout(const std::shared_ptr<State> &state);

		static void backPropagate(std::shared_ptr<Node> node, int outcome);

		void search(int timeLimit);
		[[nodiscard]] double value(const Node &node) const;
		void searchRollout(int rollouts);

		void performAction(Action action);

		Action getBestAction();

		explicit Mcts(const std::shared_ptr<State> &rootState, const unsigned int seed = 0,
		              const int rolloutDepth = 100, const double exploration = sqrt(2))
			: EXPLORATION(exploration),
			  ROLLOUT_DEPTH(rolloutDepth),
			  _rootState(State::DeepCopy(*rootState)),
			  _rootNode(std::make_shared<Node>(Node(Action::none, nullptr, _valueHeuristic))) {
			if (seed != 0) {
				_rng = std::mt19937(seed);
			} else {
				_rng = std::mt19937(std::random_device{}());
			}

		}

		Mcts() : _rng(std::random_device{}()) {
		}
	};
}

#endif //MCTS_H
