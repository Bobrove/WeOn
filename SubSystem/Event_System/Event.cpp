#include "Event_System.h"
#include "D:/Work_Misha/Programing/FrameWorks/WeOn/IPlugin.h"
#include <iostream>

namespace WeOn
{
    namespace EventSystem
    {
        bool Manager::PublishEvent(const Event& event)
        {
            size_t next = (_eventHead + 1) % _MAX_EVENT;
            if (next == _eventTail)
            {
                std::cerr << "[Manager] Event buffer overflow, cannot publish event: " << event._name << "\n";
                return false;
            }

            _eventBuffer[_eventHead] = event;
            _eventHead = next;

            std::cout << "[Manager] Publishing event: " << event._name << "\n";

            // уведомляем всех подписчиков
            for (size_t i = 0; i < _eventSubCount; ++i)
            {
                if (std::strncmp(_eventSubscribers[i].name, event._name, _MAX_EVENT_NAME) == 0)
                {
                    if (_eventSubscribers[i].plugin)
                    {
                        std::cout << "[Manager] Notifying plugin for event: " << event._name << "\n";
                        _eventSubscribers[i].plugin->OnEvent(event);
                    }
                }
            }
            return true;
        }

        bool Manager::PublishData(const Data& data)
        {
            // ищем активную запись
            for (size_t i = 0; i < _MAX_DATA; ++i)
            {
                if (_dataActive[i] && _dataBuffer[i]._ID == data._ID)
                {
                    _dataBuffer[i]._file_size_now = data._file_size_now;
                    std::cout << "[Manager] Updating active data ID: " << data._ID << "\n";
                    CheckAndNotifyDataComplete(_dataBuffer[i]);
                    return true;
                }
            }

            // новая запись
            for (size_t i = 0; i < _MAX_DATA; ++i)
            {
                if (!_dataActive[i])
                {
                    _dataBuffer[i] = data;
                    _dataActive[i] = true;
                    std::cout << "[Manager] Added new data ID: " << data._ID << ", name: " << data._name << "\n";
                    CheckAndNotifyDataComplete(_dataBuffer[i]);
                    return true;
                }
            }

            std::cerr << "[Manager] Data buffer full, cannot publish data ID: " << data._ID << "\n";
            return false;
        }

        bool Manager::SubscribeEvent(Plugin::IPlugin& plugin, const char* event_name)
        {
            if (_eventSubCount >= _MAX_EVENT)
            {
                std::cerr << "[Manager] Event subscription limit reached for: " << event_name << "\n";
                return false;
            }

            strncpy_s(_eventSubscribers[_eventSubCount].name,
                sizeof(_eventSubscribers[_eventSubCount].name),
                event_name,
                _TRUNCATE);

            _eventSubscribers[_eventSubCount].plugin = &plugin;
            ++_eventSubCount;

            std::cout << "[Manager] Plugin subscribed to event: " << event_name << "\n";
            return true;
        }

        bool Manager::SubscribeData(Plugin::IPlugin& plugin, const char* data_name)
        {
            if (_dataSubCount >= _MAX_DATA)
            {
                std::cerr << "[Manager] Data subscription limit reached for: " << data_name << "\n";
                return false;
            }

            strncpy_s(_dataSubscribers[_dataSubCount].name,
                sizeof(_dataSubscribers[_dataSubCount].name),
                data_name,
                _TRUNCATE);

            _dataSubscribers[_dataSubCount].plugin = &plugin;
            ++_dataSubCount;

            std::cout << "[Manager] Plugin subscribed to data: " << data_name << "\n";
            return true;
        }

        bool Manager::UnsubscribeEvent(Plugin::IPlugin& plugin, const char* event_name)
        {
            for (size_t i = 0; i < _eventSubCount; ++i)
            {
                if (_eventSubscribers[i].plugin == &plugin &&
                    std::strncmp(_eventSubscribers[i].name, event_name, _MAX_EVENT_NAME) == 0)
                {
                    _eventSubscribers[i] = _eventSubscribers[_eventSubCount - 1];
                    --_eventSubCount;

                    std::cout << "[Manager] Plugin unsubscribed from event: " << event_name << "\n";
                    return true;
                }
            }

            std::cerr << "[Manager] Plugin tried to unsubscribe from unknown event: " << event_name << "\n";
            return false;
        }

        bool Manager::UnsubscribeData(Plugin::IPlugin& plugin, const char* data_name)
        {
            for (size_t i = 0; i < _dataSubCount; ++i)
            {
                if (_dataSubscribers[i].plugin == &plugin &&
                    std::strncmp(_dataSubscribers[i].name, data_name, _MAX_DATA_NAME) == 0)
                {
                    _dataSubscribers[i] = _dataSubscribers[_dataSubCount - 1];
                    --_dataSubCount;

                    std::cout << "[Manager] Plugin unsubscribed from data: " << data_name << "\n";
                    return true;
                }
            }

            std::cerr << "[Manager] Plugin tried to unsubscribe from unknown data: " << data_name << "\n";
            return false;
        }

        bool Manager::PopEvent(Event& outEvent)
        {
            if (_eventHead == _eventTail)
            {
                //std::cout << "[Manager] Event queue empty.\n";
                return false; // пусто
            }

            outEvent = _eventBuffer[_eventTail];
            _eventTail = (_eventTail + 1) % _MAX_EVENT;

            std::cout << "[Manager] Popped event: " << outEvent._name << "\n";
            return true;
        }

        bool Manager::GetData(uint64_t id, Data& outData)
        {
            for (size_t i = 0; i < _MAX_DATA; ++i)
            {
                if (_dataActive[i] && _dataBuffer[i]._ID == id)
                {
                    outData = _dataBuffer[i];
                    std::cout << "[Manager] Retrieved data ID: " << id << "\n";
                    return true;
                }
            }

            std::cerr << "[Manager] Data ID not found: " << id << "\n";
            return false;
        }

        void Manager::Update()
        {
            for (size_t i = 0; i < _MAX_DATA; ++i)
            {
                if (_dataActive[i] && _dataBuffer[i]._file_size_now >= _dataBuffer[i]._file_size_expected)
                {
                    std::cout << "[Manager] Data complete, clearing ID: " << _dataBuffer[i]._ID << "\n";
                    _dataActive[i] = false;
                }
            }
        }

        void Manager::CheckAndNotifyDataComplete(Data& data)
        {
            if (data._file_size_now >= data._file_size_expected)
            {
                std::cout << "[Manager] Data complete for ID: " << data._ID << ", notifying subscribers...\n";

                for (size_t i = 0; i < _dataSubCount; ++i)
                {
                    if (std::strncmp(_dataSubscribers[i].name, data._name, _MAX_DATA_NAME) == 0)
                    {
                        if (_dataSubscribers[i].plugin)
                        {
                            std::cout << "[Manager] Notifying plugin about data: " << data._name << "\n";
                            _dataSubscribers[i].plugin->OnData(data);
                        }
                    }
                }
            }
        }
    }
}
