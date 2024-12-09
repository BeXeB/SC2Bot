#include <pybind11/pybind11.h>
#pragma once
#include "Sc2State.h"
#include "Mcts.h"

namespace py = pybind11;

namespace pymodule {
	PYBIND11_MODULE(sc2_mcts, module) {
		module.doc() = "sc2_mcts";

		py::class_<Sc2::State, std::shared_ptr<Sc2::State> >(module, "sc2_state")
				.def(py::init<>())
				.def("to_string", &Sc2::State::toString)
				.def("get_minerals", &Sc2::State::getMinerals)
				.def("get_vespene", &Sc2::State::getVespene)
				.def("get_incoming_population", &Sc2::State::getIncomingPopulation)
				.def("get_population_limit", &Sc2::State::getPopulationLimit)
				.def("get_population", &Sc2::State::getPopulation)
				.def("get_occupied_population", &Sc2::State::getOccupiedPopulation)
				.def("mineral_gained_per_time_step", &Sc2::State::mineralGainedPerTimestep)
				.def("vespene_gained_per_time_step", &Sc2::State::vespeneGainedPerTimestep)
				.def("get_mineral_workers", &Sc2::State::getMineralWorkers)
				.def("get_vespene_workers", &Sc2::State::getVespeneWorkers)
				.def_readwrite("id", &Sc2::State::id);

		py::class_<Sc2::Base>(module, "Base")
		.def(py::init<const int, const int, const int, const int>())
		.def(py::init<>())
		.def_readwrite("id", &Sc2::Base::id)
		.def_readwrite("mineral_fields", &Sc2::Base::mineralFields)
		.def_readwrite("vespene_geysers", &Sc2::Base::vespeneGeysers)
		.def_readwrite("vespene_collectors",  &Sc2::Base::vespeneCollectors);

		py::enum_<Action>(module, "Action")
		.value("none", Action::none)
		.value("build_worker", Action::buildWorker)
		.value("build_base", Action::buildBase)
		.value("build_vespene_collectors", Action::buildVespeneCollector)
		.value("build_house", Action::buildHouse);
		
		py::class_<Sc2::Construction>(module, "Construction")
		.def(py::init<const int, Action>())
		.def("get_time_left", &Sc2::Construction::getTimeLeft);

		py::class_<Sc2::Mcts::Mcts>(module, "Mcts")
		.def(py::init<>())
		.def("update_root_state", static_cast<void (Sc2::Mcts::Mcts::*)(
							 const int minerals,
							 const int vespene,
							 const int population,
							 const int incomingPopulation,
							 const int populationLimit,
							 const std::vector<Sc2::Base> &bases,
							 std::list<Sc2::Construction> &constructions,
							 const std::vector<int> &occupiedWorkerTimers)>(&Sc2::Mcts::Mcts::updateRootState)
							 )
		.def("search", &Sc2::Mcts::Mcts::search)
		.def("search_rollout", &Sc2::Mcts::Mcts::searchRollout)
		.def("get_best_action", &Sc2::Mcts::Mcts::getBestAction)
		.def("perform_action", &Sc2::Mcts::Mcts::performAction)
		;
		
	}
}
