//
// Created by marco on 25/11/2024.
//

#include <iostream>

#include "Mcts.h"
#include "Sc2State.h"
using namespace Sc2::Mcts;

void benchmark1() {
	// const auto seed = std::random_device{}();
	constexpr unsigned int seed = 3942438306;

	std::cout << "seed: " << seed << std::endl;
	const auto state = std::make_shared<Sc2::State>();
	const auto mcts = new Mcts(state, seed);

	for (auto i = 0; i < 20; i++) {
		mcts->searchRollout(1000);
		const Action a = mcts->getBestAction();

		state->performAction(a);
		mcts->performAction(a);

		std::cout << a << i << std::endl;
	}

	std::cout << "State 1 value: " << state->getValue() << std::endl;
}

void benchmark2() {
	const auto state = std::make_shared<Sc2::State>();

	for (auto i = 0; i < 4; i++) {
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildHouse);
		state->performAction(Action::buildVespeneCollector);
		state->performAction(Action::buildBase);
	}

	std::cout << "State 2 value: " << state->getValue() << std::endl;
}

int main() {
	benchmark1();
	benchmark2();
}
