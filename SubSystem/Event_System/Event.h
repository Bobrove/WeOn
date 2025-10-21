#pragma once

#include <cstring>
#include <atomic>

namespace WeOn
{
	namespace Plugin {
		struct IPlugin; // <--- вот это ключевой момент
	}

	namespace EventSystem
	{
		const size_t _MAX_EVENT = 1000;
		const size_t _MAX_DATA = 512;

		const size_t _MAX_EVENT_NAME = 100;
		const size_t _MAX_EVENT_DATA = 300;

		const size_t _MAX_DATA_NAME = 100;
		const size_t _MAX_DATA_FILEPATH = 100;
		const size_t _MAX_DATA_FILE_SIZE = 100;

		struct Event
		{
			uint64_t	_ID;
			uint64_t	_timespawn;
			char		_name[_MAX_EVENT_NAME];
			uint8_t		_data[_MAX_EVENT_DATA];
		};

		struct Data
		{
			uint64_t    _ID;
			uint64_t    _timespawn;
			char        _name[_MAX_DATA_NAME];
			char        _filepath[_MAX_DATA_FILEPATH];
			uint64_t    _file_size_now;
			uint64_t    _file_size_expected;
		};

		// Подписка на событие или данные
		struct Subscription
		{
			char name[_MAX_EVENT_NAME];
			WeOn::Plugin::IPlugin* plugin;
		};

		class Manager
		{
		public:
			Manager() = default;

			// --- публикация ---
			bool PublishEvent(const Event& event);

			bool PublishData(const Data& data);

			// --- подписка ---
			bool SubscribeEvent(Plugin::IPlugin& plugin, const char* event_name);
			bool SubscribeData(Plugin::IPlugin& plugin, const char* data_name);

			// --- отписка ---
			bool UnsubscribeEvent(Plugin::IPlugin& plugin, const char* event_name);

			bool UnsubscribeData(Plugin::IPlugin& plugin, const char* data_name);

			// --- извлечение ---
			bool PopEvent(Event& outEvent);

			bool GetData(uint64_t id, Data& outData);

			void Update();

		private:
			void CheckAndNotifyDataComplete(Data& data);

			// Кольцевой буфер для событий
			Event _eventBuffer[_MAX_EVENT];
			std::atomic<size_t> _eventHead{ 0 };
			std::atomic<size_t> _eventTail{ 0 };

			// Таблица данных
			Data _dataBuffer[_MAX_DATA];
			bool _dataActive[_MAX_DATA];

			// Подписчики
			Subscription _eventSubscribers[_MAX_EVENT];
			Subscription _dataSubscribers[_MAX_DATA];
			size_t _eventSubCount = 0;
			size_t _dataSubCount = 0;
		};
	}
};
