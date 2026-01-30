

//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2011-2015 Joerg Henrichs, Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifdef WIN32
#  ifdef __CYGWIN__
#    include <unistd.h>
#  endif
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  ifdef _MSC_VER
#    include <direct.h>
#  endif
#else
#  include <signal.h>
#  include <unistd.h>
#endif
#include <stdlib.h>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <limits>

#include <IEventReceiver.h>

#include "pystk.hpp"
#include "main_loop.hpp"
#include "achievements/achievements_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "challenges/story_mode_timer.hpp"
#include "config/stk_config.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "font/font_manager.hpp"
#include "graphics/camera/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/frame_buffer.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/referee.hpp"
#include "graphics/render_target.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shader_based_renderer.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "IrrlichtDevice.h"
#include "io/file_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/combined_characteristic.hpp"
#include "karts/controller/ai_base_lap_controller.hpp"
#include "karts/controller/pystk_controller.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/world.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/history.hpp"
#include "race/highscore_manager.hpp"
#include "race/race_manager.hpp"
#include "scriptengine/property_animator.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/command_line.hpp"
#include "utils/constants.hpp"
#include "utils/crash_reporting.hpp"
#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/string_utils.hpp"
#include "objecttype.hpp"
#include "util.hpp"
#include "buffer.hpp"
#include "tips/tips_manager.hpp"
#include "utils/translation.hpp"
#include "fake_input_device.hpp"

#ifndef SERVER_ONLY
#include "../lib/irrlicht/source/Irrlicht/CIrrDeviceSDL.h"
#endif

#ifdef RENDERDOC
#include "renderdoc_app.h"
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif
#endif

const PySTKGraphicsConfig & PySTKGraphicsConfig::hd() {
    static PySTKGraphicsConfig config = {600,400, 0,
        true, true, true, true, true, 
        2,     // particle_effects
        true,  // animated_characters
        true,  // motionblur
        true,  // mlaa
        true,  // texture_compression
        true,  // ssao
        false, // degraded_IBL
        1 | 2, // high_definition_textures
    };
    return config;
}
const PySTKGraphicsConfig & PySTKGraphicsConfig::sd() {
    static PySTKGraphicsConfig config = {600,400, 0,
        false, false, false, false, false,
        2,     // particle_effects
        true,  // animated_characters
        false,  // motionblur
        true,  // mlaa
        true,  // texture_compression
        true,  // ssao
        false, // degraded_IBL
        1 | 2, // high_definition_textures
    };
    return config;
}
const PySTKGraphicsConfig & PySTKGraphicsConfig::ld() {
    static PySTKGraphicsConfig config = {600,400, 0,
        false, false, false, false, false,
        0,     // particle_effects
        false, // animated_characters
        false, // motionblur
        false, // mlaa
        false, // texture_compression
        false, // ssao
        false, // degraded_IBL
        0,     // high_definition_textures
    };
    return config;
}
const PySTKGraphicsConfig & PySTKGraphicsConfig::none() {
    static PySTKGraphicsConfig config = {1,1, 0,
                                         false, false, false, false, false,
                                         0,     // particle_effects
                                         false, // animated_characters
                                         false, // motionblur
                                         false, // mlaa
                                         false, // texture_compression
                                         false, // ssao
                                         false, // degraded_IBL
                                         0,     // high_definition_textures
                                         false, // render
                                         false, // display
    };
    return config;
}

#ifdef RENDERDOC
static RENDERDOC_API_1_1_2 *rdoc_api = NULL;
#endif

#ifndef SERVER_ONLY
class PySTKRenderTarget {
    friend class PySTKRace;

private:
    const int BUF_SIZE = 2;
    std::unique_ptr<RenderTarget> rt_;
    std::vector<std::shared_ptr<NumpyPBO> > color_buf_, depth_buf_, instance_buf_;
    int buf_num_=0;

protected:
    void render(irr::scene::ICameraSceneNode* camera, float dt);
    void fetch(std::shared_ptr<PySTKRenderData> data);
    
public:
    PySTKRenderTarget(std::unique_ptr<RenderTarget>&& rt);
    
};

PySTKRenderTarget::PySTKRenderTarget(std::unique_ptr<RenderTarget>&& rt):rt_(std::move(rt)) {
    int W = rt_->getTextureSize().Width, H = rt_->getTextureSize().Height;
    buf_num_ = 0;
    for(int i=0; i<BUF_SIZE; i++) {
        color_buf_.push_back(std::make_shared<NumpyPBO>(W, H, GL_RGB, GL_UNSIGNED_BYTE));
        depth_buf_.push_back(std::make_shared<NumpyPBO>(W, H, GL_DEPTH_COMPONENT, GL_FLOAT));
        instance_buf_.push_back(std::make_shared<NumpyPBO>(W, H, GL_RED_INTEGER, GL_UNSIGNED_INT));
    }
}
void PySTKRenderTarget::render(irr::scene::ICameraSceneNode* camera, float dt) {
    rt_->renderToTexture(camera, dt);
}

void PySTKRenderTarget::fetch(std::shared_ptr<PySTKRenderData> data) {
    // Fetch the image
    if (const auto rt_gl3 = dynamic_cast<GL3RenderTarget*>(rt_.get())) {
        RTT * rtts = rt_gl3->getRTTs();
        if (rtts && data) {
            // unsigned int W = rtts->getWidth(), H = rtts->getHeight();
            // Read the color and depth image
            data->color_buf_ = color_buf_[buf_num_];
            data->depth_buf_ = depth_buf_[buf_num_];
            data->instance_buf_ = instance_buf_[buf_num_];

            data->depth_buf_->read(rtts->getDepthStencilTexture());
            // Read from the post-processed frame buffer (tone mapping, bloom, etc.)
            // rather than the raw RTT_COLOR which is linear HDR pre-post-processing
            FrameBuffer* fb = rt_gl3->getFrameBuffer();
            if (fb && !fb->getRTT().empty())
                data->color_buf_->read(fb->getRTT()[0]);
            else
                data->color_buf_->read(rtts->getRenderTarget(RTT_COLOR));
            if (rtts->hasRenderTarget(RTT_LENS_128))
                data->instance_buf_->read(rtts->getRenderTarget(RTT_LENS_128));
            buf_num_ = (buf_num_+1) % BUF_SIZE;
        }
    }
    
}
#endif  // SERVER_ONLY

void PySTKAction::set(KartControl * control) const {
    control->setAccel(acceleration);
    control->setBrake(brake);
    control->setFire(fire);
    control->setNitro(nitro);
    control->setRescue(rescue);
    control->setSteer(steering_angle);
    control->setSkidControl(drift ? (steering_angle > 0 ? KartControl::SC_RIGHT : KartControl::SC_LEFT) : KartControl::SC_NONE);
}
void PySTKAction::get(const KartControl * control) {
    acceleration = control->getAccel();
    brake = control->getBrake();
    fire = control->getFire();
    nitro = control->getNitro();
    rescue = control->getRescue();
    steering_angle = control->getSteer();
    drift = control->getSkidControl() != KartControl::SC_NONE;
}

PySTKRace * PySTKRace::running_kart = 0;


bool PySTKRace::isRunning() { return running_kart; }
PySTKRace::PySTKRace(const PySTKRaceConfig & config) {
    if (running_kart)
        throw std::invalid_argument("Cannot run more than one supertux instance per process!");

    // Keep a copy of the environment so it is destroyed after us
    environment = PyGlobalEnvironment::instance();
    
    running_kart = this;
    
    resetObjectId();
    
    setupConfig(config);

}
std::vector<std::string> PySTKRace::listTracks() {
    if (track_manager)
        return track_manager->getAllTrackIdentifiers();
    return std::vector<std::string>();
}
std::vector<std::string> PySTKRace::listTracks(PySTKRaceConfig::RaceMode mode) {
    std::vector<std::string> tracks;
    if (track_manager) 
    {
        for(size_t i = 0; i < track_manager->getNumberOfTracks(); ++i) 
        {
            auto track = track_manager->getTrack(i);
            bool include = false;
            switch (mode) 
            {
                case PySTKRaceConfig::RaceMode::NORMAL_RACE:
                    include = track->isRaceTrack();
                    break;
                default:
                    throw std::invalid_argument("Unhandled mode");
            }
            if (include)
            {
                tracks.push_back(track->getIdent());
            }
        }
    }
    return tracks;
}
std::vector<std::string> PySTKRace::listKarts() {
    if (kart_properties_manager)
        return kart_properties_manager->getAllAvailableKarts();
    return std::vector<std::string>();
}
PySTKRace::~PySTKRace() {
    Log::debug("pystk", "Destroying PySTK Race");
#ifndef SERVER_ONLY
    render_targets_.clear();
#endif
    if (World::getWorld()) {
        RaceManager::get()->exitRace();
    }
    running_kart = nullptr;
}

/**
 * @brief Wrapper around a AI Controller
 * TODO: evaluate if needed
 */
class LocalPlayerAIController: public Controller {
public:
    Controller * ai_controller_;
public:
    LocalPlayerAIController(Controller * ai_controller):Controller(ai_controller->getKart()), ai_controller_(ai_controller) {}
    ~LocalPlayerAIController() {
        if (ai_controller_) delete ai_controller_;
    }
    virtual void  reset              ()
    { ai_controller_->reset(); }
    virtual void  update             (int ticks)
    { ai_controller_->update(ticks); }
    virtual void  handleZipper       (bool play_sound)
    { ai_controller_->handleZipper(play_sound); }
    virtual void  collectedItem      (const ItemState &item,
                                      float previous_energy=0)
    { ai_controller_->collectedItem(item, previous_energy); }
    virtual void  crashed            (const AbstractKart *k)
    { ai_controller_->crashed(k); }
    virtual void  crashed            (const Material *m)
    { ai_controller_->crashed(m); }
    virtual void  setPosition        (int p)
    { ai_controller_->setPosition(p); }
    /** This function checks if this is a local player. A local player will get 
     *  special graphical effects enabled, has a camera, and sound effects will
     *  be played with normal volume. */
    virtual bool  isLocalPlayerController () const { return true; }
    /** This function checks if this player is not an AI, i.e. it is either a
     *  a local or a remote/networked player. This is tested e.g. by the AI for
     *  rubber-banding. */
    virtual bool  isPlayerController () const { return true; }
    virtual bool  disableSlipstreamBonus() const
    { return ai_controller_->disableSlipstreamBonus(); }

    // ------------------------------------------------------------------------
    /** Default: ignore actions. Only PlayerController get them. */
    virtual bool action(PlayerAction action, int value, bool dry_run=false)
    { return ai_controller_->action(action, value, dry_run); }
    // ------------------------------------------------------------------------
    /** Callback whenever a new lap is triggered. Used by the AI
     *  to trigger a recomputation of the way to use.            */
    virtual void  newLap(int lap)
    { return ai_controller_->newLap(lap); }
    // ------------------------------------------------------------------------
    virtual void  skidBonusTriggered()
    { return ai_controller_->skidBonusTriggered(); }
    // ------------------------------------------------------------------------
    /** Called whan this controller's kart finishes the last lap. */
    virtual void  finishedRace(float time)
    { return ai_controller_->finishedRace(time); }

    virtual bool  saveState(BareNetworkString *buffer) const {
        return ai_controller_->saveState(buffer);
    }
    virtual void  rewindTo(BareNetworkString *buffer) {
        ai_controller_->rewindTo(buffer);
    }

};
void PySTKRace::restart() {
    if (World::getWorld())
    {
        World::getWorld()->reset(true /* restart */);
        ItemManager::updateRandomSeed(config_.seed);
        powerup_manager->setRandomSeed(config_.seed);
    }
}

bool PySTKRace::activePlayerCamera(size_t player_ix) {
    auto const & player = config_.players[player_ix];
    return (player.controller == PySTKPlayerConfig::PLAYER_CONTROL && player.cameraMode == PySTKPlayerConfig::AUTO) || (player.cameraMode == PySTKPlayerConfig::ON);
}

void PySTKRace::start() {
    auto race_manager = RaceManager::get();
    
    // This will setup karts for all players
    race_manager->setupPlayerKartInfo();

    // Karts are initialized here
    race_manager->startNew(false);

    // Setup cameras for some players
    if (!GUIEngine::isNoGraphics()) 
    {
        // Setup a camera on the first player if nothing else...
        if (Camera::getNumCameras() == 0) 
        {
            Log::fatal("pystk", "a camera should be setup");
        }

        std::size_t camera_ix = 0;

        if (config_.num_cameras > 0) 
        {
            Log::info("pystk", "Setting up %d cameras", config_.num_cameras);
            for(std::size_t ix = 0; ix < config_.num_cameras; ++ix)
            {
                auto kart = World::getWorld()->getKart(ix);
                (ix == 0 ? Camera::getCamera(ix) : Camera::createCamera(kart, ix))->setKart(kart);
            }
        }
        else 
        {
            Log::info("pystk", "Setting up player cameras", config_.players.size());
            for(std::size_t ix = 0; ix < config_.players.size(); ++ix)
            {
                if (activePlayerCamera(ix)) {

                    auto kart = World::getWorld()->getKart(ix);
                    Log::info("pystk", "Setting up camera %d to follow kart %d", camera_ix+1, ix+1);

                    if (camera_ix == 0) {
                        Camera::getCamera(camera_ix)->setKart(kart);
                    } else {
                        Camera::createCamera(kart, camera_ix);
                    }
                    ++camera_ix;
                }
            }
        }
    }

    StateManager::get();
    PlayerManager::get();


#if !defined(SERVER_ONLY)
    if (!GUIEngine::isReallyNoGraphics()) {
        auto* renderer = dynamic_cast<ShaderBasedRenderer*>(irr_driver->getRenderer());
        RTT* saved_rtts = renderer ? renderer->getRTTs() : nullptr;

        for(unsigned long int i=0; i<Camera::getNumCameras(); i++) {
            auto render_target = irr_driver->createRenderTarget(
                {(unsigned int)UserConfigParams::m_width, (unsigned int)UserConfigParams::m_height},
                "Player " + std::to_string(i)
            );
            Camera::getCamera(i)->activate(false);
            render_target->renderToTexture(Camera::getCamera(i)->getCameraSceneNode(), 0.);
            render_targets_.push_back(std::make_unique<PySTKRenderTarget>(std::move(render_target)));
        }

        if (renderer) renderer->setRTT(saved_rtts);
    }
#endif  // SERVER_ONLY
    time_leftover_ = 0.f;
    
    // Setup controllers
    for(int i=0; i<config_.players.size(); i++) {
        AbstractKart * kart = World::getWorld()->getKart(i);
        if (config_.players[i].controller == PySTKPlayerConfig::AI_CONTROL)
        {
            kart->setController(
                (Controller*)new LocalPlayerAIController(
                    World::getWorld()->loadAIController(kart)
                )
            );                

        }

        // Setup player name
        Log::info("pystk", "Setting player name %d: %s", i, config_.players[i].name.c_str());
        core::stringw player_name(config_.players[i].name.c_str());
        kart->setOnScreenText(player_name);
    }

    // Set on-screen names for all remaining (non-player) AI karts
    for(int i=config_.players.size(); i<World::getWorld()->getNumKarts(); i++) {
        AbstractKart * kart = World::getWorld()->getKart(i);
        core::stringw kart_name(kart->getName());
        kart->setOnScreenText(kart_name);
    }

    ItemManager::updateRandomSeed(config_.seed);
    powerup_manager->setRandomSeed(config_.seed);
}
void PySTKRace::stop() {
#ifndef SERVER_ONLY
    render_targets_.clear();
#endif  // SERVER_ONLY
    if (World::getWorld())
    {
        RaceManager::get()->exitRace();
        // World::getWorld()->update();
    }
}

PySTKAction PySTKRace::getKartAction(std::size_t kart_ix) {
    PySTKAction action;

    KartControl const & control = World::getWorld()->getPlayerKart(kart_ix)->getControls();
    action.get(&control);

    return action;
}


void PySTKRace::render(float dt) {
    World *world = World::getWorld();
#ifndef SERVER_ONLY
    if (world && !GUIEngine::isReallyNoGraphics())
    {
        auto* renderer = dynamic_cast<ShaderBasedRenderer*>(irr_driver->getRenderer());
        RTT* saved_rtts = renderer ? renderer->getRTTs() : nullptr;

        // Render all views
        for(unsigned int i = 0; i < Camera::getNumCameras() && i < render_targets_.size(); i++) {
            Camera::getCamera(i)->activate(false);
            render_targets_[i]->render(Camera::getCamera(i)->getCameraSceneNode(), dt);
        }

        if (renderer) renderer->setRTT(saved_rtts);

        while (render_data_.size() < render_targets_.size())
            render_data_.push_back( std::make_shared<PySTKRenderData>() );

        // Fetch all views
        for(unsigned int i = 0; i < render_targets_.size(); i++) {
            render_targets_[i]->fetch(render_data_[i]);
        }
    }
#endif  // SERVER_ONLY
}

bool PySTKRace::step(const std::vector<PySTKAction> & a) {
    if (a.size() != m_controlled.size())
        throw std::invalid_argument("Expected " + std::to_string(m_controlled.size()) + " actions, got " + std::to_string(a.size()));

    for(int i=0; i<a.size(); i++) {
        auto ix = m_controlled[i];
        auto *kart = World::getWorld()->getPlayerKart(ix);
        KartControl & control = kart->getControls();
        a[i].set(&control);

        // Uses acceleration for the startup boost
        if (a[i].acceleration > 0) 
        {
            kart->getController()->action(PA_ACCEL, a[i].acceleration, false);
        }
    }
    return step();
}
bool PySTKRace::step(const PySTKAction & a) {
    return step(std::vector<PySTKAction> { a });
}

bool PySTKRace::step() {
    const float dt = config_.step_size;
    if (!World::getWorld()) return false;
    
#ifdef RENDERDOC
    if(rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
#endif

    // Update first
    time_leftover_ += dt;
    int ticks = stk_config->time2Ticks(time_leftover_);
    time_leftover_ -= stk_config->ticks2Time(ticks);
    for(int i=0; i<ticks; i++) {
        World::getWorld()->updateWorld(1);
        World::getWorld()->updateTime(1);
    }
    
    // Update karts
    if (config_.num_cameras > 0) {
        World::KartList karts = World::getWorld()->getKarts();

        std::sort(karts.begin(), karts.end(), [](auto const & a, auto const & b) {
            return a->getPosition() < b->getPosition();
        });
        
        for(std::size_t ix = 0; ix < config_.num_cameras; ++ix)
        {
            core::stringc name(karts[ix]->getName());
            Camera::getCamera(ix)->setKart(karts[ix].get());
        }
    }

    PropertyAnimator::get()->update(dt);
    
    // Then render
    if (PyGlobalEnvironment::graphics_config().render) {
        World::getWorld()->updateGraphics(dt);

        if (PyGlobalEnvironment::graphics_config().display) {
            irr_driver->update(dt);
        }

        render(dt);
    }

    if (PyGlobalEnvironment::graphics_config().render && !irr_driver->getDevice()->run())
        return false;
#ifdef RENDERDOC
    if(rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
#endif
    auto race_manager = RaceManager::get();
    Log::debug("pystk", "Step: %s / %d < %d ?", race_manager ? "race": "no race", 
        race_manager ? race_manager->getFinishedPlayers() : 0,
        race_manager ? race_manager->getNumPlayers() : 0);
    return race_manager && race_manager->getFinishedPlayers() < race_manager->getNumPlayers();
}

static RaceManager::MinorRaceModeType translate_mode(PySTKRaceConfig::RaceMode mode) {
    switch (mode) {
        case PySTKRaceConfig::NORMAL_RACE: return RaceManager::MINOR_MODE_NORMAL_RACE;
        case PySTKRaceConfig::TIME_TRIAL: return RaceManager::MINOR_MODE_TIME_TRIAL;
        case PySTKRaceConfig::FOLLOW_LEADER: return RaceManager::MINOR_MODE_FOLLOW_LEADER;
        case PySTKRaceConfig::THREE_STRIKES: return RaceManager::MINOR_MODE_3_STRIKES;
        case PySTKRaceConfig::FREE_FOR_ALL: return RaceManager::MINOR_MODE_FREE_FOR_ALL;
        case PySTKRaceConfig::CAPTURE_THE_FLAG: return RaceManager::MINOR_MODE_CAPTURE_THE_FLAG;
        case PySTKRaceConfig::SOCCER: return RaceManager::MINOR_MODE_SOCCER;
    }
    return RaceManager::MINOR_MODE_NORMAL_RACE;
}

void PySTKRace::setupConfig(const PySTKRaceConfig & config) {
    config_ = config;

    InputDevice *device = FakeInputDevice::instance();

    auto player_manager = PlayerManager::get();
    auto state_manager = StateManager::get();

    // Create as many players as needed
    while (state_manager->activePlayerCount() < config.players.size()) {
        auto profile = player_manager->addNewPlayer("pystk");
        state_manager->createActivePlayer(
            profile, device
        );
        profile->initRemainingData();
    }


    auto race_manager = RaceManager::get();
    race_manager->setDifficulty(RaceManager::Difficulty(config.difficulty));
    race_manager->setMinorMode(translate_mode(config.mode));

    // All karts are players
    m_controlled.clear();
    size_t playerCameras = 0;
    for(size_t ix = 0; ix < config.players.size(); ++ix) {
        if (activePlayerCamera(ix)) {
            ++playerCameras;
        }

        if (config.players[ix].controller == PySTKPlayerConfig::PLAYER_CONTROL) 
        {
            m_controlled.push_back(ix);
        }
    }

    auto num_cameras = config.num_cameras > 0 ? config.num_cameras : playerCameras;

    // Sets the total number of karts
    race_manager->setNumKarts(config.num_kart);

    // The number of "local" players determine the number of cameras...
    // so we use it (to avoid having cameras for AIs)
    race_manager->setNumPlayers(config.players.size(), num_cameras);

    for(int i=0; i<config.players.size(); i++) {
        std::string kart = config.players[i].kart.size() ? config.players[i].kart : (std::string)UserConfigParams::m_default_kart;
        const KartProperties *prop = kart_properties_manager->getKart(kart);
        if (!prop)
            kart = UserConfigParams::m_default_kart;
        
        race_manager->setPlayerKart(i, kart);
        race_manager->setKartTeam(i, (KartTeam)config.players[i].team);

        auto kart_info = race_manager->getKartInfo(i);
        kart_info.setDefaultKartColor(0.5);
        // kart_info.setPlayerName(std::wstring_convert<char>(config.players[i].name));
    }

    race_manager->setReverseTrack(config.reverse);
    if (config.track.length())
        race_manager->setTrack(config.track);
    else
        race_manager->setTrack("lighthouse");
    
    race_manager->setNumLaps(config.laps);
    race_manager->setMaxGoal(1<<30);

}


// =====
// ===== PyGlobalEnvironment
// =====

std::shared_ptr<PyGlobalEnvironment> PyGlobalEnvironment::_instance = nullptr;

PyGlobalEnvironment::PyGlobalEnvironment(const PySTKGraphicsConfig & config, const std::string & data_dir) : graphics_config_(config) {
    Log::info("pystk", "Using data directory %s", data_dir.c_str());
    initUserConfig(data_dir);
    stk_config->load(file_manager->getAsset("stk_config.xml"));
    initGraphicsConfig(config);
    story_mode_timer = new StoryModeTimer();
    initRest();

    load();
}

PyGlobalEnvironment::~PyGlobalEnvironment() {
    Log::debug("pystk", "Cleanup pystk2 environment");
    clean();
}

bool PyGlobalEnvironment::is_initialized() {
    return _instance != nullptr;
}

void PyGlobalEnvironment::init(const PySTKGraphicsConfig & config, const std::string & data_dir) {
    if (PySTKRace::running_kart)
        throw std::invalid_argument("Cannot init while supertuxkart is running!");

    _instance = std::shared_ptr<PyGlobalEnvironment>(new PyGlobalEnvironment(config, data_dir));
#ifdef RENDERDOC

#ifdef _WIN32
    if(HMODULE mod = GetModuleHandleA("renderdoc.dll"))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
        assert(ret == 1);
    }
#elif defined(__linux__)
    if(void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
        assert(ret == 1);
    }
#endif

#endif
}

PySTKGraphicsConfig const & PyGlobalEnvironment::graphics_config() {
    return instance()->graphics_config_;
}


void PyGlobalEnvironment::initGraphicsConfig(const PySTKGraphicsConfig & config) {
    UserConfigParams::m_width  = config.screen_width;
    UserConfigParams::m_height = config.screen_height;
    UserConfigParams::m_real_width  = config.screen_width;
    UserConfigParams::m_real_height = config.screen_height;
    UserConfigParams::m_glow = config.glow;
    UserConfigParams::m_bloom = config.bloom;
    UserConfigParams::m_light_shaft = config.light_shaft;
    UserConfigParams::m_dynamic_lights = config.dynamic_lights;
    UserConfigParams::m_dof = config.dof;
    UserConfigParams::m_particles_effects = config.particles_effects;
    UserConfigParams::m_animated_characters = config.animated_characters;
    UserConfigParams::m_motionblur = config.motionblur;
    UserConfigParams::m_mlaa = config.mlaa;
    UserConfigParams::m_texture_compression=  config.texture_compression;
    UserConfigParams::m_ssao = config.ssao;
    UserConfigParams::m_degraded_IBL = config.degraded_IBL;
    UserConfigParams::m_high_definition_textures = config.high_definition_textures;
}


//=============================================================================
/** Initialises the minimum number of managers to get access to user_config.
 */
void PyGlobalEnvironment::initUserConfig(const std::string & data_dir)
{
    // Use environment variable
    const auto data_dir_env = getenv("SUPERTUXKART_DATADIR");

    if (!data_dir_env) {
#ifdef WIN32
    _putenv_s("SUPERTUXKART_DATADIR", data_dir.c_str());
#else
   setenv("SUPERTUXKART_DATADIR", data_dir.c_str(), true);
#endif
    }

    file_manager = new FileManager();
    // Some parts of the file manager needs user config (paths for models
    // depend on artist debug flag). So init the rest of the file manager
    // after reading the user config file.
    file_manager->init();

    translations            = new Translations();   // needs file_manager
    stk_config              = new STKConfig();      // in case of --stk-config
                                                    // command line parameters
}   // initUserConfig

//=============================================================================
void PyGlobalEnvironment::initRest()
{

    if (!graphics_config_.render) {
        // Fully disable graphics
        GUIEngine::reallyDisableGraphics();
    }

#ifndef SERVER_ONLY
        TipsManager::create();
#endif

    irr_driver = new IrrDriver();
    // Now create the actual non-null device in the irrlicht driver
    irr_driver->initDevice();

#ifndef SERVER_ONLY
    // Hide the window when rendering without display
    if (graphics_config_.render && !graphics_config_.display) {
        auto* sdl_device = dynamic_cast<irr::CIrrDeviceSDL*>(irr_driver->getDevice());
        if (sdl_device && sdl_device->getWindow())
            SDL_HideWindow(sdl_device->getWindow());
    }
#endif

    StkTime::init();   // grabs the timer object from the irrlicht device

    if (irr_driver->getDevice() == NULL)
    {
        Log::fatal("main", "Couldn't initialise irrlicht device. Quitting.\n");
    }


    IrrlichtDevice* device = irr_driver->getDevice();
    video::IVideoDriver* driver = device->getVideoDriver();
    font_manager = new FontManager();

    if (graphics_config_.render) 
    {
        SP::setMaxTextureSize();

        GUIEngine::init(device, driver, StateManager::get());
        // GUIEngine::renderLoading(true, true, false);
        // GUIEngine::flushRenderLoading(true/*launching*/);

        SP::loadShaders();
    } else {
        GUIEngine::init(device, driver, StateManager::get());
    }

    PlayerManager::create();

    music_manager = new MusicManager();
    history = new History();

    SFXManager::create();
    // The order here can be important, e.g. KartPropertiesManager needs
    // defaultKartProperties, which are defined in stk_config.
    material_manager        = new MaterialManager      ();
    track_manager           = new TrackManager         ();
    kart_properties_manager = new KartPropertiesManager();
    ProjectileManager::create();
    powerup_manager         = new PowerupManager       ();
    attachment_manager      = new AttachmentManager    ();
    highscore_manager       = new HighscoreManager     ();

#ifndef SERVER_ONLY
    if (!GUIEngine::isReallyNoGraphics())
    {
        // The maximum texture size can not be set earlier, since
        // e.g. the background image needs to be loaded in high res.
        irr_driver->setMaxTextureSize();
    }
#endif
    KartPropertiesManager::addKartSearchDir(
                 file_manager->getAddonsFile("karts/"));
    track_manager->addTrackSearchDir(
                 file_manager->getAddonsFile("tracks/"));

    {
        XMLNode characteristicsNode(file_manager->getAsset("kart_characteristics.xml"));
        kart_properties_manager->loadCharacteristics(&characteristicsNode);
    }

    track_manager->loadTrackList();

    grand_prix_manager      = new GrandPrixManager();
    // Consistency check for challenges, and enable all challenges
    // that have all prerequisites fulfilled
    grand_prix_manager->checkConsistency();

    RaceManager::create();
    auto race_manager = RaceManager::get();
    // default settings for Quickstart
    race_manager->setNumPlayers(1);
    race_manager->setNumLaps   (3);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_NORMAL_RACE);
    race_manager->setDifficulty(
                 (RaceManager::Difficulty)(int)UserConfigParams::m_difficulty);

    kart_properties_manager -> loadAllKarts(false);

}   // initRest

//=============================================================================
/** Frees all manager and their associated memory.
 */
void PyGlobalEnvironment::cleanSuperTuxKart()
{
    // Stop music (this request will go into the sfx manager queue, so it needs
    // to be done before stopping the thread).
    RaceManager::destroy();
    if(attachment_manager)      delete attachment_manager;
    attachment_manager = nullptr;
    ItemManager::removeTextures();
    if(powerup_manager)         delete powerup_manager;
    powerup_manager = nullptr;

    ProjectileManager::destroy();

    if(kart_properties_manager) delete kart_properties_manager;
    kart_properties_manager = nullptr;
    if(track_manager)           delete track_manager;
    track_manager = nullptr;
    if(material_manager)        delete material_manager;
    material_manager = nullptr;

    if(history)                 delete history;
    history = nullptr;

    PlayerManager::destroy();
    if(unlock_manager)          delete unlock_manager;

    if (music_manager)          delete music_manager;
    music_manager = nullptr;
    
    Referee::cleanup();
    ParticleKindManager::get()->cleanup();
    if(font_manager)            delete font_manager;
    font_manager = nullptr;
    
    // StkTime::destroy();

    // Now finish shutting down objects which a separate thread. The
    // RequestManager has been signaled to shut down as early as possible,
    // the NewsManager thread should have finished quite early on anyway.
    // But still give them some additional time to finish. It avoids a
    // race condition where a thread might access the file manager after it
    // was deleted (in cleanUserConfig below), but before STK finishes and
    // the OS takes all threads down.

    cleanUserConfig();
}   // cleanSuperTuxKart

//=============================================================================
/**
 * Frees all the memory of initUserConfig()
 */
void PyGlobalEnvironment::cleanUserConfig()
{
    if(stk_config)              delete stk_config;
    stk_config = nullptr;

    if(translations)            delete translations;
    translations = nullptr;

    if(irr_driver)              delete irr_driver;
    irr_driver = nullptr;
}   // cleanUserConfig


std::shared_ptr<PyGlobalEnvironment> PyGlobalEnvironment::instance() {
    if (!_instance) {
        throw std::invalid_argument("pystk2 has not been initialized");
    }

    return _instance;
}

void PyGlobalEnvironment::cleanup() {
    _instance = nullptr;
}

void PyGlobalEnvironment::load() {
    material_manager->loadMaterial();
    // Preload the explosion effects (explode.png)
    ParticleKindManager::get()->getParticles("explosion.xml");
    ParticleKindManager::get()->getParticles("explosion_bomb.xml");
    ParticleKindManager::get()->getParticles("explosion_cake.xml");
    ParticleKindManager::get()->getParticles("jump_explosion.xml");

    // Creates the main loop
    main_loop = new MainLoop(0 /* parent_pid */);

    // Reading the rest of the player data needs the unlock manager to
    // initialise the game slots of all players and the AchievementsManager
    // to initialise the AchievementsStatus, so it is done only now.
    ProjectileManager::get()->loadData();

    // Needs the kart and track directories to load potential challenges
    // in those dirs, so it can only be created after reading tracks
    // and karts.
    unlock_manager = new UnlockManager();
    AchievementsManager::create();

    // Both item_manager and powerup_manager load models and therefore
    // textures from the model directory. To avoid reading the
    // materials.xml twice, we do this here once for both:
    file_manager->pushTextureSearchPath(file_manager->getAsset(FileManager::MODEL,""), "models");
    const std::string materials_file = file_manager->getAsset(FileManager::MODEL,"materials.xml");
    if(materials_file!="")
    {
        // Some of the materials might be needed later, so just add
        // them all permanently (i.e. as shared). Adding them temporary
        // will actually not be possible: powerup_manager adds some
        // permanent icon materials, which would (with the current
        // implementation) make the temporary materials permanent anyway.
        material_manager->addSharedMaterial(materials_file);
    }
    Referee::init();
    powerup_manager->loadPowerupsModels();
    ItemManager::loadDefaultItemMeshes();
    attachment_manager->loadModels();
    file_manager->popTextureSearchPath();


    // create a player if we have none
    auto player_manager = PlayerManager::get();
    player_manager->enforceCurrentPlayer();

    // Create first player and associate input device
    auto profile = player_manager->getPlayer(0);
    StateManager::get()->createActivePlayer(
        profile, FakeInputDevice::instance()
    );
    profile->initRemainingData();
    player_manager->setCurrentPlayer(profile);
}


void PyGlobalEnvironment::clean() {
    if (PySTKRace::running_kart)
        throw std::invalid_argument("Cannot clean up while supertuxkart is running!");
    cleanSuperTuxKart();
    Log::flushBuffers();

    delete file_manager;
    file_manager = NULL;
}