#include <pybind11/pybind11.h>
#pragma once
#include "Sc2State.h"


namespace py = pybind11;

namespace pymodule
{
	PYBIND11_MODULE(pybind11module, module) {
		module.doc() = "pybind11Module";

		py::class_<Sc2::State>(module, "sc2_state")
		.def(pybind11::init<int, std::string>(), "ctor", py::arg("id"), py::arg("name"))
		.def("print_name", &Sc2::State::printName)
		.def_readwrite("id", &Sc2::State::i)
		.def_readwrite("name", &Sc2::State::name);
	}
}
