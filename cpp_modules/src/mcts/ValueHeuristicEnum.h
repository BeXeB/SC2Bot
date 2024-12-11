//
// Created by marco on 25/11/2024.
//

#ifndef VALUEHEURISTICENUM_H
#define VALUEHEURISTICENUM_H

enum class ValueHeuristic {
	UCT,
	UcbNormal
};

inline std::ostream &operator<<(std::ostream &os, const ValueHeuristic &heuristic) {
	switch (heuristic) {
		case ValueHeuristic::UCT:
			os << "UCT";
			break;
		case ValueHeuristic::UcbNormal:
			os << "Ucb1-Normal2";
			break;
		default:
			os << "Unknown";
			break;
	}
	return os;
}
#endif //VALUEHEURISTICENUM_H
