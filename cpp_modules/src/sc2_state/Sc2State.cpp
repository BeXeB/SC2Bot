#include "Sc2State.h"

std::shared_ptr<Sc2::State> Sc2::State::DeepCopy(const State &state, const bool onRollout) {
    auto copyState = std::make_shared<State>(state);

    const auto stateConstructions = state.getConstructions();
    for (const auto &stateConstruction: stateConstructions) {
        copyState->_constructions.emplace_back(
            stateConstruction._timeLeft, copyState, stateConstruction._constructionFunction);
    }

    const auto stateBases = state.getBases();
    for (const auto &base: stateBases) {
        copyState->_bases.emplace_back(base);
    }

    copyState->setBiases(state._combatBiases);
    copyState->setEnemyActions(state._enemyActions);

    copyState->_onRollout = onRollout;

    return copyState;
}

void Sc2::State::advanceConstructions() {
    if (_constructions.empty()) {
        return;
    }

    auto constructionIter = _constructions.begin();
    int availableWorkers = _workerPopulation;

    do {
        constructionIter->advanceTime(1);
        if (constructionIter->getIsFinished()) {
            constructionIter = _constructions.erase(constructionIter);
        } else {
            ++constructionIter;
        }
        availableWorkers--;
    } while ((constructionIter != _constructions.end()) && (availableWorkers > 0));
}

void Sc2::State::advanceResources() {
    _minerals += mineralGainedPerTimestep();
    _vespene += vespeneGainedPerTimestep();
}

void Sc2::State::advanceOccupiedWorkers() {
    int availableWorkers = _workerPopulation;

    const auto lastIndex = static_cast<int>(_occupiedWorkerTimers.size()) - 1;


    for (int i = lastIndex; i >= 0 && availableWorkers > 0; i--) {
        _occupiedWorkerTimers[i] -= 1;
        availableWorkers--;

        if (_occupiedWorkerTimers[i] <= 0) {
            _occupiedWorkerTimers.erase(_occupiedWorkerTimers.begin() + i);
        }
    }
}

void Sc2::State::advanceEnemyAction() {
    const auto action = generateEnemyAction();

    switch (action) {
        case Action::addEnemyUnit:
            if (_currentTime < 90) {
                break;
            }
            addEnemyUnit();
            break;
        case Action::attackPlayer:
            if (_currentTime < 140) {
                break;
            }
            if (_onRollout) {
                attackPlayer();
            }
            break;
        case Action::none:
            return;
        default:
            throw std::invalid_argument("invalid action");
    }
}

void Sc2::State::advanceTime() {
    _currentTime++;
    advanceResources();
    advanceOccupiedWorkers();
    advanceConstructions();
    advanceEnemyAction();
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
    if (_workerPopulation <= _occupiedWorkerTimers.size()) {
        return 0;
    }
    int availablePopulation = _workerPopulation - static_cast<int>(_occupiedWorkerTimers.size());
    int availableMineralJobs = 0;

    for (const auto &base: _bases) {
        availableMineralJobs += base.getMineralWorkerLimit();
    }

    availablePopulation -= getVespeneWorkers();

    return availablePopulation <= availableMineralJobs ? availablePopulation : availableMineralJobs;
}

int Sc2::State::getVespeneWorkers() const {
    if (_workerPopulation <= _occupiedWorkerTimers.size()) {
        return 0;
    }
    const int availablePopulation = _workerPopulation - static_cast<int>(_occupiedWorkerTimers.size()) - getScoutWorkers();

    int availableVespeneJobs = 0;
    for (const auto &base: _bases) {
        availableVespeneJobs += base.getVespeneWorkerLimit();
    }

    return availablePopulation <= availableVespeneJobs ? availablePopulation : availableVespeneJobs;
}

int Sc2::State::getScoutWorkers() const {
    const int occupiedWorkers = static_cast<int>(_occupiedWorkerTimers.size());
    if (_workerPopulation <= occupiedWorkers) {
        return 0;
    }
    const int availableWorkers = _workerPopulation - occupiedWorkers;
    const int scoutWorkers = availableWorkers >= _MAX_SCOUT_POPULATION ? _MAX_SCOUT_POPULATION : availableWorkers;

    return scoutWorkers;
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
    return getIncomingPopulation() + getPopulation() >= _populationLimit;
}

bool Sc2::State::hasFreeBase() const {
    return _bases.size() > _incomingWorkers;
}

bool Sc2::State::hasFreeBarracks() const {
    return _barracksAmount > _incomingMarines;
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
    const auto hasWorkers = _workerPopulation > 0;

    if (_bases.size() + _incomingBases < MAX_BASES && hasWorkers) {
        actions.emplace_back(Action::buildBase);
    }

    if (_populationLimit < MAX_POPULATION_LIMIT && hasWorkers) {
        actions.emplace_back(Action::buildHouse);
    }

    if (!populationLimitReached() && !_bases.empty()) {
        actions.emplace_back(Action::buildWorker);
    }

    if (hasUnoccupiedGeyser() && hasWorkers) {
        actions.emplace_back(Action::buildVespeneCollector);
    }

    if ((_barracksAmount > 0) && !populationLimitReached()) {
        actions.emplace_back(Action::buildMarine);
    }

    if (hasWorkers && (_hasHouse || _incomingHouse)) {
        actions.emplace_back(Action::buildBarracks);
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
    while (!_hasHouse) {
        if (!_incomingHouse) {
            return;
        }
        advanceTime();
    }

    while (!canAffordConstruction(buildBarracksCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasUnoccupiedWorker()) {
        advanceTime();
        if (_workerPopulation + _incomingWorkers <= 0) {
            return;
        }
    }


    _minerals -= buildBarracksCost.minerals;
    _vespene -= buildBarracksCost.vespene;

    _occupiedWorkerTimers.emplace_back(buildBarracksCost.buildTime);
    _constructions.emplace_back(buildBarracksCost.buildTime, shared_from_this(), &State::addBarracks);
}

void Sc2::State::buildMarine() {
    if (_barracksAmount < 1) {
        return;
    }

    while (!canAffordConstruction(buildMarineCost)) {
        const auto initialMineral = _minerals;
        advanceTime();
        if (initialMineral == _minerals) {
            return;
        }
    }

    while (!hasFreeBarracks()) {
        advanceTime();
        if (_barracksAmount < 1) return;
    }

    if (!populationLimitReached()) {
        _minerals -= buildMarineCost.minerals;
        _vespene -= buildMarineCost.vespene;
        _incomingMarines += 1;

        auto c = Construction(this->buildMarineCost.buildTime, shared_from_this(), &State::addMarine);
        _constructions.emplace_back(c);
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
        if (_workerPopulation + _incomingWorkers <= 0) {
            return;
        }
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
        if (_workerPopulation + _incomingWorkers <= 0) {
            return;
        }
    }

    _minerals -= buildBaseCost.minerals;
    _vespene -= buildBaseCost.vespene;

    _incomingBases++;
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
        if ((_bases.empty() && _workerPopulation == 0) || endTimeReached()) {
            return;
        }
    }

    if (!populationLimitReached()) {
        _minerals -= buildWorkerCost.minerals;
        _vespene -= buildWorkerCost.vespene;
        _incomingWorkers += 1;

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
        if (_workerPopulation + _incomingWorkers <= 0) {
            return;
        }
    }


    _minerals -= buildHouseCost.minerals;
    _vespene -= buildHouseCost.vespene;
    _incomingHouse = true;
    _occupiedWorkerTimers.emplace_back(buildHouseCost.buildTime);
    _constructions.emplace_back(buildHouseCost.buildTime, shared_from_this(), &State::addHouse);
}

void Sc2::State::setBiases(const std::shared_ptr<std::map<int, std::tuple<double, double> > > &combatBiases) {
    _combatBiases = combatBiases;
}

void Sc2::State::setEnemyActions(const std::shared_ptr<std::map<int, Action> > &enemyActions) {
    _enemyActions = enemyActions;
}

