#pragma once
#include <vector>

#include "Base.h"
#include "Construction.h"

namespace Sc2 {
    class State : public std::enable_shared_from_this<State> {
        int _minerals = 0;
        int _vespene = 0;
        int _population = 5;
        int _incomingPopulation = 0;
        int _populationLimit = 15;
        std::vector<Base> _bases{}; // (maybe) Replace with list
        std::vector<Construction> _constructions{}; // Replace with list

        std::vector<int> _occupiedWorkerTimers{};

        struct ActionCost {
            int minerals;
            int vespene;
            int buildTime;

            ActionCost(const int minerals, const int vespene, const int buildTime): minerals(minerals),
                vespene(vespene),
                buildTime(buildTime) {
            }
        };

        ActionCost buildWorkerCost = ActionCost(1, 2, 3);
        ActionCost buildBaseCost = ActionCost(1, 2, 3);
        ActionCost buildHouseCost = ActionCost(1, 2, 3);
        ActionCost buildVespeneCollectorCost = ActionCost(1, 2, 3);


        void advanceConstructions(int amount);

        void advanceResources(int amount);

        void advanceOccupiedWorkers(int amount);

        void advanceTime(int amount);

        void addWorker() {
            _population += 1;
            _incomingPopulation -= 1;
        }

        void addHouse() {
            _populationLimit += 8;
        }

        void occupyWorker(int time) {
            _occupiedWorkerTimers.emplace_back(time);
        };

    public:
        [[nodiscard]] int getMinerals() const { return _minerals; }
        [[nodiscard]] int getVespene() const { return _vespene; }
        [[nodiscard]] int getIncomingPopulation() const { return _incomingPopulation; }
        [[nodiscard]] int getPopulationLimit() const { return _populationLimit; }
        [[nodiscard]] int getPopulation() const { return _population; }
        std::vector<Base> getBases() { return _bases; }
        std::vector<Construction> getConstructions() const { return _constructions; }

        void buildWorker();


        void buildHouse();

        void buildBase();

        void buildVespeneCollector();

        void wait();

        void wait(int amount);

        static std::shared_ptr<State> DeepCopy(const State &state);

        State(const State &state) : enable_shared_from_this(state) {
            _minerals = state._minerals;
            _vespene = state._vespene;
            _population = state._population;
            _incomingPopulation = state._incomingPopulation;
            _populationLimit = state._populationLimit;

            buildWorkerCost = state.buildWorkerCost;
            buildBaseCost = state.buildBaseCost;
            buildHouseCost = state.buildHouseCost;
            buildVespeneCollectorCost = state.buildVespeneCollectorCost;

            _bases = std::vector<Base>();
            _constructions = std::vector<Construction>();
            _occupiedWorkerTimers = std::vector<int>();
        };

        State() = default;

        friend State;
    };
}
