#include "Sc2State.h"
// #include <bits/ranges_algo.h>


void Sc2::State::advanceConstructions() {
    const auto lastIndex = static_cast<int>(_constructions.size()) - 1;

    for (int i = lastIndex; i >= 0; i--) {
        _constructions[i].advanceTime(1);

        if (_constructions[i].getIsFinished()) {
            _constructions.erase(_constructions.begin() + i);
        }
    }
}

// edit later to be more accurate
void Sc2::State::advanceResources() {
    // for (auto i = 0; i < amount; i++) {
    // _minerals += _population - static_cast<int>(_occupiedWorkerTimers.size());
    // _vespene += _population - static_cast<int>(_occupiedWorkerTimers.size());
    _minerals += mineralGainedPerTimestep();
    _vespene += vespeneGainedPerTimestep();
    // }
}

void Sc2::State::advanceOccupiedWorkers() {
    const auto lastIndex = static_cast<int>(_occupiedWorkerTimers.size()) - 1;

    for (int i = lastIndex; i >= 0; i--) {
        _occupiedWorkerTimers[i] = -1;

        if (_occupiedWorkerTimers[i] <= 0) {
            _occupiedWorkerTimers.erase(_occupiedWorkerTimers.begin() + i);
        }
    }
}

int Sc2::State::getMineralWorkers() const {
    int availablePopulation = _population - static_cast<int>(_occupiedWorkerTimers.size());
    int availableMineralJobs = 0;

    for (const auto &base: _bases) {
        availablePopulation -= base.getVespeneWorkerLimit();
        availableMineralJobs += base.getMineralWorkerLimit();
    }

    return availablePopulation <= availableMineralJobs ? availablePopulation : availableMineralJobs;
}

int Sc2::State::mineralGainedPerTimestep() const {
    const int workers = getMineralWorkers();

    return workers * MINERAL_PER_WORKER;
}

int Sc2::State::getVespeneWorkers() const {
    const int availablePopulation = _population - static_cast<int>(_occupiedWorkerTimers.size());
    int availableVespeneJobs = 0;
    for (const auto &base: _bases) {
        availableVespeneJobs += base.getVespeneWorkerLimit();
    }

    return availablePopulation <= availableVespeneJobs ? availablePopulation : availableVespeneJobs;
}

int Sc2::State::vespeneGainedPerTimestep() const {
    const int workers = getVespeneWorkers();

    return workers * VESPENE_PER_WORKER;
}

void Sc2::State::advanceTime(int amount) {
    for (int i = 0; i < amount; i++) {
        advanceResources();
        advanceOccupiedWorkers();
        advanceConstructions();
    }
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
    auto stateBases = state.getBases();

    for (const auto &base: stateBases) {
        copyState->_bases.emplace_back(base);
    }

    return copyState;
}

void Sc2::State::buildWorker() {
    if (_incomingPopulation + _population < _populationLimit &&
        hasEnoughMinerals(buildWorkerCost.minerals) &&
        hasEnoughVespene(buildWorkerCost.vespene)) {
        _minerals -= buildWorkerCost.minerals;
        _vespene -= buildWorkerCost.vespene;
        _incomingPopulation += 1;
        advanceTime(1);

        auto c = Construction(this->buildWorkerCost.buildTime, shared_from_this(), &State::addWorker);
        _constructions.emplace_back(c);
    } else {
        advanceTime(1);
    }
}

void Sc2::State::buildHouse() {
    if (hasEnoughMinerals(buildHouseCost.minerals)
        && hasEnoughVespene(buildHouseCost.vespene)
        && hasUnoccupiedWorker()) {
        _minerals -= buildHouseCost.minerals;
        _vespene -= buildHouseCost.vespene;
        advanceTime(1);
        _occupiedWorkerTimers.emplace_back(buildHouseCost.buildTime);
        _constructions.emplace_back(buildHouseCost.buildTime, shared_from_this(), &State::addHouse);
    } else {
        advanceTime(1);
    }
}

void Sc2::State::buildBase() {
    if (hasEnoughMinerals(buildBaseCost.minerals)
        && hasEnoughVespene(buildBaseCost.vespene)
        && hasUnoccupiedWorker()) {
        _minerals -= buildBaseCost.minerals;
        _vespene -= buildBaseCost.vespene;
        advanceTime(1);
        _occupiedWorkerTimers.emplace_back(buildBaseCost.buildTime);
        _constructions.emplace_back(buildBaseCost.buildTime, shared_from_this(), &State::addBase);
    } else {
        advanceTime(1);
    }
}

void Sc2::State::addVespeneCollector() {
    for (auto &base: _bases) {
        if (base.getUnoccupiedGeysers() > 0) {
            base.addVespeneCollector();

            break;
        }
    }
    incomingVespeneCollectors--;
}

bool Sc2::State::hasUnoccupiedGeyser() const {
    // return std::ranges::any_of(_bases, [](const Base &base) { return base.getUnoccupiedGeysers() > 0; });
    for (auto &base: _bases) {
        if (base.getUnoccupiedGeysers() - incomingVespeneCollectors > 0) {
            return true;
        }
    }
    return false;
}

void Sc2::State::buildVespeneCollector() {
    if (hasEnoughMinerals(buildVespeneCollectorCost.minerals)
        && hasEnoughVespene(buildVespeneCollectorCost.vespene)
        && hasUnoccupiedWorker()
        && hasUnoccupiedGeyser()) {
        _minerals -= buildVespeneCollectorCost.minerals;
        _vespene -= buildVespeneCollectorCost.vespene;
        incomingVespeneCollectors++;
        advanceTime(1);
        _occupiedWorkerTimers.emplace_back(buildVespeneCollectorCost.buildTime);
        _constructions.emplace_back(
            buildVespeneCollectorCost.buildTime,
            shared_from_this(),
            &State::addVespeneCollector);
    } else {
        advanceTime(1);
    }
}

// int main() {
//     std::cout << "Hello World!" << std::endl;
//     return 0;
// }
