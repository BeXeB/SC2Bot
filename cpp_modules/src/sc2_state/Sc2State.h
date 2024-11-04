#pragma once
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "Base.h"
#include "Construction.h"

namespace Sc2 {
    class State: public std::enable_shared_from_this<State> {
        int minerals = 0;
        int vespene = 0;
        int population = 0;
        int incomingPopulation = 0;
        int populationLimit = 15;
        std::vector<Base> bases{}; // (maybe) Replace with list
        std::vector<Construction> constructions{}; // Replace with list

        std::vector<int> occupiedWorkerTimers{};

        int workerBuildTime = 3;
        int houseBuildTime = 3;
        int baseBuildTime = 3;
        int vespeneCollectorBuildTime = 3;

        void advanceTime(int amount);
        void addWorker() {
            population += 1;
            incomingPopulation -= 1;
        }
    public:
        [[nodiscard]] int getMinerals() const {return minerals;}
        [[nodiscard]] int getVespene() const {return vespene;}
        [[nodiscard]] int getIncomingPopulation() const {return incomingPopulation;}
        [[nodiscard]] int getPopulationLimit() const {return populationLimit;}
        [[nodiscard]] int getPopulation() const {return population;}
        std::vector<Base> getBases(){return bases;}
        std::vector<Construction> getConstructions() const {return constructions;}

        void buildWorker();
        void buildHouse();
        void buildBase();
        void buildVespeneCollector();
        void wait();

        static std::shared_ptr<State> DeepCopy(const State& state);

        State(const State& state)  : enable_shared_from_this(state) {
            minerals = state.minerals;
            vespene = state.vespene;
            population = state.population;
            incomingPopulation = state.incomingPopulation;
            populationLimit = state.populationLimit;

            workerBuildTime = state.workerBuildTime;
            houseBuildTime = state.houseBuildTime;
            baseBuildTime = state.baseBuildTime;
            vespeneCollectorBuildTime = state.vespeneCollectorBuildTime;

            bases = std::vector<Base>();
            constructions = std::vector<Construction>();
            occupiedWorkerTimers = std::vector<int>();

        };
        State()= default;
        friend State;
    };
}
