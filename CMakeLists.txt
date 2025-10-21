project(Test_Main)

add_executable(Test_Main
    main.cpp
)

target_include_directories(Test_Main PUBLIC
    ${CMAKE_SOURCE_DIR}/SubSystem/Event_System
    ${CMAKE_SOURCE_DIR}/SubSystem/Serializer
    ${CMAKE_SOURCE_DIR}/Plugins/Platform/Platform_Win32_1.0
)

target_link_libraries(Test_Main
    Event_System
    Serializer
)

set_target_properties(Test_Main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
