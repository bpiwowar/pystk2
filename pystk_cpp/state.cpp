#include <cassert>
#include "state.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "config/stk_config.hpp"
#include "graphics/camera/camera.hpp"
#include "karts/skidding.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "karts/kart_properties.hpp"
#include "items/attachment.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "items/powerup_manager.hpp"
#include "items/powerup_manager.hpp"
#include "modes/world.hpp"
#include "modes/linear_world.hpp"
#include "modes/soccer_world.hpp"
#include "modes/free_for_all.hpp"
#include "modes/three_strikes_battle.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/check_goal.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_sector.hpp"
#include "utils/vec3.hpp"
#include "view.hpp"
#include "pickle.hpp"
#include <physics/btKart.hpp>

namespace py = pybind11;

// AUTO Generated
struct PyAttachment;
void pickle(std::ostream & s, const PyAttachment & o);
void unpickle(std::istream & s, PyAttachment * o);
struct PyPowerup;
void pickle(std::ostream & s, const PyPowerup & o);
void unpickle(std::istream & s, PyPowerup * o);
struct PyKart;
void pickle(std::ostream & s, const PyKart & o);
void unpickle(std::istream & s, PyKart * o);
struct PyItem;
void pickle(std::ostream & s, const PyItem & o);
void unpickle(std::istream & s, PyItem * o);
struct PyCamera;
void pickle(std::ostream & s, const PyCamera & o);
void unpickle(std::istream & s, PyCamera * o);
struct PyPlayer;
void pickle(std::ostream & s, const PyPlayer & o);
void unpickle(std::istream & s, PyPlayer * o);
struct PyTrack;
void pickle(std::ostream & s, const PyTrack & o);
void unpickle(std::istream & s, PyTrack * o);
struct PyWorldState;
void pickle(std::ostream & s, const PyWorldState & o);
void unpickle(std::istream & s, PyWorldState * o);
// End AUTO Generated

typedef std::array<float, 3> PyVec3;
typedef std::array<float, 4> PyQuaternion;

// What a great idea to #define _ !!!
#undef _
namespace pybind11 { namespace detail {
template <> struct type_caster<PyVec3>: array_caster<PyVec3, float, false, 3> {
public:
	static constexpr auto name = _("float3");
};
template <size_t N> struct type_caster<std::array<int,N> >: array_caster<std::array<int,N> , int, false, N> {
public:
	static constexpr auto name = _("int[")+_<N>()+_("]");
};
template <> struct type_caster<PyQuaternion>: array_caster<PyQuaternion, float, false, 4> {
public:
	static constexpr auto name = _("Quaternion");
};
}}

PyVec3 P(const Vec3 & v) {
	return {v.getX(), v.getY(), v.getZ()};
}
Vec3 P(const PyVec3 & v) {
	return Vec3(v[0], v[1], v[2]);
}
void pickle(std::ostream & s, const core::matrix4 & o) {
	s.write((const char*)o.pointer(), 16*sizeof(irr::f32));
}
void unpickle(std::istream & s, core::matrix4 * o) {
	s.read((char*)o->pointer(), 16*sizeof(irr::f32));
}
template<typename T>
void pickle(std::ostream & s, const py::array_t<T> & o) {
	uint32_t n = o.ndim();
	s.write((const char*)&n, sizeof(n));
	for(uint32_t i=0; i<n; i++) {
		uint32_t d = o.shape(i);
		s.write((const char*)&d, sizeof(d));
	}
	s.write((const char*)o.data(), o.nbytes());
}
template<typename T>
void unpickle(std::istream & s, py::array_t<T> * o) {
	uint32_t n;
	s.read((char *)&n, sizeof(n));
	std::vector<uint32_t> shape(n);
	s.read((char *)&shape[0], sizeof(uint32_t)*n);
	*o = py::array_t<T>(shape);
	s.read((char*)o->mutable_data(), o->nbytes());
}
template<typename T>
void pickle(std::ostream & s, const std::shared_ptr<T> & o) {
	if (o) {
		s.write("\1", 1);
		pickle(s, *o);
	} else {
		s.write("\0", 1);
	}
}
template<typename T>
void unpickle(std::istream & s, std::shared_ptr<T> * o) {
	char c;
	s.read(&c, 1);
	if (c) {
		*o = std::make_shared<T>();
		unpickle(s, o->get());
	} else {
		o->reset();
	}
}
namespace std {
	std::string to_string(const PyVec3 & v) {
		char buf[256] = {0};
		sprintf(buf, "[%0.2f, %0.2f, %0.2f]", v[0], v[1], v[2]);
		return buf;
	}
}

#define V(X) .value(#X, Type::X)

struct PyAttachment {
	enum Type: uint8_t {
		NOTHING = Attachment::ATTACH_NOTHING,
		PARACHUTE = Attachment::ATTACH_PARACHUTE,
		ANVIL = Attachment::ATTACH_ANVIL,
		BOMB = Attachment::ATTACH_BOMB,
		SWATTER = Attachment::ATTACH_SWATTER,
		BUBBLEGUM_SHIELD = Attachment::ATTACH_BUBBLEGUM_SHIELD,
	};
	Type type = NOTHING;
	float time_left = 0.f;
	static void define(py::object m) {
		py::class_<PyAttachment, std::shared_ptr<PyAttachment>> c(m, "Attachment");
		py::enum_<Type> E(c, "Type");
		E V(NOTHING)
		  V(PARACHUTE)
		  V(ANVIL)
		  V(BOMB)
		  V(SWATTER)
		  V(BUBBLEGUM_SHIELD);
		c.def_readonly("type", &PyAttachment::type, "Attachment type")
		 .def_readonly("time_left", &PyAttachment::time_left, "Seconds until attachment detaches/explodes")
		 .def("__repr__", [E](const PyAttachment &a) { return "<Attachment type=" + std::string(py::repr(E(a.type))) + " time_left="+std::to_string(a.time_left)+">"; });
		add_pickle(c);
	}
	PyAttachment(const Attachment * a = nullptr) {
		update(a);
	}
	void update(const Attachment * a) {
		if (a) {
			type = (Type)a->getType();
			time_left = stk_config->ticks2Time(a->getTicksLeft());
		} else {
			type = NOTHING;
			time_left = 0;
		}
	}
};

struct PyPowerup {
	enum Type: uint8_t {
		NOTHING    = PowerupManager::POWERUP_NOTHING,
		BUBBLEGUM  = PowerupManager::POWERUP_BUBBLEGUM,
		CAKE       = PowerupManager::POWERUP_CAKE,
		BOWLING    = PowerupManager::POWERUP_BOWLING,
		ZIPPER     = PowerupManager::POWERUP_ZIPPER,
		PLUNGER    = PowerupManager::POWERUP_PLUNGER,
		SWITCH     = PowerupManager::POWERUP_SWITCH,
		SWATTER    = PowerupManager::POWERUP_SWATTER,
		RUBBERBALL = PowerupManager::POWERUP_RUBBERBALL,
		PARACHUTE  = PowerupManager::POWERUP_PARACHUTE,
		ANVIL      = PowerupManager::POWERUP_ANVIL
	};
	Type type;
	int num = 0;
	static void define(py::object m) {
		py::class_<PyPowerup, std::shared_ptr<PyPowerup>> c(m, "Powerup");
		py::enum_<Type> E(c, "Type");
		E V(NOTHING)
		  V(BUBBLEGUM)
		  V(CAKE)
		  V(BOWLING)
		  V(ZIPPER)
		  V(PLUNGER)
		  V(SWITCH)
		  V(SWATTER)
		  V(RUBBERBALL)
		  V(PARACHUTE)
		  V(ANVIL);
		c.def_readonly("type", &PyPowerup::type, "Powerup type")
		 .def_readonly("num" , &PyPowerup::num, "Number of powerups")
		 .def("__repr__", [E](const PyPowerup &p) { return "<Powerup type=" + std::string(py::repr(E(p.type))) + " num="+std::to_string(p.num)+">"; });
		add_pickle(c);
	}
	PyPowerup(const Powerup * p = nullptr) {
		update(p);
	}
	void update(const Powerup * p) {
		if (p) {
			type = (Type)p->getType();
			num = stk_config->ticks2Time(p->getNum());
		} else {
			type = NOTHING;
			num = 0;
		}
	}
};


void setVector(py::array_t<float>  &x, btQuaternion const &q) {
	assert(x.size() == 4);
	auto _x = x.mutable_unchecked<1>(); // x must have ndim = 3; can be non-writeable

	_x(0) = q.w();
	_x(1) = q.x();
	_x(2) = q.y();
	_x(3) = q.z();
}

void setVector(py::array_t<float>  &x, Vec3 const &y) {
	assert(x.size() == 3);
	memcpy(x.mutable_data(0), y.m_floats, 3*sizeof(float));
}

/// @brief Returns a vector of the given dimension
py::array_t<float> py_tensor(std::size_t dim) {
	return py::array_t<float>(py::array::ShapeContainer({dim}));
} 

struct PyKart {
	static void define();
	int id = 0, player_id = -1;
	std::string name;
	py::array_t<float> location = py_tensor(3);
	py::array_t<float> rotation = py_tensor(4);
	py::array_t<float> front = py_tensor(3);
	py::array_t<float> velocity = py_tensor(3);
	py::array_t<float> velocity_lc = py_tensor(3);
	py::array_t<float> angular_velocity = py_tensor(3);
	py::array_t<float> size = py_tensor(3);
	float shield_time = 0.f;
	float speed = 0.f;
	bool race_result = false;
	bool jumping = false;
	int finished_laps = 0;
	float lap_time = 0;
	float finish_time = 0;
	float overall_distance = 0;
	float distance_down_track = 0;
	int position = 0;
	float energy = 0;
	float max_steer_angle = 0;
	float wheel_base = 0;
	float skeed_factor = 0;
	int lives = 0;
	bool has_finished_race;
	int node = 0;
	bool is_on_road;
	
	PyAttachment attachment;
	
	PyPowerup powerup;
	static void define(py::object m) {
		py::class_<PyKart, std::shared_ptr<PyKart>> c(m, "Kart");
		c
#define R(x, d) .def_readonly(#x, &PyKart::x, d)
		  R(id, "Kart id compatible with instance labels")
		  R(player_id, "Player id")
		  R(name, "Player name")
		  R(location, "3D world location of the kart")
		  R(rotation, "Quaternion rotation of the kart [w, x, y, z]")
		  R(front, "Front direction of kart 1/2 kart length forward from location")
		  R(velocity, "Velocity of kart")
		  R(velocity_lc, "Velocity of kart (in the kart referential)")
		  R(angular_velocity, "Angular velocity of kart")
		  R(speed, "Speed of the kart in meters/second")
		  R(size, "Width, height and length of kart")
		  R(shield_time, "Second the shield is up for")
		  R(race_result, "Did the kart win the race?")
		  R(jumping, "Is the kart jumping?")
		  R(lap_time, "Time to completion for last lap")
		  R(finished_laps, "Number of laps completed")
		  R(overall_distance, "Overall distance traveled")
		  R(distance_down_track, "Distance traveled on current lap")
		  R(position, "Current position of this kart in the race")
		  R(energy, "Remaining collected energy")
		  R(finish_time, "Time to complete race")
		  R(has_finished_race, "True if the kart has finished the race")
		  R(skeed_factor, "Skid factor")
		  R(attachment, "Attachment of kart")
		  R(powerup, "Powerup collected")
		  R(max_steer_angle, "Maximum steering angle (depends on speed)")
		  R(wheel_base, "Wheel base (distance front to rear axis)")
		  R(lives, "Lives in three strikes battle")
		  R(node, "Closest node")
		  R(is_on_road, "Whether the kart is on track")
#undef R
		 .def("__repr__", [](const PyKart &k) { return "<Kart id=" + std::to_string(k.id)+" player_id=" + std::to_string(k.player_id)+" name='"+k.name+"' ...>"; });
		add_pickle(c);
	}
	PyKart(const AbstractKart * k = nullptr) {
		update(k);
	}
	void update(const AbstractKart * k) {
		if (k) {
			const WorldWithRank * w = dynamic_cast<WorldWithRank*>(World::getWorld());

			// TODO: add skidding information
			id = k->getWorldKartId();
			speed = k->getSpeed();
			name = k->getKartProperties()->getNonTranslatedName();
			setVector(location, k->getXYZ());

			// Sets the proper rotation so we can convert to the kart frame of reference
			setVector(rotation, k->getRotation().inverse());
			setVector(front, k->getFrontXYZ());
			setVector(velocity_lc, k->getVelocityLC());
			setVector(velocity, k->getVelocity());
			setVector(angular_velocity, k->getAngularVelocity());
			setVector(size, Vec3 {k->getKartWidth(), k->getKartHeight(), k->getKartLength()});
			shield_time = k->getShieldTime();
			race_result = k->getRaceResult();
			jumping = k->isJumping();
			position = k->getPosition();
			energy = k->getEnergy();
			attachment.update(k->getAttachment());
			powerup.update(k->getPowerup());
			max_steer_angle = k->getMaxSteerAngle();
			wheel_base = k->getKartProperties()->getWheelBase();
			finish_time = k->getFinishTime();
			has_finished_race = k->hasFinishedRace();
			skeed_factor = k->getSkidding()->getSkidFactor();

			auto sector = w->getTrackSector(id);
			is_on_road = sector->isOnRoad();
			node = sector->getCurrentGraphNode();
		}
	}
};

struct PyItem {
	unsigned int id = 0;
	py::array_t<float> location = py_tensor(3);
	float size = 1.1f/* sqrt(1.2) */;
	enum Type
	{
		BONUS_BOX = Item::ITEM_BONUS_BOX,
		BANANA = Item::ITEM_BANANA,
		NITRO_BIG = Item::ITEM_NITRO_BIG,
		NITRO_SMALL = Item::ITEM_NITRO_SMALL,
		BUBBLEGUM = Item::ITEM_BUBBLEGUM,
		EASTER_EGG = Item::ITEM_EASTER_EGG,
		NUM_ITEM
	};
	Type type = BONUS_BOX;
	static bool isValid(const Item * i) {
		if (!i) return false;
		return 0 <= i->getType() && (int)i->getType() < NUM_ITEM;
	}
	static void define(py::object m) {
		py::class_<PyItem, std::shared_ptr<PyItem>> c(m, "Item");
		py::enum_<Type> E(c, "Type");
		E V(BONUS_BOX)
		  V(BANANA)
		  V(NITRO_BIG)
		  V(NITRO_SMALL)
		  V(BUBBLEGUM)
		  V(EASTER_EGG);
#define R(x, d) .def_readonly(#x, &PyItem::x, d)
		c R(id, "Item id compatible with instance data")
		  R(location, "3D world location of the item")
		  R(size, "Size of the object")
		  R(type, "Item type")
#undef R
		 .def("__repr__", [E](const PyItem &i) { 
			auto loc = i.location.unchecked<1>();
			return "<Item id=" + std::to_string(i.id)+" location=(" + std::to_string(loc[0])+","+std::to_string(loc[1])+","+std::to_string(loc[2])
		 +") size="+std::to_string(i.size)+" type="+std::string(py::repr(E(i.type)))+">"; });
		add_pickle(c);
	}
	PyItem(const Item * i = nullptr) {
		update(i);
	}
	void update(const Item * i) {
		if (i) {
			id = i->getItemId();
			setVector(location, i->getXYZ());
			size = i->getAvoidancePoint(0) ? (i->getXYZ() - *i->getAvoidancePoint(0)).length() : 1.1;
			type = (Type)(int)i->getType();
		}
	}
};
struct PySoccerBall {
	int id = 0;
	py::array_t<float> location = py_tensor(3);
	float size = 0;
	
	static void define(py::object m) {
		py::class_<PySoccerBall, std::shared_ptr<PySoccerBall>> c(m, "SoccerBall");
#define R(x, d) .def_readonly(#x, &PySoccerBall::x, d)
		c R(id, "Object id of the soccer ball")
		  R(location, "3D world location of the item")
		  R(size, "Size of the ball")
#undef R
		.def("__repr__", [](const PySoccerBall &i) { 
			auto loc = i.location.unchecked<1>();
			return "<SoccerBall id=" + std::to_string(i.id)+" location=(" + std::to_string(loc[0])+","+std::to_string(loc[1])+","+std::to_string(loc[2])+")  size="+std::to_string(i.size)+">"; 
		});
		add_pickle(c);
	}
	PySoccerBall(const SoccerWorld * w = nullptr) {
		update(w);
	}
	void update(const SoccerWorld * w) {
		if (w) {
			// id = w->ballID();
			id = w->getBallNode();
			setVector(location, w->getBallPosition());
			size = w->getBallDiameter();
		}
	}
};
struct PySoccer {
	std::array<int, 2> score = {0, 0};
	PySoccerBall ball;
	std::array<std::array<PyVec3, 2>, 2> goal_line;
	
	static void define(py::object m) {
		py::class_<PySoccer, std::shared_ptr<PySoccer>> c(m, "Soccer");
#define R(x, d) .def_readonly(#x, &PySoccer::x, d)
		c R(score, "Score of the soccer match")
		  R(ball, "Soccer ball information")
		  R(goal_line, "Start and end of the goal line for each team")
#undef R
		 .def("__repr__", [](const PySoccer &s) { return "<Soccer score=" + std::to_string(s.score[0])+":"+std::to_string(s.score[1]) +">"; });
		add_pickle(c);
	}
	PySoccer(const SoccerWorld * w = nullptr) {
		update(w);
	}
	void update(const SoccerWorld * w) {
		if (w) {
			auto check_manager = Track::getCurrentTrack()->getCheckManager();
			score = {w->getScore((KartTeam)0), w->getScore((KartTeam)1)};
			ball.update(w);
            unsigned int n = check_manager->getCheckStructureCount();
            for (unsigned int i = 0; i < n; i++)
            {
                CheckGoal* goal = dynamic_cast<CheckGoal*>
                    (check_manager->getCheckStructure(i));
                if (goal)
                    goal_line[(int)goal->getTeam()] = {P(goal->getPoint(CheckGoal::POINT_FIRST)), P(goal->getPoint(CheckGoal::POINT_LAST))};
            }
		}
	}
};

struct PyFFA {
	std::vector<int> scores;

	static void define(py::object m) {
		py::class_<PyFFA, std::shared_ptr<PyFFA>> c(m, "FFA");
#define R(x, d) .def_readonly(#x, &PyFFA::x, d)
		c R(scores, "Score of every kart")
#undef R
		 .def("__repr__", [](const PyFFA &s) { return "<Free score_size=" + std::to_string(s.scores.size()) +">"; });
		add_pickle(c);
	}
	PyFFA(const FreeForAll * w = nullptr) {
		update(w);
	}
	void update(const FreeForAll * w) {
		if (w) {
			World::KartList karts = w->getKarts();
			for (auto k: karts) {
				unsigned int kart_id = k->getWorldKartId();
				if (kart_id <= scores.size())
    				scores.resize(kart_id+1, 0);
				scores[kart_id] = w->getKartScore(kart_id);
			}
		}
	}
};

struct PyCamera {
	Camera::Mode mode = Camera::CM_NORMAL;
	float aspect = 0, fov = 0;
	core::matrix4 view, projection;
	
	static void define(py::object m) {
		py::class_<PyCamera, std::shared_ptr<PyCamera> > c(m, "Camera");
		py::enum_<Camera::Mode>(c, "Mode")
		 .value("NORMAL", Camera::CM_NORMAL)
		 .value("CLOSEUP", Camera::CM_CLOSEUP)
		 .value("REVERSE", Camera::CM_REVERSE)
		 .value("LEADER_MODE", Camera::CM_LEADER_MODE)
		 .value("FALLING", Camera::CM_FALLING);
		c
         .def(py::init<>())
#define R(x, d) .def_readonly(#x, &PyCamera::x, d)
		  R(mode, "Camera mode")
		  R(aspect, "Aspect ratio")
		  R(fov, "Field of view")
#undef R
		  .def_property_readonly("view", [](const PyCamera & c) { return py::ro_view(c.view.pointer(), {4, 4}); }, "View matrix (float 4x4)")
		  .def_property_readonly("projection", [](const PyCamera & c) { return py::ro_view(c.projection.pointer(), {4, 4}); }, "Projection matrix (float 4x4)")
		  
		 .def("__repr__", [](const PyCamera &t) { return "<Camera mode="+std::to_string(t.mode)+">"; });
		add_pickle(c);
	}
	
	void update(int id) {
		Camera * c = Camera::getCamera(id);
		mode = c->getMode();
		scene::ICameraSceneNode * n = c->getCameraSceneNode();
		aspect = n->getAspectRatio();
		fov = n->getFOV();
		view = n->getViewMatrix();
		projection = n->getProjectionMatrix();
	}
};

struct PyPlayer {
	int id = -1;
	std::shared_ptr<PyKart> kart;
	std::shared_ptr<PyCamera> camera;
	static void define(py::object m) {
		py::class_<PyPlayer, std::shared_ptr<PyPlayer> > c(m, "Player");
		c.def(py::init<>())
#define R(x, d) .def_readonly(#x, &PyPlayer::x, d)
		  R(kart, "Kart of the player")
		  R(camera, "Camera parameters of the player")
#undef R
		 .def("__repr__", [](const PyPlayer &t) { return "<Player id="+std::to_string(t.id)+">"; });
	}
	
	void update(int player_id) {
		id = player_id;
		// if (!GUIEngine::isNoGraphics()) {
		// 	if (!camera)
		// 		camera.reset(new PyCamera());
		// 	camera->update(player_id);
		// }
	}
};

struct PyTrack {
	float length;
	py::array_t<float> path_nodes;
	py::array_t<float> path_width;
	py::array_t<float> path_distance;
	std::vector<std::vector<int>> successors;
	
	static void define(py::object m) {
		py::class_<PyTrack, std::shared_ptr<PyTrack> > c(m, "Track");
		c.def(py::init<>())
#define R(x, d) .def_readonly(#x, &PyTrack::x, d)
		  R(length, "length of the track")
		  R(path_nodes, "Center line of the drivable area as line segments of 3d coordinates (float N x 2 x 3)")
		  R(path_width, "Width of the path segment (float N)")
		  R(path_distance, "Distance down the track of each line segment (float N x 2)")
		  R(successors, "For each node, its successors (N lists)")
#undef R
		 .def("update", &PyTrack::update) 
		 .def("__repr__", [](const PyTrack &t) { return "<Track length="+std::to_string(t.length)+">"; })
		 
		 ;
		add_pickle(c);
	}
	
	void update() {
		const Track * t = Track::getCurrentTrack();
		if (t) {
			length = t->getTrackLength();
		}
		const DriveGraph * g = DriveGraph::get();
		if (g) {
			path_nodes = py::array_t<float>(py::array::ShapeContainer({g->getNumNodes(), 2, 3}));
			path_width = py::array_t<float>(py::array::ShapeContainer({g->getNumNodes(), 1}));
			path_distance = py::array_t<float>(py::array::ShapeContainer({g->getNumNodes(), 2}));
			for(int i=0; i<g->getNumNodes(); i++) {
				DriveNode * node = g->getNode(i);
				memcpy(path_nodes.mutable_data(i,0), node->getLowerCenter().m_floats, 3*sizeof(float));
				memcpy(path_nodes.mutable_data(i,1), node->getUpperCenter().m_floats, 3*sizeof(float));
				*path_width.mutable_data(i) = node->getPathWidth();
				*path_distance.mutable_data(i,0) = node->getDistanceFromStart();
				*path_distance.mutable_data(i,1) = node->getDistanceFromStart() + node->getNodeLength();

				std::vector<int> node_successors;
				for(int i = 0; i < node->getNumberOfSuccessors(); ++i) {
					node_successors.push_back(node->getSuccessor(i));
				}
				successors.push_back(std::move(node_successors));
			}
		}
	}
};

struct PyWorldState {
	std::vector<std::shared_ptr<PyPlayer> > players;
	std::vector<std::shared_ptr<PyKart> > karts;
	std::vector<std::shared_ptr<PyItem> > items;
	float time = 0;
	int aux_ticks = 0;
	std::shared_ptr<PySoccer> soccer;
	std::shared_ptr<PyFFA> ffa;
	WorldStatus::Phase phase = WorldStatus::Phase::SETUP_PHASE;

	static void define(py::object m) {
		py::class_<PyWorldState, std::shared_ptr<PyWorldState>> c(m, "WorldState");
		py::enum_<WorldStatus::Phase>(c, "Phase")
			.value("TRACK_INTRO_PHASE", WorldStatus::Phase::TRACK_INTRO_PHASE)
        	.value("SETUP_PHASE", WorldStatus::Phase::SETUP_PHASE)
        	.value("WAIT_FOR_SERVER_PHASE", WorldStatus::Phase::WAIT_FOR_SERVER_PHASE)
        	.value("SERVER_READY_PHASE", WorldStatus::Phase::SERVER_READY_PHASE)
        	.value("READY_PHASE", WorldStatus::Phase::READY_PHASE)
        	.value("SET_PHASE", WorldStatus::Phase::SET_PHASE)
        	.value("GO_PHASE", WorldStatus::Phase::GO_PHASE)
        	.value("MUSIC_PHASE", WorldStatus::Phase::MUSIC_PHASE)
        	.value("RACE_PHASE", WorldStatus::Phase::RACE_PHASE)
        	.value("DELAY_FINISH_PHASE", WorldStatus::Phase::DELAY_FINISH_PHASE)
        	.value("RESULT_DISPLAY_PHASE", WorldStatus::Phase::RESULT_DISPLAY_PHASE)
        	.value("FINISH_PHASE", WorldStatus::Phase::FINISH_PHASE)
        	.value("IN_GAME_MENU_PHASE", WorldStatus::Phase::IN_GAME_MENU_PHASE)
        	.value("UNDEFINED_PHASE", WorldStatus::Phase::UNDEFINED_PHASE)
		;

		c.def(py::init<>())
#define R(x, d) .def_readonly(#x, &PyWorldState::x, d)
		  R(phase, "World status phase")
		  R(players, "State of active players")
		  R(karts, "State of karts")
		  R(items, "State of items")
		  R(time, "Game time")
		  R(aux_ticks, "Ticks since ready")
		  R(soccer, "Soccer match info")
		  R(ffa, "Free for all match info")
#undef R
		 .def("update", &PyWorldState::update, "Update this object with the current world state")
		 .def("__repr__", [](const PyWorldState &k) { return "<WorldState #karts="+std::to_string(k.karts.size())+">"; })
		 .def_static("set_kart_location", &PyWorldState::set_kart_location, py::arg("kart_id"), py::arg("position"), py::arg("rotation")=PyQuaternion{0,0,0,1}, py::arg("speed")=0, "Move a kart to a specific location.");
		// TODO: Add pickling and make sure players are updated
		add_pickle(c);
	}
	void assignPlayersKart() {
		for(auto k: karts) 
			if (k->player_id >= 0)
				players[k->player_id]->kart = k;
	}
	void update() {
		World * w = World::getWorld();
		LinearWorld * lw = dynamic_cast<LinearWorld*>(w);
		SoccerWorld * sw = dynamic_cast<SoccerWorld*>(w);
		FreeForAll  * fw = dynamic_cast<FreeForAll*>(w);
		ThreeStrikesBattle * tw = dynamic_cast<ThreeStrikesBattle*>(w);
		if (w) {
			World::KartList k = w->getKarts();
			karts.resize(k.size());
			players.resize(k.size());
			int pid = 0;
			for(int i=0; i<k.size(); i++) {
				if (!karts[i])
					karts[i].reset(new PyKart());
				karts[i]->update(k[i].get());
				if (k[i]->getController()->isLocalPlayerController()) {
					karts[i]->player_id = pid;
					if (!players[pid])
						players[pid].reset(new PyPlayer());
					players[pid]->update(pid);
					pid++;
				} else {
					karts[i]->player_id = -1;
				}
				if (lw) {
					karts[i]->finished_laps = lw->getFinishedLapsOfKart(i);
					karts[i]->overall_distance = lw->getOverallDistance(i);
					karts[i]->distance_down_track = lw->getDistanceDownTrackForKart(i, true);
					karts[i]->lap_time = stk_config->ticks2Time(lw->getTicksAtLapForKart(i));
				}
				if (tw) {
					karts[i]->lives = tw->getKartLife(i);
				}
			}
			players.resize(pid);
			assignPlayersKart();
			aux_ticks = w->getAuxiliaryTicks();
			time = w->getTime();
			phase = w->getPhase();

			if (sw) {
				if (!soccer)
					soccer = std::make_shared<PySoccer>();
				soccer->update(sw);
			}

			if (fw) {
				if (!ffa)
					ffa = std::make_shared<PyFFA>();
				ffa->update(fw);
			}
		}

		ItemManager * im = Track::getCurrentTrack()->getItemManager(); //ItemManager::get();
		if (im) {
			items.clear();
			for(int i=0; i<im->getNumberOfItems(); i++) {
				const Item * I = dynamic_cast<const Item*>(im->getItem(i));
				if (PyItem::isValid(I))
					items.push_back(std::make_shared<PyItem>(I));
			}
		}
	}
	static void set_kart_location(std::size_t id, const PyVec3 & position, const PyQuaternion & rotation, float speed) {
		World * w = World::getWorld();
		World::KartList k = w->getKarts();
		if (0 <= id && id < k.size()) {
			auto kart = k[id];
			btTransform transform = kart->getTrans();
			transform.setOrigin(P(position));
			transform.setRotation(btQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]));
			kart->getBody()->proceedToTransform(transform);
			kart->setTrans(transform);
			// Reset all btKart members (bounce back ticks / rotation ticks..)
			kart->getVehicle()->reset();
			kart->setSpeed(speed);
		}
	}
};

// AUTO Generated //
void pickle(std::ostream & s, const PyAttachment & o) {
    pickle(s, o.type);
    pickle(s, o.time_left);
}
void unpickle(std::istream & s, PyAttachment * o) {
    unpickle(s, &o->type);
    unpickle(s, &o->time_left);
}
void pickle(std::ostream & s, const PyPowerup & o) {
    pickle(s, o.type);
    pickle(s, o.num);
}
void unpickle(std::istream & s, PyPowerup * o) {
    unpickle(s, &o->type);
    unpickle(s, &o->num);
}
void pickle(std::ostream & s, const PyKart & o) {
    pickle(s, o.id);
    pickle(s, o.player_id);
    pickle(s, o.name);
    ::pickle(s, o.location);
    ::pickle(s, o.rotation);
    ::pickle(s, o.front);
    ::pickle(s, o.velocity);
    ::pickle(s, o.velocity_lc);
    ::pickle(s, o.size);
    pickle(s, o.shield_time);
    pickle(s, o.race_result);
    pickle(s, o.jumping);
    pickle(s, o.position);
    pickle(s, o.energy);
    pickle(s, o.finished_laps);
    pickle(s, o.lap_time);
    pickle(s, o.finish_time);
    pickle(s, o.overall_distance);
    pickle(s, o.distance_down_track);
    pickle(s, o.max_steer_angle);
    pickle(s, o.wheel_base);
    pickle(s, o.attachment);
    pickle(s, o.powerup);
    pickle(s, o.lives);
    pickle(s, o.skeed_factor);
    pickle(s, o.has_finished_race);
	pickle(s, o.node);
	pickle(s, o.is_on_road);
}
void unpickle(std::istream & s, PyKart * o) {
    unpickle(s, &o->id);
    unpickle(s, &o->player_id);
    unpickle(s, &o->name);
    unpickle(s, &o->location);
    unpickle(s, &o->rotation);
    unpickle(s, &o->front);
    unpickle(s, &o->velocity);
    unpickle(s, &o->velocity_lc);
    unpickle(s, &o->size);
    unpickle(s, &o->shield_time);
    unpickle(s, &o->race_result);
    unpickle(s, &o->jumping);
    unpickle(s, &o->position);
    unpickle(s, &o->energy);
    unpickle(s, &o->finished_laps);
    unpickle(s, &o->lap_time);
    unpickle(s, &o->finish_time);
    unpickle(s, &o->overall_distance);
    unpickle(s, &o->distance_down_track);
    unpickle(s, &o->max_steer_angle);
    unpickle(s, &o->wheel_base);
    unpickle(s, &o->attachment);
    unpickle(s, &o->powerup);
    unpickle(s, &o->lives);
    unpickle(s, &o->skeed_factor);
    unpickle(s, &o->has_finished_race);
    unpickle(s, &o->node);
    unpickle(s, &o->is_on_road);
}
void pickle(std::ostream & s, const PyItem & o) {
    pickle(s, o.id);
    ::pickle(s, o.location);
    pickle(s, o.size);
    pickle(s, o.type);
}
void unpickle(std::istream & s, PyItem * o) {
    unpickle(s, &o->id);
    unpickle(s, &o->location);
    unpickle(s, &o->size);
    unpickle(s, &o->type);
}
void pickle(std::ostream & s, const PySoccerBall & o) {
    pickle(s, o.id);
    ::pickle(s, o.location);
    pickle(s, o.size);
}
void unpickle(std::istream & s, PySoccerBall * o) {
    unpickle(s, &o->id);
    unpickle(s, &o->location);
    unpickle(s, &o->size);
}
void pickle(std::ostream & s, const PyCamera & o) {
    pickle(s, o.mode);
    pickle(s, o.aspect);
    pickle(s, o.fov);
    pickle(s, o.view);
    pickle(s, o.projection);
}
void unpickle(std::istream & s, PyCamera * o) {
    unpickle(s, &o->mode);
    unpickle(s, &o->aspect);
    unpickle(s, &o->fov);
    unpickle(s, &o->view);
    unpickle(s, &o->projection);
}
void pickle(std::ostream & s, const PyTrack & o) {
    pickle(s, o.length);
    ::pickle(s, o.path_nodes);
    ::pickle(s, o.path_width);
    ::pickle(s, o.path_distance);
    ::pickle(s, o.successors);
}
void unpickle(std::istream & s, PyTrack * o) {
    unpickle(s, &o->length);
    unpickle(s, &o->path_nodes);
    unpickle(s, &o->path_width);
    unpickle(s, &o->path_distance);
    unpickle(s, &o->successors);
}
void pickle(std::ostream & s, const PyPlayer & o) {
    pickle(s, o.id);
    pickle(s, o.camera);
}
void unpickle(std::istream & s, PyPlayer * o) {
    unpickle(s, &o->id);
    unpickle(s, &o->camera);
}
void pickle(std::ostream & s, const PySoccer& o) {
    pickle(s, o.score);
    pickle(s, o.ball);
    pickle(s, o.goal_line);
}
void unpickle(std::istream & s, PySoccer * o) {
    unpickle(s, &o->score);
    unpickle(s, &o->ball);
    unpickle(s, &o->goal_line);
}
void pickle(std::ostream & s, const PyFFA& o) {
    pickle(s, o.scores);
}
void unpickle(std::istream & s, PyFFA * o) {
    unpickle(s, &o->scores);
}
void pickle(std::ostream & s, const PyWorldState & o) {
    pickle(s, o.time);
    pickle(s, o.players);
    pickle(s, o.karts);
    pickle(s, o.items);
	pickle(s, o.soccer);
}
void unpickle(std::istream & s, PyWorldState * o) {
    unpickle(s, &o->time);
    unpickle(s, &o->players);
    unpickle(s, &o->karts);
    unpickle(s, &o->items);
	unpickle(s, &o->soccer);
	o->assignPlayersKart();
}
// End AUTO Generated //



void defineState(py::object m) {
	PyAttachment::define(m);
	PyPowerup::define(m);
	PyKart::define(m);
	PyItem::define(m);
	PyCamera::define(m);
	PyPlayer::define(m);
	PySoccerBall::define(m);
	PySoccer::define(m);
	PyFFA::define(m);
	PyWorldState::define(m);
	PyTrack::define(m);
};

