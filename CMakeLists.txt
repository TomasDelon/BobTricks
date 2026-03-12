cmake_minimum_required(VERSION 3.20)
project(BobTricks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EMSCRIPTEN)
    find_package(SDL2 REQUIRED)
endif()

add_executable(bobtricks
    src/main.cpp
    src/app.cpp
)

target_include_directories(bobtricks PRIVATE include)
target_compile_options(bobtricks PRIVATE -Wall -Wextra -Wpedantic)

if(EMSCRIPTEN)
    set_target_properties(bobtricks PROPERTIES SUFFIX ".html")
    target_link_options(bobtricks PRIVATE
        "-sUSE_SDL=2"
        "-sALLOW_MEMORY_GROWTH=1"
    )
else()
    target_link_libraries(bobtricks PRIVATE SDL2::SDL2)
endif()
