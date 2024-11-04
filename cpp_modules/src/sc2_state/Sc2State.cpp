#include "Sc2State.h"
#include <iostream>


void Sc2::State::advanceConstructions(int amount) {
    const auto lastIndex = static_cast<int>(_constructions.size()) - 1;

    for (int i = lastIndex; i >= 0; i--) {
        _constructions[i].advanceTime(amount);

        if (_constructions[i].getIsFinished()) {
            _constructions.erase(_constructions.begin() + i);
        }
    }
}

// edit later to be more accurate
void Sc2::State::advanceResources(int amount) {
    for (auto i = 0; i < amount; i++) {
        _minerals += _population - static_cast<int>(_occupiedWorkerTimers.size());
        _vespene += _population - static_cast<int>(_occupiedWorkerTimers.size());
    }
}

void Sc2::State::advanceOccupiedWorkers(int amount) {
    const auto lastIndex = static_cast<int>(_occupiedWorkerTimers.size()) - 1;

    for (int i = lastIndex; i >= 0; i--) {
        _occupiedWorkerTimers[i] = -amount;

        if (_occupiedWorkerTimers[i] <= 0) {
            _occupiedWorkerTimers.erase(_occupiedWorkerTimers.begin() + i);
        }
    }
}

void Sc2::State::advanceTime(int amount) {
    advanceResources(amount);
    advanceOccupiedWorkers(amount);
    advanceConstructions(amount);
}

void Sc2::State::wait() {
    advanceTime(1);
}

void Sc2::State::wait(const int amount) {
    for (auto i = 0; i < amount; i++) {
        wait();
    }
}

std::shared_ptr<Sc2::State> Sc2::State::DeepCopy(const State &state) {
    auto copyState = std::make_shared<State>(state);

    auto stateConstructions = state.getConstructions();

    for (const auto &stateConstruction: stateConstructions) {
        copyState->_constructions.emplace_back(
            stateConstruction._timeLeft, copyState, stateConstruction._constructionFunction);
    }

    return copyState;
}

void Sc2::State::buildWorker() {
    if (_incomingPopulation + _population < _populationLimit &&
        _minerals >= buildWorkerCost.minerals &&
        _vespene >= buildWorkerCost.vespene) {
        _incomingPopulation += 1;

        auto c = Construction(this->buildWorkerCost.buildTime, shared_from_this(), &State::addWorker);
        _constructions.emplace_back(c);
    }
    advanceTime(1);
}

void Sc2::State::buildHouse() {
    if (_minerals >= buildHouseCost.minerals
        && _vespene >= buildHouseCost.vespene
        && _population - _occupiedWorkerTimers.size() > 0) {
        _minerals -= buildHouseCost.minerals;
        _vespene -= buildHouseCost.vespene;
        _occupiedWorkerTimers.emplace_back(buildWorkerCost.buildTime);
        _constructions.emplace_back(this->buildWorkerCost.buildTime, shared_from_this(), &State::addHouse);
    }
    advanceTime(1);
}

void printState(const Sc2::State &state, const std::string &name) {
    std::cout
            << "State: " << name << " {" << std::endl
            << "minerals: " << state.getMinerals() << std::endl
            << "vespene: " << state.getVespene() << std::endl
            << "Constructions: " << state.getConstructions().size() << std::endl
            << "population: " << state.getPopulation() << std::endl
            << "incomingPopulation: " << state.getIncomingPopulation() << std::endl
            << "populationLimit: " << state.getPopulationLimit() << std::endl
            << "}" << std::endl;
}

int main() {
    const auto state = std::make_shared<Sc2::State>();
    printState(*state, "1");
    state->wait(10);
    state->buildWorker();
    state->wait(10);
    // auto state2 = Sc2::State::DeepCopy(*state);

    printState(*state, "1");
}
