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
        int incomingVespeneCollectors = 0;
        int _populationLimit = 15;
        std::vector<Base> _bases = std::vector{Base()}; // (maybe) Replace with list
        std::list<Construction> _constructions{}; // Replace with list
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

        ActionCost buildWorkerCost = ActionCost(50, 0, 12);
        ActionCost buildBaseCost = ActionCost(400, 0, 71);
        ActionCost buildHouseCost = ActionCost(100, 0, 21);
        ActionCost buildVespeneCollectorCost = ActionCost(75, 0, 21);

        void advanceConstructions();
        void advanceResources();
        void advanceOccupiedWorkers();
        void advanceTime(int amount);

        bool hasEnoughMinerals(const int cost) const { return _minerals >= cost; };
        bool hasEnoughVespene(const int cost) const { return _vespene >= cost; }
        bool hasUnoccupiedWorker() const { return _population - _occupiedWorkerTimers.size() > 0; }

        void occupyWorker(int time) {
            _occupiedWorkerTimers.emplace_back(time);
        };

        void addVespeneCollector();

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

        int getValue() const { return mineralGainedPerTimestep() + vespeneGainedPerTimestep(); }

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
            _constructions = std::list<Construction>();
            _occupiedWorkerTimers = state._occupiedWorkerTimers;
        };

        State() = default;

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
            str += "}\n";
            // str += "    Population: " + std::to_string(_population) + "\n";
            // str += "    PopulationLimit: " + std::to_string(_populationLimit) + "\n";
            // str += "    numberOfBases: " + std::to_string(_bases.size()) + "\n";
            // str += "}";

            return str;
        }
    };

    inline std::ostream &operator<<(std::ostream &os, const State &state) {
        os << state.toString();
        return os;
    }
}
