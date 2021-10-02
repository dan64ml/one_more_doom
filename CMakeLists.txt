cmake_minimum_required(VERSION 3.10)

project(one_more_doom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)

add_compile_options(-Wall -Wextra)
#add_compile_options(-Ofast -g -no-pie)
#add_compile_options(-pg)
#add_link_options(-pg)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

#find_package(GTest REQUIRED)

add_subdirectory(src/app)
add_subdirectory(src/graphics)
add_subdirectory(src/objects)
add_subdirectory(src/renderer)
add_subdirectory(src/utils)
add_subdirectory(src/world)
add_subdirectory(src/weapon)
add_subdirectory(src/structures)
