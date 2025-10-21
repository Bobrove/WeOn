#include "Platform_Win32.h"

#include <windows.h>
#include <string>
#include <thread>
#include <iostream>

#include "Platform_Win32.h"

#define WIN32_LEAN_AND_MEAN

// ==========================================================
// 🪟 WINDOW MANAGEMENT IMPLEMENTATION
// ==========================================================

// 💠 Создать простое окно
void Platform_Win32::Handle_CreateWindow(const WeOn::EventSystem::Event& event)
{
    std::cout << "[EventLog Call] -> Handle_CreateWindow triggered\n";

    size_t offset = 0;
    std::string title = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    int width = StaticSerializer::readInt(event._data, offset, sizeof(event._data));
    int height = StaticSerializer::readInt(event._data, offset, sizeof(event._data));
    int posX = StaticSerializer::readInt(event._data, offset, sizeof(event._data));
    int posY = StaticSerializer::readInt(event._data, offset, sizeof(event._data));
    int style = StaticSerializer::readInt(event._data, offset, sizeof(event._data));

    std::wstring wtitle(title.begin(), title.end());

    std::thread([this, wtitle, width, height, posX, posY, style]() {

        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        const wchar_t CLASS_NAME[] = L"WeOnWindowClass";

        WNDCLASS wc = {};
        wc.lpfnWndProc = [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {
            Platform_Win32* self = reinterpret_cast<Platform_Win32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (self) return self->HandleMessage(uMsg, wParam, lParam);
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        };
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(
            0,
            CLASS_NAME,
            wtitle.c_str(),
            style ? style : WS_OVERLAPPEDWINDOW,
            posX, posY,
            width ? width : 800,
            height ? height : 600,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            nullptr
        );

        if (!hwnd)
        {
            std::cerr << "[EventLog Error] Failed to create window.\n";
            CoUninitialize();
            return;
        }

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        _hwnd = hwnd;

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        std::cout << "[EventLog Call] Window  created successfully at ("
            << posX << "," << posY << ") with size " << width << "x" << height << std::endl;

        MSG msg{};
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        CoUninitialize();
        std::cout << "[EventLog Call] Window thread exited.\n";

        }).detach();
}

// 💠 Уничтожить окно
void Platform_Win32::Handle_DestroyWindow(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Destroying window\n";
    DestroyWindow(_hwnd);
    _hwnd = nullptr;
}

// 💠 Показать окно
void Platform_Win32::Handle_ShowWindow(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Showing window\n";
    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);
}

// 💠 Изменить размер окна
void Platform_Win32::Handle_ResizeWindow(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Resizing window\n";
    SetWindowPos(_hwnd, nullptr, 0, 0, 1280, 720, SWP_NOMOVE | SWP_NOZORDER);
}

// 💠 Переместить окно
void Platform_Win32::Handle_MoveWindow(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Moving window\n";
    SetWindowPos(_hwnd, nullptr, 100, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

// 💠 Фокусировка
void Platform_Win32::Handle_SetFocus(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Setting focus\n";
    SetFocus(_hwnd);
}

// 💠 Полноэкранный режим
void Platform_Win32::Handle_ToggleFullscreen(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;

    static bool fullscreen = false;
    fullscreen = !fullscreen;

    if (fullscreen)
    {
        std::cout << "[EventLog Call] -> Entering fullscreen\n";

        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(MonitorFromWindow(_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            SetWindowPos(_hwnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }
    }
    else
    {
        std::cout << "[EventLog Call] -> Exiting fullscreen\n";
        SetWindowLong(_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetWindowPos(_hwnd, HWND_TOP, 100, 100, 800, 600, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}

// 💠 Свернуть окно
void Platform_Win32::Handle_Minimize(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Minimize window\n";
    ShowWindow(_hwnd, SW_MINIMIZE);
}

// 💠 Развернуть окно
void Platform_Win32::Handle_Maximize(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Maximize window\n";
    ShowWindow(_hwnd, SW_MAXIMIZE);
}

// 💠 Изменить заголовок окна
void Platform_Win32::Handle_SetWindowTitle(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Setting window title\n";
    SetWindowText(_hwnd, L"WeOn Platform — New Title");
}

// 💠 Изменить иконку окна
void Platform_Win32::Handle_SetWindowIcon(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Setting window icon\n";
    HICON hIcon = LoadIcon(nullptr, IDI_INFORMATION);
    SendMessage(_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

// 💠 Обновить окно (перерисовать)
void Platform_Win32::Handle_UpdateWindow(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    std::cout << "[EventLog Call] -> Updating window\n";
    InvalidateRect(_hwnd, nullptr, TRUE);
    UpdateWindow(_hwnd);
}

// 💠 Обработка WM_PAINT
void Platform_Win32::Handle_Paint(const WeOn::EventSystem::Event& event)
{
    if (!_hwnd) return;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(_hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    TextOut(hdc, 10, 10, L"WeOn Platform Window", 23);
    EndPaint(_hwnd, &ps);
}
