cmake_minimum_required(VERSION 3.16)
project(WeOn)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Пути
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SUBSYSTEM_DIR ${ROOT_DIR}/SubSystem)
set(PLUGINS_DIR ${ROOT_DIR}/Plugins)
set(TEST_DIR ${ROOT_DIR}/Test_Main)

# Включаем подпроекты
add_subdirectory(${SUBSYSTEM_DIR}/Event_System)
add_subdirectory(${SUBSYSTEM_DIR}/Serializer)
add_subdirectory(${PLUGINS_DIR}/Platform/Platform_Win32_1.0)
add_subdirectory(${TEST_DIR})
