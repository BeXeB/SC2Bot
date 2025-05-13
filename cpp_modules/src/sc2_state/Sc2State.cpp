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
    _enemy.takeAction(_currentTime);
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

int Sc2::State::getVespeneCollectorsAmount() const {
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
    const int scoutWorkers = availableWorkers >= MAX_SCOUT_POPULATION ? MAX_SCOUT_POPULATION : availableWorkers;

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

bool Sc2::State::withinPopulationLimit(int populationIncrease) const
{
    return getIncomingPopulation() + getPopulation() + populationIncrease <= _populationLimit;
}

bool Sc2::State::hasFreeBase() const {
    return _bases.size() > _incomingWorkers;
}

bool Sc2::State::hasFreeBarracks() const {
    return _barracksAmount > _incomingMarines;
}

bool Sc2::State::hasFreeFactory() const {
    return _factoryAmount > _incomingTanks;
}

bool Sc2::State::hasFreeStarPort() const
{
    return _starPortAmount > _incomingVikings;
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

    if ((_barracksAmount > 0 || _incomingBarracks) && !populationLimitReached()) {
        actions.emplace_back(Action::buildMarine);
    }

    if (hasWorkers && (_hasHouse || _incomingHouse)) {
        actions.emplace_back(Action::buildBarracks);
    }

    if (hasWorkers && (_barracksAmount > 0 || _incomingBarracks) && (_incomingVespeneCollectors > 0 || getVespeneCollectorsAmount() > 0)) {
        actions.emplace_back(Action::buildFactory);
    }

    if (hasWorkers && (_factoryAmount > 0 || _incomingFactory > 0)) {
        actions.emplace_back(Action::buildStarPort);
    }

    // if (_factoryAmount + _incomingFactory > _factoryTechLabAmount + _incomingFactoryTechLab) {
    //     actions.emplace_back(Action::buildFactoryTechLab);
    // }

    if (_factoryAmount + _incomingFactory > 0 && withinPopulationLimit(TANK_SUPPLY)) {
        actions.emplace_back(Action::buildTank);
    }

    if (_starPortAmount > 0 && withinPopulationLimit(VIKING_SUPPLY)) {
        actions.emplace_back(Action::buildViking);
    }

    if (actions.empty()) {
        actions.emplace_back(Action::none);
    }


    return actions;
}

std::tuple<double, double, double> Sc2::State::getWinProbabilities() {
    const double successProb = getCombatSuccessProbability();
    const double endProb = getEndProbability();

    double winProb = successProb * endProb;
    double lossProb = (1 - successProb) * endProb;
    double continueProb = 1 - endProb;

    return {winProb, lossProb, continueProb};
}

std::vector<float> Sc2::State::getFeatureVector() {
    std::vector<float> features = {};

    for (int i = 0; i < static_cast<int>(PlayerUnitType::Last); i++) {
        auto type = static_cast<PlayerUnitType>(i);
        switch (type) {
            case PlayerUnitType::SIEGETANK:
                features.emplace_back(_tankPopulation);
                break;
            case PlayerUnitType::VIKINGFIGHTER:
                features.emplace_back(_vikingPopulation);
                break;
            case PlayerUnitType::MARINE:
                features.emplace_back(_marinePopulation);
                break;
            default:
                features.emplace_back(0);
                break;
        }
    }

    for (int i = 0; i < static_cast<int>(EnemyUnitType::Last); i++) {
        auto type = static_cast<EnemyUnitType>(i);
        features.emplace_back(_enemy.units[type]);
    }

    // on_creep column always set to 0 in mcts
    features.emplace_back(0);

    return features;
}

double Sc2::State::getCombatSuccessProbability() const {
    switch (_armyValueFunction) {
        case ArmyValueFunction::AveragePower:
            return getValueArmyPowerAverage();
        case ArmyValueFunction::MinPower:
            return getValueMinArmyPower();
        case ArmyValueFunction::ScaledPower:
            return getValueArmyPowerScaled();
        case ArmyValueFunction::MarinePower:
            return getValueMarines();
        case ArmyValueFunction::None:
            throw std::invalid_argument("ArmyValueFunction::None");
        default:
            throw std::invalid_argument("Unknown ArmyValueFunction");
    }
}

double Sc2::State::getEndProbability() const {
    const double successProb = getCombatSuccessProbability();

    switch (END_PROBABILITY_FUNCTION) {
        case 0:
            return std::pow(successProb - 0.5, 2) * 4;
        case 1:
            return std::pow(successProb - 0.5, 4) * 16;
        case 2:
            return std::pow(successProb - 0.5, 8) * 200;
        default:
            throw std::runtime_error("Unknown EndProbabilityFunction: " + std::to_string(END_PROBABILITY_FUNCTION));
    }
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
    _incomingBarracks = true;

    _occupiedWorkerTimers.emplace_back(buildBarracksCost.buildTime);
    _constructions.emplace_back(buildBarracksCost.buildTime, shared_from_this(), &State::addBarracks);
}

void Sc2::State::buildFactory()
{
    while (!_barracksAmount > 0) {
        if (!_incomingBarracks) {
            return;
        }
        advanceTime();
    }

    while (!canAffordConstruction(buildFactoryCost))
    {
        const auto initialMineral = _minerals;
        const auto initialVespene = _vespene;
        advanceTime();
        if (initialMineral == _minerals || (initialVespene == _vespene && _incomingVespeneCollectors < 1 )) {
            return;
        }
    }

    while (!hasUnoccupiedWorker())
    {
        advanceTime();
        if (_workerPopulation + _incomingWorkers <= 0) {
            return;
        }
    }

    _minerals -= buildFactoryCost.minerals;
    _vespene -= buildFactoryCost.vespene;
    _incomingFactory += 1;

    _occupiedWorkerTimers.emplace_back(buildFactoryCost.buildTime);
    _constructions.emplace_back(buildFactoryCost.buildTime, shared_from_this(), &State::addFactory);
}

// void Sc2::State::buildFactoryTechLab()
// {
//     while (!_factoryAmount > 0) {
//         if (_incomingFactory <= 0) {
//             return;
//         }
//         advanceTime();
//     }
//
//     while (!canAffordConstruction(buildTechLabCost))
//     {
//         const auto initialMineral = _minerals;
//         const auto initialVespene = _vespene;
//         advanceTime();
//         if (initialMineral == _minerals || initialVespene == _vespene) {
//             return;
//         }
//     }
//
//     _minerals -= buildTechLabCost.minerals;
//     _vespene -= buildTechLabCost.vespene;
//     _incomingFactoryTechLab += 1;
//
//     _constructions.emplace_back(buildTechLabCost.buildTime, shared_from_this(), &State::addFactoryTechLab);
// }

void Sc2::State::buildStarPort()
{
    while (!_factoryAmount > 0) {
        if (_incomingFactory <= 0) {
            return;
        }
        advanceTime();
    }

    while (!canAffordConstruction(buildStarPortCost))
    {
        const auto initialMineral = _minerals;
        const auto initialVespene = _vespene;
        advanceTime();
        if (initialMineral == _minerals || initialVespene == _vespene) {
            return;
        }
    }

    while (!hasUnoccupiedWorker())
    {
        advanceTime();
        if (_workerPopulation + _incomingWorkers <= 0) {
            return;
        }
    }

    _minerals -= buildStarPortCost.minerals;
    _vespene -= buildStarPortCost.vespene;

    _occupiedWorkerTimers.emplace_back(buildStarPortCost.vespene);
    _constructions.emplace_back(buildFactoryCost.buildTime, shared_from_this(), &State::addStarPort);
}


void Sc2::State::buildMarine() {
    while (!_barracksAmount > 0) {
        if (!_incomingBarracks) {
            return;
        }
        advanceTime();
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

        auto c = Construction(buildMarineCost.buildTime, shared_from_this(), &State::addMarine);
        _constructions.emplace_back(c);
    }
}

void Sc2::State::buildTank()
{
    while (!_factoryAmount > 0) {
        if (_incomingFactory <= 0) {
            return;
        }
        advanceTime();
    }

    while (!hasFreeFactory())
    {
        advanceTime();
        if (_factoryAmount < 1) return;
    }

    while (!canAffordConstruction(buildTankCost)) {
        const auto initialMineral = _minerals;
        const auto initialVespene = _vespene;
        advanceTime();
        if (initialMineral == _minerals || initialVespene == _vespene) {
            return;
        }
    }

    if (withinPopulationLimit(TANK_SUPPLY))
    {
        _minerals -= buildTankCost.minerals;
        _vespene -= buildTankCost.vespene;
        _incomingTanks += 1;

        auto c = Construction(buildTankCost.buildTime, shared_from_this(), &State::addTank);
        _constructions.emplace_back(c);
    }
}

void Sc2::State::buildViking()
{
    if (_starPortAmount < 1)
    {
        return;
    }

    while (!canAffordConstruction(buildVikingCost)) {
        const auto initialMineral = _minerals;
        const auto initialVespene = _vespene;
        advanceTime();
        if (initialMineral == _minerals || initialVespene == _vespene) {
            return;
        }
    }

    while (!hasFreeStarPort())
    {
        advanceTime();
        if (_starPortAmount < 1) return;
    }

    if (withinPopulationLimit(VIKING_SUPPLY))
    {
        _minerals -= buildVikingCost.minerals;
        _vespene -= buildVikingCost.vespene;
        _incomingVikings += 1;

        auto c = Construction(buildVikingCost.buildTime, shared_from_this(), &State::addViking);
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

// void Sc2::State::setBiases(const std::shared_ptr<std::map<int, std::tuple<double, double> > > &combatBiases) {
//     _combatBiases = combatBiases;
// }
//
// void Sc2::State::setEnemyActions(const std::shared_ptr<std::map<int, Action> > &enemyActions) {
//     _enemyActions = enemyActions;
// }

