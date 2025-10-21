project(Serializer)

add_library(Serializer STATIC
    Serializer.cpp
    Serializer.h
)

target_include_directories(Serializer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
