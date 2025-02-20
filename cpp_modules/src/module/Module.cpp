#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma once
#include "Sc2State.h"
#include "Mcts.h"

namespace py = pybind11;

namespace pymodule {
	PYBIND11_MODULE(sc2_mcts, module) { 
		module.doc() = "sc2_mcts";


		module.def("state_builder", &Sc2::State::StateBuilder, "A function that builds a State",
						 py::arg("minerals"),
						 py::arg("vespene"),
						 py::arg("worker_population"),
						 py::arg("marine_population"),
						 py::arg("incoming_workers"),
						 py::arg("incoming_marines"),
						 py::arg("population_limit"),
						 py::arg("bases"),
						 py::arg("barracks_amount"),
						 py::arg("constructions"),
						 py::arg("occupied_worker_timers"),
						 py::arg("current_time"),
						 py::arg("end_time"),
						 py::arg("enemy_combat_units"),
						 py::arg("has_house"),
						 py::arg("max_bases"));

		py::class_<Sc2::State, std::shared_ptr<Sc2::State> >(module, "State") 
				.def(py::init<>())
				.def("perform_action", &Sc2::State::performAction,
					py::arg("action"))
				.def("to_string", &Sc2::State::toString)
				.def("get_minerals", &Sc2::State::getMinerals)
				.def("get_vespene", &Sc2::State::getVespene)
				.def("get_incoming_population", &Sc2::State::getIncomingPopulation)
				.def("get_population_limit", &Sc2::State::getPopulationLimit)
				.def("get_population", &Sc2::State::getPopulation)
				.def("get_worker_population", &Sc2::State::getWorkerPopulation)
		        .def("get_marine_population", &Sc2::State::getMarinePopulation)
				.def("get_incoming_workers", &Sc2::State::getIncomingWorkers)
				.def("get_incoming_marines", &Sc2::State::getIncomingMarines)
				.def("get_enemy_units", &Sc2::State::getEnemyCombatUnits)
				.def("get_occupied_population", &Sc2::State::getOccupiedPopulation)
				.def("mineral_gained_per_time_step", &Sc2::State::mineralGainedPerTimestep)
				.def("vespene_gained_per_time_step", &Sc2::State::vespeneGainedPerTimestep)
				.def("get_mineral_workers", &Sc2::State::getMineralWorkers)
				.def("get_vespene_workers", &Sc2::State::getVespeneWorkers)
				.def("get_constructions", &Sc2::State::getConstructions)
				.def("get_value", &Sc2::State::getValue)
				.def("get_barracks_amount", &Sc2::State::getBarracksAmount)
				.def_readwrite("id", &Sc2::State::id);

		py::class_<Sc2::Base>(module, "Base")
		.def(py::init<const int, const int, const int, const int>(),
			py::arg("id"),
			py::arg("mineral_fields"),
			py::arg("vespene_geysers"),
			py::arg("vespene_collectors"))
		.def(py::init<>())
		.def_readwrite("id", &Sc2::Base::id)
		.def_readwrite("mineral_fields", &Sc2::Base::mineralFields)
		.def_readwrite("vespene_geysers", &Sc2::Base::vespeneGeysers)
		.def_readwrite("vespene_collectors",  &Sc2::Base::vespeneCollectors);

		py::enum_<Action>(module, "Action")
		.value("none", Action::none)
		.value("build_worker", Action::buildWorker)
		.value("build_base", Action::buildBase)
		.value("build_vespene_collector", Action::buildVespeneCollector)
		.value("build_house", Action::buildHouse)
		.value("build_marine", Action::buildMarine)
		.value("build_barracks", Action::buildBarracks)
		.value("attack_player", Action::attackPlayer)
		.value("add_enemy_unit", Action::addEnemyUnit);

		py::enum_<ValueHeuristic>(module, "ValueHeuristic")
		.value("UCT", ValueHeuristic::UCT)
		.value("UCB1Normal2", ValueHeuristic::UCB1Normal2)
		.value("UCB1Normal", ValueHeuristic::UCB1Normal)
		.value("EpsilonGreedy", ValueHeuristic::EpsilonGreedy);

		py::enum_<RolloutHeuristic>(module, "RolloutHeuristic")
		.value("random", RolloutHeuristic::Random)
		.value("weighted_choice", RolloutHeuristic::WeightedChoice);
		
		py::class_<Sc2::Construction>(module, "Construction")
		.def(py::init<const int, Action>(),
			py::arg("time_left"),
			py::arg("action"))
		.def("get_time_left", &Sc2::Construction::getTimeLeft)
		.def("to_string", &Sc2::Construction::toString);

		py::class_<Sc2::Mcts::Node, std::shared_ptr<Sc2::Mcts::Node>>(module, "Node")
		.def("to_string", &Sc2::Mcts::Node::toString)
		.def("get_state", &Sc2::Mcts::Node::getState);

		py::class_<Sc2::Mcts::Mcts>(module, "Mcts") 
		.def(py::init<const std::shared_ptr<Sc2::State>, const unsigned int, const int, const double, const ValueHeuristic, RolloutHeuristic>(),
			py::arg("state"),
			py::arg("seed"),
			py::arg("rollout_end_time"),
			py::arg("exploration"),
			py::arg("value_heuristic"),
			py::arg("rollout_heuristic"))
		.def("update_root_state", static_cast<void (Sc2::Mcts::Mcts::*)(
			const std::shared_ptr<Sc2::State>& state
			)>(&Sc2::Mcts::Mcts::updateRootState),
			py::arg("state"))
		.def("get_root_state", &Sc2::Mcts::Mcts::getRootState)
		.def("get_root_node", &Sc2::Mcts::Mcts::getRootNode)
		.def("to_string", &Sc2::Mcts::Mcts::toString)
		.def("start_search", &Sc2::Mcts::Mcts::startSearchThread)
		.def("stop_search", &Sc2::Mcts::Mcts::stopSearchThread)
		.def("start_search_rollout", &Sc2::Mcts::Mcts::startSearchRolloutThread,
			py::arg("number_of_rollouts"))
		.def("get_best_action", &Sc2::Mcts::Mcts::getBestAction)
		.def("perform_action", &Sc2::Mcts::Mcts::performAction,
			py::arg("action"))
		.def("get_number_of_rollouts", &Sc2::Mcts::Mcts::getNumberOfRollouts); 
	}
}
