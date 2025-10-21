project(Platform_Win32_1_0)

add_library(Platform_Win32_1_0 SHARED
    Init.cpp
    Platform_Win32.cpp
    Platform_Win32.h
)

target_include_directories(Platform_Win32_1_0 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/SubSystem/Event_System
    ${CMAKE_SOURCE_DIR}/SubSystem/Serializer
)

target_link_libraries(Platform_Win32_1_0
    Event_System
    Serializer
    user32
    shell32
)
