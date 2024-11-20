#include <pybind11/pybind11.h>
#pragma once
#include "Sc2State.h"


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

		// .def(py::init(), "ctor");
		// .def("to_string", &Sc2::State::toString)
		// .def_readwrite("id", &Sc2::State::id);
	}
}
