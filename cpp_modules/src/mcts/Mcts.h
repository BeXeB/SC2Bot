//
// Created by marco on 07/11/2024.
//

#ifndef MCTS_H
#define MCTS_H
#include <random>
#include <utility>
#include <Sc2State.h>

#include "Node.h"


namespace Sc2::Mcts {
	class Node;

	class Mcts {
		std::mt19937 _rng;
		typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<State> > NodeStatePair;

		std::shared_ptr<State> _rootState;
		std::shared_ptr<Node> _rootNode;
		int _runTime = 0;
		int _nodeCount = 0;
		int _numberOfRollouts = 0;

		static double maxNodeValue(const std::map<Action, std::shared_ptr<Node> > &nodes);

		static std::vector<std::shared_ptr<Node> > getMaxNodes(
			std::map<Action, std::shared_ptr<Node> > &children, double maxValue);

	public:
		std::shared_ptr<Node> getRootNode() { return _rootNode; }
		std::shared_ptr<State> getRootState() { return _rootState; }

		std::shared_ptr<Node> RandomChoice(const std::vector<std::shared_ptr<Node> > &nodes);
		std::shared_ptr<Node> RandomChoice(const std::map<Action, std::shared_ptr<Node> > &map);

		NodeStatePair selectNode();
		static void expand(const std::shared_ptr<Node> &node, const std::shared_ptr<State> &state);
		int rollout(std::shared_ptr<State> state);
		void backPropagate(std::shared_ptr<Node> node, int outcome);


		void search(int timeLimit);

		unsigned int getRandomNum() {
			std::uniform_int_distribution<std::mt19937::result_type> dist(0, 10);
			return dist(_rng);
		}

		explicit Mcts(const std::shared_ptr<State> &rootState) : _rng(std::random_device{}()),
		                                                         _rootState(State::DeepCopy(*rootState)),
		                                                         _rootNode(std::make_shared<Node>(
			                                                         Node(Action::none, nullptr))) {
		}

		Mcts() : _rng(std::random_device{}()) {
		}

	};
}

#endif //MCTS_H
