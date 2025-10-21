#pragma once
#include <cstdint>
#include "D:/Work_Misha/Programing/FrameWorks/WeOn/SubSystem/Event_System/Event_System.h"

namespace WeOn
{
	namespace EventSystem
	{
		struct Event;
		struct Data;
		class Manager;
	}

	namespace Plugin
	{
		struct IPlugin
		{
			virtual ~IPlugin() = default;

			virtual void Init(WeOn::EventSystem::Manager* manager) = 0;
			virtual void ShotDown() = 0;

			// Возвращает имя плагина (для логов, идентификации)
			virtual const char* GetName() = 0;

			// Основная инициализация, можно вызывать при загрузке
			virtual bool OnLoad() = 0;

			// Обработка системных или пользовательских событий
			virtual void OnEvent(const WeOn::EventSystem::Event& event) = 0;

			// Обработка поступающих данных (например, файлов, сетевых чанков)
			virtual void OnData(const WeOn::EventSystem::Data& data)  = 0;

			// Освобождение ресурсов, перед выгрузкой
			virtual void OnUnload()  = 0;
		};
	}
}
