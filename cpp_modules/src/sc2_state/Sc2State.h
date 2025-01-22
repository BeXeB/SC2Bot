#pragma once
#include <format>
#include <iostream>
#include <list>
#include <vector>

#include "Base.h"
#include "Construction.h"
#include "ActionEnum.h"

namespace Sc2 {
    class State : public std::enable_shared_from_this<State> {
        int _minerals = 0;
        int _vespene = 0;
        int _population = 5;
        int _incomingPopulation = 0;
        int _incomingVespeneCollectors = 0;
        const int MAX_POPULATION_LIMIT = 200;
        const int MAX_BASES = 17;
        int _populationLimit = 15;
        std::vector<Base> _bases = std::vector{Base()};
        std::list<Construction> _constructions{};
        std::vector<int> _occupiedWorkerTimers{};

        const int _endTime;
        int _currentTime = 0;

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

        void advanceConstructions();
        void advanceResources();
        void advanceOccupiedWorkers();
        void advanceTime();

        bool hasEnoughMinerals(const int cost) const { return _minerals >= cost; };
        bool hasEnoughVespene(const int cost) const { return _vespene >= cost; }
        bool hasUnoccupiedWorker() const { return _population - _occupiedWorkerTimers.size() > 0; }

        void occupyWorker(int time) {
            _occupiedWorkerTimers.emplace_back(time);
        };

        void addVespeneCollector();

        void addBase() {
            _populationLimit += 15;
            _populationLimit = _populationLimit >= MAX_POPULATION_LIMIT ? MAX_POPULATION_LIMIT : _populationLimit;
            _bases.emplace_back();
        }

        void addWorker() {
            _population += 1;
            _incomingPopulation -= 1;
        }

        void addHouse() {
            _populationLimit += 8;
            _populationLimit = _populationLimit >= MAX_POPULATION_LIMIT ? MAX_POPULATION_LIMIT : _populationLimit;
        }

    public:
        int id = 0;
        [[nodiscard]] int getMinerals() const { return _minerals; }
        [[nodiscard]] int getVespene() const { return _vespene; }
        [[nodiscard]] int getIncomingPopulation() const { return _incomingPopulation; }
        [[nodiscard]] int getPopulationLimit() const { return _populationLimit; }
        [[nodiscard]] int getPopulation() const { return _population; }
        int getOccupiedPopulation() const { return static_cast<int>(_occupiedWorkerTimers.size()); }
        std::list<Construction> getConstructions() const { return _constructions; }
        std::vector<Base> getBases() const { return _bases; }

        ActionCost getBuildWorkerCost() const { return buildWorkerCost; }
        ActionCost getBuildBaseCost() const { return buildBaseCost; }
        ActionCost getBuildHouseCost() const { return buildHouseCost; }
        ActionCost getBuildVespeneCollectorCost() const { return buildVespeneCollectorCost; }

        bool hasUnoccupiedGeyser() const;
        bool canAffordConstruction(const ActionCost &actionCost) const;
        bool populationLimitReached() const;
        bool hasFreeBase() const;

        int mineralGainedPerTimestep() const;
        int vespeneGainedPerTimestep() const;

        int getMineralWorkers() const;
        int getVespeneWorkers() const;

        void buildWorker();
        void buildHouse();
        void buildBase();
        void buildVespeneCollector();

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
                case Action::none:
                    break;
            }
        }

        std::vector<Action> getLegalActions() const;

        double getValue() const { return mineralGainedPerTimestep() + 1.5 * vespeneGainedPerTimestep(); }
        std::vector<int> &getOccupiedWorkerTimers() { return _occupiedWorkerTimers; }


        bool endTimeReached() const {
            return _currentTime >= _endTime;
        }

        int getCurrentTime() const { return _currentTime; }
        void resetCurrentTime() { _currentTime = 0; }

        static std::shared_ptr<State> DeepCopy(const State &state);

        static std::shared_ptr<State> StateBuilder(const int minerals,
                                                   const int vespene,
                                                   const int population,
                                                   const int incomingPopulation,
                                                   const int populationLimit,
                                                   const std::vector<Base> &bases,
                                                   std::list<Construction> &constructions,
                                                   const std::vector<int> &occupiedWorkerTimers,
                                                   int current_time,
                                                   int endTime, int maxBases = 17) {
            auto state = std::make_shared<State>(minerals, vespene, population, incomingPopulation, populationLimit,
                                                 bases, occupiedWorkerTimers, current_time, endTime, maxBases);

            for (auto &construction: constructions) {
                construction.setState(state);
                state->_constructions.emplace_back(construction);
            }

            return state;
        };

        State(const int minerals, const int vespene, const int population, const int incomingPopulation,
              const int populationLimit, std::vector<Base> bases, std::vector<int> occupiedWorkerTimers,
              const int currentTime, const int endTime, const int maxBases = 17): _minerals(minerals),
            _vespene(vespene),
            _population(population),
            _incomingPopulation(incomingPopulation),
            MAX_BASES(maxBases),
            _populationLimit(populationLimit),
            _bases(std::move(bases)),
            _constructions(std::list<Construction>()),
            _occupiedWorkerTimers(
                std::move(occupiedWorkerTimers)),
            _endTime(endTime),
            _currentTime(currentTime) {
        };

        State(const State &state) : enable_shared_from_this(state), MAX_BASES(state.MAX_BASES),
                                    _endTime(state._endTime), _currentTime(state._currentTime) {
            _minerals = state._minerals;
            _vespene = state._vespene;
            _population = state._population;
            _incomingPopulation = state._incomingPopulation;
            _populationLimit = state._populationLimit;
            _incomingVespeneCollectors = state._incomingVespeneCollectors;

            buildWorkerCost = state.buildWorkerCost;
            buildBaseCost = state.buildBaseCost;
            buildHouseCost = state.buildHouseCost;
            buildVespeneCollectorCost = state.buildVespeneCollectorCost;

            _bases = std::vector<Base>();
            _constructions = std::list<Construction>();
            _occupiedWorkerTimers = state._occupiedWorkerTimers;
        };

        explicit State(const int endTime): _endTime(endTime) {
        }

        State(): _endTime(1000) {
        }

        std::string toString() const {
            std::string str;
            str += std::format("State: {} ", id) + "{ \n";
            str += std::format("    Minerals: {} \n", _minerals);
            str += std::format("    Vespene: {} \n", _vespene);
            str += std::format("    Constructions: {} \n", _constructions.size());
            str += std::format("    Occupied workers: {} \n", _occupiedWorkerTimers.size());
            str += std::format("    Population: {} \n", _population);
            str += std::format("    PopulationLimit: {} \n", _populationLimit);
            str += std::format("    IncomingPopulation: {} \n", _incomingPopulation);
            str += std::format("    Number of bases: {} \n", _bases.size());
            str += std::format("current_time: {} \n", _currentTime);
            str += std::format("EndTime: {} \n", _endTime);
            str += "}\n";

            return str;
        }

        friend Construction;
    };

    inline std::ostream &operator<<(std::ostream &os, const State &state) {
        os << state.toString();
        return os;
    }
}
