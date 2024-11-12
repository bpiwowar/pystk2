#include <iostream>
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "pystk_controller.hpp"

PySTKController::PySTKController(AbstractKart *kart, const int local_player_id) : Controller(kart) 
{
    m_player = StateManager::get()->getActivePlayer(local_player_id);
    if(m_player) 
    {
        m_player->setKart(kart);
    }
}

bool PySTKController::action(PlayerAction action, int value, bool dry_run)
{
    if (action == PA_ACCEL && value != 0 && !m_has_started)
    {
        m_has_started = true;
        float f = m_kart->getStartupBoostFromStartTicks(
            World::getWorld()->getAuxiliaryTicks());
        m_kart->setStartupBoost(f);
    }

    return true;
}

void PySTKController::reset() 
{

}
