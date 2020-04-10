cmake_minimum_required(VERSION 3.8)
project(raster VERSION 0.0.1 LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(raster main.cpp converters.cpp util.cpp rasterizer.cpp user_data.cpp)
target_link_libraries(raster OpenGL::GL glfw glm)

