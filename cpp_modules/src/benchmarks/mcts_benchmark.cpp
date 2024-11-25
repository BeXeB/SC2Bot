//
// Created by marco on 25/11/2024.
//

#include <iostream>

#include "Mcts.h"
#include "Sc2State.h"
using namespace Sc2::Mcts;

void benchmark1() {
	const auto state = std::make_shared<Sc2::State>();
	const auto mcts = new Mcts(state);

	for (auto i = 0; i < 500; i++) {
		mcts->search(500);
		const Action a = mcts->getBestAction();

		state->performAction(a);
		mcts->performAction(a);

		std::cout << a << i << std::endl;
	}

	std::cout << "State 1 value: " << state->getValue() << std::endl;
}

void benchmark2() {
	const auto state = std::make_shared<Sc2::State>();
	const auto mcts = new Mcts(state);
	for (auto i = 0; i < 100; i++) {
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildHouse);
		state->performAction(Action::buildVespeneCollector);
		state->performAction(Action::buildBase);
	}

	std::cout << "State 2 value: " << state->getValue() << std::endl;
}

int main() {

	// benchmark1();
	benchmark2();


}
