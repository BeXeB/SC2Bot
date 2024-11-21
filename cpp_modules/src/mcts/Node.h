//
// Created by marco on 07/11/2024.
//

#ifndef NODE_H
#define NODE_H
#include <cmath>
#include <format>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "ActionEnum.h"
#include "MctsMeta.h"

namespace Sc2::Mcts {
	class Node : public std::enable_shared_from_this<Node> {
		Action action;
		std::shared_ptr<Node> parent;
		int depth = 0;

		// Upper confidence bound applied to trees
		// Q/N + C * (sqrt(log(parent.N)
		[[nodiscard]] double uct(const float explore) const {
			return Q / static_cast<float>(N) + explore * sqrt(
				       log(parent->N));
		}

	public:
		// Number of simulations that has been run on this node
		int N = 0;
		// The cumulative value of this node
		double Q = 0;
		std::map<Action, std::shared_ptr<Node> > children = {};

		std::shared_ptr<Node> getParent() { return parent; }
		[[nodiscard]] Action getAction() const { return action; }
		int getDepth() const { return depth; }

		void expand(const std::shared_ptr<State> &state) {
			const std::vector<Action> actions = state->getLegalActions();
			addChildren(actions);
		}

		void addChildren(const std::vector<Action> &childActions) {
			for (const auto &childAction: childActions) {
				const auto childNode = std::make_shared<Node>(Node(childAction, shared_from_this()));;
				childNode->depth = this->depth + 1;
				children[childNode->action] = childNode;
			}
		}

		[[nodiscard]] double value(const float explore = EXPLORATION) const {
			if (N == 0) {
				if (explore == 0) {
					return 0;
				}

				return INFINITY;
			}
			return uct(explore);
		}

		[[nodiscard]] std::string toString() const {
			std::string str;
			str += std::format("Node: {} ", static_cast<int>(action)) + "{ \n";
			str += std::format("parent: {} \n", parent != nullptr);
			str += std::format("children: {} \n", children.size());
			str += std::format("numberOfSimulations: {} \n", N);
			str += std::format("Q: {} \n", Q);
			str += "} \n";
			return str;
		}

		Node(): action(Action::none), parent(nullptr) {
		}

		Node(const Action action, std::shared_ptr<Node> parent) : action(action), parent(std::move(parent)) {
		}
	};

	inline std::ostream &operator<<(std::ostream &os, const Node &node) {
		os << node.toString();
		return os;
	}
}

#endif //NODE_H
