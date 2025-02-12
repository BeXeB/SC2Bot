#include "Sc2State.h"

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

void Sc2::State::advanceConstructions() {
    if (_constructions.empty()) {
        return;
    }

    auto constructionIter = _constructions.begin();

    do {
        constructionIter->advanceTime(1);
        if (constructionIter->getIsFinished()) {
            constructionIter = _constructions.erase(constructionIter);
        } else {
            ++constructionIter;
        }
    } while (constructionIter != _constructions.end());
}

void Sc2::State::advanceResources() {
    _minerals += mineralGainedPerTimestep();
    _vespene += vespeneGainedPerTimestep();
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

void Sc2::State::advanceTime() {
    _currentTime++;
    advanceResources();
    advanceOccupiedWorkers();
    advanceConstructions();
}

void Sc2::State::wait() {
    advanceTime();
}

void Sc2::State::wait(const int amount) {
    for (int i = 0; i < amount; i++) {
        advanceTime();
    }
}

int Sc2::State::getVespeneCollectorsAmount() {
    int vespeneCollectors = 0;
    for (const auto &base: _bases) {
        vespeneCollectors += base.vespeneCollectors;
    }
    return vespeneCollectors;
}

int Sc2::State::getVespeneGeysersAmount() {
    int vespeneGeysers = 0;
    for (const auto &base: _bases) {
        vespeneGeysers += base.vespeneGeysers;
    }
    return vespeneGeysers;
}

int Sc2::State::getMineralWorkers() const {
    int availablePopulation = _workerPopulation - static_cast<int>(_occupiedWorkerTimers.size());
    int availableMineralJobs = 0;

    for (const auto &base: _bases) {
        availableMineralJobs += base.getMineralWorkerLimit();
    }

    availablePopulation -= getVespeneWorkers();

    return availablePopulation <= availableMineralJobs ? availablePopulation : availableMineralJobs;
}

int Sc2::State::getVespeneWorkers() const {
    const int availablePopulation = _workerPopulation - static_cast<int>(_occupiedWorkerTimers.size());
    int availableVespeneJobs = 0;
    for (const auto &base: _bases) {
        availableVespeneJobs += base.getVespeneWorkerLimit();
    }

    return availablePopulation <= availableVespeneJobs ? availablePopulation : availableVespeneJobs;
}

int Sc2::State::mineralGainedPerTimestep() const {
    const int workers = getMineralWorkers();

    return workers * MINERAL_PER_WORKER;
}

int Sc2::State::vespeneGainedPerTimestep() const {
    const int workers = getVespeneWorkers();

    return workers * VESPENE_PER_WORKER;
}


bool Sc2::State::canAffordConstruction(const ActionCost &actionCost) const {
    const auto res = hasEnoughMinerals(actionCost.minerals) && hasEnoughVespene(actionCost.vespene);
    return res;
}

bool Sc2::State::populationLimitReached() const {
    return _incomingPopulation + getPopulation() >= _populationLimit;
}

bool Sc2::State::hasFreeBase() const {
    return _bases.size() > _incomingPopulation;
}

bool Sc2::State::hasUnoccupiedGeyser() const {
    auto incoming = _incomingVespeneCollectors;
    for (auto &base: _bases) {
        incoming -= base.getUnoccupiedGeysers();
        if (incoming < 0) {
            return true;
        }
    }
    return false;
}

std::vector<Action> Sc2::State::getLegalActions() const {
    std::vector<Action> actions = {};

    if (_bases.size() < MAX_BASES) {
        actions.emplace_back(Action::buildBase);
    }

    if (_populationLimit < MAX_POPULATION_LIMIT) {
        actions.emplace_back(Action::buildHouse);
    }

    if (!populationLimitReached()) {
        actions.emplace_back(Action::buildWorker);
    }

    if (hasUnoccupiedGeyser()) {
        actions.emplace_back(Action::buildVespeneCollector);
    }

    if (actions.empty()) {
        actions.emplace_back(Action::none);
    }


    return actions;
}

void Sc2::State::addVespeneCollector() {
    for (auto &base: _bases) {
        if (base.getUnoccupiedGeysers() > 0) {
            base.addVespeneCollector();

            break;
        }
    }
    _incomingVespeneCollectors--;
}


void Sc2::State::buildBarracks() {
    while (!canAffordConstruction(buildBarracksCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasUnoccupiedWorker()) {
        advanceTime();
    }
}

void Sc2::State::buildVespeneCollector() {
    while (!canAffordConstruction(buildVespeneCollectorCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasUnoccupiedWorker()) {
        advanceTime();
    }

    if (hasUnoccupiedGeyser()) {
        _minerals -= buildVespeneCollectorCost.minerals;
        _vespene -= buildVespeneCollectorCost.vespene;
        _incomingVespeneCollectors++;

        _occupiedWorkerTimers.emplace_back(buildVespeneCollectorCost.buildTime);
        _constructions.emplace_back(
            buildVespeneCollectorCost.buildTime,
            shared_from_this(),
            &State::addVespeneCollector);
    }
}

void Sc2::State::buildBase() {
    while (!canAffordConstruction(buildBaseCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasUnoccupiedWorker()) {
        advanceTime();
    }

    _minerals -= buildBaseCost.minerals;
    _vespene -= buildBaseCost.vespene;

    _occupiedWorkerTimers.emplace_back(buildBaseCost.buildTime);
    _constructions.emplace_back(buildBaseCost.buildTime, shared_from_this(), &State::addBase);
}

void Sc2::State::buildWorker() {
    while (!canAffordConstruction(buildWorkerCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasFreeBase()) {
        advanceTime();
    }

    if (!populationLimitReached()) {
        _minerals -= buildWorkerCost.minerals;
        _vespene -= buildWorkerCost.vespene;
        _incomingPopulation += 1;

        auto c = Construction(this->buildWorkerCost.buildTime, shared_from_this(), &State::addWorker);
        _constructions.emplace_back(c);
    }
}

void Sc2::State::buildHouse() {
    while (!canAffordConstruction(buildHouseCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasUnoccupiedWorker()) {
        advanceTime();
    }

    _minerals -= buildHouseCost.minerals;
    _vespene -= buildHouseCost.vespene;

    _occupiedWorkerTimers.emplace_back(buildHouseCost.buildTime);
    _constructions.emplace_back(buildHouseCost.buildTime, shared_from_this(), &State::addHouse);
}
