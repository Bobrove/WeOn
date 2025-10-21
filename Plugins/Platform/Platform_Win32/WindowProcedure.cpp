#include "Platform_Win32.h"

#include <windowsx.h>
#include <dbt.h> // для DEV_BROADCAST_DEVICEINTERFACE и типов уведомлений

LRESULT Platform_Win32::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    using namespace WeOn;

    switch (uMsg)
    {
    case WM_DESTROY:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Window_Destroyed", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Window destroyed");
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), reinterpret_cast<int64_t>(_hwnd));

        _manager->PublishEvent(ev);

        PostQuitMessage(0);
        return 0;
    }

    // ------------------------
    // 🖱️ Обработка мыши
    // ------------------------
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Mouse_ButtonDown", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeString(ev._data, offset, sizeof(ev._data),
            (uMsg == WM_LBUTTONDOWN) ? "Left" :
            (uMsg == WM_RBUTTONDOWN) ? "Right" : "Middle");

        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), GET_X_LPARAM(lParam));
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), GET_Y_LPARAM(lParam));

        _manager->PublishEvent(ev);
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Mouse_Move", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), GET_X_LPARAM(lParam));
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), GET_Y_LPARAM(lParam));

        _manager->PublishEvent(ev);
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Mouse_Wheel", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), GET_WHEEL_DELTA_WPARAM(wParam));
        _manager->PublishEvent(ev);
        return 0;
    }

    // ------------------------
    // ⌨️ Обработка клавиатуры
    // ------------------------
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Key_Down", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Key Pressed");
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), static_cast<int>(wParam));

        _manager->PublishEvent(ev);
        return 0;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Key_Up", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Key Released");
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), static_cast<int>(wParam));

        _manager->PublishEvent(ev);
        return 0;
    }

    // ------------------------
    // 🧱 Размер / фокус
    // ------------------------
    case WM_SIZE:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Window_Resized", _TRUNCATE);

        size_t offset = 0;
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), LOWORD(lParam));  // width
        StaticSerializer::writeInt(ev._data, offset, sizeof(ev._data), HIWORD(lParam));  // height

        _manager->PublishEvent(ev);
        return 0;
    }

    case WM_SETFOCUS:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Window_FocusGained", _TRUNCATE);
        _manager->PublishEvent(ev);
        return 0;
    }

    case WM_KILLFOCUS:
    {
        EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "Window_FocusLost", _TRUNCATE);
        _manager->PublishEvent(ev);
        return 0;
    }

    case WM_DEVICECHANGE:
    {
        EventSystem::Event ev{};
        size_t offset = 0;

        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
        {
            strncpy_s(ev._name, sizeof(ev._name), "USB_Connected", _TRUNCATE);

            PDEV_BROADCAST_HDR pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
            if (pHdr && pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDev = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>(pHdr);
                std::wstring deviceName = pDev->dbcc_name ? pDev->dbcc_name : L"Unknown Device";
                std::string utf8name(deviceName.begin(), deviceName.end());

                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), utf8name);
            }
            else
            {
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Unknown Device");
            }

            _manager->PublishEvent(ev);
            return 0;
        }

        case DBT_DEVICEREMOVECOMPLETE:
        {
            strncpy_s(ev._name, sizeof(ev._name), "USB_Disconnected", _TRUNCATE);

            PDEV_BROADCAST_HDR pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);
            if (pHdr && pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
            {
                PDEV_BROADCAST_DEVICEINTERFACE pDev = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>(pHdr);
                std::wstring deviceName = pDev->dbcc_name ? pDev->dbcc_name : L"Unknown Device";
                std::string utf8name(deviceName.begin(), deviceName.end());

                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), utf8name);
            }
            else
            {
                StaticSerializer::writeString(ev._data, offset, sizeof(ev._data), "Unknown Device");
            }

            _manager->PublishEvent(ev);
            return 0;
        }
        }
    }

    // ------------------------
    // 🪟 По умолчанию
    // ------------------------
    default:
        return DefWindowProc(_hwnd, uMsg, wParam, lParam);
    }
}
