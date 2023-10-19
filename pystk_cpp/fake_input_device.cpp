#include "fake_input_device.hpp"

namespace 
{
    FakeInputDevice * INSTANCE = new FakeInputDevice();
}

bool FakeInputDevice::processAndMapInput(Input::InputType type,  const int id,
    InputManager::InputDriverMode mode,
    PlayerAction *action, 
    int* value
)
{
    return false;
}

FakeInputDevice * FakeInputDevice::instance() 
{
    return INSTANCE;
}