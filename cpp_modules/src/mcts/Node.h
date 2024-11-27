//
// Created by marco on 07/11/2024.
//

#ifndef NODE_H
#define NODE_H
#include <format>
#include <map>
#include <memory>
#include <utility>
#include <ValueHeuristicEnum.h>
#include <vector>

#include "ActionEnum.h"

namespace Sc2::Mcts {
	class Node : public std::enable_shared_from_this<Node> {
		ValueHeuristic _valueHeuristic;

		Action _action;
		int depth = 0;

		std::shared_ptr<Node> _parent;

	public:
		// Number of simulations that has been run on this node
		int N = 0;
		// The cumulative value of this node
		double Q = 0;
		std::map<Action, std::shared_ptr<Node> > children = {};

		void setParent(std::shared_ptr<Node> parent) { _parent = std::move(parent); }
		std::shared_ptr<Node> getParent() { return _parent; }
		[[nodiscard]] Action getAction() const { return _action; }
		int getDepth() const { return depth; }

		void expand(const std::shared_ptr<State> &state) {
			const std::vector<Action> actions = state->getLegalActions();
			addChildren(actions);
		}

		void addChildren(const std::vector<Action> &childActions) {
			for (const auto &childAction: childActions) {
				const auto childNode = std::make_shared<Node>(Node(childAction, shared_from_this(), _valueHeuristic));;
				childNode->depth = this->depth + 1;
				children[childNode->_action] = childNode;
			}
		}

		[[nodiscard]] std::string toString() const {
			std::string str;
			str += std::format("Node: {} ", static_cast<int>(_action)) + "{ \n";
			str += std::format("parent: {} \n", _parent != nullptr);
			str += std::format("children: {} \n", children.size());
			str += std::format("numberOfSimulations: {} \n", N);
			str += std::format("Q: {} \n", Q);
			str += "} \n";
			return str;
		}

		explicit Node(const ValueHeuristic valueHeuristic): _valueHeuristic(valueHeuristic), _action(Action::none),
		                                                    _parent(nullptr) {
		}

		Node(const Action action, std::shared_ptr<Node> parent,
		     const ValueHeuristic valueHeuristic) : _valueHeuristic(valueHeuristic), _action(action),
		                                            _parent(std::move(parent)) {
		}
	};

	inline std::ostream &operator<<(std::ostream &os, const Node &node) {
		os << node.toString();
		return os;
	}
}

#endif //NODE_H
