#pragma once

#include "karts/controller/controller.hpp"

/**
  * \brief Specialisation of Controller for PySTK
  * \ingroup input
  * 
  * Controller is controlled by PySTK directly
  */
class PySTKController : public Controller
{
    /** Stores the active player data structure. */
    StateManager::ActivePlayer *m_player;

    /** True if the race has started */
    bool           m_has_started;

public:
    PySTKController(AbstractKart *kart, const int local_player_id);

    // Just do nothing!
    virtual void reset() OVERRIDE;
    virtual void update (int ticks) OVERRIDE {}
    virtual bool disableSlipstreamBonus() const OVERRIDE { return true; }
    virtual void crashed(const Material *m) OVERRIDE {}
    virtual void crashed(const AbstractKart *k) OVERRIDE {}
    virtual void handleZipper(bool play_sound) OVERRIDE {}
    virtual void finishedRace(float time) OVERRIDE {}
    virtual void collectedItem(const ItemState &item,
                               float previous_energy=0) OVERRIDE {}
    virtual void setPosition(int p) OVERRIDE {}
    virtual bool isPlayerController() const OVERRIDE { return true; }
    virtual bool isLocalPlayerController() const OVERRIDE { return true; }
    virtual bool action(PlayerAction action, int value,
                        bool dry_run=false) OVERRIDE;
    virtual void skidBonusTriggered() OVERRIDE {}
    virtual void newLap(int lap) OVERRIDE {}
    virtual bool saveState(BareNetworkString *buffer) const OVERRIDE
                                                              { return false; }
    virtual void  rewindTo(BareNetworkString *buffer) OVERRIDE {}
};