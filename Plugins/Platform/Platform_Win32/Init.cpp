#include "Platform_Win32.h"

#include <map>

extern "C" __declspec(dllexport) WeOn::Plugin::IPlugin * CreatePlugin()
{
    return new Platform_Win32();
}

extern "C" __declspec(dllexport) void DestroyPlugin(WeOn::Plugin::IPlugin * plugin)
{
    delete plugin;
}

void Platform_Win32::Init(WeOn::EventSystem::Manager* manager)
{
    _manager = manager;

    std::cout << "[Platform_Win32] Initializing and registering event handlers..." << std::endl;

    //
    // 🪟 WINDOW MANAGEMENT
    //
    _eventHandlers["CreateWindow"] = [this](const auto& e) { Handle_CreateWindow(e); };
    _eventHandlers["DestroyWindow"] = [this](const auto& e) { Handle_DestroyWindow(e); };
    _eventHandlers["ResizeWindow"] = [this](const auto& e) { Handle_ResizeWindow(e); };
    _eventHandlers["MoveWindow"] = [this](const auto& e) { Handle_MoveWindow(e); };
    _eventHandlers["SetFocus"] = [this](const auto& e) { Handle_SetFocus(e); };
    _eventHandlers["ToggleFullscreen"] = [this](const auto& e) { Handle_ToggleFullscreen(e); };
    _eventHandlers["MinimizeWindow"] = [this](const auto& e) { Handle_Minimize(e); };
    _eventHandlers["MaximizeWindow"] = [this](const auto& e) { Handle_Maximize(e); };
    _eventHandlers["SetWindowTitle"] = [this](const auto& e) { Handle_SetWindowTitle(e); };
    _eventHandlers["SetWindowIcon"] = [this](const auto& e) { Handle_SetWindowIcon(e); };
    _eventHandlers["UpdateWindow"] = [this](const auto& e) { Handle_UpdateWindow(e); };
    _eventHandlers["PaintWindow"] = [this](const auto& e) { Handle_Paint(e); };

    //
    // 📁 USER INTERACTION
    //
    _eventHandlers["Show_ModalWindow"] = [this](const auto& e) { Handle_OpenModalWindow(e); };
    _eventHandlers["Open_FileDialog"] = [this](const auto& e) { Handle_OpenFileDialog(e); };
    _eventHandlers["Save_FileDialog"] = [this](const auto& e) { Handle_SaveFileDialog(e); };
    _eventHandlers["Open_FolderDialog"] = [this](const auto& e) { Handle_OpenFolderDialog(e); };
    //_eventHandlers["Show_ConfirmDialog"] = [this](const auto& e) { Handle_ConfirmDialog(e); };
    _eventHandlers["Show_Toast"] = [this](const auto& e) { Handle_ShowToastNotification(e); };
    _eventHandlers["Show_TrayIcon"] = [this](const auto& e) { Handle_ShowSystemTrayIcon(e); };
    _eventHandlers["Remove_TrayIcon"] = [this](const auto& e) { Handle_RemoveSystemTrayIcon(e); };
    _eventHandlers["Show_InputDialog"] = [this](const auto& e) { Handle_ShowInputDialog(e); };
    _eventHandlers["Show_ColorDialog"] = [this](const auto& e) { Handle_ShowColorDialog(e); };
    _eventHandlers["Show_FontDialog"] = [this](const auto& e) { Handle_ShowFontDialog(e); };

    //
    // ⚙️ SYSTEM INFORMATION
    //
    _eventHandlers["GetMemoryInfo"] = [this](const auto& e) { Handle_GetMemory(e); };
    _eventHandlers["GetSystemInfo"] = [this](const auto& e) { Handle_GetSystemInfo(e); };
    _eventHandlers["GetDisplayInfo"] = [this](const auto& e) { Handle_GetDisplayInfo(e); };
    _eventHandlers["GetPowerStatus"] = [this](const auto& e) { Handle_GetPowerStatus(e); };
    _eventHandlers["GetUSBPort"] = [this](const auto& e) { Handle_GetUSBPort(e); };
    _eventHandlers["GetNetworkAdapters"] = [this](const auto& e) { Handle_GetNetworkAdapters(e); };
    _eventHandlers["GetDiskDrives"] = [this](const auto& e) { Handle_GetDiskDrives(e); };
    _eventHandlers["GetProcesses"] = [this](const auto& e) { Handle_GetProcesses(e); };
    _eventHandlers["GetEnvVariables"] = [this](const auto& e) { Handle_GetEnvironmentVariables(e); };
    _eventHandlers["GetRegistryValue"] = [this](const auto& e) { Handle_GetRegistryValue(e); };
    _eventHandlers["SetRegistryValue"] = [this](const auto& e) { Handle_SetRegistryValue(e); };
    _eventHandlers["ExecuteCommand"] = [this](const auto& e) { Handle_ExecuteCommand(e); };

    //
    // ⏱️ TIMERS AND TASKS
    //
    _eventHandlers["CreateTimer"] = [this](const auto& e) { Handle_CreateTimer(e); };
    _eventHandlers["DestroyTimer"] = [this](const auto& e) { Handle_DestroyTimer(e); };
    _eventHandlers["StartTimer"] = [this](const auto& e) { Handle_StartTimer(e); };
    _eventHandlers["StopTimer"] = [this](const auto& e) { Handle_StopTimer(e); };
    _eventHandlers["ScheduleTask"] = [this](const auto& e) { Handle_ScheduleTask(e); };

    //
    // 🎧 INPUT & MULTIMEDIA
    //
    _eventHandlers["GetKeyboardState"] = [this](const auto& e) { Handle_GetKeyboardState(e); };
    _eventHandlers["GetMousePosition"] = [this](const auto& e) { Handle_GetMousePosition(e); };
    _eventHandlers["SetMousePosition"] = [this](const auto& e) { Handle_SetMousePosition(e); };
    _eventHandlers["RegisterHotKey"] = [this](const auto& e) { Handle_RegisterHotKey(e); };
    _eventHandlers["UnregisterHotKey"] = [this](const auto& e) { Handle_UnregisterHotKey(e); };
    _eventHandlers["PlaySound"] = [this](const auto& e) { Handle_PlaySound(e); };
    _eventHandlers["GetAudioDevices"] = [this](const auto& e) { Handle_GetAudioDevices(e); };

    //
    // 🌐 MODERN WINDOWS FEATURES
    //
    _eventHandlers["GetSystemTheme"] = [this](const auto& e) { Handle_GetSystemTheme(e); };
    _eventHandlers["SetSystemTheme"] = [this](const auto& e) { Handle_SetSystemTheme(e); };
    _eventHandlers["EnableBlurBehind"] = [this](const auto& e) { Handle_EnableBlurBehindWindow(e); };
    _eventHandlers["EnableDarkTitleBar"] = [this](const auto& e) { Handle_EnableDarkTitleBar(e); };
    _eventHandlers["GetClipboardText"] = [this](const auto& e) { Handle_GetClipboardText(e); };
    _eventHandlers["SetClipboardText"] = [this](const auto& e) { Handle_SetClipboardText(e); };

    for (const auto& pair : _eventHandlers)
    {
        _manager->SubscribeEvent(static_cast<WeOn::Plugin::IPlugin&>(*this), pair.first.c_str());
        std::cout << "[Platform_Win32] Subscribed to event: " << pair.first << std::endl;
    }


    std::cout << "[Platform_Win32] Registered " << _eventHandlers.size() << " event handlers." << std::endl;
}

// Завершение работы плагина
void Platform_Win32::ShotDown()
{
    std::cout << "[Platform_Win32] Plugin shutting down.\n";

    if (_hwnd)
    {
        PostMessage(_hwnd, WM_CLOSE, 0, 0); // корректно закрываем окно
        _hwnd = nullptr;
    }
}

// Подготовка плагина к загрузке (вызывается менеджером)
bool Platform_Win32::OnLoad()
{
    std::cout << "[Platform_Win32] OnLoad called.\n";
    // Здесь можно сделать инициализацию ресурсов, если нужно
    return true;
}

// Очистка ресурсов плагина
void Platform_Win32::OnUnload()
{
    std::cout << "[Platform_Win32] OnUnload called.\n";
    ShotDown();
}