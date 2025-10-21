#define _WIN32_WINNT 0x0A00 // Windows 10
#include "Platform_Win32.h"

#include <windows.h>
#include <string>
#include <thread>
#include <iostream>

#include <shobjidl.h>     // IFileOpenDialog, IShellItem
#include <shlobj.h>       // BROWSEINFO, SHBrowseForFolder
#include <commdlg.h>      // GetOpenFileName, GetSaveFileName
#include <shellapi.h>     // Shell_NotifyIcon

#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.ui.notifications.h>
#include <windows.data.xml.dom.h>
#include <roapi.h> // для RoInitialize

#pragma comment(lib, "runtimeobject.lib")

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;

#define WIN32_LEAN_AND_MEAN

// ==========================================================
// 🪟 WINDOW MANAGEMENT IMPLEMENTATION
// ==========================================================

void Platform_Win32::Handle_OpenModalWindow(const WeOn::EventSystem::Event& event)
{
    size_t offset = 0;
    std::string title = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::string message = StaticSerializer::readString(event._data, offset, sizeof(event._data));

    std::wstring wtitle(title.begin(), title.end());
    std::wstring wmessage(message.begin(), message.end());

    std::thread([this, wtitle, wmessage]() {
        int result = MessageBox(nullptr, wmessage.c_str(), wtitle.c_str(), MB_OK | MB_ICONINFORMATION);

        // Если нужен результат — можно отправить обратно
        WeOn::EventSystem::Event evResult{};
        strncpy_s(evResult._name, sizeof(evResult._name), "ModalWindow_Result", _TRUNCATE);
        size_t offset = 0;
        std::string resStr = std::to_string(result);
        StaticSerializer::writeString(evResult._data, offset, sizeof(evResult._data), resStr);

        _manager->PublishEvent(evResult);
        }).detach();
}

void Platform_Win32::Handle_OpenFileDialog(const WeOn::EventSystem::Event& event)
{
    size_t offset = 0;
    std::string filter = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::wstring wfilter(filter.begin(), filter.end());

    std::thread([this, wfilter]() {
        OPENFILENAME ofn = {};
        wchar_t szFile[MAX_PATH] = {};

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = wfilter.c_str();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileName(&ofn))
        {
            std::string utf8path(szFile, szFile + wcslen(szFile));

            WeOn::EventSystem::Event evResult{};
            strncpy_s(evResult._name, sizeof(evResult._name), "FileDialog_Result", _TRUNCATE);
            size_t offset = 0;
            StaticSerializer::writeString(evResult._data, offset, sizeof(evResult._data), utf8path);

            _manager->PublishEvent(evResult);
        }
        }).detach();
}

void Platform_Win32::Handle_SaveFileDialog(const WeOn::EventSystem::Event& event)
{
    size_t offset = 0;
    std::string filter = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::wstring wfilter(filter.begin(), filter.end());

    std::thread([this, wfilter]() {
        OPENFILENAME ofn = {};
        wchar_t szFile[MAX_PATH] = {};

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;
        ofn.lpstrFilter = wfilter.c_str();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn))
        {
            std::string utf8path(szFile, szFile + wcslen(szFile));

            WeOn::EventSystem::Event evResult{};
            strncpy_s(evResult._name, sizeof(evResult._name), "SaveFileDialog_Result", _TRUNCATE);
            size_t offset = 0;
            StaticSerializer::writeString(evResult._data, offset, sizeof(evResult._data), utf8path);

            _manager->PublishEvent(evResult);
        }
        }).detach();
}

void Platform_Win32::Handle_OpenFolderDialog(const WeOn::EventSystem::Event& event)
{
    std::thread([this]() {
        BROWSEINFO bi = {};
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

        if (pidl)
        {
            wchar_t path[MAX_PATH];
            if (SHGetPathFromIDList(pidl, path))
            {
                std::string utf8path(path, path + wcslen(path));
                WeOn::EventSystem::Event evResult{};
                strncpy_s(evResult._name, sizeof(evResult._name), "FolderDialog_Result", _TRUNCATE);
                size_t offset = 0;
                StaticSerializer::writeString(evResult._data, offset, sizeof(evResult._data), utf8path);
                _manager->PublishEvent(evResult);
            }
            CoTaskMemFree(pidl);
        }
        }).detach();
}

void Platform_Win32::Handle_ShowToastNotification(const WeOn::EventSystem::Event& event)
{
    struct ToastOptions {
        std::string title;
        std::string message;
        UINT iconType;      // NIIF_INFO, NIIF_WARNING, NIIF_ERROR, NIIF_USER
        UINT timeout;       // миллисекунды
        std::string tip;    // подсказка при наведении
        std::wstring customIconPath; // если нужен кастомный .ico
    };

    size_t offset = 0;
    ToastOptions options{};
    options.title = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    options.message = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    options.iconType = StaticSerializer::readInt(event._data, offset, sizeof(event._data));
    options.timeout = StaticSerializer::readInt(event._data, offset, sizeof(event._data));
    options.tip = StaticSerializer::readString(event._data, offset, sizeof(event._data));

    std::string iconPath = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    options.customIconPath.assign(iconPath.begin(), iconPath.end());

    std::wstring wtitle(options.title.begin(), options.title.end());
    std::wstring wmessage(options.message.begin(), options.message.end());
    std::wstring wtip(options.tip.begin(), options.tip.end());

    std::thread([options]() {
        // Локальные копии строк — гарантируем, что память живёт до конца потока
        std::wstring wtitle(options.title.begin(), options.title.end());
        std::wstring wmessage(options.message.begin(), options.message.end());
        std::wstring wtip(options.tip.begin(), options.tip.end());

        NOTIFYICONDATA nid = { sizeof(nid) };
        nid.hWnd = nullptr;
        nid.uID = 1;
        nid.uFlags = NIF_INFO | NIF_TIP | NIF_ICON;

        // Загружаем иконку
        if (!options.customIconPath.empty()) {
            nid.hIcon = (HICON)LoadImageW(
                nullptr,
                options.customIconPath.c_str(),
                IMAGE_ICON,
                32, 32,
                LR_LOADFROMFILE
            );
            if (!nid.hIcon) {
                nid.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
            }
        }
        else {
            nid.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
        }

        // 🔒 Безопасное копирование строк с автообрезкой
        wcsncpy_s(nid.szTip, wtip.c_str(), _TRUNCATE);
        wcsncpy_s(nid.szInfoTitle, wtitle.c_str(), _TRUNCATE);
        wcsncpy_s(nid.szInfo, wmessage.c_str(), _TRUNCATE);

        nid.dwInfoFlags = options.iconType;
        nid.uTimeout = options.timeout;

        // Показываем уведомление
        if (Shell_NotifyIcon(NIM_ADD, &nid)) {
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            std::wcout << L"[Toast] Notification shown: " << wtitle << L" - " << wmessage << std::endl;
        }
        else {
            std::wcerr << L"[Toast] Failed to add notification icon!" << std::endl;
        }

        // Ждём немного, чтобы balloon успел показаться
        Sleep(options.timeout + 1000);

        // Удаляем иконку
        Shell_NotifyIcon(NIM_DELETE, &nid);
        }).detach();

}

void Platform_Win32::Handle_ShowSystemTrayIcon(const WeOn::EventSystem::Event& event)
{
    // Простейший шаблон
    std::thread([this]() {
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = _hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wcscpy_s(nid.szTip, L"WeOn Tray Icon");

        Shell_NotifyIcon(NIM_ADD, &nid);
        }).detach();
}

void Platform_Win32::Handle_RemoveSystemTrayIcon(const WeOn::EventSystem::Event& event)
{
    std::thread([this]() {
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(nid);
        nid.hWnd = _hwnd;
        nid.uID = 1;

        Shell_NotifyIcon(NIM_DELETE, &nid);
        }).detach();
}
