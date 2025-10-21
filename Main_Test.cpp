#include <Windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "D:/Work_Misha/Programing/FrameWorks/WeOn/IPlugin.h"
#include "D:/Work_Misha/Programing/FrameWorks/WeOn/SubSystem/Event_System/Event_System.h"
#include "D:/Work_Misha/Programing/FrameWorks/Serializer/Serializer.h"

#pragma comment(lib, "D:\\Work_Misha\\Programing\\FrameWorks\\WeOn\\x64\\Debug\\Serializer.lib")
#pragma comment(lib, "D:\\Work_Misha\\Programing\\FrameWorks\\WeOn\\x64\\Debug\\Event_System.lib")

using namespace WeOn;

typedef Plugin::IPlugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(Plugin::IPlugin*);

int main()
{
    WeOn::EventSystem::Manager manager;

    // ----------------------------
    // Загружаем DLL-плагин
    // ----------------------------
    HMODULE hPlugin = LoadLibrary(L"D:\\Work_Misha\\Programing\\FrameWorks\\WeOn\\x64\\Debug\\Platform_Win32_1.0.dll");
    if (!hPlugin)
    {
        std::cerr << "[Main] ❌ Failed to load plugin DLL.\n";
        return -1;
    }

    auto createFunc = (CreatePluginFunc)GetProcAddress(hPlugin, "CreatePlugin");
    auto destroyFunc = (DestroyPluginFunc)GetProcAddress(hPlugin, "DestroyPlugin");

    if (!createFunc || !destroyFunc)
    {
        std::cerr << "[Main] ❌ Failed to find plugin entry points.\n";
        FreeLibrary(hPlugin);
        return -1;
    }

    // ----------------------------
    // Создаём плагин
    // ----------------------------
    WeOn::Plugin::IPlugin* platform = createFunc();
    platform->Init(&manager);

    std::cout << "[Main] ✅ Plugin loaded and initialized.\n";

    // ----------------------------
    // Тест 1: Создание главного окна
    // ----------------------------
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "CreateWindow", _TRUNCATE);
        manager.PublishEvent(ev);
    }

    // ----------------------------
    // Интерактивная консоль управления
    // ----------------------------
    std::thread commandThread([&manager, &platform, &hPlugin, &createFunc, &destroyFunc]() {
        std::string line;
        while (true)
        {
            std::cout << "\n> ";
            std::getline(std::cin, line);

            if (line == "exit")
            {
                std::cout << "[IDE] 🧩 Exiting...\n";
                exit(0);
            }
            else if (line == "toast")
            {
                WeOn::EventSystem::Event ev{};
                strncpy_s(ev._name, sizeof(ev._name), "Show_Toast", _TRUNCATE);

                size_t offset = 0;
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "IDE Command");
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Live toast notification!");
                StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), NIIF_INFO);
                StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), 5000);
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "IDE Command Triggered");
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "");

                manager.PublishEvent(ev);
            }
            else if (line == "modal")
            {
                WeOn::EventSystem::Event ev{};
                strncpy_s(ev._name, sizeof(ev._name), "Show_ModalWindow", _TRUNCATE);

                size_t offset = 0;
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Live IDE Modal");
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "This modal came from your interactive console!");

                manager.PublishEvent(ev);
            }
            else if (line == "window")
            {
                WeOn::EventSystem::Event ev{};
                strncpy_s(ev._name, sizeof(ev._name), "CreateWindow", _TRUNCATE);
                manager.PublishEvent(ev);
            }
            else if (line == "reload")
            {
                std::cout << "[IDE] ♻️ Reloading plugin...\n";

                platform->ShotDown();
                destroyFunc(platform);
                FreeLibrary(hPlugin);

                HMODULE newPlugin = LoadLibrary(L"D:\\Work_Misha\\Programing\\FrameWorks\\WeOn\\x64\\Debug\\Platform_Win32_1.0.dll");
                if (!newPlugin)
                {
                    std::cerr << "[IDE] ❌ Failed to reload DLL.\n";
                    continue;
                }

                auto newCreateFunc = (CreatePluginFunc)GetProcAddress(newPlugin, "CreatePlugin");
                auto newDestroyFunc = (DestroyPluginFunc)GetProcAddress(newPlugin, "DestroyPlugin");

                platform = newCreateFunc();
                platform->Init(&manager);
                std::cout << "[IDE] ✅ Plugin reloaded successfully!\n";
            }
            else
            {
                std::cout << "[IDE] ⚠️ Unknown command.\n";
            }
        }
        });
    commandThread.detach();

    // ----------------------------
    // Основной “жизненный цикл”
    // ----------------------------
    std::cout << "[Main] 🔄 Running main loop (press Ctrl+C to exit)...\n";

    
    for (;;)
    {
      //  std::this_thread::sleep_for(std::chrono::seconds(2));
    //    std::cout << "[Main] (heartbeat) main thread alive...\n";
    }
    

    platform->ShotDown();
    destroyFunc(platform);
    FreeLibrary(hPlugin);

    return 0;
}
