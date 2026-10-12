cmake_minimum_required(VERSION 3.15)
project(IGS_Lab01 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)
if(MSVC)
    add_compile_options(/utf-8 /W3)
else()
    add_compile_options(-Wall)
endif()

# Часть 1: два графика в одних осях
add_executable(Graphs WIN32 Part1_Graphs/Graphs.cpp)

# Часть 2: меню, сегмент эллипса и шестиконечная звезда
add_executable(MenuWin WIN32
    Part2_Menu/MenuWin.cpp
    Part2_Menu/MenuWin.rc
    Part2_Menu/resource.h)
