cmake_minimum_required(VERSION 3.20)
project(BobTricks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EMSCRIPTEN)
    find_package(SDL2 REQUIRED)
endif()

add_executable(bobtricks
    src/app/Application.cpp
    src/app/SimulationLoop.cpp
    src/platform/sdl/SDLPlatformRuntime.cpp
    src/input/InputMapper.cpp
    src/core/locomotion/LocomotionController.cpp
    src/core/locomotion/ProceduralAnimator.cpp
    src/core/simulation/SimulationCore.cpp
    src/core/debug/DebugCommandBus.cpp
    src/render/RenderStateAdapter.cpp
    src/render/sdl/SDLRenderer.cpp
    src/debug_ui/console/ConsoleDebugUI.cpp
    src/debug_ui/imgui/ImGuiDebugUI.cpp
    src/main.cpp
)

if(EMSCRIPTEN)
    target_sources(bobtricks PRIVATE
        src/platform/web/WebInputBridge.cpp
    )
endif()

target_include_directories(bobtricks PRIVATE include src)
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
