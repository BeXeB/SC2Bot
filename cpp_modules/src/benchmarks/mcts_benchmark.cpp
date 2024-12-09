//
// Created by marco on 25/11/2024.
//

#include <iostream>
#include <chrono>

#include "Mcts.h"
#include "Sc2State.h"
using namespace Sc2::Mcts;
using namespace std::chrono;

void printRootNode(const std::shared_ptr<Node> &rootNode) {
	std::cout << "-------ROOT-------\n"
			<< *rootNode << std::endl;

	//print children
	for (const auto &[action, node]: rootNode->children) {
		std::cout << "---CHILD---\n"
				<< *node << std::endl;
	}
}

void benchmark1() {
	constexpr unsigned int seed = 3942438306;

	std::cout << "seed: " << seed << std::endl;
	const auto state = std::make_shared<Sc2::State>();
	const auto mcts = new Mcts(state, seed);

	for (auto i = 0; i < 200; i++) {
		mcts->updateRootState(state);

		mcts->searchRollout(2000);
		const Action a = mcts->getBestAction();

		state->performAction(a);

		std::cout << a << i << std::endl;
	}

	printRootNode(mcts->getRootNode());

	std::cout << "State 1 value: " << state->getValue() << std::endl;
}

void benchmark2() {
	const auto state = std::make_shared<Sc2::State>();

	for (auto i = 0; i < 40; i++) {
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildWorker);
		state->performAction(Action::buildHouse);
		state->performAction(Action::buildVespeneCollector);
		state->performAction(Action::buildBase);
	}

	std::cout << "State 2 value: " << state->getValue() << std::endl;
}

int main() {
	const auto startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	benchmark1();
	const auto elapsedTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - startTime;
	std::cout << "Elapsed time: " << elapsedTime << " ms" << std::endl;
	benchmark2();
}
