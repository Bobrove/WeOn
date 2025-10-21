#include "Platform_Win32.h"

void Platform_Win32::OnEvent(const WeOn::EventSystem::Event& event)
{
    auto it = _eventHandlers.find(event._name);
    if (it != _eventHandlers.end())
    {
        it->second(event); // вызываем соответствующую функцию
    }
    else
    {
        std::cout << "[Platform_Win32] ⚠️ Event not handled: " << event._name << std::endl;
    }
}

void Platform_Win32::OnData(const WeOn::EventSystem::Data&)
{
    // пусто
}