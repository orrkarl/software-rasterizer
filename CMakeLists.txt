cmake_minimum_required(VERSION 3.8)
project(raster VERSION 0.0.1 LANGUAGES CXX)

find_package(raylib REQUIRED)
find_package(glm REQUIRED)

add_executable(raster main.cpp)
target_link_libraries(raster raylib glm)

