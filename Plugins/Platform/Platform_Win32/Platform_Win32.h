#pragma once

#include <Windows.h>
#include <string>
#include <iostream>

#include <functional>
#include <unordered_map>

#include "WeOn/IPlugin.h"
#include "WeOn/SubSystem/Event_System/Event.h"
#include "WeOn/SubSystem/Event_System//Serializer.h"

#pragma comment(lib, "WeOn\\x64\\Debug\\Serializer.lib")
#pragma comment(lib, "WeOn\\x64\\Debug\\Event.lib")

class Platform_Win32 : public WeOn::Plugin::IPlugin
{
	// Унаследовано через IPlugin
	void Init(WeOn::EventSystem::Manager* manager);
	void ShotDown();
	const char* GetName() { return "Platform_Win32"; }
	bool OnLoad() override;
	void OnEvent(const WeOn::EventSystem::Event& event);
	void OnData(const WeOn::EventSystem::Data& data);
	void OnUnload();

private:
	std::unordered_map<std::string, std::function<void(const WeOn::EventSystem::Event&)>> _eventHandlers;

	WeOn::EventSystem::Manager* _manager = nullptr;
	HWND _hwnd = nullptr;

private:

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	// 🖥️ 1. Оконные операции
	void Handle_CreateWindow(const WeOn::EventSystem::Event& event);              // создать основное окно
	void Handle_DestroyWindow(const WeOn::EventSystem::Event& event);             // уничтожить окно
	void Handle_ShowWindow(const WeOn::EventSystem::Event& event);                // показать / скрыть окно
	void Handle_ResizeWindow(const WeOn::EventSystem::Event& event);              // изменение размера окна
	void Handle_MoveWindow(const WeOn::EventSystem::Event& event);                // перемещение окна
	void Handle_SetFocus(const WeOn::EventSystem::Event& event);                  // установка фокуса
	void Handle_ToggleFullscreen(const WeOn::EventSystem::Event& event);          // полноэкранный режим
	void Handle_Minimize(const WeOn::EventSystem::Event& event);                  // свернуть
	void Handle_Maximize(const WeOn::EventSystem::Event& event);                  // развернуть
	void Handle_SetWindowTitle(const WeOn::EventSystem::Event& event);            // изменить заголовок окна
	void Handle_SetWindowIcon(const WeOn::EventSystem::Event& event);             // задать иконку окна
	void Handle_UpdateWindow(const WeOn::EventSystem::Event& event);              // перерисовать
	void Handle_Paint(const WeOn::EventSystem::Event& event);                     // обработка WM_PAINT

	// 📁 2. Диалоги и взаимодействие с пользователем
	void Handle_OpenModalWindow(const WeOn::EventSystem::Event& event);           // модальное окно (MessageBox)
	void Handle_OpenFileDialog(const WeOn::EventSystem::Event& event);            // диалог открытия файла
	void Handle_SaveFileDialog(const WeOn::EventSystem::Event& event);            // диалог сохранения файла
	void Handle_OpenFolderDialog(const WeOn::EventSystem::Event& event);          // выбор папки
	void Handle_ShowToastNotification(const WeOn::EventSystem::Event& event);     // уведомление Windows 10+
	void Handle_ShowSystemTrayIcon(const WeOn::EventSystem::Event& event);        // добавить иконку в трей
	void Handle_RemoveSystemTrayIcon(const WeOn::EventSystem::Event& event);      // удалить иконку из трея
	void Handle_ShowInputDialog(const WeOn::EventSystem::Event& event) {};           // простое текстовое поле ввода
	void Handle_ShowColorDialog(const WeOn::EventSystem::Event& event) {};           // выбор цвета
	void Handle_ShowFontDialog(const WeOn::EventSystem::Event& event) {};            // выбор шрифта

	// ⚙️ 3. Системные и аппаратные запросы
	void Handle_GetMemory(const WeOn::EventSystem::Event& event);                 // информация об оперативке
	void Handle_GetSystemInfo(const WeOn::EventSystem::Event& event);             // CPU, архитектура, ОС
	void Handle_GetDisplayInfo(const WeOn::EventSystem::Event& event);            // мониторы, DPI, разрешения
	void Handle_GetPowerStatus(const WeOn::EventSystem::Event& event);            // заряд батареи, питание
	void Handle_GetUSBPort(const WeOn::EventSystem::Event& event);                // подключенные USB устройства
	void Handle_GetNetworkAdapters(const WeOn::EventSystem::Event& event);        // сетевые интерфейсы
	void Handle_GetDiskDrives(const WeOn::EventSystem::Event& event);             // диски и тома
	void Handle_GetProcesses(const WeOn::EventSystem::Event& event);              // список процессов
	void Handle_GetEnvironmentVariables(const WeOn::EventSystem::Event& event);   // переменные среды
	void Handle_GetRegistryValue(const WeOn::EventSystem::Event& event);          // чтение из реестра
	void Handle_SetRegistryValue(const WeOn::EventSystem::Event& event);          // запись в реестр
	void Handle_ExecuteCommand(const WeOn::EventSystem::Event& event);            // запуск внешнего процесса

	// ⏱️ 4. Таймеры и события
	void Handle_CreateTimer(const WeOn::EventSystem::Event& event) {};               // создать таймер
	void Handle_DestroyTimer(const WeOn::EventSystem::Event& event) {};              // уничтожить таймер
	void Handle_StartTimer(const WeOn::EventSystem::Event& event) {};                // запустить таймер
	void Handle_StopTimer(const WeOn::EventSystem::Event& event) {};                 // остановить таймер
	void Handle_ScheduleTask(const WeOn::EventSystem::Event& event) {};              // отложенное выполнение

	// 🎧 5. Ввод, устройства и мультимедиа
	void Handle_GetKeyboardState(const WeOn::EventSystem::Event& event) {};          // состояние клавиатуры
	void Handle_GetMousePosition(const WeOn::EventSystem::Event& event) {};          // позиция курсора
	void Handle_SetMousePosition(const WeOn::EventSystem::Event& event) {};          // переместить курсор
	void Handle_RegisterHotKey(const WeOn::EventSystem::Event& event) {};            // глобальный хоткей
	void Handle_UnregisterHotKey(const WeOn::EventSystem::Event& event) {};
	void Handle_PlaySound(const WeOn::EventSystem::Event& event) {};                 // воспроизведение звука
	void Handle_GetAudioDevices(const WeOn::EventSystem::Event& event) {};           // список звуковых устройств

	// 🌐 6. Современные фишки (Windows 10/11+)
	void Handle_GetSystemTheme(const WeOn::EventSystem::Event& event) {};            // светлая/тёмная тема
	void Handle_SetSystemTheme(const WeOn::EventSystem::Event& event) {};            // изменить тему
	void Handle_EnableBlurBehindWindow(const WeOn::EventSystem::Event& event) {};    // эффект прозрачности / blur
	void Handle_EnableDarkTitleBar(const WeOn::EventSystem::Event& event) {};        // тёмный заголовок
	void Handle_GetClipboardText(const WeOn::EventSystem::Event& event) {};          // получить текст из буфера
	void Handle_SetClipboardText(const WeOn::EventSystem::Event& event) {};          // установить текст в буфер

};
