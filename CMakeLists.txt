cmake_minimum_required(VERSION 3.16)
project(NetworkAnalysisLab VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(BUILD_GUI "Build the GUI application" OFF)

add_subdirectory(core)
if(BUILD_GUI)
    add_subdirectory(gui)
endif()
