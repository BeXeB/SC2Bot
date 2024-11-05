#pragma once
#include <iostream>
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
        std::vector<Base> _bases = std::vector{Base()}; // (maybe) Replace with list
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

        bool hasEnoughMinerals(const int cost) const { return _minerals >= cost; };
        bool hasEnoughVespene(const int cost) const { return _vespene >= cost; }
        bool hasUnoccupiedWorker() const { return _population - _occupiedWorkerTimers.size() > 0; }

        void addBase() {
            _populationLimit += 15;
            _bases.emplace_back();
        }

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
        int id = 0;
        [[nodiscard]] int getMinerals() const { return _minerals; }
        [[nodiscard]] int getVespene() const { return _vespene; }
        [[nodiscard]] int getIncomingPopulation() const { return _incomingPopulation; }
        [[nodiscard]] int getPopulationLimit() const { return _populationLimit; }
        [[nodiscard]] int getPopulation() const { return _population; }
        int getOccupiedPopulation() const { return static_cast<int>(_occupiedWorkerTimers.size()); }
        // std::vector<Base> getBases() { return _bases; }
        std::vector<Construction> getConstructions() const { return _constructions; }
        std::vector<Base> getBases() const { return _bases; }
        ActionCost getBuildWorkerCost() const { return buildWorkerCost; }
        ActionCost getBuildBaseCost() const { return buildBaseCost; }
        ActionCost getBuildHouseCost() const { return buildHouseCost; }
        ActionCost getBuildVespeneCollectorCost() const { return buildVespeneCollectorCost; }


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
            _occupiedWorkerTimers = state._occupiedWorkerTimers;
        };

        State() = default;
    };

    inline std::ostream &operator<<(std::ostream &os, const State &state) {
        os
                << "State: " << state.id << " {" << std::endl
                << "minerals: " << state.getMinerals() << std::endl
                << "vespene: " << state.getVespene() << std::endl
                << "Constructions: " << state.getConstructions().size() << std::endl
                << "population: " << state.getPopulation() << std::endl
                << "incomingPopulation: " << state.getIncomingPopulation() << std::endl
                << "populationLimit: " << state.getPopulationLimit() << std::endl
                << "number of bases: " << state.getBases().size() << std::endl
                << "}" << std::endl;
        return os;
    }
}
