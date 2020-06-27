cmake_minimum_required(VERSION 3.8)
project(raster VERSION 0.0.1 LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

SET(SRCS 
	main.cpp 
	converters.cpp 
	util.cpp 
	user_data.cpp 
	clipping.cpp
	stb_image.c)

add_executable(raster ${SRCS})
target_link_libraries(raster OpenGL::GL glfw glm)

