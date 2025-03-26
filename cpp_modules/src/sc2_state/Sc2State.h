#pragma once
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <vector>
#include <sstream>

#include "Base.h"
#include "Construction.h"
#include "ActionEnum.h"

namespace Sc2 {
    class State : public std::enable_shared_from_this<State> {
        int _minerals = 50;
        int _vespene = 0;
        int _workerPopulation = 12;
        int _MAX_SCOUT_POPULATION = 1;
        int _marinePopulation = 0;
        int _tankPopulation = 0;
        int _vikingPopulation = 0;
        int _incomingWorkers = 0;
        int _incomingMarines = 0;
        int _incomingTanks = 0;
        int _incomingVikings = 0;
        int _incomingVespeneCollectors = 0;
        const int MAX_POPULATION_LIMIT = 200;
        const int MAX_BASES = 17;
        int _populationLimit = 15;
        int _barracksAmount = 0;
        int _factoryAmount = 0;
        int _factoryTechLabAmount = 0;
        int _starPortAmount = 0;
        std::vector<Base> _bases = std::vector{Base()};
        std::list<Construction> _constructions{};
        std::vector<int> _occupiedWorkerTimers{};
        std::mt19937 _rng;
        std::shared_ptr<std::map<int, Action> > _enemyActions;
        std::shared_ptr<std::map<int, std::tuple<double, double> > > _combatBiases;

        int _enemyCombatUnits = 0;

        const int _endTime;
        int _currentTime = 0;
        bool _hasHouse = false;

        bool _onRollout = false;
        bool _wasAttacked = false;
        bool _incomingHouse = false;
        bool _incomingBarracks = false;
        int _incomingFactory = 0;
        int _incomingFactoryTechLab = 0;
        int _incomingBases = 0;

        struct ActionCost {
            int minerals;
            int vespene;
            int buildTime;

            ActionCost(const int minerals, const int vespene, const int buildTime): minerals(minerals),
                vespene(vespene),
                buildTime(buildTime) {
            }
        };

        ActionCost buildWorkerCost = ActionCost(50, 0, 12);
        ActionCost buildBaseCost = ActionCost(400, 0, 71);
        ActionCost buildHouseCost = ActionCost(100, 0, 21);
        ActionCost buildVespeneCollectorCost = ActionCost(75, 0, 21);
        ActionCost buildMarineCost = ActionCost(50, 0, 18);
        ActionCost buildBarracksCost = ActionCost(150, 0, 46);
        ActionCost buildFactoryCost = ActionCost(150, 100, 43);
        ActionCost buildTechLabCost = ActionCost(50, 25, 18);
        ActionCost buildStarPortCost = ActionCost(150,100,36);
        ActionCost buildTankCost = ActionCost(150, 125, 32);
        ActionCost buildVikingCost = ActionCost(150,75,30);

        void advanceConstructions();
        void advanceResources();
        void advanceOccupiedWorkers();
        void advanceEnemyAction();
        void advanceTime();

        bool hasEnoughMinerals(const int cost) const { return _minerals >= cost; };
        bool hasEnoughVespene(const int cost) const { return _vespene >= cost; }
        bool hasUnoccupiedWorker() const { return _workerPopulation - _occupiedWorkerTimers.size() > 0; }

        void occupyWorker(int time) {
            _occupiedWorkerTimers.emplace_back(time);
        };

        void addVespeneCollector();

        void addBase() {
            _populationLimit += 15;
            _incomingBases--;
            _populationLimit = _populationLimit >= MAX_POPULATION_LIMIT ? MAX_POPULATION_LIMIT : _populationLimit;
            _bases.emplace_back();
        }

        void addWorker() {
            _workerPopulation += 1;
            _incomingWorkers -= 1;
        }

        void addMarine() {
            _marinePopulation += 1;
            _incomingMarines -= 1;
        }

        void addTank() {
            _tankPopulation += 1;
            _incomingTanks -= 1;
        }

        void addViking() {
            _vikingPopulation += 1;
            _incomingVikings -= 1;
        }

        void addBarracks() {
            _barracksAmount += 1;
            _incomingBarracks = false;
        }

        void addFactory() {
            _factoryAmount += 1;
            _incomingFactory -= 1;
        }

        void addFactoryTechLab() {
            _factoryTechLabAmount += 1;
            _incomingFactoryTechLab -= 1;
        }

        void addStarPort() {
            _starPortAmount +=1;
        }

        void addHouse() {
            _populationLimit += 8;
            _populationLimit = _populationLimit >= MAX_POPULATION_LIMIT ? MAX_POPULATION_LIMIT : _populationLimit;
            _hasHouse = true;
            _incomingHouse = false;
        }

        /*
         *This method is not currently being used, but is kept for future purposes.
         */
        void destroyPlayerBase() {
            _workerPopulation = _workerPopulation >= 10 ? _workerPopulation - 10 : 0;
            // _marinePopulation -= _marinePopulation >= 10 ? _marinePopulation - 10 : 0;
            // _barracksAmount = _barracksAmount <= 0 ? 0 : _barracksAmount - 1;

            if (!_bases.empty()) {
                _bases.pop_back();
                _populationLimit -= 15;
            }
        };

        void simulateBattle() {
            const double initialUnits = _marinePopulation;
            const double initialEnemies = _enemyCombatUnits;
            const auto bias = (*_combatBiases)[_currentTime];
            const auto updatedUnits = std::floor(initialUnits - (initialEnemies * std::get<0>(bias)));
            _marinePopulation = updatedUnits > 0 ? updatedUnits : 0;

            const auto updatedEnemies = std::floor(initialEnemies - (initialUnits * std::get<1>(bias)));
            _enemyCombatUnits = updatedEnemies > 0 ? updatedEnemies : 0;
        }

    public:
        int id = 0;
        [[nodiscard]] int getMinerals() const { return _minerals; }
        void setMinerals(int minerals) { _minerals = minerals; }
        [[nodiscard]] int getVespene() const { return _vespene; }
        [[nodiscard]] int getIncomingPopulation() const { return _incomingWorkers + _incomingMarines + _incomingTanks * TANK_SUPPLY + _incomingVikings * VIKING_SUPPLY; }
        [[nodiscard]] int getIncomingWorkers() const { return _incomingWorkers; }
        [[nodiscard]] int getIncomingMarines() const { return _incomingMarines; }
        [[nodiscard]] int getPopulationLimit() const { return _populationLimit; }
        [[nodiscard]] int getPopulation() const { return _workerPopulation + _marinePopulation + _tankPopulation * TANK_SUPPLY + _vikingPopulation * VIKING_SUPPLY; }
        int getWorkerPopulation() const { return _workerPopulation; }
        int getMarinePopulation() const { return _marinePopulation; }
        int getOccupiedPopulation() const { return static_cast<int>(_occupiedWorkerTimers.size()); }
        [[nodiscard]] int getEnemyCombatUnits() const { return _enemyCombatUnits; }
        std::list<Construction> getConstructions() const { return _constructions; }
        std::vector<Base> getBases() const { return _bases; }
        [[nodiscard]] int getBarracksAmount() const { return _barracksAmount; }
        [[nodiscard]] int getFactoryAmount() const { return _factoryAmount; }
        [[nodiscard]] int getFactoryTechLabAmount() const { return _factoryTechLabAmount; }
        [[nodiscard]] int getStarPortAmount() const { return _starPortAmount; }

        ActionCost getBuildWorkerCost() const { return buildWorkerCost; }
        ActionCost getBuildBaseCost() const { return buildBaseCost; }
        ActionCost getBuildHouseCost() const { return buildHouseCost; }
        ActionCost getBuildVespeneCollectorCost() const { return buildVespeneCollectorCost; }
        ActionCost getBuildBarracksCost() const { return buildBarracksCost; }
        ActionCost getBuildMarineCost() const { return buildMarineCost; }

        bool hasUnoccupiedGeyser() const;
        bool canAffordConstruction(const ActionCost &actionCost) const;
        bool populationLimitReached() const;
        bool withinPopulationLimit(int populationIncrease) const;
        bool hasFreeBase() const;
        bool hasFreeBarracks() const;
        bool hasFreeFactoryTechLab() const;
        bool hasFreeStarPort() const;

        int mineralGainedPerTimestep() const;
        int vespeneGainedPerTimestep() const;

        int getMineralWorkers() const;
        int getVespeneWorkers() const;
        int getScoutWorkers() const;

        void buildWorker();
        void buildHouse();

        void setBiases(const std::shared_ptr<std::map<int, std::tuple<double, double> > > &combatBiases);

        void setEnemyActions(const std::shared_ptr<std::map<int, Action> > &enemyActions);


        void buildBase();
        void buildVespeneCollector();
        void buildBarracks();
        void buildFactory();
        void buildFactoryTechLab();
        void buildStarPort();
        void buildMarine();
        void buildTank();
        void buildViking();
        void addEnemyUnit() { _enemyCombatUnits += 1; }

        void attackPlayer() {
            _wasAttacked = true;
        }

        void wait();
        void wait(int amount);

        int getVespeneCollectorsAmount();
        int getVespeneGeysersAmount();

        void performAction(const Action action) {
            switch (action) {
                case Action::buildWorker:
                    buildWorker();
                    break;
                case Action::buildHouse:
                    buildHouse();
                    break;
                case Action::buildBase:
                    buildBase();
                    break;
                case Action::buildVespeneCollector:
                    buildVespeneCollector();
                    break;
                case Action::buildBarracks:
                    buildBarracks();
                    break;
                case Action::buildMarine:
                    buildMarine();
                    break;
                case Action::none:
                case Action::attackPlayer:
                case Action::addEnemyUnit:
                    break;
            }
        }

        std::vector<Action> getLegalActions() const;


        double getValue() const {
            return softmax(std::vector{
                               static_cast<double>(_marinePopulation) / 4, static_cast<double>(_enemyCombatUnits) / 4
                           }, 0);
        }

        static double softmax(std::vector<double> vector, const int index) {
            double sum = 0;
            for (const auto &value: vector) {
                sum += std::exp(value);
            }
            return std::exp(vector.at(index)) / sum;
        }

        std::vector<int> &getOccupiedWorkerTimers() { return _occupiedWorkerTimers; }


        bool endTimeReached() const {
            return _currentTime >= _endTime;
        }

        bool GameOver() {
            return endTimeReached() || _wasAttacked;
        }

        int getCurrentTime() const { return _currentTime; }
        void resetCurrentTime() { _currentTime = 0; }

        Action generateEnemyAction() {
            // Over the span of 60 seconds we assume that the enemy:
            // Specifies how many enemy units will be built
            constexpr double buildUnitAction = 8;
            // Specifies how many times the enemy will attack
            constexpr double attackAction = 0.4;
            // Specifies how many times the enemy will do nothing
            constexpr double noneAction = 60 - buildUnitAction - attackAction;

            const auto actionWeights = {noneAction, buildUnitAction, attackAction};
            std::discrete_distribution<int> dist(actionWeights.begin(), actionWeights.end());
            // 0: None, 1: Build unit, 2: Attack
            switch (dist(_rng)) {
                case 1:
                    return Action::addEnemyUnit;
                case 2:
                    return Action::attackPlayer;
                default:
                    return Action::none;
            }
        }


        static std::shared_ptr<State> DeepCopy(const State &state, bool onRollout = false);

        static std::shared_ptr<State> StateBuilder(const int minerals,
                                                   const int vespene,
                                                   const int workerPopulation,
                                                   const int marinePopulation,
                                                   const int incomingWorkers,
                                                   const int incomingMarines,
                                                   const int populationLimit,
                                                   const std::vector<Base> &bases,
                                                   const int barracksAmount,
                                                   std::list<Construction> &constructions,
                                                   const std::vector<int> &occupiedWorkerTimers,
                                                   const int current_time,
                                                   const int endTime, const int enemyCombatUnits,
                                                   const bool hasHouse, const bool incomingHouse,
                                                   const int incomingBases = 0,
                                                   const int maxBases = 17) {
            const unsigned int seed = std::random_device{}();
            return InternalStateBuilder(minerals, vespene, workerPopulation, marinePopulation, incomingWorkers,
                                        incomingMarines,
                                        populationLimit, bases, barracksAmount, constructions, occupiedWorkerTimers,
                                        current_time,
                                        endTime, enemyCombatUnits, seed, hasHouse, nullptr, nullptr,
                                        incomingHouse, incomingBases, maxBases);
        }

        /*
         * An internal version of the StateBuilder, which has enemyActions and combatBiases as pointers, as well as a seed.
         * These cannot be set via the python script.
         */
        static std::shared_ptr<State> InternalStateBuilder(const int minerals,
                                                           const int vespene,
                                                           const int workerPopulation,
                                                           const int marinePopulation,
                                                           const int incomingWorkers,
                                                           const int incomingMarines,
                                                           const int populationLimit,
                                                           const std::vector<Base> &bases,
                                                           const int barracksAmount,
                                                           std::list<Construction> &constructions,
                                                           const std::vector<int> &occupiedWorkerTimers,
                                                           int current_time,
                                                           int endTime, const int enemyCombatUnits, unsigned int seed,
                                                           bool hasHouse,
                                                           const std::shared_ptr<std::map<int, Action> > &enemyActions,
                                                           const std::shared_ptr<std::map<int, std::tuple<double,
                                                               double> > > &combatBiases, const bool incomingHouse,
                                                           const int incomingBases,
                                                           int maxBases) {
            auto state = std::make_shared<State>(minerals, vespene, workerPopulation, marinePopulation, incomingWorkers,
                                                 incomingMarines, populationLimit,
                                                 bases, barracksAmount, occupiedWorkerTimers, current_time, endTime,
                                                 enemyCombatUnits, seed, hasHouse, enemyActions, combatBiases,
                                                 incomingHouse, incomingBases, maxBases);

            for (auto &construction: constructions) {
                construction.setState(state);
                state->_constructions.emplace_back(construction);
            }

            return state;
        };

        State(const int minerals, const int vespene, const int workerPopulation, const int marinePopulation,
              const int incomingWorkers, const int incomingMarines, const int populationLimit, std::vector<Base> bases,
              const int barracksAmount, std::vector<int> occupiedWorkerTimers, const int currentTime, const int endTime,
              const int enemyCombatUnits, const unsigned int seed, const bool hasHouse,
              const std::shared_ptr<std::map<int, Action> > &enemyActions,
              const std::shared_ptr<std::map<int, std::tuple<double, double> > > &combatBiases,
              const bool incomingHouse,
              const int incomingBases = 0, const int maxBases = 17): _minerals(minerals),
                                                                     _vespene(vespene),
                                                                     _workerPopulation(workerPopulation),
                                                                     _marinePopulation(marinePopulation),
                                                                     _incomingWorkers(incomingWorkers),
                                                                     _incomingMarines(incomingMarines),
                                                                     MAX_BASES(maxBases),
                                                                     _populationLimit(populationLimit),
                                                                     _barracksAmount(barracksAmount),
                                                                     _bases(std::move(bases)),
                                                                     _constructions(std::list<Construction>()),
                                                                     _occupiedWorkerTimers(
                                                                         std::move(occupiedWorkerTimers)),
                                                                     _enemyCombatUnits(enemyCombatUnits),
                                                                     _endTime(endTime),
                                                                     _currentTime(currentTime),
                                                                     _hasHouse(hasHouse),
                                                                     _incomingHouse(incomingHouse),
                                                                     _incomingBases(incomingBases) {
            _rng = std::mt19937(seed);
            _enemyActions = enemyActions;
            _combatBiases = combatBiases;
        };

        State(const State &state) : enable_shared_from_this(state), MAX_BASES(state.MAX_BASES),
                                    _endTime(state._endTime), _currentTime(state._currentTime) {
            _minerals = state._minerals;
            _vespene = state._vespene;
            _barracksAmount = state._barracksAmount;
            _workerPopulation = state._workerPopulation;
            _marinePopulation = state._marinePopulation;
            _incomingWorkers = state._incomingWorkers;
            _incomingMarines = state._incomingMarines;
            _populationLimit = state._populationLimit;
            _incomingVespeneCollectors = state._incomingVespeneCollectors;

            buildWorkerCost = state.buildWorkerCost;
            buildBaseCost = state.buildBaseCost;
            buildHouseCost = state.buildHouseCost;
            buildVespeneCollectorCost = state.buildVespeneCollectorCost;

            _hasHouse = state._hasHouse;
            _bases = std::vector<Base>();
            _constructions = std::list<Construction>();
            _occupiedWorkerTimers = state._occupiedWorkerTimers;

            _enemyCombatUnits = state._enemyCombatUnits;

            _rng = state._rng;

            _combatBiases = state._combatBiases;
            _enemyActions = state._enemyActions;

            _incomingHouse = state._incomingHouse;
            _incomingBases = state._incomingBases;
        };

        explicit State(const int endTime, const unsigned int seed,
                       const std::shared_ptr<std::map<int, Action> > &enemyActions,
                       const std::shared_ptr<std::map<int, std::tuple<double, double> > > &combatBiases): _endTime(
            endTime) {
            _rng = std::mt19937(seed);
            _combatBiases = combatBiases;
            _enemyActions = enemyActions;
        }

        State(): _rng(std::mt19937(std::random_device{}())), _endTime(1000) {
        }

        std::string toString() const {
           std::ostringstream str;
           str << "State: " << id << "{\n"
           << "    Minerals: " << _minerals << "\n"
           << "    Vespene: " << _vespene << "\n"
           << "    IncomingVespeneCollectors: " << _incomingVespeneCollectors << "\n"
           << "    Constructions: " << _constructions.size() << "\n"
           << "    Occupied workers: " << _occupiedWorkerTimers.size() << "\n"
           << "    Population: " << getPopulation() << "\n"
           << "    Marine population: " << _marinePopulation << "\n"
           << "    PopulationLimit: " << _populationLimit << "\n"
           << "    MaxPopulationLimit: " << MAX_POPULATION_LIMIT << "\n"
           << "    MaxBases: " << MAX_BASES << "\n"
           << "    IncomingPopulation: " << getIncomingPopulation() << "\n"
           << "    IncomingMarines: " << getIncomingMarines() << "\n"
           << "    Number of bases: " << _bases.size() << "\n"
           << "    Number of barracks: " << _barracksAmount << "\n"
           << "    Number of constructions: " << _constructions.size() << "\n"
           << "    Enemy combat units: " << _enemyCombatUnits << "\n"
           << "    current_time: " << _currentTime << "\n"
           << "    EndTime: " << _endTime << "\n"
           << "    HasHouse: " << _hasHouse << "\n"
           << "}\n";

            return str.str();
        }

        friend Construction;
    };

    inline std::ostream &operator<<(std::ostream &os, const State &state) {
        os << state.toString();
        return os;
    }
}
