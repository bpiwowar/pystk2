#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "modes/world.hpp"
#include "simulation.hpp"

namespace py = pybind11;


struct PyWorld {
	std::shared_ptr<World> world;
	
	static void define(py::object m) {
		py::class_<PyWorld, std::shared_ptr<PyWorld>> c(m, "World");
		c.def(py::init<>())
// #define R(x, d) .def_readonly(#x, &PyWorld>::x, d)
// 		  R(players, "State of active players")
// 		  R(karts, "State of karts")
// 		  R(items, "State of items")
// 		  R(time, "Game time")
// 		  R(soccer, "Soccer match info")
// 		  R(ffa, "Free for all match info")
// #undef R
		 .def("test", &PyWorld::test, "Test world simulation")
		//  .def("__repr__", [](const PyWorld> &k) { return "<WorldState #karts="+std::to_string(k.karts.size())+">"; })
		//  .def_static("set_kart_location", &PyWorld>::set_kart_location, py::arg("kart_id"), py::arg("position"), py::arg("rotation")=PyQuaternion{0,0,0,1}, py::arg("speed")=0, "Move a kart to a specific location.");
		// TODO: Add pickling and make sure players are updated
		// add_pickle(c);
        ;
	}
    void test() {
    }

};


void defineSimulation(py::object m) {
	PyWorld::define(m);
};

