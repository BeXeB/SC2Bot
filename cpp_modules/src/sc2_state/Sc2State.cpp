#include "Sc2State.h"
#include <algorithm>
#include <iostream>



void Sc2::State::advanceTime(int amount) {
    const auto lastIndex =  static_cast<int>(constructions.size()) - 1;

    for (int i = lastIndex; i >= 0; i--) {
        constructions[i].advanceTime(amount);

        if(constructions[i].getIsFinished()) {
            constructions.erase(constructions.begin() + i);
        }
    }
}

void Sc2::State::wait() {
    advanceTime(1);
}

std::shared_ptr<Sc2::State> Sc2::State::DeepCopy(const State &state) {
    auto copyState = std::make_shared<State>(state);

    auto stateConstructions = state.getConstructions();

    for (const auto& stateConstruction: stateConstructions) {
        copyState->constructions.emplace_back(
            stateConstruction._timeLeft, copyState, stateConstruction._constructionFunction);
    }

    return copyState;
}

void Sc2::State::buildWorker() {
    advanceTime(1);
    if ((incomingPopulation + population) < populationLimit) {
        incomingPopulation += 1;

        auto c = Construction(this->workerBuildTime, shared_from_this(), &State::addWorker);
        constructions.emplace_back(c);
    }
}

int main()
{
    const auto state = std::make_shared<Sc2::State>();
    state->buildWorker();
    std::cout << "Constructions of State 1: " << state->getConstructions().size() << std::endl;

    // auto state2 = std::make_shared<Sc2::State>(*state);
    auto state2 = Sc2::State::DeepCopy(*state);
    std::cout << "Constructions of State 2: " << state2->getConstructions().size() << std::endl;
    // state2->wait();
    // state2->wait();
    // state2->wait();
    // state2->wait();
    state->wait();
    state->wait();
    state->wait();
    state->wait();

    std::cout << "Population of state 1: " << state->getPopulation() << std::endl;
    // state2->wait();
    // state2->wait();
    // state2->wait();
    // state2->wait();
    std::cout << "Population of state 2: " << state2->getPopulation() << std::endl;
    // std::cout << state2.getPopulation() << std::endl;
}
