//
// Created by marco on 25/11/2024.
//

#ifndef VALUEHEURISTICENUM_H
#define VALUEHEURISTICENUM_H

enum class ValueHeuristic {
	UCT,
	UCB1Normal2,
	UCB1Normal,
	EpsilonGreedy,
};

inline std::ostream &operator<<(std::ostream &os, const ValueHeuristic &heuristic) {
	switch (heuristic) {
		case ValueHeuristic::UCT:
			os << "UCT";
			break;
		case ValueHeuristic::UCB1Normal2:
			os << "Ucb1-Normal2";
			break;
		case ValueHeuristic::UCB1Normal:
			os << "Ucb1-Normal";
		case ValueHeuristic::EpsilonGreedy:
			os << "EpsilonGreedy";
		default:
			os << "Unknown";
			break;
	}
	return os;
}
#endif //VALUEHEURISTICENUM_H
