//
// Created by marco on 07/11/2024.
//

#ifndef NODE_H
#define NODE_H
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include <sstream>

#include "ActionEnum.h"

namespace Sc2::Mcts {
	class Node : public std::enable_shared_from_this<Node> {
		Action _action;
		int depth = 0;

		std::weak_ptr<Node> _parent;

		std::shared_ptr<State> _state;

	public:
		// Number of simulations that has been run on this node
		int N = 0;
		// The cumulative value of this node
		double Q = 0;

		// used to calculate variance, which is needed for UCB1normal2
		double M2 = 0;

		std::tuple<double, double, double> winProbabilities;

		double getSampleVariance() const {
			return M2 / N - 1;
		}

		std::map<Action, std::shared_ptr<Node> > children = {};

		std::shared_ptr<State> getState() { return _state; }
		void setParent(const std::shared_ptr<Node> &parent) { _parent = parent; }
		std::shared_ptr<Node> getParent() const { return _parent.lock(); }
		[[nodiscard]] Action getAction() const { return _action; }
		int getDepth() const { return depth; }

		void expand() {
			const std::vector<Action> actions = _state->getLegalActions();

			if (actions[0] == Action::none) { return; }

			addChildren(actions);
		}

		void addChildren(const std::vector<Action> &childActions) {
			for (const auto &childAction: childActions) {
				const auto state = State::DeepCopy(*_state);

				const auto childNode = std::make_shared<Node>(Node(childAction, shared_from_this(), state));
				childNode->depth = this->depth + 1;
				children[childNode->_action] = childNode;
			}
		}

		[[nodiscard]] std::string toString() const {
			std::ostringstream str;
			str << "Node: " << static_cast<int>(_action) << "{ \n"
			<< "parent: " << (_parent.lock() != nullptr) << "\n"
			<< "children: " << children.size() << "\n"
			<< "numberOfSimulations: " << N << "\n"
			<< "Q: " << Q << "\n"
			<< "} \n";
			return str.str();
		}

		bool gameOver() const {
			return _state->endTimeReached() || _state->getLegalActions()[0] == Action::none;
		}

		explicit Node(std::shared_ptr<State> state): _action(Action::none),
		                                             _parent(), _state(std::move(state)) {
		}

		Node(const Action action, const std::shared_ptr<Node> &parent, std::shared_ptr<State> state) : _action(action),
			_parent(parent), _state(std::move(state)) {
			_state->performAction(action);
			winProbabilities = _state->getWinProbabilities();
		}
	};

	inline std::ostream &operator<<(std::ostream &os, const Node &node) {
		os << node.toString();
		return os;
	}
}

#endif //NODE_H
