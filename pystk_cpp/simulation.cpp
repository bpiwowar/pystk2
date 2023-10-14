#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "race/race_manager.hpp"
#include "modes/world.hpp"

#include "pystk.hpp"
#include "simulation.hpp"

namespace py = pybind11;

class ConfigurationKeeper {
public:
	World * world;
	RaceManager * race_manager;

	ConfigurationKeeper() {
		// Save current values
		world = World::getWorld();
		World::clear();

		race_manager = RaceManager::get();
		RaceManager::clear();
	}

	~ConfigurationKeeper() {
		// Put back old values

		// World::setWorld(world);
		// RaceManager::set(race_manager);
	}
};

struct PyRace {
	std::shared_ptr<World> world;
	PyRace(const PyRace &) = delete;
	PyRace& operator=(const PyRace &) = delete;

	PyRace(const PySTKRaceConfig & config) {
		
		ConfigurationKeeper keeper;

		// World::setWorld(new World());
		// RaceManager::create();

		// auto race_manager = RaceManager::get();
		// race_manager->setDifficulty(RaceManager::Difficulty(config.difficulty));
		// race_manager->setMinorMode(translate_mode(config.mode));
		// race_manager->setNumPlayers(config.players.size());
		// for(int i=0; i<config.players.size(); i++) {
		// 	std::string kart = config.players[i].kart.size() ? config.players[i].kart : (std::string)UserConfigParams::m_default_kart;
		// 	const KartProperties *prop = kart_properties_manager->getKart(kart);
		// 	if (!prop)
		// 		kart = UserConfigParams::m_default_kart;
		// 	race_manager->setPlayerKart(i, kart);
		// 	race_manager->setKartTeam(i, (KartTeam)config.players[i].team);
		// }
		// race_manager->setReverseTrack(config.reverse);
		// if (config.track.length())
		// 	race_manager->setTrack(config.track);
		// else
		// 	race_manager->setTrack("lighthouse");
		
		// race_manager->setNumLaps(config.laps);
		// race_manager->setNumKarts(config.num_kart);
		// race_manager->setMaxGoal(1<<30);
	}

	void test() {

    }

	static void define(py::object m) {
		py::class_<PyRace, std::shared_ptr<PyRace>> c(m, "World");
        
		c
		.def(py::init<const PySTKRaceConfig &>(), py::arg("config"))
		.def("test", &PyRace::test, "Test world simulation")
		//  .def("__repr__", [](const PyRace> &k) { return "<WorldState #karts="+std::to_string(k.karts.size())+">"; })
		//  .def_static("set_kart_location", &PyRace>::set_kart_location, py::arg("kart_id"), py::arg("position"), py::arg("rotation")=PyQuaternion{0,0,0,1}, py::arg("speed")=0, "Move a kart to a specific location.");
		// TODO: Add pickling and make sure players are updated
		// add_pickle(c);
        ;
	}

};


void defineSimulation(py::object m) {
	PyRace::define(m);
};

