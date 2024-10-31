#include <pybind11/pybind11.h>
#pragma once
#include "Sc2State.h"


namespace py = pybind11;

namespace pymodule
{
	PYBIND11_MODULE(pybind11module, module) {
		module.doc() = "pybind11Module";

		py::class_<sc2::Sc2State>(module, "sc2_state")
		.def(pybind11::init<int, std::string>(), "ctor", py::arg("id"), py::arg("name"))
		.def("print_name", &sc2::Sc2State::printName)
		.def_readwrite("id", &sc2::Sc2State::i)
		.def_readwrite("name", &sc2::Sc2State::name);
	}
}
