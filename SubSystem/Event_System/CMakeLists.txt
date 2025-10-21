project(Event_System)

add_library(Event_System STATIC
    Event_System.cpp
    Event_System.h
)

target_include_directories(Event_System PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
