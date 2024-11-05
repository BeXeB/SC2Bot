//
// Created by marco on 04/11/2024.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "Sc2State.h"


TEST_CASE("Check that the correct resources are gathered whenever an action is taken") {
	const auto state = std::make_shared<Sc2::State>();
	auto activeWorkers = state->getPopulation() - state->getOccupiedPopulation();

	SUBCASE("Check that wait gives resources") {
		CHECK(state->getVespene() == 0);
		CHECK(state->getMinerals() == 0);

		auto waitTime = 2;
		state->wait(waitTime);
		CHECK(state->getMinerals() == activeWorkers * waitTime);
		CHECK(state->getVespene() == activeWorkers * waitTime);
	}

	state->wait(50);
	auto currentMinerals = state->getMinerals();
	auto currentVespene = state->getVespene();
	SUBCASE(
		"Check that build house takes resources as cost, "
		"and gives resources as time passes, "
		"and occupies a worker") {
		state->buildHouse();

		currentMinerals = currentMinerals - state->getBuildHouseCost().minerals + (activeWorkers) * 1;
		CHECK(state->getMinerals() == currentMinerals);

		currentVespene = currentVespene - state->getBuildHouseCost().vespene + (activeWorkers) * 1;
		CHECK(state->getVespene() == currentVespene);
	}
	state->wait(50);
	SUBCASE(
		"Check build worker takes resources, "
		"And gives resources as time passes, ") {
		currentMinerals = state->getMinerals();
		currentVespene = state->getVespene();

		state->buildWorker();

		currentMinerals = currentMinerals - state->getBuildWorkerCost().minerals + (activeWorkers) * 1;
		CHECK(state->getMinerals() == currentMinerals);

		currentVespene = currentVespene - state->getBuildWorkerCost().vespene + (activeWorkers) * 1;
		CHECK(state->getVespene() == currentVespene);
	}
}

TEST_CASE("Test that anything in the build queue will eventually be built") {
	const auto state = std::make_shared<Sc2::State>();
	state->wait(100);

	SUBCASE("Check that population increases after a delay when workers are built") {
		auto workers = state->getPopulation();
		state->buildWorker();

		// check that the worker is not added to the population before the build time is over
		for (int i = 0; i < state->getBuildWorkerCost().buildTime; i++) {
			CHECK(state->getPopulation() == workers);
			state->wait();
		}

		CHECK(state->getPopulation() == workers + 1);
	}
	SUBCASE("Check that the population cap increase after a delay when houses are built") {
		auto limit = state->getPopulationLimit();
		state->buildHouse();

		// check that the population limit is not increased before the build time is over
		for (int i = 0; i < state->getBuildHouseCost().buildTime; i++) {
			CHECK(state->getPopulationLimit() == limit);
			state->wait();
		}

		CHECK(state->getPopulationLimit() == limit + 8);
	}
}

TEST_CASE("Test that you can not build more workers than the population cap") {
	const auto state = std::make_shared<Sc2::State>();
	state->wait(400);
	const auto workersToBuild = state->getPopulationLimit() + 100;

	for (auto i = 0; i < workersToBuild; i++) {
		state->buildWorker();
	}

	CHECK(state->getPopulation() == state->getPopulationLimit());
}

TEST_CASE("Test deep copy of states") {
	const auto state1 = std::make_shared<Sc2::State>();

	state1->wait(10);
	state1->buildHouse();
	state1->buildWorker();

	const auto state2 = Sc2::State::DeepCopy(*state1);

	CHECK(state1->getPopulationLimit() == state2->getPopulationLimit());
	CHECK(state1->getPopulation() == state2->getPopulation());
	CHECK(state1->getMinerals() == state2->getMinerals());
	CHECK(state1->getVespene() == state2->getVespene());
	CHECK(state1->getIncomingPopulation() == state2->getIncomingPopulation());
}

TEST_CASE("Test that copies of a state does not share any part of the state") {
	const auto state1 = std::make_shared<Sc2::State>();

	state1->wait(10);
	state1->buildHouse();
	state1->buildWorker();

	const auto state2 = Sc2::State::DeepCopy(*state1);
	state1->wait(50);
	SUBCASE("Check that state 1 is different from state2") {
		CHECK(state1->getPopulationLimit() != state2->getPopulationLimit());
		CHECK(state1->getPopulation() != state2->getPopulation());
		CHECK(state1->getMinerals() != state2->getMinerals());
		CHECK(state1->getVespene() != state2->getVespene());
		CHECK(state1->getIncomingPopulation() != state2->getIncomingPopulation());
	}
	state2->wait(50);
	SUBCASE("Check that if the same actions are taken they will become identical") {
		CHECK(state1->getPopulationLimit() == state2->getPopulationLimit());
		CHECK(state1->getPopulation() == state2->getPopulation());
		CHECK(state1->getMinerals() == state2->getMinerals());
		CHECK(state1->getVespene() == state2->getVespene());
		CHECK(state1->getIncomingPopulation() == state2->getIncomingPopulation());
	}
}

