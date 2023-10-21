#include "pystk_controller.hpp"

PySTKController::PySTKController(AbstractKart *kart, const int local_player_id) : Controller(kart) 
{
    m_player = StateManager::get()->getActivePlayer(local_player_id);
    if(m_player)
        m_player->setKart(kart);

}

bool PySTKController::action(PlayerAction action, int value, bool dry_run)
{
    return true;
}

void PySTKController::reset() 
{

}
