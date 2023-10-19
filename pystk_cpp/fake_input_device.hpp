#pragma once

#include "input/input_device.hpp"

/**
  * \brief Fake specialisation of InputDevice for pystk
  * \ingroup input
  */
class FakeInputDevice : public InputDevice
{
public:
    virtual bool processAndMapInput(Input::InputType type,  const int id,
                                    InputManager::InputDriverMode mode,
                                    PlayerAction *action, int* value = NULL
                                    );

    static FakeInputDevice * instance();
};