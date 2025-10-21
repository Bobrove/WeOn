// Platform_Win32_System.cpp
#include "Platform_Win32.h"

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <sstream>
#include <locale>
#include <codecvt>

#include <psapi.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#include <setupapi.h>
#include <shlobj.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "shell32.lib")

using namespace WeOn;

// --- небольшие утилиты ---
static std::wstring utf8_to_wstring(const std::string& s)
{
    if (s.empty()) return {};
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
    std::wstring w(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], size_needed);
    return w;
}

static std::string wstring_to_utf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), NULL, 0, NULL, NULL);
    std::string s(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], size_needed, NULL, NULL);
    return s;
}

static std::string ptrToString(const char* s)
{
    return s ? std::string(s) : std::string();
}

// --- 1) Memory info ---
void Platform_Win32::Handle_GetMemory(const WeOn::EventSystem::Event& /*event*/)
{
    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    if (!GlobalMemoryStatusEx(&mem))
    {
        std::cerr << "[System] GlobalMemoryStatusEx failed\n";
        return;
    }

    DWORDLONG totalMB = mem.ullTotalPhys / (1024ULL * 1024ULL);
    DWORDLONG availMB = mem.ullAvailPhys / (1024ULL * 1024ULL);

    std::ostringstream oss;
    oss << "TotalMB=" << totalMB << ";FreeMB=" << availMB;
    std::string out = oss.str();

    std::cout << "[System] RAM: " << out << std::endl;

    // publish result
    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetMemory_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 2) System info ---
void Platform_Win32::Handle_GetSystemInfo(const WeOn::EventSystem::Event& /*event*/)
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);

    std::string arch;
    switch (si.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64: arch = "x64"; break;
    case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
    case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
    default: arch = "Unknown"; break;
    }

    DWORD procCount = si.dwNumberOfProcessors;

    // OS version (best-effort; modern Windows require Version Helper or manifest to be accurate)
    OSVERSIONINFOEXW osv = {};
    osv.dwOSVersionInfoSize = sizeof(osv);
#pragma warning(suppress: 4996)
    GetVersionExW((OSVERSIONINFOW*)&osv);

    std::ostringstream oss;
    oss << "Arch=" << arch << ";Processors=" << procCount
        << ";OS=" << (int)osv.dwMajorVersion << "." << (int)osv.dwMinorVersion;

    std::string out = oss.str();
    std::cout << "[System] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetSystemInfo_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 3) Display info ---
void Platform_Win32::Handle_GetDisplayInfo(const WeOn::EventSystem::Event& /*event*/)
{
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HDC dc = GetDC(nullptr);
    int dpiX = GetDeviceCaps(dc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(dc, LOGPIXELSY);
    ReleaseDC(nullptr, dc);

    std::ostringstream oss;
    oss << "Resolution=" << w << "x" << h << ";DPI=" << dpiX << "x" << dpiY;
    std::string out = oss.str();
    std::cout << "[Display] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetDisplayInfo_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 4) Power status ---
void Platform_Win32::Handle_GetPowerStatus(const WeOn::EventSystem::Event& /*event*/)
{
    SYSTEM_POWER_STATUS s;
    if (!GetSystemPowerStatus(&s))
    {
        std::cerr << "[Power] GetSystemPowerStatus failed\n";
        return;
    }
    std::ostringstream oss;
    oss << "AC=" << (int)s.ACLineStatus << ";BatteryPercent=" << (int)s.BatteryLifePercent
        << ";BatteryLifeTime=" << s.BatteryLifeTime;
    std::string out = oss.str();
    std::cout << "[Power] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetPowerStatus_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 5) USB devices (basic) ---
void Platform_Win32::Handle_GetUSBPort(const WeOn::EventSystem::Event& /*event*/)
{
    HDEVINFO devInfo = SetupDiGetClassDevs(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (devInfo == INVALID_HANDLE_VALUE)
    {
        std::cerr << "[USB] SetupDiGetClassDevs failed\n";
        return;
    }

    SP_DEVINFO_DATA devData = {};
    devData.cbSize = sizeof(devData);

    std::ostringstream oss;
    oss << "USB devices:\n";

    for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devData); ++i)
    {
        wchar_t desc[512] = {};
        if (SetupDiGetDeviceRegistryPropertyW(devInfo, &devData, SPDRP_DEVICEDESC, NULL,
            (PBYTE)desc, sizeof(desc), NULL))
        {
            oss << " - " << wstring_to_utf8(desc) << "\n";
        }
    }

    SetupDiDestroyDeviceInfoList(devInfo);

    std::string out = oss.str();
    std::cout << "[USB] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetUSBPort_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 6) Network adapters ---
void Platform_Win32::Handle_GetNetworkAdapters(const WeOn::EventSystem::Event& /*event*/)
{
    ULONG outBufLen = 0;
    if (GetAdaptersInfo(NULL, &outBufLen) != ERROR_BUFFER_OVERFLOW)
    {
        std::cerr << "[Network] GetAdaptersInfo failed to query size\n";
        return;
    }

    std::vector<BYTE> buffer(outBufLen);
    PIP_ADAPTER_INFO adapters = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
    if (GetAdaptersInfo(adapters, &outBufLen) != NO_ERROR)
    {
        std::cerr << "[Network] GetAdaptersInfo failed\n";
        return;
    }

    std::ostringstream oss;
    for (PIP_ADAPTER_INFO p = adapters; p; p = p->Next)
    {
        oss << p->Description << " - IP: " << p->IpAddressList.IpAddress.String << "\n";
    }

    std::string out = oss.str();
    std::cout << "[Network] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetNetworkAdapters_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 7) Disk drives ---
void Platform_Win32::Handle_GetDiskDrives(const WeOn::EventSystem::Event& /*event*/)
{
    DWORD drives = GetLogicalDrives();
    std::ostringstream oss;
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        if (drives & (1u << (c - 'A')))
        {
            oss << c << ":\\ ";
        }
    }
    std::string out = oss.str();
    std::cout << "[Drives] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetDiskDrives_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 8) Processes (list names + pid) ---
void Platform_Win32::Handle_GetProcesses(const WeOn::EventSystem::Event& /*event*/)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(pe);

    std::ostringstream oss;
    if (Process32First(snap, &pe))
    {
        do {
            oss << pe.szExeFile << " (PID=" << pe.th32ProcessID << ")\n";
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);

    std::string out = oss.str();
    std::cout << "[Processes] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetProcesses_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 9) Environment variables ---
void Platform_Win32::Handle_GetEnvironmentVariables(const WeOn::EventSystem::Event& /*event*/)
{
    LPWCH env = GetEnvironmentStringsW();
    if (!env) return;

    std::wostringstream woss;
    LPWCH var = env;
    while (*var)
    {
        woss << var << L"\n";
        var += wcslen(var) + 1;
    }
    FreeEnvironmentStringsW(env);

    std::string out = wstring_to_utf8(woss.str());
    std::cout << "[Env] " << out << std::endl;

    if (_manager)
    {
        WeOn::EventSystem::Event ev{};
        strncpy_s(ev._name, sizeof(ev._name), "GetEnvironmentVariables_Result", _TRUNCATE);
        size_t off = 0;
        StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
        _manager->PublishEvent(ev);
    }
}

// --- 10) Registry read ---
// ожидаем в event._data: keyPath (string), valueName (string)
void Platform_Win32::Handle_GetRegistryValue(const WeOn::EventSystem::Event& event)
{
    size_t offset = 0;
    std::string keyPath = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::string valueName = StaticSerializer::readString(event._data, offset, sizeof(event._data));

    std::wstring wkey = utf8_to_wstring(keyPath);
    std::wstring wval = utf8_to_wstring(valueName);

    HKEY hKey;
    std::wstring root;
    HKEY base = HKEY_CURRENT_USER;

    // support full path like "HKEY_CURRENT_USER\\Software\\Foo"
    if (wkey.rfind(L"HKEY_LOCAL_MACHINE\\", 0) == 0)
    {
        base = HKEY_LOCAL_MACHINE;
        wkey = wkey.substr(wcslen(L"HKEY_LOCAL_MACHINE\\"));
    }
    else if (wkey.rfind(L"HKEY_CURRENT_USER\\", 0) == 0)
    {
        base = HKEY_CURRENT_USER;
        wkey = wkey.substr(wcslen(L"HKEY_CURRENT_USER\\"));
    }

    if (RegOpenKeyExW(base, wkey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        std::cerr << "[Registry] Failed to open key\n";
        return;
    }

    wchar_t buf[1024] = {};
    DWORD bufSize = sizeof(buf);
    DWORD type = 0;
    if (RegQueryValueExW(hKey, wval.empty() ? nullptr : wval.c_str(), NULL, &type, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
    {
        std::string result = wstring_to_utf8(buf);
        std::cout << "[Registry] Read: " << result << std::endl;

        if (_manager)
        {
            WeOn::EventSystem::Event ev{};
            strncpy_s(ev._name, sizeof(ev._name), "GetRegistryValue_Result", _TRUNCATE);
            size_t off = 0;
            StaticSerializer::writeString(ev._data, off, sizeof(ev._data), result);
            _manager->PublishEvent(ev);
        }
    }
    else
    {
        std::cerr << "[Registry] RegQueryValueEx failed\n";
    }
    RegCloseKey(hKey);
}

// --- 11) Registry write ---
// ожидаем: keyPath (string), valueName (string), value (string)
void Platform_Win32::Handle_SetRegistryValue(const WeOn::EventSystem::Event& event)
{
    size_t offset = 0;
    std::string keyPath = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::string valueName = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::string value = StaticSerializer::readString(event._data, offset, sizeof(event._data));

    std::wstring wkey = utf8_to_wstring(keyPath);
    std::wstring wval = utf8_to_wstring(valueName);
    std::wstring wvalue = utf8_to_wstring(value);

    HKEY base = HKEY_CURRENT_USER;
    if (wkey.rfind(L"HKEY_LOCAL_MACHINE\\", 0) == 0)
    {
        base = HKEY_LOCAL_MACHINE;
        wkey = wkey.substr(wcslen(L"HKEY_LOCAL_MACHINE\\"));
    }
    else if (wkey.rfind(L"HKEY_CURRENT_USER\\", 0) == 0)
    {
        base = HKEY_CURRENT_USER;
        wkey = wkey.substr(wcslen(L"HKEY_CURRENT_USER\\"));
    }

    HKEY hKey;
    LONG res = RegCreateKeyExW(base, wkey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    if (res != ERROR_SUCCESS)
    {
        std::cerr << "[Registry] RegCreateKeyEx failed\n";
        return;
    }

    if (RegSetValueExW(hKey, wval.empty() ? nullptr : wval.c_str(), 0, REG_SZ,
        (const BYTE*)wvalue.c_str(), (DWORD)((wvalue.size() + 1) * sizeof(wchar_t))) != ERROR_SUCCESS)
    {
        std::cerr << "[Registry] RegSetValueEx failed\n";
    }
    else
    {
        std::cout << L"[Registry] Wrote value successfully\n";
    }
    RegCloseKey(hKey);
}

// --- 12) Execute command ---
// ожидаем: command (string)
void Platform_Win32::Handle_ExecuteCommand(const WeOn::EventSystem::Event& event)
{
    size_t offset = 0;
    std::string cmd = StaticSerializer::readString(event._data, offset, sizeof(event._data));
    std::cout << "[Execute] Command: " << cmd << std::endl;

    // Запускаем в потоке, чтобы не блокировать
    std::thread([this, cmd]() {
        int rc = system(cmd.c_str()); // простая реализация
        std::ostringstream oss;
        oss << "ExitCode=" << rc;
        std::string out = oss.str();

        std::cout << "[Execute] " << out << std::endl;
        if (_manager)
        {
            WeOn::EventSystem::Event ev{};
            strncpy_s(ev._name, sizeof(ev._name), "ExecuteCommand_Result", _TRUNCATE);
            size_t off = 0;
            StaticSerializer::writeString(ev._data, off, sizeof(ev._data), out);
            _manager->PublishEvent(ev);
        }
        }).detach();
}
